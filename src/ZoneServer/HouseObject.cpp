
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
#include "ObjectFactory.h"
#include "HouseObject.h"
#include "PlayerObject.h"
#include "Inventory.h"
#include "CellObject.h"

#include "ResourceContainer.h"
#include "StructureManager.h"
#include "UIManager.h"

#include "MessageLib/MessageLib.h"

#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"

#include <cassert>

//=============================================================================

HouseObject::HouseObject() : BuildingObject()
{
    mType = ObjType_Building;

    setWidth(64);
    setHeight(64);

}

//=============================================================================

HouseObject::~HouseObject()
{

}



void HouseObject::checkCellPermission(PlayerObject* player)
{


    if(this->getPublic())
    {
        //structure is public - are we banned ?
        StructureAsyncCommand command;
        command.Command = Structure_Command_CellEnterDenial;
        command.PlayerId = player->getId();
        command.StructureId = this->getId();

        gStructureManager->checkNameOnPermissionList(this->getId(),player->getId(),player->getFirstName().getAnsi(),"BAN",command);

    }
    else
    {
        //structure is private - do we have access ?
        StructureAsyncCommand command;
        command.Command = Structure_Command_CellEnter;
        command.PlayerId = player->getId();
        command.StructureId = this->getId();

        gStructureManager->checkNameOnPermissionList(this->getId(),player->getId(),player->getFirstName().getAnsi(),"ENTRY",command);

    }
}


//========================================================================0
//
//

void HouseObject::handleObjectReady(Object* object,DispatchClient* client, uint64 hopper)
{
    Item* item = dynamic_cast<Item*>(gWorldManager->getObjectById(hopper));
    if(!item)
    {
        assert(false && "HouseObject::handleObjectReady could not find hopper");
    }
}



//=============================================================================
//handles the radial selection

void HouseObject::handleObjectMenuSelect(uint8 messageType,Object* srcObject)
{
    PlayerObject* player = dynamic_cast<PlayerObject*>(srcObject);
    if(!player)
    {
        return;
    }

}

//=============================================================================
// ´not needed - this is handled over the structures terminal
void HouseObject::prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount)
{

}



void HouseObject::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
    StructureManagerAsyncContainer* asynContainer = (StructureManagerAsyncContainer*)ref;

//	switch(asynContainer->mQueryType)
//	{


//		default:break;

//	}

    SAFE_DELETE(asynContainer);
}


bool HouseObject::hasAdmin(uint64 id)
{
    ObjectIDList		adminList =	getHousingList();

    ObjectIDList::iterator it =	 adminList.begin();

    while (it != adminList.end())
    {
        if( id == (*it))
            return true;

        it++;
    }
    return false;
}




