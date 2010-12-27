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

#include <glog/logging.h>

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

    if(serverservice)
    {

        mServerService = true;
        mMessageMaxSize = network_configuration.getServerToServerReliableSize();

    }
    else
    {
        mServerService = false;
        mMessageMaxSize = network_configuration.getServerToClientReliableSize();
    }


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

    unCount = 	reCount = 0;
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

    uint32 packets = 50;
    if(mServerService)
        packets = 1000;


    // Main loop
    while(!mExit)
    {

        uint32 sessionCount = mSessionQueue.size();

        for(uint32 i = 0; i < sessionCount; i++)
        {
            uint32 packetCount = 0;
            if(!mSessionQueue.pop(session))
                continue;

            // Process our session
			//active_.Send( [=] {
			session->ProcessWriteThread();
			//}
			//);

            _send(session);
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
}

//======================================================================================================================

/*

while(!mExit)    {
		utils::ActiveObject				active_;
        uint32 sessionCount = mSessionQueue.size();

		if(!this->mServerService)
			DLOG(INFO) << "SocketWriteThread::run() START";
		if(!this->mServerService)
			DLOG(INFO) << "";
        for(uint32 i = 0; i < sessionCount; i++)
        {
            uint32 packetCount = 0;

            if(!mSessionQueue.pop(session))
                continue;

            // Process our session
			// we want to put part of the load in an active Object, but not all
			// otherwise our writethread will be finished long before the activeObject churned through all the sessions
			// and we will lag until the thread starts processing again
			flipflop = true;
			if(flipflop && (!session->getSessionQueue()) && (session->getStatus() != SSTAT_Disconnected))	{
				session->setSessionQueue(true);
				//if(!this->mServerService)
					//DLOG(INFO) << "Put Session :" << session->getId() << " on wire";
				//I1227 20:07:02.208938  3580 SocketWriteThread.cpp:165] Put Session :96 on wire
				//I1227 20:07:02.208938  7568 SocketWriteThread.cpp:168] Session :96 active start
				//I1227 20:07:02.210938  7568 SocketWriteThread.cpp:172] Session :96 active end
				active_.Send( [=] {
					if(!this->mServerService)
						DLOG(INFO) << "Session :" << session->getId() << " active start";
					session->ProcessWriteThread();
					mAsyncSessionQueue.push(session);	
					if(!this->mServerService)
						DLOG(INFO) << "Session :" << session->getId() << " active end";
					
				}
				);
			}	else 
			if((!session->getSessionQueue()))	{
				if(!this->mServerService)
					DLOG(INFO) << "Session :" << session->getId() << " oldway start";
				session->setSessionQueue(true);
				session->ProcessWriteThread();
				if(!this->mServerService)
					DLOG(INFO) << "Session :" << session->getId() << " oldway end";
				_send(session);
				if(!this->mServerService)
					DLOG(INFO) << "Session :" << session->getId() << " oldway send";
			}

			
	
        }

		sessionCount = mAsyncSessionQueue.size();

		//now get all the workerthreads results and put them on wire
        for(uint32 i = 0; i < sessionCount; i++)
        {
			if(!mAsyncSessionQueue.pop(session))
                continue;
			if(!this->mServerService)
				DLOG(INFO) << "Session :" << session->getId() << " active send start";

			_send(session);
			
			if(!this->mServerService)
				DLOG(INFO) << "Session :" << session->getId() << " active send end";
		}
		if(!this->mServerService)
			DLOG(INFO) << "SocketWriteThread::run() END";
		if(!this->mServerService)
			DLOG(INFO) << "";

        boost::this_thread::sleep(boost::posix_time::milliseconds(1));
    }

    // Shutdown internally
    _shutdown();
}
*/
//======================================================================================================================

void SocketWriteThread::_send(Session* session)
{
	Packet*             packet;

	// amount of reliables to process per pass - we dont want stalling or drowning the receiver in packetbursts
	// remember we have hundreds of clientsessions but only one serversession
	uint32 reliable_packets = 50;
	uint32 unreliable_packets = 500;
    
	if(mServerService)	{
        reliable_packets	= 1000;
		unreliable_packets	= 5000;
	}

	uint32 count = session->getOutgoingReliablePacketCount();

	if(count > reliable_packets)
		count = reliable_packets;

	while (count)
    {
		count--;
        packet = session->getOutgoingReliablePacket();
        _sendPacket(packet, session);
    }

    // Send any outgoing unreliable packets
    count = session->getOutgoingUnreliablePacketCount();
	if(count > unreliable_packets)
		count = unreliable_packets;

    while (count)
    {
		count--;        
        packet = session->getOutgoingUnreliablePacket();
        _sendPacket(packet, session);
        session->DestroyPacket(packet);
    }

	// If the session is still in a connected state, Put us back in the queue.
	if (session->getStatus() != SSTAT_Disconnected)
	{
		mSessionQueue.push(session);
	}
	else
	{
		session->setStatus(SSTAT_Destroy);
		mService->AddSessionToProcessQueue(session);
	}

}
