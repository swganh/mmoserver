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

#ifndef ANH_ZONESERVER_ADMIN_MANAGER_H
#define ANH_ZONESERVER_ADMIN_MANAGER_H

#include <map>
#include "Utils/bstring.h"
#include "Utils/typedefs.h"
//=============================================================================

class AdminRequestObject;
class MessageDispatch;
class DispatchClient;
class Message;

typedef std::map<uint64, AdminRequestObject*> AdminRequests;

enum AdminRequestt
{
    AdminScheduledShutdown		= 0,
    AdminEmergenyShutdown		= 1
};

//=============================================================================

class AdminManager
{
public:

    static AdminManager* Instance(void);
    static AdminManager* Init(MessageDispatch* messageDispatch);

    static inline void deleteManager(void)
    {
        if (mInstance)
        {
            delete mInstance;
            mInstance = nullptr;
        }
    }

    void registerCallbacks(void);
    void unregisterCallbacks(void);
    void _processScheduleShutdown(Message* message, DispatchClient* client);
    void _processCancelScheduledShutdown(Message* message, DispatchClient* client);

    uint64 handleAdminRequest(uint64 requestId, uint64 timeOverdue);
    void addAdminRequest(uint64 type, BString message, int32 ttl);
    void cancelAdminRequest(uint64 type, BString message);

    bool shutdownPending(void) {
        return mPendingShutdown;
    }
    bool shutdownZone(void) {
        return mTerminateServer;
    }

protected:
    // AdminManager(AdminRequestObject* adminObject);
    // AdminManager();
    AdminManager(MessageDispatch* messageDispatch);
    ~AdminManager();

private:
    // This constructor prevents the default constructor to be used, since it is private.
    AdminManager();

    static AdminManager* mInstance;
    AdminRequests mAdminRequests;
    MessageDispatch* mMessageDispatch;
    bool	mPendingShutdown;
    bool	mTerminateServer;
};

//=============================================================================



#endif

