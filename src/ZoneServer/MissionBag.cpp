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

#include "MissionBag.h"
#include "MissionObject.h"
#include "PlayerObject.h"
#include "MessageLib/MessageLib.h"

//=============================================================================

MissionBag::MissionBag() : TangibleObject()
{
    mTanGroup = TanGroup_PlayerInternal;
    mTanType = TanType_MissionBag;
}

//=============================================================================

MissionBag::MissionBag(uint64 id,PlayerObject* parent,BString model,BString name,BString file) : TangibleObject(id,parent->getId(),model,TanGroup_PlayerInternal,TanType_MissionBag,name,file),
    mCapacity(14)
{
    mTanGroup = TanGroup_PlayerInternal;
    mTanType = TanType_MissionBag;
    mParent = parent;

    //Load the mission bag with blank missions
    for(int i = 0; i < 10; i++)
    {
        MissionObject* mission = new MissionObject(parent, id);
        mMissions.push_back(mission);
    }

}

//=============================================================================

MissionBag::~MissionBag()
{
    MissionList::iterator it = mMissions.begin();
    while(it != mMissions.end())
    {
        delete(*it);
        it = mMissions.erase(it);
    }
}

//=============================================================================

/*
 * This function creates a new mission, adds it to this bag
 * and then spawns it client side.
*/
void MissionBag::spawnNAdd()
{
    if(mCapacity <= 0)
    {
    }

    //create a new mission
    MissionObject* mission = new MissionObject(mParent,mId);

    //add it to this bag
    mMissions.push_back(mission);
    mCapacity--;

    //Spawn the mission client side
    gMessageLib->sendCreateObjectByCRC(mission, mParent, false);
    gMessageLib->sendContainmentMessage(mission->getId(), mId, 0xffffffff, mParent);
    gMessageLib->sendBaselinesMISO_3(mission, mParent);
    gMessageLib->sendBaselinesMISO_6(mission, mParent);
    gMessageLib->sendBaselinesMISO_8(mission, mParent);
    gMessageLib->sendBaselinesMISO_9(mission, mParent);
    gMessageLib->sendEndBaselines(mission->getId(), mParent);

    return;
}

//=============================================================================

MissionObject* MissionBag::getMissionById(uint64 id)
{
    MissionList::iterator it = mMissions.begin();

    while(it != mMissions.end())
    {
        if((*it)->getId() == id) return(*it);
        ++it;
    }

    return NULL;
}

//=============================================================================

bool MissionBag::removeMission(MissionObject* mission)
{
    MissionList::iterator it = mMissions.begin();

    while(it != mMissions.end())
    {
        if((*it) == mission)
        {
            mMissions.erase(it);
            mCapacity++;
            return true;
        }

        ++it;
    }

    return false;
}

//=============================================================================

bool MissionBag::removeMission(uint64 id)
{
    MissionList::iterator it = mMissions.begin();

    while(it != mMissions.end())
    {
        if((*it)->getId() == id)
        {
            mMissions.erase(it);
            mCapacity++;
            return true;
        }

        ++it;
    }

    return false;
}

//=============================================================================


