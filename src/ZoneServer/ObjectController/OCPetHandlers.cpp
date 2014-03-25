/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2014 The SWG:ANH Team
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

//common includes

#include "Zoneserver/Objects/Item.h"
#include "Zoneserver/ObjectController/ObjectController.h"
#include "ZoneServer/ObjectController/ObjectControllerOpcodes.h"
#include "ZoneServer/ObjectController/ObjectControllerCommandMap.h"
#include "ZoneServer/Objects/Player Object/PlayerObject.h"
#include "ZoneServer/GameSystemManagers/UI Manager/UIManager.h"
#include "ZoneServer/Objects/VehicleController.h"
#include "Zoneserver/Objects/VehicleControllerFactory.h"
#include "ZoneServer/WorldConfig.h"
#include "ZoneServer/WorldManager.h"
#include "MessageLib/MessageLib.h"

//#include "DatabaseManager/Database.h"
//#include "DatabaseManager/DatabaseResult.h"
//#include "DatabaseManager/DataBinding.h"

#include "NetworkManager/Message.h"
#include "NetworkManager/MessageFactory.h"

void ObjectController::_handleMount(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    // The very idea with using ID's instead of object refs are that you can TEST them without using the object itself.
    // And some parameter validation...
    if (targetId == 0)
    {
        DLOG(info) << "ObjectController::_handleMount : Cannot find vehicle ID :(";
        return;
    }
	CreatureObject* player_body = dynamic_cast<CreatureObject*>(mObject);
	PlayerObject* player	= player_body->GetGhost();

	if(!player)	{

		DLOG(info) << "ObjectController::_handleMount : Cannot find player :(" << mObject->getId();
		return;
	}

    if(player_body->getParentId() != 0)	{

		gMessageLib->SendSystemMessage(L"You need to be outside to mount.", player);
		return;
	}
	
	if (!player->getMount() )	{
		DLOG(info) << "ObjectController::_handleMount : Cannot find mount :(" << mObject->getId();
		return;
	}

    // Do we have a valid target?
    if (player->checkIfMounted())        {
		
		gMessageLib->SendSystemMessage(L"You cannot mount this because you are already mounted.", player);
		return;
	}

    // verify its player's mount
    MountObject* pet	= dynamic_cast<MountObject*>(gWorldManager->getObjectById(targetId));
    if (pet && (pet->owner() == player->getId()))    {

        // get the mount Vehicle object by the id (Creature object id - 1 )

        if(VehicleController* vehicle = dynamic_cast<VehicleController*>(gWorldManager->getObjectById(pet->controller())))
        {
            //The /mount command can work up to 32m on live
            if(glm::distance(vehicle->body()->mPosition, player_body->mPosition) <= 32)	{
                //change locomotion
                vehicle->MountPlayer();
            }	else {
                gMessageLib->SendSystemMessage(L"Your target is too far away to mount.", player);
            }
        }
        else
        {
            DLOG(info) << "ObjectController::_handleMount : Cannot find vehicle";
        }
    }
     
}

//===============================================================================================

void ObjectController::_handleDismount(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    // The very idea with using ID's instead of object refs are that you can TEST them without using the object itself.
    // And some parameter validation...

    CreatureObject* creature  = dynamic_cast<CreatureObject*>(mObject); 
	PlayerObject* player = creature->GetGhost();

    if (player && player->getMount())
    {
        if (player->checkIfMounted())
        {
            // verify its player's mount
            MountObject* pet = player->getMount();
            
            if (pet && (pet->owner() == player->getId()))
            {
                // get the pets controller for a swoop its the vehicle
                if(VehicleController* vehicle = dynamic_cast<VehicleController*>(gWorldManager->getObjectById(pet->controller())))
                {
                    vehicle->DismountPlayer();
                }
            }
        }
        else
        {
            gMessageLib->SendSystemMessage(L"You are not mounted to perform this action.", player);
        }
    }
}

//===============================================================================================
