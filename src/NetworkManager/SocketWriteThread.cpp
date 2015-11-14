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

#include "SocketWriteThread.h"

#include "Utils/logger.h"

#include "CompCryptor.h"
#include "Packet.h"
#include "Service.h"
#include "Session.h"



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

SocketWriteThread::SocketWriteThread(SOCKET socket, Service* service, bool serverservice, NetworkConfig& network_configuration) :
    mService(0),
    mCompCryptor(0),
    mSocket(0),
    mIsRunning(false)
{
    mSocket = socket;
    mService = service;

	// amount of reliables to process per pass - we dont want stalling or drowning the receiver in packetbursts
	// remember we have hundreds of clientsessions but only one serversession
	// however without respective Priority queues this can lead to lag
    if(serverservice)    {
		reliablePackets = 10000;
		unReliablePackets = 20000;
        mServerService = true;
        mMessageMaxSize = network_configuration.getServerToServerReliableSize();

    }    else    {
		reliablePackets = 50;
		unReliablePackets = 500;
        mServerService = false;
        mMessageMaxSize = network_configuration.getServerToClientReliableSize();
    }
    //added for later use Obi
//sessionPipe = new Pipe();

    // We do have a global clock object, don't use seperate clock and times for every process.
    // mClock = new Anh_Utils::Clock();

    // Create our CompCryptor object.
    mCompCryptor = new CompCryptor();

    // start our thread
    boost::thread t(std::tr1::bind(&SocketWriteThread::run, this));

    mThread = boost::move(t);

#ifdef _WIN32
    HANDLE mtheHandle = mThread.native_handle();
    SetPriorityClass(mtheHandle,REALTIME_PRIORITY_CLASS);
#endif


    //our thread load values
    //mThreadTime = mLastThreadTime = 0;
    mLastTime =   Anh_Utils::Clock::getSingleton()->getLocalTime();
    //lastThreadProcessingTime = threadProcessingTime = 0;

}

SocketWriteThread::~SocketWriteThread()
{
    LOG(INFO) << "Socket Write Thread Ended.";

    // shutdown our thread
    mExit = true;

    mThread.interrupt();
    mThread.join();

    delete mCompCryptor;

    // delete(mClock);
}

//======================================================================================================================
void SocketWriteThread::run()
{
    Session*            session;


    // Call our internal _startup method
    _startup();

    // Main loop
    while(!mExit)    {

		//if((!this->mServerService) && sessionCount)	{
			//DLOG(INFO) << "SocketWriteThread::run() START";
			//DLOG(INFO) << "servicing : " << sessionCount << " Sessions";
			//DLOG(INFO) << "NO ACTIVE OBJECT";
		//}
		uint32 packetsSend = 0;

        while(mSessionQueue.pop(session))	{
            uint32 packetCount = 0;

            // Process our session
			active_.Send( [=] {
				session->ProcessWriteThread();
				mAsyncSessionQueue.push(session);
			}
			);

        }

		while(mAsyncSessionQueue.pop(session))	{
			_send(session);
		}

		//if((!this->mServerService) && sessionCount)	{
			//DLOG(INFO) << "SocketWriteThread::run() END";
			//DLOG(INFO) << "sending : " << packetsSend << "Packets";
		//}

       // boost::this_thread::sleep(boost::posix_time::milliseconds(1));
        sessionPipe->wait(2000);
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


    // Going to simulate network packet loss here.
    //seed_rand_mwc1616(gClock->getLocalTime());
    //if (rand_mwc1616() < 0xffffffff / 5)  // 20%
    //{
    //return;
    //}

    packet->setReadIndex(0);
    uint16 packetType = packet->getUint16();
    uint8  packetTypeLow = *(packet->getData());
    //uint8  packetTypeHigh = *(packet->getData()+1);

    // Set our TimeSent
    packet->setTimeSent(Anh_Utils::Clock::getSingleton()->getStoredTime());

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

        packet->setCRC(mCompCryptor->GenerateCRC(mSendBuffer, outLen, session->getEncryptKey()));


        mSendBuffer[outLen] = (uint8)(packet->getCRC() >> 8);
        mSendBuffer[outLen + 1] = (uint8)packet->getCRC();
        outLen += 2;
    }

    //LOG(INFO) << "Sending message to " << session->getAddressString() << " on port " << ntohs(session->getPort());
    sent = sendto(mSocket, mSendBuffer, outLen, 0, &toAddr, toLen);

    if (sent < 0)
    {
        LOG(WARNING) << "Unkown Error from socket sendto: " << errno;
    }
}

//======================================================================================================================

void SocketWriteThread::NewSession(Session* session)
{
    //using concurrent queue that has a recursive mutex
    mSessionQueue.push(session);
//    sessionPipe->send('X'); //added for later use Obi
}

//======================================================================================================================


//======================================================================================================================

void SocketWriteThread::_send(Session* session)
{
	Packet*             packet;
	
	// Send any outgoing reliable packets - retain those
	while (session->getOutgoingReliablePacket(packet))    {
		//dont destroy reliables - they get queued until acked or resend
        _sendPacket(packet, session);
    }

    // Send any outgoing unreliable packets	
    while (session->getOutgoingUnreliablePacket(packet))    {
        _sendPacket(packet, session);
        session->DestroyPacket(packet);
    }

	// If the session is still in a connected state, Put us back in the queue.
	// otherwise inform the service that we need destroying
	if (session->getStatus() != SSTAT_Disconnected)	{
		mSessionQueue.push(session);
		//sessionPipe->send('X');//added for later use Obi
	}	else	{
		session->setStatus(SSTAT_Destroy);
		mService->AddSessionToProcessQueue(session);
	}

}
