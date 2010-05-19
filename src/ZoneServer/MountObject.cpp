/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "MountObject.h"

#include "Datapad.h"
#include "PlayerObject.h"
#include "VehicleController.h"

//=============================================================================

MountObject::MountObject()
: CreatureObject()
, mOwner(0)
, mController(0) {}

//=============================================================================

void MountObject::prepareCustomRadialMenu(CreatureObject* creature, uint8 item_count) {

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
  if(player->getId() == mOwner)	{

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
		  if(Datapad* datapad = dynamic_cast<Datapad*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Datapad))) {			
        if(VehicleController* vehicle = dynamic_cast<VehicleController*>(datapad->getDataById(mId-1))) {
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
