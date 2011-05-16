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

#pragma once

#include "TangibleObject.h"

#define gFireworkManager FireworkManager::Instance()

class PlayerObject;
class FireworkEvent;

class FireworkManager
{
public:
    static FireworkManager*	getSingletonPtr() {
        return mSingleton;
    }

    static FireworkManager*	Instance()
    {
        if (!mSingleton)
        {
            mSingleton = new FireworkManager();
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

    void Process();

    //bool createFirework(uint32 typeId, PlayerObject* player, bool isShow=false);
    TangibleObject* createFirework(uint32 typeId, PlayerObject* player, const glm::vec3& position);


protected:
    ~FireworkManager(void);

private:
    static FireworkManager*	mSingleton;

    std::list<FireworkEvent*> fireworkEvents;

    FireworkManager() {}

};
