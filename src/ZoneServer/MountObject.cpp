/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "MountObject.h"
#include "MessageLib/MessageLib.h"
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

	uint8 radId = 0;

	//if we have a prefab Menu (we will have one as a swoop)  iterate through it and add it to our response
	//this way we will have our menu item numbering done right
	MenuItemList* menuItemList = 		getMenuList();
	if(menuItemList)
	{
		MenuItemList::iterator it	=	menuItemList->begin();

		while(it != menuItemList->end())
		{
			radId++;
			
			radial->addItem((*it)->sItem,(*it)->sSubMenu,(RadialIdentifier)(*it)->sIdentifier,(*it)->sOption,"");
			it++;
		}
	}



	if(getCreoGroup() == CreoGroup_Vehicle)
	{

		//The radial for the swoop is done in this manner to make sure that enter/exit is the default action
		//for double click... many people have expressed that they would rather have misnumbered radials then 
		//not have the default action be enter/exit
		if(radial) delete radial;
		radial = new RadialMenu();

		if(creatureObject->getId() == mOwner)
		{
			PlayerObject* owner = dynamic_cast<PlayerObject*>(creatureObject);
			if(owner->checkIfMounted())
			{
				radial->addItem(2,0,radId_serverVehicleExit,radAction_ObjCallback,"@pet/pet_menu:menu_exit");
			}
			else
			{
				radial->addItem(2,0,radId_serverVehicleEnter,radAction_ObjCallback,"@pet/pet_menu:menu_enter");
			}
			radial->addItem(3,0,radId_vehicleStore,radAction_ObjCallback,"@pet/pet_menu:menu_store");
			//TODO: Check if near a garage then add repair
		}

		radial->addItem(1,0,radId_examine,radAction_Default);

		mRadialMenu = RadialMenuPtr(radial);
	}



}

