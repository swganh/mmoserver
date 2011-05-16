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

#include "MountObject.h"

#include "Datapad.h"
#include "PlayerObject.h"
#include "VehicleController.h"

//=============================================================================

MountObject::MountObject()
    : CreatureObject()
    , controller_(0)
    , owner_(0) {}

//=============================================================================

void MountObject::prepareCustomRadialMenu(CreatureObject* creature, uint8_t item_count) {

    PlayerObject*	player	= dynamic_cast<PlayerObject*>(creature);

    if (!player) {
        // Verify the data passed in is what is expected. In debug mode the assert will
        // trigger and crash the server.
        assert(false && "MountObject::handleObjectMenuSelect - Menu preparation requested for a non-player object.");
        return;
    }

    // Reset the radial menu with a new instance.
    mRadialMenu.reset(new RadialMenu());

    // Add the radial options for the custom menu.
    mRadialMenu->addItem(1, 0, radId_examine, radAction_Default);

    // Check to see if the player requesting the menu is the owner of the mount.
    if(player->getId() == owner_)	{

        // Check to see if the player is mounted or not and display the appropriate exit/enter option.
        if(player->checkIfMounted())	{
            mRadialMenu->addItem(2, 0, radId_serverVehicleExit,radAction_Default, "@pet/pet_menu:menu_enter_exit");
        }	else {
            mRadialMenu->addItem(2, 0, radId_serverVehicleEnter,radAction_Default, "@pet/pet_menu:menu_enter_exit");
        }

        mRadialMenu->addItem(3, 0, radId_vehicleStore,radAction_ObjCallback, "@pet/pet_menu:menu_store");

        // @TODO: Check if near a garage then add repair
    }
}

//=============================================================================

void MountObject::handleObjectMenuSelect(uint8 message_type, Object* source_object) {

    PlayerObject*	player	= dynamic_cast<PlayerObject*>(source_object);

    if (!player) {
        // Verify the data passed in is what is expected. In debug mode the assert will
        // trigger and crash the server.
        assert(false && "MountObject::handleObjectMenuSelect - Menu selection requested from a non-player object.");
        return;
    }

    switch (message_type) {
    case radId_vehicleStore:
    {
        Datapad* datapad			= player->getDataPad();
        if(datapad) {
            if(VehicleController* vehicle = dynamic_cast<VehicleController*>(datapad->getDataById(mId-1))) {
                vehicle->Store();
            }
        }
    }
    break;

	case radId_serverVehicleEnter:
    case radId_serverVehicleExit:
    {
        DLOG(INFO) << "MountObject::handleObjectMenuSelect - still in radial selection";
    }
    break;

    default:
    {
        DLOG(INFO) << "MountObject::handleObjectMenuSelect - unknown radial selection: " << message_type;
    }
    break;
    }
}
