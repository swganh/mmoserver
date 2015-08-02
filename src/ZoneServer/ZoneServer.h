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

#ifndef ANH_ZONESERVER_ZONESERVER_H
#define ANH_ZONESERVER_ZONESERVER_H

#include <memory>
#include "Utils/bstring.h"
#include "Utils/typedefs.h"
#include "Common/Server.h"

//======================================================================================================================

namespace anh {
namespace event_dispatcher {
    class IEventDispatcher;
}}  // namespace anh::event_dispatcher

class NetworkManager;
class Service;
class DatabaseManager;
class Database;

class MessageDispatch;
class CharacterLoginHandler;
class ObjectControllerDispatch;

// @note: for initial testing the new "Service" classes will be manually set up here
// in the future we should have a map container of string to services that they are
// stored in to make it easier to dynamically register services.
namespace zone {
class HamService;
}

//======================================================================================================================

class ProcessAddress
{
public:

    uint32	mType;
    BString		mAddress;
    uint16	mPort;
    uint32	mStatus;
    uint32	mActive;
};

//======================================================================================================================

class ZoneServer : public common::BaseServer
{
public:

    ZoneServer(int argc, char* argv[]);
    ~ZoneServer(void);

    void	Process(void);

    void	handleWMReady();

    std::string  getZoneName()  {
        return mZoneName;
    }

private:
    // Disable compiler generated methods.
    ZoneServer();
    ZoneServer(const ZoneServer&);
    const ZoneServer& operator=(const ZoneServer&);

    void	_updateDBServerList(uint32 status);
    void	_connectToConnectionServer(void);

    std::string                   mZoneName;
    uint32						  mLastHeartbeat;

    std::shared_ptr<anh::event_dispatcher::IEventDispatcher> event_dispatcher_;
    NetworkManager*               mNetworkManager;
    DatabaseManager*              mDatabaseManager;

    Service*                      mRouterService;
    Database*                     mDatabase;

    MessageDispatch*              mMessageDispatch;
    CharacterLoginHandler*        mCharacterLoginHandler;
    ObjectControllerDispatch*     mObjectControllerDispatch;

    std::unique_ptr<zone::HamService>   ham_service_;
};

//======================================================================================================================

#endif  // ANH_ZONESERVER_ZONESERVER_H






