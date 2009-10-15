/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_CONNECTIONSERVER_CLIENTMANAGER_H
#define ANH_CONNECTIONSERVER_CLIENTMANAGER_H

#include "ConnectionDispatchCallback.h"
#include "NetworkManager/NetworkCallback.h"
#include "DatabaseManager/DatabaseCallback.h"

#include <boost/thread/recursive_mutex.hpp>
#include <map>


//======================================================================================================================

class NetworkClient;
class ConnectionClient;
class MessageRouter;
class Message;
class ConnectionDispatch;
class Service;
class Session;
class Database;

typedef std::map<uint32,ConnectionClient*>    PlayerClientMap;

//======================================================================================================================

class ClientManager : public NetworkCallback, public ConnectionDispatchCallback, public DatabaseCallback
{
	public:

		ClientManager(void);
		~ClientManager(void);

		void                        Startup(Service* service, Database* database, MessageRouter* router, ConnectionDispatch* connectionDispatch);
		void                        Shutdown(void);
		void                        Process(void);

		void                        SendMessageToClient(Message* message);

		// Inherited NetworkCallback
		virtual NetworkClient*	    handleSessionConnect(Session* session, Service* service);
		virtual void          	    handleSessionDisconnect(NetworkClient* client);
		virtual void				handleSessionMessage(NetworkClient* client, Message* message);

		// Inherited ConnectionDispatchCallback
		virtual void                handleDispatchMessage(uint32 opcode, Message* message, ConnectionClient* client);

		// Inherited DatabaseCallback
		virtual void                handleDatabaseJobComplete(void* ref, DatabaseResult* result);

		// handle server down
		void						handleServerDown(uint32 serverId);

		private:
		void						_processClientIdMsg(ConnectionClient* client, Message* message);
		void                        _processSelectCharacter(ConnectionClient* client, Message* message);
		void                        _processClusterZoneTransferCharacter(ConnectionClient* client, Message* message);

		void                        _handleQueryAuth(ConnectionClient* client, DatabaseResult* result);


		Service*                    mClientService;
		Database*                   mDatabase;
		MessageRouter*              mMessageRouter;
		ConnectionDispatch*         mConnectionDispatch;

        boost::recursive_mutex		mServiceMutex;
		PlayerClientMap             mPlayerClientMap;
};

//======================================================================================================================

#endif //MMOSERVER_CONNECTIONSERVER_CLIENTMANAGER_H


