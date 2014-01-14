/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "MountObject.h"
#include "MessageLib/MessageLib.h"
#include "Datapad.h"
#include "Vehicle.h"
#include "PlayerObject.h"

//=============================================================================
//handles building custom radials
void MountObject::prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount)
{

	PlayerObject*	playerObject	= dynamic_cast<PlayerObject*>(creatureObject);

	if(!playerObject)
	{
		return;
	}

	RadialMenu*		radial			= new RadialMenu();

	uint8 radId = 1;

	//if we have a prefab Menu (we will have one as a swoop)  iterate through it and add it to our response
	//this way we will have our menu item numbering done right
	/*
	MenuItemList* menuItemList = 		getMenuList();
	if(menuItemList)
	{
		MenuItemList::iterator it	=	menuItemList->begin();

		while(it != menuItemList->end())
		{
			radId++;
			
			if((*it)->sIdentifier == 7)
			{
				radial->addItem((*it)->sItem,(*it)->sSubMenu,(RadialIdentifier)(*it)->sIdentifier,radAction_ObjCallback,"");
			}
			else
				radial->addItem((*it)->sItem,(*it)->sSubMenu,(RadialIdentifier)(*it)->sIdentifier,radAction_Default,"");

			gLogger->logMsgF("menu item id : %u",MSG_HIGH,(*it)->sIdentifier);
			it++;
		}
	}
	  */


	if(getCreoGroup() == CreoGroup_Vehicle)
	{

		//The radial for the swoop is done in this manner to make sure that enter/exit is the default action
		//for double click... many people have expressed that they would rather have misnumbered radials then 
		//not have the default action be enter/exit
		//if(radial) delete radial;
		//radial = new RadialMenu();

		if(creatureObject->getId() == mOwner)
		{
			PlayerObject* owner = dynamic_cast<PlayerObject*>(creatureObject);
			if(owner->checkIfMounted())
			{
				radial->addItem(radId++,0,radId_serverVehicleExit,radAction_Default,"@pet/pet_menu:menu_exit");
			}
			else
			{
				radial->addItem(radId++,0,radId_serverVehicleEnter,radAction_Default,"@pet/pet_menu:menu_enter");
			}
			radial->addItem(radId++,0,radId_vehicleStore,radAction_ObjCallback,"@pet/pet_menu:menu_store");
			//TODO: Check if near a garage then add repair
		}

		radial->addItem(radId++,0,radId_examine,radAction_2);

		
	}

	mRadialMenu = RadialMenuPtr(radial);


}



//=============================================================================
//handles the radial selection

void MountObject::handleObjectMenuSelect(uint8 messageType,Object* srcObject)
{

	if(PlayerObject* player = dynamic_cast<PlayerObject*>(srcObject))
	{
		switch(messageType)
		{
			case radId_vehicleStore:
			{
				if(Datapad* datapad = dynamic_cast<Datapad*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Datapad)))
				{
					IntangibleObject* itno = datapad->getDataById(mId-1);
					if(itno)
					{
						if(Vehicle* vehicle = dynamic_cast<Vehicle*>(itno))
						{
							vehicle->store();
						}
					}
				}
			}
			break;
			case radId_serverVehicleEnter: //An associated packet is sent
			case radId_serverVehicleExit: //mount and dismount logic is contained within OCPetHandlers.cpp
				gLogger->logErrorF("radials","CreatureObject::Error: still in radial selection",MSG_NORMAL);
				break;

			default:
				gLogger->logErrorF("radials","CreatureObject::Error: unknown radial selection: %d",MSG_NORMAL,messageType);
			break;
		}
	}
}
