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

#ifndef ANH_NETWORKMANAGER_SOCKETWRITETHREAD_H
#define ANH_NETWORKMANAGER_SOCKETWRITETHREAD_H

#include "Utils/typedefs.h"
#include "Utils/clock.h"
#include "Utils/concurrent_queue.h"

#include <boost/thread/thread.hpp>

#define SEND_BUFFER_SIZE 8192

//======================================================================================================================

class Service;
class Packet;
class Session;
class CompCryptor;

typedef Anh_Utils::concurrent_queue<Session*>    SessionQueue;

//======================================================================================================================

class SocketWriteThread
{
	public:

		SocketWriteThread(SOCKET socket, Service* service, bool serverservice);
		~SocketWriteThread();

		virtual void	run();

		void			NewSession(Session* session);

		bool			getIsRunning(void){ return mIsRunning; }
		void			requestExit(){ mExit = true; }

	private:

		void			_startup(void);
		void			_shutdown(void);

		void			_sendPacket(Packet* packet, Session* session);

		//void				*mtheHandle;

		uint16				mMessageMaxSize;
		int8				mSendBuffer[SEND_BUFFER_SIZE];  
		Service*			mService;
		CompCryptor*		mCompCryptor;
		SOCKET				mSocket;
		bool				mIsRunning;
		uint64			    mLastTime;
		uint64			    mLastThreadTime;
		uint64			    mNewThreadTime;
		uint32				mCpuUsage;
		uint64			    mThreadTime;
		uint64				lastThreadProcessingTime;
		uint64				threadProcessingTime;

		uint32				unCount;
		uint32				reCount;
		bool				mServerService;
		// Anh_Utils::Clock*	mClock;

		SessionQueue				mSessionQueue;

        boost::thread   			mThread;
		boost::recursive_mutex      mSocketWriteMutex;
		bool						mExit;
};

//======================================================================================================================

#endif //ANH_NETWORKMANAGER_SOCKETWRITETHREAD_H





