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
#include "ItemTerminal.h"
#include "CharacterBuilderTerminal.h"

ItemTerminal::ItemTerminal() : Item()
{
    mItemType = ItemType_BlueFrog;
    cbt_ = std::shared_ptr<CharacterBuilderTerminal>(new CharacterBuilderTerminal());
}

ItemTerminal::~ItemTerminal()
{
}

void ItemTerminal::prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount)
{
    mRadialMenu = RadialMenuPtr(new RadialMenu());

    // any object with callbacks needs to handle those (received with menuselect messages) !
    mRadialMenu->addItem(1,0,radId_itemUse,radAction_ObjCallback);
    mRadialMenu->addItem(2,0,radId_examine,radAction_Default);
}
void ItemTerminal::handleObjectMenuSelect(uint8 messageType,Object* srcObject)
{
    cbt_->handleObjectMenuSelect(messageType, srcObject);
}
void ItemTerminal::handleUIEvent(uint32 action,int32 element,BString inputStr,UIWindow* window)
{
    cbt_->handleUIEvent(action, element, inputStr, window);
}
