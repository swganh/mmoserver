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

#include "RadialMenu.h"

//=======================================================================

RadialMenu::RadialMenu()
{
}

//=======================================================================

RadialMenu::~RadialMenu()
{
    RadialItemList::iterator it = mItemList.begin();
    while(it != mItemList.end())
    {
        delete(*it);
        mItemList.erase(it);
        it = mItemList.begin();
    }
}

//=======================================================================

void RadialMenu::addItem(uint8 index,uint8 parentItem,RadialIdentifier identifier,uint8 action,const int8* description)
{
    mItemList.push_back(new RadialMenuItem(index,parentItem,identifier,action,description));
}

//=======================================================================
