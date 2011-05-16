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

#ifndef ANH_CONNECTIONSERVER_CONNECTIONSERVER_H
#define ANH_CONNECTIONSERVER_CONNECTIONSERVER_H

#include "Utils/typedefs.h"
#include "Common/Server.h"
#include <boost/program_options.hpp>
#include <boost/thread/thread.hpp>

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

class ConnectionServer : public common::BaseServer
{

public:

    ConnectionServer(int argc, char* argv[]);
    ~ConnectionServer(void);

    void	Process(void);
    void    ToggleLock();

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
    bool					mLocked;
    uint64					mLastHeartbeat;
};

//======================================================================================================================

#endif  //MMOSERVER_CONNECTIONSERVER_CONNECTIONSERVER_H





