/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "Service.h"
#include "Session.h"
#include "Packet.h"
#include "SocketReadThread.h"
#include "SocketWriteThread.h"
#include "NetworkManager.h"
#include "NetworkCallback.h"
#include "NetworkClient.h"
#include "Common/Message.h"
#include "LogManager/LogManager.h"

#include "Utils/typedefs.h"

#include <stdio.h>
#include <assert.h>


//======================================================================================================================

bool Service::mSocketsSubsystemInitComplete = false;

//======================================================================================================================

Service::Service(NetworkManager* networkManager, bool serverservice) :
mNetworkManager(networkManager),
mSocketReadThread(0),
mSocketWriteThread(0),
mLocalAddress(0),
mLocalPort(0),
mLocalSocket(0),
mQueued(false),
avgTime(0),
avgPacketsbuild (0),
mServerService(serverservice)
{

}

//======================================================================================================================

Service::~Service(void)
{


}

//======================================================================================================================

void Service::Startup(int8* localAddress, uint16 localPort,uint32 mfHeapSize)
{

	lasttime = Anh_Utils::Clock::getSingleton()->getLocalTime(); 
	assert(strlen(localAddress) < 256);
	strcpy(mLocalAddressName, localAddress);
	mLocalAddress = inet_addr(localAddress);
	mLocalPort = htons(localPort);

#if(ANH_PLATFORM == ANH_PLATFORM_WIN32) 
	// Startup the windows socket layer if it's not already started.
	if (!mSocketsSubsystemInitComplete)
	{
		mSocketsSubsystemInitComplete = true;
		WSADATA data;
		WSAStartup(MAKEWORD(2,0), &data);
	}
#endif //WIN32

	// Create our socket descriptors
	SOCKET mLocalSocket = socket(PF_INET, SOCK_DGRAM, 0);

	// Bind to our listen port.
	sockaddr_in   server;
	server.sin_family   = AF_INET;
	server.sin_port     = mLocalPort;
	server.sin_addr.s_addr = INADDR_ANY;
	int serverLen = sizeof(server);

	// Attempt to bind to our socket
	int32 result = bind(mLocalSocket, (sockaddr*)&server, sizeof(server));

	// We need to call connect on the socket to an address before we can know which address we have.
	// The address specified in the connect call determines which interface our socket is associated with
	// based on routing.  1.1.1.1 should give us the default adaptor.  Not sure what to do one multihomed hosts yet.
	struct sockaddr   toAddr;
	int32             sent, toLen = sizeof(toAddr);

	toAddr.sa_family = AF_INET;
	*((uint32*)&toAddr.sa_data[2]) = 0;
	*((uint16*)&(toAddr.sa_data[0])) = 0;

	// This connect will make the socket only acceept packets from the destination.  Need to reset at end.
	//sent = sendto(mLocalSocket, mSendBuffer, 1, 0, &toAddr, toLen);
	sent = connect(mLocalSocket, &toAddr, toLen);

	// Create our read/write socket classes
	mSocketWriteThread = new SocketWriteThread();
	mSocketReadThread = new SocketReadThread();

	mSocketWriteThread->Startup(mLocalSocket,this,mServerService);
	mSocketReadThread->Startup(mLocalSocket, mSocketWriteThread,this,mfHeapSize, mServerService);

	// Query the stack for the actual address and port we got and store it in the service.
	//getsockname(mLocalSocket, (sockaddr*)&server, &serverLen);
	//mLocalAddress = server.sin_addr.s_addr;
	//mLocalPort = server.sin_port;

	// Reset the connect call to universe.
	toAddr.sa_family = AF_INET;
	*((uint32*)&toAddr.sa_data[2]) = 0;
	*((uint16*)&(toAddr.sa_data[0])) = 0;
	sent = connect(mLocalSocket, &toAddr, toLen);

}

//======================================================================================================================

void Service::Shutdown(void)
{
	Session* session = 0;

	while(mSessionProcessQueue.size())
	{
		session = mSessionProcessQueue.pop();

		if(session)
		{
			mSocketReadThread->RemoveAndDestroySession(session);
		}
	}

	mSocketWriteThread->Shutdown();
	mSocketReadThread->Shutdown();

	delete mSocketWriteThread;
	delete mSocketReadThread;

	closesocket(mLocalSocket);
	mLocalSocket = INVALID_SOCKET;

#if(ANH_PLATFORM == ANH_PLATFORM_WIN32)
	WSACleanup();
#endif
}

//======================================================================================================================

void Service::Process(uint32 loop)
{
	//we only ever get here with a connected session
	
	// Get the current count of Sessions to be processed.  We can't just check to see if the queue is empty, since
	// the other threads could keep placing more Packets in the queue, and this could cause a stall in the
	// main thread.
	Session* session = 0;
	Message* message = 0;
	NetworkClient* newClient = 0;
	uint32 sessionCount = mSessionProcessQueue.size();

	//prevent stalling the servers
	//we really should shorten the thread by shortening packetwritetime and leave the sessioncount
	//but we need to modify that for the server service 
	if(sessionCount>loop)
		sessionCount =loop;

	for(uint32 i = 0; i < sessionCount; i++)
	{
		// Grab our next Service to process
		session = mSessionProcessQueue.pop();

		if(!session)
			continue;
	
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
		  mSocketReadThread->RemoveAndDestroySession(session);

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

				//actually had a few messages of sessions pending delete
				//as the socketwrite thread sets the status and in busy situations we can get problems
				//fixed with that ???
			
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
		}

		msleep(1);
	}

	client->setSession(mSocketReadThread->getNewConnectionInfo()->mSession);
	mSocketReadThread->getNewConnectionInfo()->mSession->setClient(client);
}

//======================================================================================================================

void Service::AddSessionToProcessQueue(Session* session)
{
	//do the mutexes here really cause a stall ????????
	//mServiceMutex.acquire();

	if(!session->getInIncomingQueue())
	{
		session->setInIncomingQueue(true);
		mSessionProcessQueue.push(session);
		
	}

	//mServiceMutex.release();

	mNetworkManager->AddServiceToProcessQueue(this);
}

//======================================================================================================================

int8* Service::getLocalAddress(void)                             
{ 
  in_addr address;
  address.S_un.S_addr = mLocalAddress;

  return inet_ntoa(address); 
}

//======================================================================================================================

uint16 Service::getLocalPort(void)
{ 
  return ntohs(mLocalPort); 
  
}

//======================================================================================================================


