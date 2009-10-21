/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

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

		SocketWriteThread(void);

		void			Startup(SOCKET socket, Service* service, bool serverservice);
		void			Shutdown(void);
		virtual void	run();

		void			NewSession(Session* session);

		bool			getIsRunning(void){ return mIsRunning; }
		void			requestExit(){ mExit = true; }

	private:

		void			_startup(void);
		void			_shutdown(void);

		void			_sendPacket(Packet* packet, Session* session);

		uint16				mMessageMaxSize;
		int8				mSendBuffer[SEND_BUFFER_SIZE];  
		Service*			mService;
		CompCryptor*		mCompCryptor;
		SOCKET				mSocket;
		bool				mIsRunning;
		uint64			    lasttime;
		uint32				unCount;
		uint32				reCount;
		bool				mServerService;
		// Anh_Utils::Clock*	mClock;

		SessionQueue		mSessionQueue;

		boost::mutex                  mSocketReadMutex;
        boost::thread   	mThread;
		bool				mExit;
};

//======================================================================================================================

#endif //ANH_NETWORKMANAGER_SOCKETWRITETHREAD_H





