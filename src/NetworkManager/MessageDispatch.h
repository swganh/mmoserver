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

#ifndef ANH_COMMON_MESSAGEDISPATCH_H
#define ANH_COMMON_MESSAGEDISPATCH_H

#include "NetworkManager/NetworkCallback.h"
#include "Utils/typedefs.h"

#include <boost/thread/recursive_mutex.hpp>
#include <map>
#include <functional>

//======================================================================================================================

class Service;
class DispatchClient;
class Message;

typedef std::map<uint32, std::function<void (Message*,DispatchClient*)>>   MessageCallbackMap;
typedef std::map<uint32, DispatchClient*>            AccountClientMap;


// This should be defined as a pre-set in DB account table.
// Maybe reserv the first 10 or 100 id's for internal usage?
// TODO: Find a better place to store stuff like this...
#define AdminAccountId ((uint32)(3000))

//======================================================================================================================

class MessageDispatch : public NetworkCallback
{
public:

    MessageDispatch(Service* service);
    ~MessageDispatch(void);

    void						Process(void);

    void						RegisterMessageCallback(uint32 opcode, std::function<void (Message*,DispatchClient*)> callback);
    void						UnregisterMessageCallback(uint32 opcode);
    AccountClientMap*			getClientMap() {
        return(&mAccountClientMap);
    }

    // Inherited NetworkCallback
    virtual NetworkClient*		handleSessionConnect(Session* session, Service* service);
    virtual void				handleSessionDisconnect(NetworkClient* client);
    virtual void				handleSessionMessage(NetworkClient* client, Message* message);

    // Sessionless clients
    void						registerSessionlessDispatchClient(uint32 accountId);
    void						unregisterSessionlessDispatchClient(uint32 accountId);
private:

    Service*					mRouterService;

    MessageCallbackMap			mMessageCallbackMap;
    AccountClientMap			mAccountClientMap;
    boost::recursive_mutex		mSessionMutex;
};

//======================================================================================================================

#endif //MMOSERVER_COMMON_MESSAGEDISPATCH_H




