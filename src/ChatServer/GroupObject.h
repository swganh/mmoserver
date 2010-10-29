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

#ifndef ANH_CHATSERVER_GROUPOBJECT_H
#define ANH_CHATSERVER_GROUPOBJECT_H

#include <vector>

#include "Utils/typedefs.h"

class Channel;
class Player;

//======================================================================================================================

typedef std::vector<Player*>		PlayerList;

//======================================================================================================================

class GroupObject
{

public :

    GroupObject(Player* leader, uint64 groupId);
    ~GroupObject();

    Player*				getLeader() {
        return mMembers[0];
    }
    PlayerList*			getMembers() {
        return &mMembers;
    }
    uint64				getId() {
        return mId;
    }
    uint16				getMemberCount() {
        return mMemberCount;
    }
    uint32				getLootMode() {
        return mLootMode;
    }
    uint64				getMasterLooter() {
        return mMasterLooter;
    }

    uint16				getSize() {
        return mMembers.size();
    }
    Channel*			getChannel() {
        return mChannel;
    }

    void				addTempMember(Player* player);
    void				removeTempMember(Player* player);
    void				addMember(Player* player);
    void				removeMember(Player* player);
    void				changeLeader(Player* player);
    void				changeLootMode(uint32 newLootMode);
    void				changeMasterLooter(Player* newMasterLooter);

    void				disband();

    void				resetIndexes();
    void				sendCreate(Player* player);
    void				createChannel();
    void				sendUpdate(Player* player);			// Used when entering a new location (new scene)

    void				broadcastMessage(Message* message, bool ignoreLeader = false);
    void				broadcastPlayerPositionNotification(Player* player);
    void				broadcastPlayerZoneNotification(Player* player);
    void				sendGroupLocationNotification(Player* player);

    void				sendBaseline(Player* player);
    void				broadcastDeltaResetAll();
    void				broadcastDeltaAdd(Player* player);
    void				broadcastDeltaRemove(Player* player);
    void				broadcastUpdateLootMode();
    void				broadcastUpdateMasterLooter();




private :

    uint64				mId;
    PlayerList			mMembers;
    uint16				mMemberCount;
    uint32				mMembersUpdateCount;
    uint64				mMasterLooter;
    uint32				mLootMode;
    Channel*			mChannel;

};
#endif
//======================================================================================================================
