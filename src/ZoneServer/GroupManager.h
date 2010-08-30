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

#ifndef ANH_ZONESERVER_GroupManager_H
#define ANH_ZONESERVER_GroupManager_H

#include <list>
#include <map>
#include <vector>

#include "DatabaseManager/DatabaseCallback.h"
#include "GroupManagerCallback.h"
#include "Utils/bstring.h"
#include "Utils/typedefs.h"

//======================================================================================================================

#define	gGroupManager	GroupManager::getSingletonPtr()

//======================================================================================================================

class Database;
class DispatchClient;
class GroupObject;
class Message;
class MessageDispatch;
class MissionObject;
class PlayerObject;
class GroupManagerCallbackContainer;

typedef std::vector<std::pair<MissionObject*,uint32> >	MissionGroupRangeList;
typedef std::vector<GroupObject*>						GroupList;
//======================================================================================================================

class GroupManager : public DatabaseCallback
{
public:

    static GroupManager*	getSingletonPtr() {
        return mSingleton;
    }
    static GroupManager*	Init(Database* database,MessageDispatch* dispatch);

    GroupManager(Database* database,MessageDispatch* dispatch);
    ~GroupManager();

    static inline void deleteManager(void)
    {
        if (mSingleton)
        {
            delete mSingleton;
            mSingleton = 0;
        }
    }

    void				Shutdown();

    void				sendGroupMissionUpdate(GroupObject* group);
    MissionObject*		getZoneGroupMission(std::list<uint64>* members);

    GroupObject*		getGroupObject(uint64 id);

    void				getGroupLeader(PlayerObject* requester, uint64 groupId, uint32 operation, GroupManagerCallback* callback);
    void				getGroupLeader(PlayerObject* requester, uint64 groupId, uint32 operation, GroupManagerCallback* callback, BString arg);
    void				getGroupLeader(PlayerObject* requester, uint64 groupId, uint32 operation, GroupManagerCallback* callback, uint32 flourishId);

    void				addGroupObject(GroupObject* group) {
        mGroupList.push_back(group);
    }
    void				deleteGroupObject(uint64 id);
    GroupList*			getGroupList() {
        return &mGroupList;
    }

private:

    void				_processIsmInviteRequest(Message* message, DispatchClient* client);
    void				_processIsmGroupCREO6deltaGroupId(Message* message, DispatchClient* client);
    void				_processIsmGroupLootModeResponse(Message* message, DispatchClient* client);
    void				_processIsmGroupLootMasterResponse(Message* message, DispatchClient* client);
    void				_processIsmGroupInviteInRangeRequest(Message* message, DispatchClient* client);
    void				_processIsmIsGroupLeaderResponse(Message* message, DispatchClient* client);

    uint64				_insertLeaderRequest(GroupManagerCallbackContainer* container);

    static GroupManager*	mSingleton;
    static bool				mInsFlag;

    Database*				mDatabase;
    MessageDispatch*		mMessageDispatch;
    GroupList				mGroupList;

    std::map<uint64, GroupManagerCallbackContainer*> mLeaderRequests;
    uint64					mLeaderRequestInc;

};

#endif
