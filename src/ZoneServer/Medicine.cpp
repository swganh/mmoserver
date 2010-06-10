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

#include "Medicine.h"
#include "Inventory.h"
#include "MedicManager.h"
#include "ObjectFactory.h"
#include "PlayerObject.h"
#include "WorldManager.h"
#include "MessageLib/MessageLib.h"
#include "DatabaseManager/Database.h"
#include "ObjectControllerOpcodes.h"
#include "Utils\rand.h"

//consts
const char* const woundpack = "woundpack";
const char* const stim = "stim";
const char* const rangedstim = "ranged";
const char* const self = "self";
const char* const action = "action";
const char* const constitution = "constitution";
const char* const health = "health";
const char* const quickness = "quickness";
const char* const stamina = "stamina";
const char* const strength = "strength";

Medicine::Medicine(void)
{
}

Medicine::~Medicine(void)
{
}

void Medicine::handleStimpackMenuSelect(uint8 messageType, PlayerObject* player, std::string medpackType)
{
	if (medpackType == "")
		medpackType = stim;
	switch(messageType)
	{
		case radId_itemUse:
		{
			//get heal target
			if (PlayerObject* target = dynamic_cast<PlayerObject*>(player->getHealingTarget(player)))
			{
				//check Medic has enough Mind
				if(player->getHam()->checkMainPools(0, 0, 140))
				{
					//Try to Heal Damage
					if(gMedicManager->CheckMedicine(player, target,0, medpackType))
					{
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

void Medicine::handleWoundPackMenuSelect(uint8 messageType, PlayerObject* player, std::string medpackType)
{
	if (medpackType == "" )
		medpackType = woundpack;

	switch(messageType)
	{
		case radId_itemUse:
		{
			//get wound heal target
			if (PlayerObject* target = dynamic_cast<PlayerObject*>(player->getHealingTarget(player)))
			{
				//check Medic has enough Mind
				if(player->getHam()->checkMainPools(0, 0, 140))
				{
					//Try to Heal Damage
					if(gMedicManager->CheckMedicine(player, target, 0, medpackType))
					{
						//Call the event
						gMedicManager->startWoundTreatmentEvent(player);
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
					handleStimpackMenuSelect(messageType, player, stim);
					break;
				case ItemType_Ranged_Stimpack_A:
				case ItemType_Ranged_Stimpack_B:
				case ItemType_Ranged_Stimpack_C:
				case ItemType_Ranged_Stimpack_D:
				case ItemType_Ranged_Stimpack_E:
					handleStimpackMenuSelect(messageType, player, rangedstim);
					break;
				case ItemType_Wound_Action_A:
				case ItemType_Wound_Action_B:			
				case ItemType_Wound_Action_C:
				case ItemType_Wound_Action_D:
				case ItemType_Wound_Action_E:
					handleWoundPackMenuSelect(messageType, player, action);
					break;
				case ItemType_Wound_Constitution_A:
				case ItemType_Wound_Constitution_B:			
				case ItemType_Wound_Constitution_C:
				case ItemType_Wound_Constitution_D:
				case ItemType_Wound_Constitution_E:
					handleWoundPackMenuSelect(messageType, player, constitution);
					break;
				case ItemType_Wound_Health_A:
				case ItemType_Wound_Health_B:
				case ItemType_Wound_Health_C:
				case ItemType_Wound_Health_D:
				case ItemType_Wound_Health_E:
					handleWoundPackMenuSelect(messageType, player, health);
					break;
				case ItemType_Wound_Quickness_A:
				case ItemType_Wound_Quickness_B:
				case ItemType_Wound_Quickness_C:
				case ItemType_Wound_Quickness_D:
				case ItemType_Wound_Quickness_E:
					handleWoundPackMenuSelect(messageType, player, quickness);
					break;
				case ItemType_Wound_Stamina_A:
				case ItemType_Wound_Stamina_B:
				case ItemType_Wound_Stamina_C:
				case ItemType_Wound_Stamina_D:
				case ItemType_Wound_Stamina_E:
					handleWoundPackMenuSelect(messageType, player, stamina);
					break;
				case ItemType_Wound_Strength_A:
				case ItemType_Wound_Strength_B:
				case ItemType_Wound_Strength_C:
				case ItemType_Wound_Strength_D:
				case ItemType_Wound_Strength_E:
					handleWoundPackMenuSelect(messageType, player, strength);
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
uint Medicine::getSkillRequired(string skill)
{
	return this->getAttribute<uint32>(skill);
}
uint32 Medicine::getHealthHeal()
{
	return (uint32)this->getAttribute<float>("examine_heal_damage_health");
}
uint32 Medicine::getActionHeal()
{
	return (uint32)this->getAttribute<float>("examine_heal_damage_action");
}
//Wound Heals
uint32 Medicine::getHealWoundAction()
{
	return (uint32)this->getAttribute<float>("examine_heal_wound_action");
}
uint32 Medicine::getHealWoundConstitution()
{
	return (uint32)this->getAttribute<float>("examine_heal_wound_constitution");
}
uint32 Medicine::getHealWoundHealth()
{
	return (uint32)this->getAttribute<float>("examine_heal_wound_health");
}
uint32 Medicine::getHealWoundQuickness()
{
	return (uint32)this->getAttribute<float>("examine_heal_wound_quickness");
}
uint32 Medicine::getHealWoundStamina()
{
	return (uint32)this->getAttribute<float>("examine_heal_wound_stamina");
}
uint32 Medicine::getHealWoundStrength()
{
	return (uint32)this->getAttribute<float>("examine_heal_wound_strength");
}
uint32 Medicine::getHealWound(string attribute)
{
	//this should return us the attribute type we are trying to heal
	//TODO replace with std::string after bstring is removed...
	string examine = "examine_heal_wound_";
	std::string tmp = examine.getAnsi();
	tmp.append(attribute.getAnsi());
	string attr = tmp.c_str();
	return (uint32)this->getAttribute<float>(attr);
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
