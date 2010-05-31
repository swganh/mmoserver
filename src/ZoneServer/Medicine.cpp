/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

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
#include "Utils/clock.h"

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
			//get heal target
			if (CreatureObject* target = dynamic_cast<CreatureObject*>(player->getHealingTarget(player)))
			{
				//check Medic has enough Mind
				if(player->getHam()->checkMainPools(0, 0, 140))
				{
					//Try to Heal Damage
					if(gMedicManager->CheckStim(player, target, 0))
					{
						//If we succeed, reduce Medics Mind
						player->getHam()->updatePropertyValue(HamBar_Mind, HamProperty_CurrentHitpoints, -140);
						//Call the event
						gMedicManager->startInjuryTreatmentEvent(player);
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

			case radId_itemUse:
			{
				switch(mItemType)
				{
				case ItemType_Stimpack_A:
				case ItemType_Stimpack_B:
				case ItemType_Stimpack_C:
				case ItemType_Stimpack_D:
				case ItemType_Stimpack_E:
				case ItemType_Ranged_Stimpack_A:
				case ItemType_Ranged_Stimpack_B:
				case ItemType_Ranged_Stimpack_C:
				case ItemType_Ranged_Stimpack_D:
				case ItemType_Ranged_Stimpack_E:
					handleStimpackMenuSelect(messageType, player);
					break;
				}

			}
			break;

			default: break;
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
uint32 Medicine::getHealthHeal()
{
	return (uint32)this->getAttribute<float>("examine_heal_damage_health");
}
uint32 Medicine::getActionHeal()
{
	return (uint32)this->getAttribute<float>("examine_heal_damage_action");
}
uint32 Medicine::getUsesRemaining()
{
	return (uint32)this->getAttribute<float>("counter_uses_remaining");
}
bool Medicine::ConsumeUse(PlayerObject* playerObject)
{
	

	float quantity = this->getAttribute<float>("counter_uses_remaining");
	quantity--;

	if(quantity)
	{
		this->setAttribute("counter_uses_remaining",boost::lexical_cast<std::string>(quantity));
		gWorldManager->getDatabase()->ExecuteSqlAsync(0,0,"UPDATE item_attributes SET value='%f' WHERE item_id=%"PRIu64" AND attribute_id=%u",quantity,this->getId(),AttrType_CounterUsesRemaining);

		//now update the uses display
		gMessageLib->sendUpdateUses(this,playerObject);
		return false;
	}
	else //destroy us
	{
		//the db
		gObjectFactory->deleteObjectFromDB(this);

		//destroy it in the client
		gMessageLib->sendDestroyObject(this->getId(),playerObject);

		//delete it out of the inventory
		//uint64 now = Anh_Utils::Clock::getSingleton()->getLocalTime();
		//playerObject->getController()->addEvent(new ItemDeleteEvent(now+100,this->getId());
		
		//the above commented code is used for other consumeables
		//with medicine the medicmanager handles the destruction
		return true;
	}

}

//=============================================================================

void Medicine::prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount)
{
	RadialMenu* radial	= new RadialMenu();

	radial->addItem(1,0,radId_itemUse,radAction_ObjCallback,"");
	radial->addItem(2,0,radId_examine,radAction_ObjCallback,"");
	radial->addItem(3,0,radId_itemDestroy,radAction_ObjCallback,"");
	RadialMenuPtr radialPtr(radial);
	mRadialMenu = radialPtr;

}
