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

#include "MountObject.h"
#include "Datapad.h"
#include "VehicleController.h"
#include "PlayerObject.h"

//=============================================================================
//handles building custom radials
void MountObject::prepareCustomRadialMenu(CreatureObject* creature, uint8 item_count) {

	PlayerObject*	player	= dynamic_cast<PlayerObject*>(creature);

  // Verify the data passed in is what is expected. In debug mode the assert will
  // trigger and crash the server.
  assert(player && "MountObject::prepareCustomRadialMenu - Menu preparation requested for a non-player object.");

  // In release mode asserts don't trigger so an additional check is needed.
	if(! player) {
		gLogger->logErrorF("radials", "MountObject::prepareCustomRadialMenu - Menu preparation requested for a non-player object",MSG_NORMAL);
		return;
	}

  // Reset the radial menu with a new instance.
  mRadialMenu.reset(new RadialMenu());

  // Add the radial options for the custom menu.
	mRadialMenu->addItem(1, 0, radId_examine, radAction_Default);

  // Check to see if the player requesting the menu is the owner of the mount.
  if(player->getId() == mOwner)	{

    // Check to see if the player is mounted or not and display the appropriate exit/enter option.
    if(player->checkIfMounted())	{
      mRadialMenu->addItem(2, 0, radId_serverVehicleExit,radAction_Default, "@pet/pet_menu:menu_exit");
    }	else {
		  mRadialMenu->addItem(2, 0, radId_serverVehicleEnter,radAction_Default, "@pet/pet_menu:menu_enter");
    }
    
    mRadialMenu->addItem(3, 0, radId_vehicleStore,radAction_ObjCallback, "@pet/pet_menu:menu_store");

    // @TODO: Check if near a garage then add repair
  }
}



//=============================================================================
//handles the radial selection

void MountObject::handleObjectMenuSelect(uint8 message_type, Object* source_object) {
  
	PlayerObject*	player	= dynamic_cast<PlayerObject*>(source_object);

  // Verify the data passed in is what is expected. In debug mode the assert will
  // trigger and crash the server.
  assert(player && "MountObject::handleObjectMenuSelect - Menu selection requested from a non-player object.");

  // In release mode asserts don't trigger so an additional check is needed.
	if(! player) {
		gLogger->logErrorF("radials", "MountObject::handleObjectMenuSelect - Menu selection requested from a non-player object", MSG_NORMAL);
		return;
	}

  switch (message_type) 
  {
    case radId_vehicleStore:
      {
		if(Datapad* datapad = dynamic_cast<Datapad*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Datapad))) 
		{			
          if(VehicleController* vehicle = dynamic_cast<VehicleController*>(datapad->getDataById(mId-1)))	
		  {
            vehicle->store();
          }
		}
      }
    break;

    case radId_serverVehicleEnter:
    case radId_serverVehicleExit:
      {
				gLogger->logErrorF("radials", "MountObject::handleObjectMenuSelect - still in radial selection", MSG_NORMAL);
      }
    break;

    default:
      {
				gLogger->logErrorF("radials", "MountObject::handleObjectMenuSelect - unknown radial selection: %d",MSG_NORMAL,message_type);
      }
    break;
  }
}
