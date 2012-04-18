
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

#pragma once

#include "AttackableCreature.h"
#include "Utils/typedefs.h"

#define gScoutManager ScoutManager::getSingletonPtr()

class Database;
class Message;
class MessageDispatch;
class PlayerObject;
class ObjectControllerCommandMap;

#define gScoutManager	ScoutManager::getSingletonPtr()

enum HarvestSelection
{
    HARVEST_ANY = 0,
    HARVEST_MEAT,
    HARVEST_HIDE,
    HARVEST_BONE
};


class ScoutManager
{
public:
    static ScoutManager*	getSingletonPtr() {
        return mSingleton;
    }

    static ScoutManager*	Instance()
    {
        if (!mSingleton)
        {
            mSingleton = new ScoutManager();

        }
        return mSingleton;
    }

    static inline void deleteManager(void)
    {
        if (mSingleton)
        {
            delete mSingleton;
            mSingleton = 0;
        }
    }

    //camps
    bool createCamp(uint32 typeId,uint64 parentId, const glm::vec3& position, const BString& customName, PlayerObject* player);

    //foraging
    static void successForage(PlayerObject* player);

    //harvesting
    void handleHarvestCorpse(PlayerObject* player, CreatureObject* target, HarvestSelection harvest);
    uint32 getHarvestSkillFactor(CreatureObject* object);
    uint32 getCreatureFactor(CreatureObject* object);

protected:
    ScoutManager();
    ~ScoutManager(void);

private:
    static ScoutManager*	mSingleton;

};
