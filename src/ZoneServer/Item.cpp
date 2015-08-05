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

#include "Item.h"
#include "PlayerObject.h"
#include "WorldManager.h"
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
    gWorldManager->getDatabase()->executeSqlAsync(0,0,"UPDATE %s.items SET parent_id ='%" PRIu64 "', oX='%f',oY='%f', oZ='%f', oW='%f', x='%f', y='%f', z='%f' WHERE id='%" PRIu64 "'",gWorldManager->getDatabase()->galaxy(),this->getParentId(), this->mDirection.x, this->mDirection.y, this->mDirection.z, this->mDirection.w, this->mPosition.x, this->mPosition.y, this->mPosition.z, this->getId());
   
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
            gMessageLib->sendDataTransform053(this);
        }
        break;

        case radId_itemRotateLeft:
        {
            // Rotate the item 90 degrees to the left
            rotateLeft(90.0f);
            gMessageLib->sendDataTransform053(this);
        }
        break;

        default:
            break;
        }
    }
}

//=============================================================================


