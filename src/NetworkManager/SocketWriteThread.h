/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_NETWORKMANAGER_SOCKETWRITETHREAD_H
#define ANH_NETWORKMANAGER_SOCKETWRITETHREAD_H

#include "zthread/ZThread.h"
#include "Utils/typedefs.h"
#include "Utils/Clock.h"
#include "Utils/concurrent_queue.h"

#define SEND_BUFFER_SIZE 8192

//======================================================================================================================

class Service;
class Packet;
class Session;
class CompCryptor;

typedef unsigned int SOCKET;
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

		ZThread::Thread*	mThread;
		bool				mExit;
};


//======================================================================================================================

class SocketWriteThreadRunnable : public ZThread::Runnable
{
	public:

		SocketWriteThreadRunnable(SocketWriteThread* r){ mWriteThread = r; }
		~SocketWriteThreadRunnable(){}

		virtual void run(){ mWriteThread->run(); }

		SocketWriteThread* mWriteThread;
};

//======================================================================================================================

#endif //ANH_NETWORKMANAGER_SOCKETWRITETHREAD_H





