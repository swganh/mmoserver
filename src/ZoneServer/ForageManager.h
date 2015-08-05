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

#include <vector>
#include "Utils/typedefs.h"

#define gForageManager ForageManager::getSingletonPtr()

class Database;
class Message;
class MessageDispatch;
class PlayerObject;
class ObjectControllerCommandMap;

enum forageClasses
{
    ForageClass_Scout,
    ForageClass_Medic
};

enum forageFails
{
    NOT_OUTSIDE,
    PLAYER_MOVED,
    ACTION_LOW,
    IN_COMBAT,
    AREA_EMPTY,
    ENTERED_COMBAT,
    NO_SKILL,
    ALREADY_FORAGING
};

class ForageAttempt;
class ForagePocket;

#define gForageManager	ForageManager::getSingletonPtr()

class ForageManager
{
public:
    static ForageManager*	getSingletonPtr() {
        return mSingleton;
    }

    static ForageManager*	Instance()
    {
        if (!mSingleton)
        {
            mSingleton = new ForageManager();

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

    //foraging
    void forageUpdate();
    void startForage(PlayerObject* player, forageClasses);
    static void failForage(PlayerObject* player, forageFails fail);
    static void successForage(PlayerObject* player, forageClasses forageClass);

protected:
    ForageManager();
    ~ForageManager(void);

private:
    static ForageManager*	mSingleton;
    
    ForagePocket*			pHead;

};
