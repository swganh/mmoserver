/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_EV_WEAPON_H
#define ANH_ZONESERVER_EV_WEAPON_H

#include "EnqueueValidator.h"


//=======================================================================

class EVWeapon : public EnqueueValidator
{
	public:

		EVWeapon(ObjectController* controller) : EnqueueValidator(controller){}
		virtual ~EVWeapon(){}

		virtual bool validate(uint32 &reply1,uint32 &reply2,uint64 targetId,uint32 opcode,ObjectControllerCmdProperties*& cmdProperties)
		{
			CreatureObject* creature = dynamic_cast<CreatureObject*>(mController->getObject());

			// check our equipped weapon
			uint32	weaponGroup = WeaponGroup_Unarmed;

			if(Item* weapon = dynamic_cast<Item*>(creature->getEquipManager()->getEquippedObject(CreatureEquipSlot_Weapon)))
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
				reply2 = 0;

				if(PlayerObject* player = dynamic_cast<PlayerObject*>(creature))
				{
					gMessageLib->sendSystemMessage(player,L"","cbt_spam","no_attack_wrong_weapon");
				}
				
				return(false);
			}

			return(true);
		}
};

//=======================================================================

#endif