/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2010 The SWG:ANH Team
---------------------------------------------------------------------------------------
Use of this source code is governed by the GPL v3 license that can be found
in the COPYING file or at http://www.gnu.org/licenses/gpl-3.0.html

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
---------------------------------------------------------------------------------------
*/

#include "Service.h"

#include "NetworkCallback.h"
#include "NetworkClient.h"
#include "NetworkManager.h"
#include "Packet.h"
#include "Session.h"
#include "SocketReadThread.h"
#include "SocketWriteThread.h"

#include "LogManager/LogManager.h"

#include "Common/Message.h"

#include "ConfigManager/ConfigManager.h"
#include "Utils/typedefs.h"

#include <boost/thread/thread.hpp>

#if defined(_MSC_VER)
	#ifndef _WINSOCK2API_
#include <WINSOCK2.h>
	#endif
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

Service::Service(NetworkManager* networkManager, bool serverservice, uint32 id, int8* localAddress, uint16 localPort,uint32 mfHeapSize) :
mNetworkManager(networkManager),
mSocketReadThread(0),
mSocketWriteThread(0),
mLocalSocket(0),
avgTime(0),
avgPacketsbuild (0),
mLocalAddress(0),
mLocalPort(0),
mQueued(false),
mServerService(serverservice)
{
	mCallBack = NULL;
	mId = id;

	//localAddress = (char*)gConfig->read<std::string>("BindAddress").c_str();
	lasttime = Anh_Utils::Clock::getSingleton()->getLocalTime();
	assert(strlen(localAddress) < 256 && "Address length should be less than 256");
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

	// Attempt to bind to our socket
	bind(mLocalSocket, (sockaddr*)&server, sizeof(server));

	// We need to call connect on the socket to an address before we can know which address we have.
	// The address specified in the connect call determines which interface our socket is associated with
	// based on routing.  1.1.1.1 should give us the default adaptor.  Not sure what to do one multihomed hosts yet.
//	struct sockaddr   toAddr;
//	int32             sent, toLen = sizeof(toAddr);
/*
	toAddr.sa_family = AF_INET;
	*((uint32*)&toAddr.sa_data[2]) = 0;
	*((uint16*)&(toAddr.sa_data[0])) = 0;

	// This connect will make the socket only acceept packets from the destination.  Need to reset at end.
	//sent = sendto(mLocalSocket, mSendBuffer, 1, 0, &toAddr, toLen);
	sent = connect(mLocalSocket, &toAddr, toLen);
*/
	//set the socketbuffer so we dont suffer internal dataloss
	int value;
	int valuelength = sizeof(value);
	value = 524288;
	int configvalue = gConfig->read<int32>("UDPBufferSize",4096);
	gLogger->log(LogManager::INFORMATION, "UDPBuffer set to %ukb", configvalue);

	if(configvalue < 128)
		configvalue = 128;
	
	if(configvalue > 8192)
		configvalue = 8192;

	value = configvalue *1024;
	
	setsockopt(mLocalSocket,SOL_SOCKET,SO_RCVBUF,(char*)&value,valuelength);

	int temp = 1;
	//9 is IP_DONTFRAG (PK told me to put that here so we know wtf 9 means :P
	setsockopt(mLocalSocket, IPPROTO_IP, 9, (char*)&temp, sizeof(temp));


	// Create our read/write socket classes
	mSocketWriteThread = new SocketWriteThread(mLocalSocket,this,mServerService);
	mSocketReadThread = new SocketReadThread(mLocalSocket, mSocketWriteThread,this,mfHeapSize, mServerService);

	// Query the stack for the actual address and port we got and store it in the service.
	//getsockname(mLocalSocket, (sockaddr*)&server, &serverLen);
	//mLocalAddress = server.sin_addr.s_addr;
	//mLocalPort = server.sin_port;
/*
	// Reset the connect call to universe.
	toAddr.sa_family = AF_INET;
	*((uint32*)&toAddr.sa_data[2]) = 0;
	*((uint16*)&(toAddr.sa_data[0])) = 0;
	sent = connect(mLocalSocket, &toAddr, toLen);
*/
}

//======================================================================================================================

Service::~Service(void)
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

	delete mSocketWriteThread;
	delete mSocketReadThread;

	closesocket(mLocalSocket);
	mLocalSocket = INVALID_SOCKET;

	#if(ANH_PLATFORM == ANH_PLATFORM_WIN32)
		WSACleanup();
	#endif
}

//======================================================================================================================

void Service::Process()
{
	//we only ever get here with a connected session

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

		if(!session)
			continue;

		session->setInIncomingQueue(false);

		// Check to see if we're in the process of connecting or disconnecting.
		if(session->getStatus() == SSTAT_Connecting)
		{
			//for(NetworkCallbackList::iterator iter = mNetworkCallbackList.begin(); iter != mNetworkCallbackList.end(); ++iter)
			//{
			//mCallBack->handleSessionMessage(session->getClient(), message);
				newClient = mCallBack->handleSessionConnect(session, this);

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
			//}
		}
		else if(session->getStatus() == SSTAT_Disconnecting)
		{

			NetworkCallbackList::iterator iter;

			//for(iter = mNetworkCallbackList.begin(); iter != mNetworkCallbackList.end(); ++iter)
			//{
				if(session->getClient())
				{
					mCallBack->handleSessionDisconnect(session->getClient());
					session->setClient(0);
				}
			//}

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

		if(!session->getClient())
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
				//NetworkCallbackList::iterator iter;

				mCallBack->handleSessionMessage(session->getClient(), message);
				//for(iter = mNetworkCallbackList.begin(); iter != mNetworkCallbackList.end(); ++iter)
				//{
					//(*iter)->handleSessionMessage(session->getClient(), message);
				//}
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

        boost::this_thread::sleep(boost::posix_time::milliseconds(10));
	}

	client->setSession(mSocketReadThread->getNewConnectionInfo()->mSession);
	mSocketReadThread->getNewConnectionInfo()->mSession->setClient(client);
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


