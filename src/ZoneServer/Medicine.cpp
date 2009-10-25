/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "Medicine.h"
#include "Inventory.h"
#include "MedicManager.h"
#include "ObjectFactory.h"
#include "PlayerObject.h"
#include "WorldManager.h"
#include "MessageLib/MessageLib.h"
#include "DatabaseManager/Database.h"

Medicine::Medicine(void)
{
}

Medicine::~Medicine(void)
{
}

void Medicine::handleStimpackMenuSelect(uint8 messageType, PlayerObject* player)
{
	switch(messageType)
	{
		case radId_itemUse:
		{
			//Check we have a valid target
			if(CreatureObject* target = dynamic_cast<CreatureObject*>(player->getTarget()))
			{
				//check Medic has enough Mind
				if(player->getHam()->checkMainPools(0, 0, 140))
				{
					//Try to Heal Damage
					if(gMedicManager->HealDamage(player, target, mId, 0))
					{
						//If we succeed, reduce Medics Mind
						player->getHam()->updatePropertyValue(HamBar_Mind, HamProperty_CurrentHitpoints, -140);
					} else {
					}
				} else {
					gMessageLib->sendSystemMessage(player,L"","healing_response","not_enough_mind");
				}
			} else {
				gMessageLib->sendSystemMessage(player,L"","healing_response","healing_response_62");
			}
		}
	}
}
//=============================================================================
//handles the radial selection

void Medicine::handleObjectMenuSelect(uint8 messageType,Object* srcObject)
{
	if(PlayerObject* player = dynamic_cast<PlayerObject*>(srcObject))
	{
		switch(mItemType)
		{
		case ItemType_Stimpack_A:
			handleStimpackMenuSelect(messageType, player);
			break;
		case ItemType_Stimpack_B:
			handleStimpackMenuSelect(messageType, player);
			break;
		case ItemType_Stimpack_C:
			handleStimpackMenuSelect(messageType, player);
			break;
		case ItemType_Stimpack_D:
			handleStimpackMenuSelect(messageType, player);
			break;
		case ItemType_Stimpack_E:
			handleStimpackMenuSelect(messageType, player);
			break;
		}
	}
}

//=============================================================================
/*
volume
serial_number
crafter
counter_uses_remaining
examine_heal_damage_action
examine_heal_damage_health
healing_ability
*/

/*
	//healing only in proper places
	EMLocationType loc = player->getPlayerLocation();
	if(loc != EMLocation_Cantina || loc != EMLocation_Camp || loc != EMLocation_PlayerStructure)
	{
		return;
	}
*/
uint Medicine::getSkillRequired()
{
	return this->getAttribute<uint32>("healing_ability");
}
uint Medicine::getHealthHeal()
{
	return this->getAttribute<uint32>("examine_heal_damage_health");
}
uint Medicine::getActionHeal()
{
	return this->getAttribute<uint32>("examine_heal_damage_action");
}
uint Medicine::getUsesRemaining()
{
	return this->getAttribute<uint32>("counter_uses_remaining");
}
bool Medicine::ConsumeUse(PlayerObject* playerObject)
{
	gMessageLib->sendSystemMessage(playerObject, L"Uses Remaining updated.");

	uint32 quantity = this->getAttribute<uint32>("counter_uses_remaining");
	quantity--;

	if(quantity)
	{
		this->setAttribute("counter_uses_remaining",boost::lexical_cast<std::string>(quantity));
		gWorldManager->getDatabase()->ExecuteSqlAsync(0,0,"UPDATE item_attributes SET value='%u' WHERE item_id=%"PRId64" AND attribute_id=%u",quantity,this->getId(),AttrType_CounterUsesRemaining);

		//now update the uses display
		gMessageLib->sendUpdateUses(this,playerObject);
		return false;
	}
	else
	{
		//Inventory* inventory = dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
		//the db
		gObjectFactory->deleteObjectFromDB(this);

		//destroy it in the client
		gMessageLib->sendDestroyObject(this->getId(),playerObject);

		//delete it out of the inventory
		//inventory->deleteObject(this);
		return true;
	}

}

//=============================================================================

void Medicine::prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount)
{
	//RadialMenu* radial	= new RadialMenu();

	//uint32 StimType = this->getItemType();

	//uint32 radId = 0;

	//RadialMenuPtr radialPtr(radial);

	//mRadialMenu = radialPtr;

}
