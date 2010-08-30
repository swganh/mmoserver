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

#include "CraftingStation.h"
#include "PlayerObject.h"


//=============================================================================

CraftingStation::CraftingStation() : Item()
{

}

//=============================================================================

CraftingStation::~CraftingStation()
{
}

//=============================================================================

void CraftingStation::prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount)
{
    // lets make sure we're not in an inventory
    if(PlayerObject* player = dynamic_cast<PlayerObject*>(creatureObject))
    {
        if(this->getParentId() != player->getId()+INVENTORY_OFFSET)
        {
            mRadialMenu	= RadialMenuPtr(new RadialMenu());
            mRadialMenu->addItem(1,0,radId_examine,radAction_Default);
            mRadialMenu->addItem(2,0,radId_craftStart,radAction_Default);
        }
    }
}
void CraftingStation::prepareCustomRadialMenuInCell(CreatureObject* creatureObject, uint8 itemCount)
{
    RadialMenu* radial	= new RadialMenu();
    uint8 i = 1;
    uint8 u = 1;

    // any object with callbacks needs to handle those (received with menuselect messages) !
    radial->addItem(i++,0,radId_examine,radAction_Default,"");

    radial->addItem(i++,0,radId_craftStart,radAction_Default);

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