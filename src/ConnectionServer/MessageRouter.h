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

    void	setClientManager(ClientManager* manager) {
        mClientManager = manager;
    }
    void	setServerManager(ServerManager* manager) {
        mServerManager = manager;
    }

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



