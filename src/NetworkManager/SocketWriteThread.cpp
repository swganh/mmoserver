/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "SocketWriteThread.h"

#include "CompCryptor.h"
#include "NetConfig.h"
#include "Packet.h"
#include "Service.h"
#include "Session.h"

#include "LogManager/LogManager.h"

#include "Utils/rand.h"

#if defined(__GNUC__)
// GCC implements tr1 in the <tr1/*> headers. This does not conform to the TR1
// spec, which requires the header without the tr1/ prefix.
#include <tr1/functional>
#else
#include <functional>
#endif

#if defined(_MSC_VER)
	#ifndef _WINSOCK2API_
#include <WINSOCK2.h>
#undef errno
#define errno WSAGetLastError()
	#endif
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>

#define INVALID_SOCKET	-1
#define SOCKET_ERROR	-1
#define closesocket		close
#endif

#include <boost/thread/thread.hpp>

//======================================================================================================================

SocketWriteThread::SocketWriteThread() :
mService(0),
mCompCryptor(0),
mSocket(0),
mIsRunning(false)
{

}

//======================================================================================================================

void SocketWriteThread::Startup(SOCKET socket, Service* service, bool serverservice)
{
	mSocket = socket;
	mService = service;

	if(serverservice)
	{

		mServerService = true;
		mMessageMaxSize = gNetConfig->getServerServerReliableSize();

	}
	else
	{
		mServerService = false;
		mMessageMaxSize = gNetConfig->getServerClientReliableSize();
	}


	// We do have a global clock object, don't use seperate clock and times for every process.
	// mClock = new Anh_Utils::Clock();

	// Create our CompCryptor object.
	mCompCryptor = new CompCryptor();
	mCompCryptor->Startup();

	// start our thread
    boost::thread t(std::tr1::bind(&SocketWriteThread::run, this));
    mThread = boost::move(t);

	lasttime =   Anh_Utils::Clock::getSingleton()->getLocalTime();
	unCount = 	reCount = 0;
}

//======================================================================================================================

void SocketWriteThread::Shutdown(void)
{
	gLogger->logMsg("SocketWriteThread ended");

	// shutdown our thread
	mExit = true;

    mThread.interrupt();
    mThread.join();

	mCompCryptor->Shutdown();
	delete(mCompCryptor);

	// delete(mClock);
}


//======================================================================================================================
void SocketWriteThread::run()
{
	Session*            session;
	Packet*             packet;

	// Call our internal _startup method
	_startup();

	// Main loop
	while(!mExit)
	{

		uint32 sessionCount = mSessionQueue.size();

		for(uint32 i = 0; i < sessionCount; i++)
		{
			session = mSessionQueue.pop();

			if(!session)
				continue;

			// Process our session
			session->ProcessWriteThread();

			// Send any outgoing reliable packets
			//uint32 rcount = 0;

			while (session->getOutgoingReliablePacketCount())
			{
			//	rcount++;
				packet = session->getOutgoingReliablePacket();
				_sendPacket(packet, session);
			}


			// Send any outgoing unreliable packets
			//uint32 ucount = 0;
			while (session->getOutgoingUnreliablePacketCount())
			{
			//	ucount++;
				packet = session->getOutgoingUnreliablePacket();
				_sendPacket(packet, session);

				//accessing the packetpool actually seems to crash regularly when we have a high load (around 150 bots in one bigger spot)
				//might it make sense to put it on a session queue for later deletion in the session to avoid threading issues?
				session->DestroyPacket(packet);
			}


			// If the session is still in a connected state, Put us back in the queue.
			if (session->getStatus() != SSTAT_Disconnected)
			{
				mSessionQueue.push(session);
			}
			else
			{
				gLogger->logMsg("SocketWriteThread destroy session");
				session->setStatus(SSTAT_Destroy);
				mService->AddSessionToProcessQueue(session);
			}
		}

        boost::this_thread::sleep(boost::posix_time::milliseconds(1));
	}

	// Shutdown internally
	_shutdown();
}

//======================================================================================================================

void SocketWriteThread::_startup(void)
{
	// Initialization is done.  All of it.  :)
	mIsRunning = true;
	mExit = false;
}

//======================================================================================================================

void SocketWriteThread::_shutdown(void)
{
	// Shutting down
	mIsRunning = false;
}

//======================================================================================================================

void SocketWriteThread::_sendPacket(Packet* packet, Session* session)
{
	struct sockaddr     toAddr;
	uint32              sent, toLen = sizeof(toAddr), outLen;


	// Some basic bounds checking.
	if(packet->getSize() > mMessageMaxSize)
	{
		gLogger->logErrorF("Netcode","packet (%u) is longer than mMessageMaxSize (%u)",MSG_HIGH,packet->getSize(),mMessageMaxSize);
		return;
	}
	//assert(packet->getSize() <= mMessageMaxSize);

	// Want a fresh send buffer for debugging purposes.
	memset(mSendBuffer, 0xcd, sizeof(mSendBuffer));
/*
  // Going to simulate network packet loss here.
  seed_rand_mwc1616(mClock->getLocalTime());
  if (rand_mwc1616() < 0xffffffff / 5)  // 20%
  {
    gLogger->logMsg("*** Packet dropped for loss simulation.", MSG_HIGH);
    return;
  }
*/
	packet->setReadIndex(0);
	uint16 packetType = packet->getUint16();
	uint8  packetTypeLow = *(packet->getData());
	//uint8  packetTypeHigh = *(packet->getData()+1);

	//gLogger->logMsgF("OnWire, Type:0x%.4x, Session:0x%x%.4x, IP: 0x%.8x, port:%u", MSG_LOW, packetType, session->getService()->getId(), session->getId(), session->getAddress(), ntohs(session->getPort()));

	// Set our TimeSent
	packet->setTimeSent(Anh_Utils::Clock::getSingleton()->getLocalTime());

	// Setup our to address
	toAddr.sa_family = AF_INET;
	*((unsigned int*)&toAddr.sa_data[2]) = session->getAddress();     // Ports and addresses are stored in network order.
	*((unsigned short*)&(toAddr.sa_data[0])) = session->getPort();    // Only need to convert for humans.

	// Copy our 2 byte header.
	*((uint16*)mSendBuffer) = *((uint16*)packet->getData());

	// Compress the packet if needed.
	if(packet->getIsCompressed())
	{
		if(packetTypeLow == 0)
		{
			// Compress our packet, but not the header
			outLen = mCompCryptor->Compress(packet->getData() + 2, packet->getSize() - 2, mSendBuffer + 2, sizeof(mSendBuffer));
		}
		else
		{
			outLen = mCompCryptor->Compress(packet->getData() + 1, packet->getSize() - 1, mSendBuffer + 1, sizeof(mSendBuffer));
		}

		// If we compressed it, place a 1 at the end of the buffer.
		if(outLen)
		{
			if(packetTypeLow == 0)
			{
				mSendBuffer[outLen + 2] = 1;
				outLen += 3;  //thats 2 (uncompressed) headerbytes plus the encryption flag
			}
			else
			{
				mSendBuffer[outLen + 1] = 1;
				outLen += 2;
			}
		}
		// else a 0 - so no compression
		else
		{
		  memcpy(mSendBuffer, packet->getData(), packet->getSize());
		  outLen = packet->getSize();

		  mSendBuffer[outLen] = 0;
		  outLen += 1;
		}
	}
	else if(packetType == SESSIONOP_SessionResponse || packetType == SESSIONOP_CriticalError)
	{
		memcpy(mSendBuffer, packet->getData(), packet->getSize());
		outLen = packet->getSize();
	}
	else
	{
		memcpy(mSendBuffer, packet->getData(), packet->getSize());
		outLen = packet->getSize();

		mSendBuffer[outLen] = 0;
		outLen += 1;
	}

	// Encrypt the packet if needed.
	if(packet->getIsEncrypted())
	{
		if(packetTypeLow == 0)
		{
			mCompCryptor->Encrypt(mSendBuffer + 2, outLen - 2, session->getEncryptKey()); // -2 header is not encrypted
		}
		else if(packetTypeLow < 0x0d)
		{
			mCompCryptor->Encrypt(mSendBuffer + 1, outLen - 1, session->getEncryptKey()); // - 1 header is not encrypted
		}
		else
		{
			gLogger->hexDump(packet->getData(),packet->getSize());
		}
		//assert(packetTypeLow < 0x0d);

		packet->setCRC(mCompCryptor->GenerateCRC(mSendBuffer, outLen, session->getEncryptKey()));


		mSendBuffer[outLen] = (uint8)(packet->getCRC() >> 8);
		mSendBuffer[outLen + 1] = (uint8)packet->getCRC();
		outLen += 2;


	}

	sent = sendto(mSocket, mSendBuffer, outLen, 0, &toAddr, toLen);

	if((outLen > mMessageMaxSize) )
	  {
		  gLogger->logMsgF("Cave Wrote Packetsize : %u Max Allowed Size : %u", MSG_HIGH, outLen,mMessageMaxSize);
		  gLogger->hexDump(mSendBuffer,outLen);
	  }

	if (sent < 0)
	{
		gLogger->logMsgF("*** Unkown error from socket sendto: %u", MSG_HIGH, errno);
	}
}

//======================================================================================================================

void SocketWriteThread::NewSession(Session* session)
{
	//using concurrent queue that has a recursive mutex
	mSessionQueue.push(session);
}

//======================================================================================================================



