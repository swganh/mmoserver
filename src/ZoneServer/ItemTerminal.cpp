/*
 This file is part of MMOServer. For more information, visit http://swganh.com
 
 Copyright (c) 2006 - 2011 The SWG:ANH Team

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
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
