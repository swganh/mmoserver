/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "Item.h"
#include "PlayerObject.h"


//=============================================================================

Item::Item() : TangibleObject(), mOwner(0)
{
	mTanGroup			= TanGroup_Item;
	mPlaced				= false;
	mNonPersistantCopy	= 0;
	mPersistantCopy		= 0;
	mLoadCount			= 0;

}

//=============================================================================

Item::~Item()
{
}

//=============================================================================


void Item::prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount)
{
	//check if musical Instrument
	//switch (this->getItemFamily())
	//{

	//	case ItemFamily_FireWork:
	//	{
	//		RadialMenu* radial	= new RadialMenu();
	//
	//		radial->addItem(1,0,radId_itemUse,radAction_ObjCallback,"");
	//		radial->addItem(2,0,radId_examine,radAction_ObjCallback,"");
	//		radial->addItem(3,0,radId_itemDestroy,radAction_ObjCallback,"");

	//		//Testing purposes lol
	//		radial->addItem(4,0,radId_itemSit,radAction_ObjCallback,"");

	//		RadialMenuPtr radialPtr(radial);
	//		mRadialMenu = radialPtr;
	//
	//	}
	//	break;
	//
	//	default:
	//	{
	//	}
	//	break;
	//}
}

//=============================================================================

void Item::handleObjectMenuSelect(uint8 messageType,Object* srcObject)
{
	if(dynamic_cast<PlayerObject*>(srcObject))
	{
		switch(messageType)
		{
			// When player hits launch on fireworks
			case radId_itemUse:
			{

			}
			break;

			default: break;
		}
	}
}

//=============================================================================


