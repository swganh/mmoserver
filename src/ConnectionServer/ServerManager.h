#ifndef ANH_CONNECTIONSERVER_SERVERMANAGER_H
#define ANH_CONNECTIONSERVER_SERVERMANAGER_H

#include "ConnectionDispatchCallback.h"
#include "NetworkManager/NetworkCallback.h"
#include "DatabaseManager/DatabaseCallback.h"
#include "Utils/typedefs.h"


//======================================================================================================================

class ClientManager;
class MessageRouter;
class IService;
class Database;
class DataBinding;
class ConnectionDispatch;

//======================================================================================================================

class ServerAddress
{
	public:

	  uint32                          mId;
	  int8                            mAddress[16];
	  uint16                          mPort;
	  uint32                          mStatus;
	  uint32                          mActive;
	  ConnectionClient*               mConnectionClient;
};

//======================================================================================================================

class ServerManager : public NetworkCallback, public ConnectionDispatchCallback, public DatabaseCallback
{
	public:

		ServerManager(IService* service, Database* database, MessageRouter* router, ConnectionDispatch* dispatch,ClientManager* clientManager);
		~ServerManager(void);

		void                            Process(void);

		void                            SendMessageToServer(Message* message);

		// Inherited NetworkCallback
		virtual NetworkClient*	        handleSessionConnect(Session* session, IService* service);
		virtual void          	        handleSessionDisconnect(NetworkClient* client);
		virtual void					handleSessionMessage(NetworkClient* client, Message* message);

		// Inherited ConnectionDispatchCallback
		virtual void                    handleDispatchMessage(uint32 opcode, Message* message, ConnectionClient* client);

		// Inherited DatabaseCallback
		virtual void                    handleDatabaseJobComplete(void* ref, DatabaseResult* result);

	private:

		void							_setupDataBindings();
		void							_destroyDataBindings();
		void                            _loadProcessAddressMap(void);
		void                            _processClusterRegisterServer(ConnectionClient* client, Message* message);
		void                            _processClusterZoneTransferRequestByTicket(ConnectionClient* client, Message* message);
		void                            _processClusterZoneTransferRequestByPosition(ConnectionClient* client, Message* message);

		MessageRouter*                  mMessageRouter;
		IService*                        mServerService;
		Database*                       mDatabase;
		ConnectionDispatch*             mConnectionDispatch;
		ClientManager*					mClientManager;

		uint32                          mClusterId;

		uint32                          mTotalActiveServers;
		uint32                          mTotalConnectedServers;
		ServerAddress                   mServerAddressMap[256];   // 256 max server ids, should be enough
		DataBinding*					mServerBinding;
};

//======================================================================================================================

#endif //ANH_CONNECTIONSERVER_SERVERMANAGER_H



