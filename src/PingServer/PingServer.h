/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_PING_SERVER_H
#define ANH_PING_SERVER_H

#include "Utils/typedefs.h"
#include "AsynchPingSocket.h"

class	ASynchPingSocket;

class PingServer
{
	public:

		PingServer();
		~PingServer();

		void	start();
		void	shutdown();
		void	process();

	private:

		ASynchPingSocket*					mAsynchPingThread;
		struct sockaddr_in listen_addr, inc_addr;

		SOCKET mSocket;
		fd_set mReadable;
		struct timeval mTimeOut;
};

#endif

