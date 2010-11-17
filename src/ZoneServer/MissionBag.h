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

#ifndef ANH_ZONESERVER_TANGIBLE_MISSIONBAG_H
#define ANH_ZONESERVER_TANGIBLE_MISSIONBAG_H

#include "TangibleObject.h"
#include <vector>

class MissionObject;

typedef std::list<MissionObject*>		MissionList;

//=============================================================================

class MissionBag : public TangibleObject
{
    friend class ObjectFactory;
    friend class PlayerObjectFactory;

public:

    MissionBag();
    MissionBag(uint64 id,PlayerObject* parent,const BString model,const BString name = "",const BString file = "");
    ~MissionBag();

    void		  setParent(PlayerObject* player) {
        mParent = player;
    }
    PlayerObject* getParent() {
        return mParent;
    }

    MissionList*   getMissions() {
        return &mMissions;
    }
    MissionObject* getMissionById(uint64 id);
    void		   addMission(MissionObject* mission) {
        mMissions.push_back(mission);
        mCapacity--;
    }
    void		   spawnNAdd();
    bool		   removeMission(uint64 id);
    bool		   removeMission(MissionObject* mission);

private:

    PlayerObject*	mParent;
    MissionList		mMissions;
    uint8			mCapacity;

};

//=============================================================================

#endif

