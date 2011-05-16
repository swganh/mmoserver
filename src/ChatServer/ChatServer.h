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

#ifndef ANH_CHATSERVER_H
#define ANH_CHATSERVER_H

#include "Utils/bstring.h"
#include "Utils/typedefs.h"
#include "Common/Server.h"

//======================================================================================================================

class CharacterAdminHandler;
class ChatManager;
class CSRManager;
class Database;
class DatabaseManager;
class DispatchClient;
class GroupManager;
class MessageDispatch;
class NetworkManager;
class PlanetMapHandler;
class Service;
class TradeManagerChatHandler;
class StructureManagerChatHandler;

//======================================================================================================================

class ProcessAddress
{
public:

    uint32		mType;
    BString        mAddress;
    uint16      mPort;
    uint32      mStatus;
    uint32      mActive;
};

//======================================================================================================================
class ChatServer : public common::BaseServer
{
public:

    ChatServer(int argc, char* argv[]);
    ~ChatServer();

    void    Process();

private:

    void    _updateDBServerList(uint32 status);
    void    _connectToConnectionServer();

    NetworkManager*				  mNetworkManager;
    DatabaseManager*              mDatabaseManager;

    Service*                      mRouterService;
    Database*                     mDatabase;

    MessageDispatch*              mMessageDispatch;

    CharacterAdminHandler*        mCharacterAdminHandler;
    PlanetMapHandler*			  mPlanetMapHandler;
    TradeManagerChatHandler*	  mTradeManagerChatHandler;
    StructureManagerChatHandler*  mStructureManagerChatHandler;
    ChatManager*				  mChatManager;
    GroupManager*				  mGroupManager;
    CSRManager*					  mCSRManager;

    DispatchClient*				  mClient;
    uint32					      mLastHeartbeat;

};

#endif



