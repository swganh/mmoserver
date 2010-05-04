/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "Item.h"
#include "PlayerObject.h"
#include "MessageLib/MessageLib.h"
#include "DatabaseManager/Database.h"


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

void Item::updateWorldPosition()
{
	gWorldManager->getDatabase()->ExecuteSqlAsync(0,0,"UPDATE items SET parent_id ='%I64u', oX='%f',oY='%f', oZ='%f', oW='%f', x='%f', y='%f', z='%f' WHERE id='%I64u'",this->getParentId(), this->mDirection.x, this->mDirection.y, this->mDirection.z, this->mDirection.w, this->mPosition.x, this->mPosition.y, this->mPosition.z, this->getId());
	
}

//=============================================================================
// its an item drop in a cell - these all have the same menu options
/*
void Item::prepareCustomRadialMenuInCell(CreatureObject* creatureObject, uint8 itemCount)
{
	RadialMenu* radial	= new RadialMenu();
	uint8 i = 1;
	uint8 u = 1;

	// any object with callbacks needs to handle those (received with menuselect messages) !
	if(this->getObjects()->size())
		radial->addItem(i++,0,radId_itemOpen,radAction_Default,"");

	radial->addItem(i++,0,radId_examine,radAction_Default,"");

	radial->addItem(i++,0,radId_itemPickup,radAction_Default,"");
	
	u = i;
	radial->addItem(i++,0,radId_itemMove,radAction_Default, "");	
	radial->addItem(i++,u,radId_itemMoveForward,radAction_Default, "");//radAction_ObjCallback
	radial->addItem(i++,u,radId_ItemMoveBack,radAction_Default, "");
	radial->addItem(i++,u,radId_itemMoveUp,radAction_Default, "");
	radial->addItem(i++,u,radId_itemMoveDown,radAction_Default, "");
	
	u = i;
	radial->addItem(i++,0,radId_itemRotate,radAction_Default, "");
	radial->addItem(i++,u,radId_itemRotateRight,radAction_Default, "");
	radial->addItem(i++,u,radId_itemRotateLeft,radAction_Default, "");

  
	RadialMenuPtr radialPtr(radial);
	mRadialMenu = radialPtr;


}
*/

void Item::prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount)
{
	//thats handled by the specific items as these are very heterogen
}

//=============================================================================

void Item::handleObjectMenuSelect(uint8 messageType,Object* srcObject)
{
	if(dynamic_cast<PlayerObject*>(srcObject))
	{
		switch(messageType)
		{
			case radId_itemRotateRight:
			{
                // Rotate the item 90 degrees to the right
                rotateRight(90.0f);
				gMessageLib->sendDataTransform(this);
			}
			break;

			case radId_itemRotateLeft:
			{
                // Rotate the item 90 degrees to the left
                rotateLeft(90.0f);
				gMessageLib->sendDataTransform(this);
			}
			break;

			default: break;
		}
	}
}

//=============================================================================


