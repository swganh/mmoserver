/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#define WIN32_LEAN_AND_MEAN
#include "SocketReadThread.h"

#include "CompCryptor.h"
#include "NetworkClient.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Service.h"
#include "Session.h"
#include "SessionFactory.h"
#include "Socket.h"
#include "SocketWriteThread.h"

#include "LogManager/LogManager.h"
#include "Common/MessageFactory.h"

#include <boost/thread/thread.hpp>

#if defined(__GNUC__)
// GCC implements tr1 in the <tr1/*> headers. This does not conform to the TR1
// spec, which requires the header without the tr1/ prefix.
#include <tr1/functional>
#else
#include <functional>
#endif

#if defined(_MSC_VER)
#define socklen_t int
#else
#include <sys/socket.h>
#include <arpa/inet.h>

#define INVALID_SOCKET	-1
#define SOCKET_ERROR	-1
#define closesocket		close
#endif

//======================================================================================================================

SocketReadThread::SocketReadThread() :
mReceivePacket(0),
mDecompressPacket(0),
mSessionFactory(0),
mPacketFactory(0),
mCompCryptor(0),
mSocket(0),
mIsRunning(false)
{

}

//======================================================================================================================

void SocketReadThread::Startup(SOCKET socket, SocketWriteThread* writeThread, Service* service,uint32 mfHeapSize, bool serverservice)
{
	if(serverservice)
	{
		mMessageMaxSize = gNetConfig->getServerServerReliableSize();
		mSessionResendWindowSize = gNetConfig->getServerPacketWindow();
	}
	else
	{
		mMessageMaxSize = gNetConfig->getServerClientReliableSize();
		mSessionResendWindowSize = gNetConfig->getClientPacketWindow();
	}

	mSocket = socket;
	mSocketWriteThread = writeThread;

	// Init our NewConnection object
	memset(mNewConnection.mAddress, 0, sizeof(mNewConnection.mAddress));
	mNewConnection.mPort = 0;
	mNewConnection.mSession = 0;

	// Startup our factories
	mMessageFactory = new MessageFactory(mfHeapSize,service->getId());

	mPacketFactory = new PacketFactory();
	mPacketFactory->Startup(serverservice);

	mSessionFactory = new SessionFactory();
	mSessionFactory->Startup(writeThread, service, mPacketFactory, mMessageFactory, serverservice);

	mCompCryptor = new CompCryptor();
	mCompCryptor->Startup();

	// Allocate our receive packets
	mReceivePacket = mPacketFactory->CreatePacket();
	mDecompressPacket = mPacketFactory->CreatePacket();

	// start our thread
    boost::thread t(std::tr1::bind(&SocketReadThread::run, this));
    mThread = boost::move(t);

	HANDLE th =  mThread.native_handle();
	SetPriorityClass(th,REALTIME_PRIORITY_CLASS);	
	//SetPriorityClass(th,NORMAL_PRIORITY_CLASS);	
	
	
}

//======================================================================================================================

void SocketReadThread::Shutdown(void)
{
	mExit = true;

    mThread.interrupt();
    mThread.join();

	mCompCryptor->Shutdown();
	mPacketFactory->Shutdown();
	mSessionFactory->Shutdown();

	delete mCompCryptor;
	delete mSessionFactory;
	delete mPacketFactory;
	delete mMessageFactory;
}

//======================================================================================================================

void SocketReadThread::run(void)
{

#ifdef NOTHIN
	struct sockaddr_in  from;
	int              address, fromLen = sizeof(from), count;
	int16               recvLen;
	uint16              port;
	Session*            session;
	fd_set              socketSet;
	struct              timeval tv;

	FD_ZERO(&socketSet);

	// Call our internal _startup method
	_startup();

	while(!mExit)
	{
		// Check to see if *WE* are about to connect to a remote server 
		if(mNewConnection.mPort != 0)
		{
			Session* newSession = mSessionFactory->CreateSession();
			newSession->setCommand(SCOM_Connect);
			newSession->setAddress(inet_addr(mNewConnection.mAddress));
			newSession->setPort(htons(mNewConnection.mPort));

			uint64 hash = newSession->getAddress() | (((uint64)newSession->getPort()) << 32);

			mNewConnection.mSession = newSession;
			mNewConnection.mPort = 0;

			// Add the new session to the main process list

            boost::recursive_mutex::scoped_lock lk(mSocketReadMutex);

			mAddressSessionMap.insert(std::make_pair(hash,newSession));
			mSocketWriteThread->NewSession(newSession);
		}

		// Reset our internal members so we can use the packet again.
		mReceivePacket->Reset();
		mDecompressPacket->Reset();

		// Build a new fd_set structure
		FD_SET(mSocket, &socketSet);

		// We're going to block for 250ms.
		tv.tv_sec   = 0;
		tv.tv_usec  = 50;


		count = select(mSocket, &socketSet, 0, 0, &tv);

		if(count && FD_ISSET(mSocket, &socketSet))
		{

			
			// Read any incoming packets.
			recvLen = recvfrom(mSocket, mReceivePacket->getData(),(int) mMessageMaxSize, 0, (sockaddr*)&from, reinterpret_cast<socklen_t*>(&fromLen));
			if (recvLen <= 2)
			{
				if (recvLen <= 2)
				{
					#if(ANH_PLATFORM == ANH_PLATFORM_WIN32)
					int error = WSAGetLastError();
					gLogger->logMsgF("*** Unkown error from socket recvFrom: %i recvL returned %i", MSG_NORMAL, error,recvLen);
					gLogger->logMsgF("*** mMessageMaxSize: %i", MSG_NORMAL, mMessageMaxSize);
					#elif(ANH_PLATFORM == ANH_PLATFORM_LINUX)
					#endif
				}

				continue;


				recvLen = mMessageMaxSize;
			}
			if(recvLen > mMessageMaxSize)
				gLogger->logMsgF("*** Received Size > mMessageMaxSize: %u", MSG_NORMAL, recvLen);

			// Get our remote Address and port
			address = from.sin_addr.s_addr;
			port = from.sin_port;
			uint64 hash = address | (((uint64)port) << 32);

			// Grab our packet type
			mReceivePacket->Reset();           // Reset our internal members so we can use the packet again.
			mReceivePacket->setSize(recvLen); // crc is subtracted by the decryption
			uint8  packetTypeLow = mReceivePacket->peekUint8();
			uint16 packetType = mReceivePacket->getUint16();

			//gLogger->logMsgF("FromWire, Type:0x%.4x, size:%u, IP:0x%.8x, port:%u", MSG_LOW, packetType, recvLen, address, ntohs(port));


			// TODO: Implement an IP blacklist so we can drop packets immediately.

            //boost::recursive_mutex::scoped_lock lk(mSocketReadMutex);
			//reading should be threadsafe ????
			AddressSessionMap::iterator i = mAddressSessionMap.find(hash);

			if(i != mAddressSessionMap.end())
			{
				session = (*i).second;
			}
			else
			{
				// We should only be creating a new session if it's a session request packet
				if(packetType == SESSIONOP_SessionRequest)
				{
					gLogger->logMsgF("new Session created hash : %I64u ",MSG_HIGH,hash);
					session = mSessionFactory->CreateSession();
					session->setSocketReadThread(this);
					session->setPacketFactory(mPacketFactory);
					session->setAddress(address);  // Store the address and port in network order so we don't have to
					session->setPort(port);  // convert them all the time.  Only convert for humans.

					// Insert the session into our address map and process list
					boost::recursive_mutex::scoped_lock lk(mSocketReadMutex);
					mAddressSessionMap.insert(std::make_pair(hash, session));
					mSocketWriteThread->NewSession(session);
					session->mHash = hash;

					gLogger->logMsgF("Added Service %i: New Session(%s, %u), AddressMap: %i",MSG_HIGH,mSessionFactory->getService()->getId(), inet_ntoa(from.sin_addr), ntohs(session->getPort()), mAddressSessionMap.size());
				}
				else
				{
					gLogger->logMsgF("*** Session not found.  Packet dropped. Type:0x%.4x", MSG_NORMAL, packetType);

					continue;
				}
			}

			session->setInIncomingQueue(false);
			session->QueueIncomingPacket(mReceivePacket);
			session->getService()->AddSessionToProcessQueue(session);
			mReceivePacket = mPacketFactory->CreatePacket();
		}
		
		boost::this_thread::sleep(boost::posix_time::microseconds(10));

	}

	_shutdown();

#endif
}

//======================================================================================================================

void SocketReadThread::NewOutgoingConnection(int8* address, uint16 port)
{
	// This will only handle a single connect call at a time right now.  At some point it would be good to make this a
	// queue so we can process these async.  This is NOT thread safe, and won't be.  Only should be called by the Service.

	// Init our NewConnection object

	strcpy(mNewConnection.mAddress, address);
	mNewConnection.mPort = port;
	mNewConnection.mSession = 0;
}

//======================================================================================================================

void SocketReadThread::RemoveAndDestroySession(Session* session)
{
	// Find and remove the session from the address map.
	// session->getPort() is uint16 !!!!!!!!!!!!!!!!!!!!
	uint64 hash = session->getAddress() | (((uint64)session->getPort()) << 32);

	gLogger->logMsgF("Added to destroy list :::Service %i: Removing Session(%s, %u), AddressMap: %i hash %I64u",MSG_NORMAL,mSessionFactory->getService()->getId(), inet_ntoa(*((in_addr*)(&hash))), ntohs(session->getPort()), mAddressSessionMap.size(),hash);
	gLogger->logMsgF("hash %I64u  vs mHash %I64u",MSG_HIGH,hash,session->mHash);
	
	boost::recursive_mutex::scoped_lock lk(mSocketReadMutex);
	//DestroyList.push_back(session->mHash);
										
	AddressSessionMap::iterator iter = mAddressSessionMap.find(hash);

	if(iter != mAddressSessionMap.end())
	{
		//boost::recursive_mutex::scoped_lock lk(mSocketReadMutex);
		mAddressSessionMap.erase(iter);
		//gLogger->logMsgF("Service %i: Removing Session(%s, %u), AddressMap: %i hash %I64u",MSG_NORMAL,mSessionFactory->getService()->getId(), inet_ntoa(*((in_addr*)(&hash))), ntohs(session->getPort()), mAddressSessionMap.size(),hash);
		mSessionFactory->DestroySession(session);
	}
	else
		gLogger->logMsgF("Service %i: Removing Session FAILED(%s, %u), AddressMap: %i hash %I64u",MSG_NORMAL,mSessionFactory->getService()->getId(), inet_ntoa(*((in_addr*)(&hash))), ntohs(session->getPort()), mAddressSessionMap.size(),hash);

	// why the %/*&+ is this not finding the session ? it completely legitimately finds it in the run() ???
	
}

//======================================================================================================================

void SocketReadThread::_startup(void)
{
	// Initialization is done.  All of it.  :)
	mIsRunning = true;
	mExit = false;
}

//======================================================================================================================

void SocketReadThread::_shutdown(void)
{
	// Shutting down
	mIsRunning = false;
}

//======================================================================================================================





