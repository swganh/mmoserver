
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
