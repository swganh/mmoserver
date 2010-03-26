/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "Service.h"

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

#include <boost/thread/thread.hpp>

#if defined(_MSC_VER)

#else
#include <sys/socket.h>
#include <arpa/inet.h>

#define INVALID_SOCKET	-1
#define SOCKET_ERROR	-1
#define closesocket		close
#endif

#include <cassert>
#include <cstdio>



//======================================================================================================================

bool Service::mSocketsSubsystemInitComplete = false;

//======================================================================================================================

Service::Service(NetworkManager* networkManager, bool serverservice) :
mNetworkManager(networkManager),
mLocalSocket(0),
avgTime(0),
avgPacketsbuild (0),
mLocalAddress(0),
mLocalPort(0),
mQueued(false),
mServerService(serverservice),
mIOService()
{

}

//======================================================================================================================

Service::~Service(void)
{


}

//======================================================================================================================

void Service::Startup(int8* localAddress, uint16 localPort,uint32 mfHeapSize)
{

	//localAddress = (char*)gConfig->read<std::string>("BindAddress").c_str();
	lasttime = Anh_Utils::Clock::getSingleton()->getLocalTime();
	assert(strlen(localAddress) < 256 && "Address length should be less than 256");
	strcpy(mLocalAddressName, localAddress);
	mLocalAddress = inet_addr(localAddress);
	mLocalPort = htons(localPort);

	// Startup Factories
	mPacketFactory = new PacketFactory();
	mPacketFactory->Startup(false);

	mMessageFactory = new MessageFactory(mfHeapSize, getId());

	mSessionFactory = new SessionFactory();
	mSessionFactory->Startup(NULL, this, mPacketFactory, mMessageFactory, false);

	mRecvPacket = mPacketFactory->CreatePacket();

	//set the socketbuffer so we dont suffer internal dataloss
	int value;
	int valuelength = sizeof(value);
	value = 524288;
	int configvalue = gConfig->read<int32>("UDPBufferSize",4096);
	gLogger->logMsgF("UDPBuffer set to %ukb",MSG_HIGH,configvalue);

	if(configvalue < 128)
		configvalue = 128;
	
	if(configvalue > 8192)
		configvalue = 8192;

	value = configvalue *1024;

	mSocket = new boost::asio::ip::udp::socket( mIOService, boost::asio::ip::udp::endpoint( boost::asio::ip::udp::v4(), localPort ) );

	//
	// Initial Async Receive From
	//
	mSocket->async_receive_from( 
		boost::asio::buffer( mRecvPacket->getData(), mRecvPacket->getMaxPayload() ),
		mRecvEndpoint,
		boost::bind(&Service::HandleRecvFrom, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)
	);

	mNetworkManager->AddServiceToProcessQueue(this);
}

//======================================================================================================================

void Service::Shutdown(void)
{
}

//======================================================================================================================

void Service::Process()
{
	mNetworkManager->AddServiceToProcessQueue(this);
	mIOService.poll();

	// Get the current count of Sessions to be processed.  We can't just check to see if the queue is empty, since
	// the other threads could keep placing more Packets in the queue, and this could cause a stall in the
	// main thread.
	Session* session = 0;
	//Message* message = 0;
	NetworkClient* newClient = 0;
	uint32 sessionCount = mSessionProcessQueue.size();

	for(uint32 i = 0; i < sessionCount; i++)
	{
		// Grab our next Service to process
		session = mSessionProcessQueue.pop();
		session->Update();


		//
		// Move this code to Session::Update(), somehow...
		//

		session->setInIncomingQueue(false);

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

		if(messageCount >avgPacketsbuild )
			avgPacketsbuild = messageCount;

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

		session->setInIncomingQueue(false);

	}

}

//======================================================================================================================

void Service::Connect(NetworkClient* client, int8* address, uint16 port)
{
#ifdef NOTHIN
	// Setup our new connection object and pass it to SocketReadThread.  This is temporary until there is time to implemnt
	// a queue/async connect method.  FIXME:  Make queue based, async using NetworkCallback for status changes.

	// We want this to be a blocking call for now, so loop waiting for change in session status from Connecting.
	mSocketReadThread->NewOutgoingConnection(address, port);

	// don't want a hard loop pegging the cpu.
	while(1)
	{
		if(mSocketReadThread->getNewConnectionInfo()->mSession)
		{
			if(mSocketReadThread->getNewConnectionInfo()->mSession->getStatus() == SSTAT_Connected)
			{
				break;
			}
			mSocketReadThread->getNewConnectionInfo()->mSession->Update();
		}

        boost::this_thread::sleep(boost::posix_time::milliseconds(10));
	}

	client->setSession(mSocketReadThread->getNewConnectionInfo()->mSession);
	mSocketReadThread->getNewConnectionInfo()->mSession->setClient(client);
#endif
}

//======================================================================================================================

void Service::AddSessionToProcessQueue(Session* session)
{
	if(!session->getInIncomingQueue())
	{
		session->setInIncomingQueue(true);
		mSessionProcessQueue.push(session);

	}

	mNetworkManager->AddServiceToProcessQueue(this);
}

//======================================================================================================================

int8* Service::getLocalAddress(void)
{
  return inet_ntoa(*(struct in_addr *)&mLocalAddress);
}

//======================================================================================================================

uint16 Service::getLocalPort(void)
{
  return ntohs(mLocalPort);

}

//======================================================================================================================

void Service::HandleRecvFrom(const boost::system::error_code &error, std::size_t bytesRecvd)
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
		AddressSessionMap2::iterator i = mAddressSessionMap.find( mRecvEndpoint );
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
				gLogger->logMsgF("Service: New Session (%s:%u)", MSG_NORMAL, mRecvEndpoint.address().to_string().c_str(), mRecvEndpoint.port());
				session = mSessionFactory->CreateSession();
				session->setPacketFactory(mPacketFactory);
				session->setRemoteEndpoint( mRecvEndpoint );
				mAddressSessionMap.insert( std::pair< boost::asio::ip::udp::endpoint, Session* >( mRecvEndpoint, session ) );
			}
		}

		//
		// Queue the packet and add the Session to the processing Queue.
		//
		session->setInIncomingQueue(false);
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
		boost::bind(&Service::HandleRecvFrom, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)
		);
}

//======================================================================================================================

void Service::HandleSendTo(Packet* msg)
{
	msg->setReadIndex(0);
	gLogger->hexDump( msg->getData(), msg->getSize() );
	mPacketFactory->DestroyPacket(msg);
}

//======================================================================================================================
