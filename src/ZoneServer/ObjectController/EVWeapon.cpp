/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2014 The SWG:ANH Team
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
#include "Zoneserver/Objects/Item.h"
#include "ZoneServer/Objects/Creature Object/CreatureObject.h"
#include "Zoneserver/ObjectController/ObjectController.h"
#include "ZoneServer/ObjectController/ObjectControllerCommandMap.h"
#include "ZoneServer/Objects/Player Object/PlayerObject.h"
#include "ZoneServer/Objects/Weapon.h"

#include "ZoneServer/WorldManager.h"
#include "MessageLib/MessageLib.h"

//#include "ZoneServer\Services\equipment\equipment_service.h"

EVWeapon::EVWeapon(ObjectController* controller)
    : EnqueueValidator(controller)
{}

EVWeapon::~EVWeapon()
{}

bool EVWeapon::validate(uint32 &reply1, uint32 &reply2, uint64 targetId, uint32 opcode, ObjectControllerCmdProperties*& cmdProperties)
{
    CreatureObject* creature = dynamic_cast<CreatureObject*>(mController->getObject());

	//auto equip_service = gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::equipment::EquipmentService>("EquipmentService");
	auto weapon = dynamic_cast<Weapon*>(gWorldManager->getObjectById(creature->GetWeaponId()));//equip_service->GetEquippedObject(creature, "hold_r"));

    // check our equipped weapon
    uint32	weaponGroup = WeaponGroup_Unarmed;

    if(weapon && weapon->getItemFamily() == ItemFamily_Weapon)        {
        weaponGroup = dynamic_cast<Weapon*>(weapon)->getGroup();
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

