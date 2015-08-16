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

    ClientManager(Service* service, Database* database, MessageRouter* router, ConnectionDispatch* connectionDispatch, uint32_t cluster_id);
    ~ClientManager(void);

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
    void                        _processAllowedChars(DatabaseCallback* callback,ConnectionClient* client);

    Service*                    mClientService;
    Database*                   mDatabase;
    MessageRouter*              mMessageRouter;
    ConnectionDispatch*         mConnectionDispatch;

    boost::recursive_mutex		mServiceMutex;
    PlayerClientMap             mPlayerClientMap;

	uint32_t					mClusterId;
};

//======================================================================================================================

#endif //MMOSERVER_CONNECTIONSERVER_CLIENTMANAGER_H


