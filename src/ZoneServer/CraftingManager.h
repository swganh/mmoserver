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
#ifndef ANH_ZONESERVER_CRAFTING_MANAGER_H
#define ANH_ZONESERVER_CRAFTING_MANAGER_H

#define 	gCraftingManager	CraftingManager::getSingletonPtr()
//=============================================
class Database;
class Object;
class Message;
class ObjectControllerCmdProperties;
class CraftingStation;
class CraftingTool;
class PlayerObject;

class CraftingManager
{
public:
    static CraftingManager*	getSingletonPtr() {
        return mSingleton;
    }
    static CraftingManager*	Init(Database* database)
    {
        if(mInsFlag == false)
        {
            mSingleton = new CraftingManager(database);
            mInsFlag = true;
            return mSingleton;
        }
        else
            return mSingleton;
    }

    CraftingManager(Database* database);
    ~CraftingManager(void);

    //
    // request draftslots batch
    //
    bool				HandleRequestDraftslotsBatch(Object* object,Object* target,Message* message,ObjectControllerCmdProperties* cmdProperties);
    bool				HandleRequestResourceWeightsBatch(Object* object,Object* target,Message* message,ObjectControllerCmdProperties* cmdProperties);
    bool				HandleSynchronizedUIListen(Object* object,Object* target,Message* message,ObjectControllerCmdProperties* cmdProperties);
    bool				HandleRequestCraftingSession(Object* object,Object* target,Message* message,ObjectControllerCmdProperties* cmdProperties);
    bool				HandleSelectDraftSchematic(Object* object,Object* target,Message* message,ObjectControllerCmdProperties* cmdProperties);
    bool				HandleCancelCraftingSession(Object* object,Object* target,Message* message,ObjectControllerCmdProperties* cmdProperties);
    void				handleCraftFillSlot(Object* object, Message* message);
    void				handleCraftEmptySlot(Object* object,Message* message);
    void				handleCraftExperiment(Object* object,Message* message);
    void				handleCraftCustomization(Object* object,Message* message);
    bool				HandleNextCraftingStage(Object* object,Object* target,Message* message,ObjectControllerCmdProperties* cmdProperties);
    bool				HandleCreatePrototype(Object* object,Object* target,Message* message,ObjectControllerCmdProperties* cmdProperties);
    bool				HandleCreateManufactureSchematic(Object* object,Object* target,Message* message,ObjectControllerCmdProperties* cmdProperties);



private:
    CraftingTool*			getCraftingStationTool(PlayerObject* player, CraftingStation* station);

    static CraftingManager*	mSingleton;
    static bool				mInsFlag;

    Database*				mDatabase;
};

#endif