/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "Item.h"
#include "food.h"
#include "EntertainerManager.h"
#include "LogManager/LogManager.h"
#include "WorldConfig.h"
#include "ZoneServer/Tutorial.h"
#include "MessageLib/MessageLib.h"


//=============================================================================

Item::Item() : TangibleObject(), mOwner(0)
{
	mTanGroup			= TanGroup_Item;
	mPlaced				= false;
	mNonPersistantCopy	= 0;
	mPersistantCopy		= 0;
	
}

//=============================================================================

Item::~Item()
{
}

//=============================================================================


void Item::prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount)
{
	//check if musical Instrument
	switch (this->getItemFamily())
	{

		case ItemFamily_FireWork:
		{
			RadialMenu* radial	= new RadialMenu();
			
			radial->addItem(1,0,radId_itemUse,radAction_ObjCallback,"");
			radial->addItem(2,0,radId_examine,radAction_ObjCallback,"");
			radial->addItem(3,0,radId_itemDestroy,radAction_ObjCallback,"");

			//Testing purposes lol
			radial->addItem(4,0,radId_itemSit,radAction_ObjCallback,"");

			RadialMenuPtr radialPtr(radial);
			mRadialMenu = radialPtr;
			
		}
		break;
		
		default:
		{
		}
		break;
	}
}

//=============================================================================

void Item::handleObjectMenuSelect(uint8 messageType,Object* srcObject)
{
	if(PlayerObject* playerObject = dynamic_cast<PlayerObject*>(srcObject))
	{	
		switch(messageType)
		{
			// When player hits launch on fireworks
			case radId_itemUse: 
			{
				gMessageLib->sendSystemMessage(playerObject, "FireWorks are under Development!");
				//gMessageLib->sendCreateObjectByCRC(theObject,player,false);
				//you cant access the chatmessagelib from the zone as the chatmessagelib is chatserver only!!!!
				//if you want to create the rack then you would have to create it as temporary object
				//how to do this is something you can see in scout.cpp done for camps :)

				//alternatively you can create a new object fireworks.h and fireworks.cpp
				//just make sure its created as such in the factory and you need to add fireworks as itemtype!!!

				//gChatMessageLib->sendSceneCreateObjectByCrc(mId, 0x788cf998, player);
				//gChatMessageLib->sendSceneDestroyObject(mId, targetPlayer);
				
				
			}
			break;

			default: break;
		}
	}
}

//=============================================================================


