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

//common includes

#include "Item.h"
#include "ObjectController.h"
#include "ObjectControllerOpcodes.h"
#include "ObjectControllerCommandMap.h"
#include "PlayerObject.h"
#include "UIManager.h"
#include "VehicleController.h"
#include "VehicleControllerFactory.h"
#include "WorldConfig.h"
#include "WorldManager.h"
#include "MessageLib/MessageLib.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include "NetworkManager/Message.h"
#include "NetworkManager/MessageFactory.h"

void ObjectController::_handleMount(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    // The very idea with using ID's instead of object refs are that you can TEST them without using the object itself.
    // And some parameter validation...
    if (targetId == 0)
    {
        DLOG(INFO) << "ObjectController::_handleMount : Cannot find vehicle ID :(";
        return;
    }

    PlayerObject* player	= dynamic_cast<PlayerObject*>(mObject);

    if (player && player->getMount() && (player->getParentId() == 0))
    {
        // Do we have a valid target?
        if (!player->checkIfMounted())
        {
            // verify its player's mount
            MountObject* pet	= dynamic_cast<MountObject*>(gWorldManager->getObjectById(targetId));
            if (pet && (pet->owner() == player->getId()))
            {
                // get the mount Vehicle object by the id (Creature object id - 1 )

                if(VehicleController* vehicle = dynamic_cast<VehicleController*>(gWorldManager->getObjectById(pet->controller())))
                {
                    //The /mount command can work up to 32m on live
                    if(glm::distance(vehicle->body()->mPosition, player->mPosition) <= 32)	{
                        //change locomotion
                        vehicle->MountPlayer();
                    }	else {
                        gMessageLib->SendSystemMessage(L"Your target is too far away to mount.", player);
                    }
                }
                else
                {
                    DLOG(INFO) << "ObjectController::_handleMount : Cannot find vehicle";
                }
            }
        } else {
            gMessageLib->SendSystemMessage(L"You cannot mount this because you are already mounted.", player);
        }
    }
}

//===============================================================================================

void ObjectController::_handleDismount(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    // The very idea with using ID's instead of object refs are that you can TEST them without using the object itself.
    // And some parameter validation...

    PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);

    if (player && player->getMount() && (player->getParentId() == 0))
    {
        if (player->checkIfMounted())
        {
            // verify its player's mount
            MountObject* pet = NULL;
            if (targetId == 0)
            {
                // No object targeted, assume the one we are riding.	- what else should we dismount ???
                pet	= player->getMount();
            }
            else
            {
                pet = dynamic_cast<MountObject*>(gWorldManager->getObjectById(targetId));
            }

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
