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

#include "EVWeapon.h"
#include "Item.h"
#include "CreatureObject.h"
#include "ObjectController.h"
#include "ObjectControllerCommandMap.h"
#include "PlayerObject.h"
#include "Weapon.h"
#include "MessageLib/MessageLib.h"

EVWeapon::EVWeapon(ObjectController* controller)
    : EnqueueValidator(controller)
{}

EVWeapon::~EVWeapon()
{}

bool EVWeapon::validate(uint32 &reply1, uint32 &reply2, uint64 targetId, uint32 opcode, ObjectControllerCmdProperties*& cmdProperties)
{
    CreatureObject* creature = dynamic_cast<CreatureObject*>(mController->getObject());

    // check our equipped weapon
    uint32	weaponGroup = WeaponGroup_Unarmed;

    if(Item* weapon = dynamic_cast<Item*>(creature->getEquipManager()->getEquippedObject(CreatureEquipSlot_Hold_Left)))
    {
        // could be an instrument
        if(weapon->getItemFamily() == ItemFamily_Weapon)
        {
            weaponGroup = dynamic_cast<Weapon*>(weapon)->getGroup();
        }
    }

    if(cmdProperties->mRequiredWeaponGroup && (weaponGroup & cmdProperties->mRequiredWeaponGroup) != weaponGroup)
    {
        reply1 = 0;
        reply2 = 1;

        if(PlayerObject* player = dynamic_cast<PlayerObject*>(creature))
        {
            gMessageLib->SendSystemMessage(::common::OutOfBand("cbt_spam", "no_attack_wrong_weapon"), player);
        }

        return(false);
    }

    return(true);
}

