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

#ifndef ANH_ZONESERVER_TANGIBLE_WEAPON_H
#define ANH_ZONESERVER_TANGIBLE_WEAPON_H

#include "Item.h"

//=============================================================================

enum WeaponGroup
{
    WeaponGroup_Unarmed			= 1,
    WeaponGroup_1h				= 2,
    WeaponGroup_2h				= 4,
    WeaponGroup_Polearm			= 8,
    WeaponGroup_Rifle			= 16,
    WeaponGroup_Pistol			= 32,
    WeaponGroup_Carbine			= 64,
    WeaponGroup_Flamethrower	= 128,
    WeaponGroup_Saber			= 256,
    WeaponGroup_Launcher		= 512
};

//=============================================================================

class Weapon : public Item
{
    friend class ItemFactory;

public:

    Weapon();
    virtual ~Weapon();

    void	setGroup(uint32 group) {
        mGroup = group;
    }
    uint32	getGroup() {
        return mGroup;
    }

    void	sendAttributes(PlayerObject* playerObject);

private:

    uint32	mGroup;
    uint8	mDamageType;
};

//=============================================================================

#endif

