/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "GameService.h"

#include "NetworkCallback.h"
#include "NetworkClient.h"
#include "NetworkManager.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Session.h"
#include "SessionFactory.h"

#include "LogManager/LogManager.h"

#include "Common/Message.h"
#include "Common/MessageFactory.h"

#include "ConfigManager/ConfigManager.h"
#include "Utils/typedefs.h"

#include <cassert>
#include <cstdio>

//======================================================================================================================

GameService::GameService(NetworkManager* networkManager, boost::asio::io_service* service) :
IService( networkManager, service ),
mNetworkManager(networkManager),
mLocalPort(0),
mLocalAddress("127.0.0.1"),
mQueued(false),
mIOService(service)
{

}

//======================================================================================================================

GameService::~GameService(void)
{


}

//======================================================================================================================

void GameService::Startup(std::string localAddress, uint16 localPort,uint32 mfHeapSize)
{
	mLocalAddress = localAddress;
	mLocalPort = localPort;

	// Startup Factories
	mPacketFactory = new PacketFactory();
	mPacketFactory->Startup(false);

	mMessageFactory = new MessageFactory(mfHeapSize, getId());

	mSessionFactory = new SessionFactory();
	mSessionFactory->Startup(this, mPacketFactory, mMessageFactory);

	mRecvPacket = mPacketFactory->CreatePacket();

	int udpBuffLen = gConfig->read<int32>("UDPBufferSize",4096);
	gLogger->logMsgF("UDPBuffer set to %ukb",MSG_HIGH,udpBuffLen);

	if(udpBuffLen < 128)
		udpBuffLen = 128;
	
	if(udpBuffLen > 8192)
		udpBuffLen = 8192;

	udpBuffLen *= 1024;

	mSocket = new boost::asio::ip::udp::socket( *mIOService, boost::asio::ip::udp::endpoint( boost::asio::ip::udp::v4(), localPort ) );
	mSocket->set_option( boost::asio::socket_base::receive_buffer_size( udpBuffLen ) );
	mSocket->set_option( boost::asio::socket_base::send_buffer_size( udpBuffLen ) );

	//
	// Initial Async Receive From
	//
	mSocket->async_receive_from( 
		boost::asio::buffer( mRecvPacket->getData(), mRecvPacket->getMaxPayload() ),
		mRecvEndpoint,
		boost::bind(&GameService::HandleRecvFrom, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)
	);

}

//======================================================================================================================

void GameService::Shutdown(void)
{
	mSocket->close();
}

//======================================================================================================================

void GameService::Process()
{
	// Get the current count of Sessions to be processed.  We can't just check to see if the queue is empty, since
	// the other threads could keep placing more Packets in the queue, and this could cause a stall in the
	// main thread.
	Session* session = 0;
	//Message* message = 0;
	NetworkClient* newClient = 0;
	uint32 sessionCount = mSessionProcessQueue.size();

	for(uint32 i = 0; i < sessionCount; i++)
	{
		// Grab our next IService to process
		session = mSessionProcessQueue.front();
		mSessionProcessQueue.pop();
		session->Update();

		//
		// Move this code to Session::Update(), somehow...
		//

		// Check to see if we're in the process of connecting or disconnecting.
		if(session->getStatus() == SSTAT_Connecting)
		{
			for(NetworkCallbackList::iterator iter = mNetworkCallbackList.begin(); iter != mNetworkCallbackList.end(); ++iter)
			{
				newClient = (*iter)->handleSessionConnect(session, this);

				// They returned a client to us, so keep the session.
				if(newClient)
				{
					session->setClient(newClient);
					newClient->setSession(session);
					session->setStatus(SSTAT_Connected);
				}
				else
				{
					// Remove the session, they don't want it.
					session->setCommand(SCOM_Disconnect);
				}
			}
		}
		else if(session->getStatus() == SSTAT_Disconnecting)
		{

			NetworkCallbackList::iterator iter;

			for(iter = mNetworkCallbackList.begin(); iter != mNetworkCallbackList.end(); ++iter)
			{
				if(session->getClient())
				{
					(*iter)->handleSessionDisconnect(session->getClient());
					session->setClient(0);
				}
			}

			// We're now dis connected.
			session->setStatus(SSTAT_Disconnected);

			continue;
		}
		else if(session->getStatus() == SSTAT_Destroy)
		{

		  continue;
		}

		// Now send up any messages waiting.

		// Iterate through our priority queue's looking for messages.
		uint32 messageCount = session->getIncomingQueueMessageCount();

		if((session->getStatus() == SSTAT_Destroy)||(!session->getClient()))
		{
			for(uint32 j = 0; j < messageCount; j++)
			{
				Message* message = session->getIncomingQueueMessage();
				message->setPendingDelete(true);
			}
		}
		else
		{
			for(uint32 j = 0; j < messageCount; j++)
			{
				Message* message = session->getIncomingQueueMessage();

				message->ResetIndex();

				// At this point we can assume we have a client object, so send the data up.
				// actually when a server crashed it happens that we crash the connectionserver this way
				NetworkCallbackList::iterator iter;

				for(iter = mNetworkCallbackList.begin(); iter != mNetworkCallbackList.end(); ++iter)
				{
					(*iter)->handleSessionMessage(session->getClient(), message);
				}
			}
		}

	}
}

//======================================================================================================================

void GameService::AddSessionToProcessQueue(Session* session)
{
	mSessionProcessQueue.push(session);
}

//======================================================================================================================

std::string GameService::getLocalAddress(void)
{
  return mLocalAddress;
}

//======================================================================================================================

uint16 GameService::getLocalPort(void)
{
  return mLocalPort;

}

//======================================================================================================================

void GameService::HandleRecvFrom(const boost::system::error_code &error, std::size_t bytesRecvd)
{

	//
	// Check for an error and make sure we actually have bytes to read!
	//
	if( !error && bytesRecvd > 2 )
	{
		//
		// Reset receive packet.
		//
		mRecvPacket->Reset();
		mRecvPacket->setSize(bytesRecvd);


		//
		// Find the session.
		//
		Session* session = 0;
		AddressSessionMap::iterator i = mAddressSessionMap.find( mRecvEndpoint );
		if(i != mAddressSessionMap.end() )
		{
			session = (*i).second;
		}
		else
		{
			//
			// If the session doesnt exist, and this is a SessionRequest
			// create a new session and insert it into the AddressSessionMap.
			//
			if( mRecvPacket->peekUint16() == SESSIONOP_SessionRequest )
			{
				gLogger->logMsgF("IService: New Session (%s:%u)", MSG_NORMAL, mRecvEndpoint.address().to_string().c_str(), mRecvEndpoint.port());
				session = mSessionFactory->CreateSession();
				session->setPacketFactory(mPacketFactory);
				session->setRemoteEndpoint( mRecvEndpoint );
				mAddressSessionMap.insert( std::pair< boost::asio::ip::udp::endpoint, Session* >( mRecvEndpoint, session ) );
			}
		}

		//
		// Queue the packet and add the Session to the processing Queue.
		//
		session->QueueIncomingPacket(mRecvPacket);
		AddSessionToProcessQueue(session);
		mRecvPacket = mPacketFactory->CreatePacket();
	}
	else
	{
	}

	//
	// Reset our AsyncRecvFrom.
	//
	mSocket->async_receive_from( 
		boost::asio::buffer( mRecvPacket->getData(), mRecvPacket->getMaxPayload() ),
		mRecvEndpoint,
		boost::bind(&GameService::HandleRecvFrom, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)
		);
}

//======================================================================================================================

void GameService::HandleSendTo(Session* source, Packet* msg)
{
	
}

//======================================================================================================================
