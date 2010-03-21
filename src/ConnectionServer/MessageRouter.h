/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_CONNECTIONSERVER_MESSAGEROUTER_H
#define ANH_CONNECTIONSERVER_MESSAGEROUTER_H

#include "Utils/typedefs.h"
#include <map>


//======================================================================================================================

class Service;
class ClientManager;
class ServerManager;
class ConnectionClient;
class Database;
class ConnectionDispatch;
class Message;

typedef std::map<uint32,uint32>   MessageRouteMap;

//======================================================================================================================

class MessageRoute
{
	public:

		uint32	mMessageId;
		uint32	mProcessId;
};

//======================================================================================================================

class MessageRouter
{
	public:

		MessageRouter(Database* database, ConnectionDispatch* dispatch);
		~MessageRouter(void);

		void	Process(void);

		void	RouteMessage(Message* message, ConnectionClient* client);

		void	setClientManager(ClientManager* manager){ mClientManager = manager; }
		void	setServerManager(ServerManager* manager){ mServerManager = manager; }

	private:

		void	_loadMessageProcessMap(void);

		ConnectionDispatch*	mConnectionDispatch;
		ClientManager*		mClientManager;
		ServerManager*		mServerManager;
		Database*			mDatabase;

		MessageRouteMap		mMessageRouteMap;
};

//======================================================================================================================

#endif //MMOSERVER_CONNECTIONSERVER_MESSAGEROUTER_H



