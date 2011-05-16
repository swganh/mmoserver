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

#ifndef ANH_CONNECTIONSERVER_SERVERMANAGER_H
#define ANH_CONNECTIONSERVER_SERVERMANAGER_H

#include "ConnectionDispatchCallback.h"
#include "NetworkManager/NetworkCallback.h"
#include "DatabaseManager/DatabaseCallback.h"
#include "Utils/typedefs.h"


//======================================================================================================================

class ClientManager;
class MessageRouter;
class Service;
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

    ServerManager(Service* service, Database* database, MessageRouter* router, ConnectionDispatch* dispatch,ClientManager* clientManager, uint32 cluster_id);
    ~ServerManager(void);

    void                            Process(void);

    void                            SendMessageToServer(Message* message);
    uint32							getConnectedServers() {
        return mTotalConnectedServers;
    };
    uint32							getActiveServers() {
        return mTotalActiveServers;
    };

    // Inherited NetworkCallback
    virtual NetworkClient*	        handleSessionConnect(Session* session, Service* service);
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
    void							_processClusterZoneTutorialTerminal(ConnectionClient* client, Message* message);

    MessageRouter*                  mMessageRouter;
    Service*                        mServerService;
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



