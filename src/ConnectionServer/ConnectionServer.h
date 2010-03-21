/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_CONNECTIONSERVER_CONNECTIONSERVER_H
#define ANH_CONNECTIONSERVER_CONNECTIONSERVER_H

#include "Utils/typedefs.h"


//======================================================================================================================

class DatabaseManager;
class Database;
class NetworkManager;
class Service;
class MessageRouter;
class ClientManager;
class ServerManager;
class ConnectionDispatch;

//======================================================================================================================

class ConnectionServer
{

	public:

		ConnectionServer(void);
		~ConnectionServer(void);

		void	Process(void);

	private:

		void	_updateDBServerList(uint32 status);

		DatabaseManager*		mDatabaseManager;
		Database*				mDatabase;
		NetworkManager*			mNetworkManager;
		MessageRouter*			mMessageRouter;
		ClientManager*			mClientManager;
		ServerManager*			mServerManager;
		ConnectionDispatch*		mConnectionDispatch;

		uint32					mClusterId;

		Service*				mClientService;
		Service*				mServerService;
};

//======================================================================================================================

#endif  //MMOSERVER_CONNECTIONSERVER_CONNECTIONSERVER_H





