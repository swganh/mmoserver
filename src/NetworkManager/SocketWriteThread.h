/*
 This file is part of MMOServer. For more information, visit http://swganh.com
 
 Copyright (c) 2006 - 2011 The SWG:ANH Team

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef ANH_NETWORKMANAGER_SOCKETWRITETHREAD_H
#define ANH_NETWORKMANAGER_SOCKETWRITETHREAD_H

#include "Utils/typedefs.h"
#include "Utils/clock.h"
#include "Utils/ConcurrentQueue.h"
#include "Utils/ActiveObject.h"

#include "NetworkConfig.h"
#include <boost/thread/thread.hpp>

#define SEND_BUFFER_SIZE 8192

//======================================================================================================================

class Service;
class Packet;
class Session;
class CompCryptor;

typedef utils::ConcurrentQueue<Session*>			SessionQueue;
typedef utils::ConcurrentQueueLight  <Session*>		SessionQueueLight;

//======================================================================================================================

class SocketWriteThread
{
public:

    SocketWriteThread(SOCKET socket, Service* service, bool serverservice, NetworkConfig& network_configuration);
    ~SocketWriteThread();

    virtual void	run();

    void			NewSession(Session* session);

    bool			getIsRunning(void) {
        return mIsRunning;
    }
    void			requestExit() {
        mExit = true;
    }

private:

    void			_startup(void);
    void			_shutdown(void);

	/**
	* Sends all the packet of the given session on the wire
    *
	* \param packet  the packet we are going to put on the wire
    * \param session the session whose packets we want to send
	*
	*/
    void			_sendPacket(Packet* packet, Session* session);
	
	/**
	* Sends all the packets the worker threads have build
    * and does basic session housekeeping
	*
    * \param session the session whose packets we want to send
	*
	*/
	void				_send(Session* session);

    uint16				mMessageMaxSize;
    int8				mSendBuffer[SEND_BUFFER_SIZE];
    Service*			mService;
    CompCryptor*		mCompCryptor;
    SOCKET				mSocket;
    bool				mIsRunning;
    uint64			    mLastTime;
    uint64			    mLastThreadTime;
    uint64			    mNewThreadTime;
    
    uint64			    mThreadTime;
    uint64				lastThreadProcessingTime;
    uint64				threadProcessingTime;

    uint32				reliablePackets;
    uint32				unReliablePackets;
    
	bool				mServerService;
	
    // Anh_Utils::Clock*	mClock;

    SessionQueueLight			mSessionQueue;
	SessionQueueLight			mAsyncSessionQueue;

    boost::thread   			mThread;
    boost::recursive_mutex      mSocketWriteMutex;

    bool						mExit;

	utils::ActiveObject				active_;
};

//======================================================================================================================

#endif //ANH_NETWORKMANAGER_SOCKETWRITETHREAD_H





