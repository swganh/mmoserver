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


#define	gGroupManager GroupManager::getSingletonPtr()

#include <map>

//======================================================================================================================

class Database;
class GroupObject;
class Message;
class MessageDispatch;

//======================================================================================================================

typedef	std::map<uint64,GroupObject*> GroupMap;

//======================================================================================================================

class GroupManager
{
public:
    static GroupManager*	getSingletonPtr() {
        return mSingleton;
    }
    static GroupManager*	Init(MessageDispatch* dispatch);

    GroupManager(MessageDispatch* dispatch);
    ~GroupManager();

    void				Shutdown();

    GroupObject*		getGroupById(uint64 groupId);
    void				removeGroup(uint64 groupId);

    uint64				getNextGroupId() {
        ++mNextGroupId;
        return mNextGroupId;
    }

    void				_processGroupInviteRequest(Message* message, DispatchClient* client);
    void				_processGroupInviteResponse(Message* message, DispatchClient* client);
    void				_processGroupUnInvite(Message* message, DispatchClient* client);
    void				_processGroupDisband(Message* message, DispatchClient* client);
    void				_processGroupLeave(Message* message, DispatchClient* client);
    void				_processGroupDismissGroupMember(Message* message, DispatchClient* client);
    void				_processGroupMakeLeader(Message* message, DispatchClient* client);
    void				_processGroupBaselineRequest(Message* message, DispatchClient* client);
    void				_processGroupPositionNotification(Message* message, DispatchClient* client);
    void				_processGroupLootModeRequest(Message* message, DispatchClient* client);
    void				_processGroupLootModeResponse(Message* message, DispatchClient* client);
    void				_processGroupLootMasterRequest(Message* message, DispatchClient* client);
    void				_processGroupLootMasterResponse(Message* message, DispatchClient* client);
    void				_processIsmInviteInRangeResponse(Message* message, DispatchClient* client);
    void				_processIsmIsGroupLeaderRequest(Message* message, DispatchClient* client);

private:

    static GroupManager*		mSingleton;
    static bool					mInsFlag;
    DispatchClient*				mClient;

    MessageDispatch*			mMessageDispatch;

    GroupMap 					mGroups;
    uint64						mNextGroupId;

    BStringVector				mLootModeMessages;
    BStringVector				mLootModeNotAllowedMessages;

};

#endif

