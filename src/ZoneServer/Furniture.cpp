/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/
#include "Furniture.h"
#include "PlayerObject.h"
#include "MessageLib/MessageLib.h"


//=============================================================================

Furniture::Furniture() : Item()
{
}

//=============================================================================

Furniture::~Furniture()
{
}

//=============================================================================
//handles the radial selection

void Furniture::handleObjectMenuSelect(uint8 messageType,Object* srcObject)
{
	if(dynamic_cast<PlayerObject*>(srcObject))
	{
		switch(messageType)
		{
			case radId_itemSit:
			{
				//gMessageLib->sendSystemMessage(player,L"WE HIT THE radId_itemSit case");
			}
		}
	}
}

//=============================================================================


//=============================================================================
// Make the custom radial for sit and examine of an item

void Furniture::prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount)
{
	uint8 count = 1;
	RadialMenu* radial	= new RadialMenu();

	radial->addItem(count++,0,radId_examine,radAction_ObjCallback,"");
	RadialMenuPtr radialPtr(radial);
	mRadialMenu = radialPtr;
}
