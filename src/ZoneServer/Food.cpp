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
#include "Food.h"
#include "Buff.h"
#include "Inventory.h"
#include "NPCObject.h"
#include "ObjectFactory.h"
#include "PlayerObject.h"
#include "WorldConfig.h"
#include "WorldManager.h"
#include "ZoneServer/Tutorial.h"
#include "DatabaseManager/Database.h"
#include "MessageLib/MessageLib.h"

#include "Utils/clock.h"

//=============================================================================

Food::Food() : Item()
{
	toDelete = false;
	mDuration=0;
	mBuff = 0;
}

//=============================================================================

Food::~Food()
{
}

void Food::prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount)
{
	RadialMenu* radial	= new RadialMenu();
		
	CreatureObject* unknownCreature;
	Inventory* creatureInventory;
	if (this->getParentId() &&
		(unknownCreature = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById(this->getParentId() - 1))) &&
		(creatureInventory = dynamic_cast<Inventory*>(unknownCreature->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory))) &&
		(creatureInventory->getId() == this->getParentId()))
	{
		// Its an object in an inventory

		NPCObject* npcObject = dynamic_cast<NPCObject*>(unknownCreature);
		if (npcObject)
		{
			if ((npcObject->getNpcFamily() == NpcFamily_AttackableCreatures) && npcObject->isDead())
			{
				// I'm pretty sure we are a loot item.
				radial->addItem(1,0,radId_itemPickup,radAction_ObjCallback,"@ui_radial:loot");
				radial->addItem(2,0,radId_examine,radAction_Default);
				mRadialMenu = RadialMenuPtr(radial);
				return;
			}
		}
	}

	// Note: If we are to never use the default "Eat", THEN remove the isTutorial()-condition test.
	if (gWorldConfig->isTutorial())
	{
		// Tutorial clearly states that we shall use the "Use"-option.
		radial->addItem(1,0,radId_itemUse,radAction_ObjCallback,"Use");
	}
	else
	{
		radial->addItem(1,0,radId_itemUse,radAction_ObjCallback,"");		// Default.
	}
	radial->addItem(2,0,radId_examine,radAction_ObjCallback,"");
	radial->addItem(3,0,radId_itemDestroy,radAction_ObjCallback,"");
	mRadialMenu = RadialMenuPtr(radial);

	
}

void Food::handleObjectMenuSelect(uint8 messageType,Object* srcObject)
{
	if(PlayerObject* playerObject = dynamic_cast<PlayerObject*>(srcObject))
	{	
		switch(messageType)
		{
			case radId_itemUse: 
			{
				if (gWorldConfig->isTutorial())
				{
					if (playerObject->isConnected())
					{
						playerObject->getTutorial()->tutorialResponse("foodUsed");
				
					}
				}

				handleFoodUse(srcObject);
			}
		}
	}
}

//=============================================================================

void Food::handleFoodUse(Object* srcObject)
{
	toDelete = true;
    
	PlayerObject* playerObject = dynamic_cast<PlayerObject*>(srcObject);
	if(!playerObject)
	{
		return;
	}

	if(playerObject->isDead() || playerObject->isIncapacitated())
	{
		gMessageLib->sendSystemMessage(playerObject, L"","error_message","wrong_state");
		return;
	}

	//we need to start by checking whether our stomach isnt full

	//we need to update our stomach
	float filling = 0;

	if(this->hasInternalAttribute("food_icon"))
	{
		mIcon = 0;
		mIcon = this->getInternalAttribute<uint32>("food_icon");					
	} else {
		gLogger->log(LogManager::DEBUG,"Food/Drink found with no Buff Icon in food.cpp: handleFoodUse()");
	}

	if(this->hasAttribute("counter_uses_remaining"))
	{
		_handleUses_Remaining(playerObject);
	}

	if(this->hasAttribute("stomach_food"))
	{

		filling = this->getAttribute<float>("stomach_food");					
		
		//do we still have place for it ?
		if(!playerObject->getStomach()->checkFood(filling))
		{
			gMessageLib->sendSysMsg(playerObject, "error_message","full_food");
			return;
		}
		
		gMessageLib->sendSysMsg(playerObject, "base_player","prose_consume_item",NULL,this);

		playerObject->getStomach()->incFood(filling);

		gMessageLib->sendPlayClientEffectLocMessage(gWorldManager->getClientEffect(548),playerObject->mPosition,playerObject);	
	}

	if(this->hasAttribute("stomach_drink"))
	{
		float filling = 0;
		filling = this->getAttribute<float>("stomach_drink");					
		
		//do we still have place for it ?
		if(!playerObject->getStomach()->checkDrink(filling))
		{
			gMessageLib->sendSysMsg(playerObject, "error_message","full_drink");
			return;
		}

		gMessageLib->sendSysMsg(playerObject, "base_player","prose_consume_item",NULL,this);
		
		playerObject->getStomach()->incDrink(filling);
	
		if(playerObject->getGender())
		{
			gMessageLib->sendPlayMusicMessage(WMSound_Drink_Human_Female,playerObject);
		}
		else
		{
			gMessageLib->sendPlayMusicMessage(WMSound_Drink_Human_Male,playerObject);
		}

	}
	if(this->hasAttribute("duration"))
	{
		mDuration= static_cast<uint32>(this->getAttribute<float>("duration"));
	}
	if(mDuration >0)
	{
		_handleBuff(playerObject);
	} else {
		_handleInstant(playerObject);
	}

	//when empty delete
	if(toDelete)
	{
		Inventory* inventory = dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
		//the db
		gObjectFactory->deleteObjectFromDB(this);

		//destroy it in the client
		gMessageLib->sendDestroyObject(this->getId(),playerObject);
		
		//delete it out of the inventory
		uint64 now = Anh_Utils::Clock::getSingleton()->getLocalTime();
		playerObject->getController()->addEvent(new ItemDeleteEvent(now+100,this->getId()),100);
		return;
	}
		
	return;
}
void Food::_handleInstant(PlayerObject* playerObject)
{
	gMessageLib->sendSystemMessage(playerObject, L"Sorry but instant use food has not been setup yet. Go kick a dev. Food::_handleInstant");
}
void Food::_handleBuff(PlayerObject* playerObject)
{
	mBuff = Buff::SimpleBuff(playerObject, playerObject, mDuration*1000, mIcon, gWorldManager->GetCurrentGlobalTick());

	AttributeMap::iterator it = mAttributeMap.begin();

	while(it != mAttributeMap.end())
	{
		uint32 amount = 0;
		try
		{
			amount =static_cast<uint32>(boost::lexical_cast<float>(it->second));
			BuffAttribute* foodAttribute = new BuffAttribute(it->first, +(int)amount,0,-(int)amount); 
			mBuff->AddAttribute(foodAttribute);	
		}
		catch(boost::bad_lexical_cast &)
		{
			//skip past as these are attributes we don't want to handle (such as string names etc)
		}

		++it;
	}

	playerObject->AddBuff(mBuff,true);
}
//void Food::_handleMask_Scent_Buff(PlayerObject* playerObject)
//{
//	uint32 amount = static_cast<uint32>(this->getAttribute<float>("bio_comp_mask_scent"));
//	uint32 duration = static_cast<uint32>(this->getAttribute<float>("duration"));
//
//	Buff* foodBuff = Buff::SimpleBuff(playerObject, playerObject, duration*1000, mIcon, gWorldManager->GetCurrentGlobalTick());
//	
//	BuffAttribute* foodAttribute = new BuffAttribute(mask_scent, +amount,0,-(int)amount); 
//	foodBuff->AddAttribute(foodAttribute);	
//
//	playerObject->AddBuff(foodBuff,true);
//}
//void Food::_handleHealth_Buff(PlayerObject* playerObject)
//{
//	uint32 amount = static_cast<uint32>(this->getAttribute<float>("attr_health"));
//	uint32 duration = 0;
//	if(this->hasAttribute("duration"))
//	{
//		duration = static_cast<uint32>(this->getAttribute<float>("duration"));
//		//get a stomach Buff to handle the filling
//		BuffAttribute* foodAttribute = new BuffAttribute(health, +amount,0,-(int)amount); 
//		Buff* foodBuff = Buff::SimpleBuff(playerObject, playerObject, duration*1000, mIcon, gWorldManager->GetCurrentGlobalTick());
//		foodBuff->AddAttribute(foodAttribute);	
//		playerObject->AddBuff(foodBuff,true);
//	}
//	else
//	{
//		playerObject->getHam()->updatePropertyValue(HamBar_Health,HamProperty_CurrentHitpoints,amount);
//		
//	}
//}
//
//void Food::_handleMind_Buff(PlayerObject* playerObject)
//{
//	uint32 amount = static_cast<uint32>(this->getAttribute<float>("attr_health"));
//	uint32 duration = 0;
//	if(this->hasAttribute("duration"))
//	{
//		duration = static_cast<uint32>(this->getAttribute<float>("duration"));
//		//get a stomach Buff to handle the filling
//		BuffAttribute* foodAttribute = new BuffAttribute(mind, +amount,0,-(int)amount); 
//		Buff* foodBuff = Buff::SimpleBuff(playerObject, playerObject, duration*1000, mIcon, gWorldManager->GetCurrentGlobalTick());
//		foodBuff->AddAttribute(foodAttribute);	
//		playerObject->AddBuff(foodBuff,true);
//	}
//	else
//	{
//		playerObject->getHam()->updatePropertyValue(HamBar_Health,HamProperty_CurrentHitpoints,amount);
//		
//	}
//}

void Food::_handleUses_Remaining(PlayerObject* playerObject)
{

	uint32 quantity = this->getAttribute<uint32>("counter_uses_remaining");
	quantity--;
	
	if(quantity)
	{
		toDelete = false;

		this->setAttributeIncDB("counter_uses_remaining",boost::lexical_cast<std::string>(quantity));

		//now update the uses display
		gMessageLib->sendUpdateUses(this,playerObject);
	}
	else
		toDelete = true;

}

//======================================================================================================================

bool						FoodCommandMapClass::mInsFlag = false;
FoodCommandMapClass*		FoodCommandMapClass::mSingleton = NULL;

//=

FoodCommandMapClass* FoodCommandMapClass::Init()
{
	if(mInsFlag == false)
	{
		mSingleton = new FoodCommandMapClass();
		mInsFlag = true;
		return mSingleton;
	}
	else
		return mSingleton;
}

FoodCommandMapClass::FoodCommandMapClass()
{
	/*mCommandMap.insert(std::make_pair(opAttributeUses_Remaining,&Food::_handleUses_Remaining));
	mCommandMap.insert(std::make_pair(opAttributeAttr_Health,&Food::_handleHealth_Buff));
	mCommandMap.insert(std::make_pair(opAttributeAttr_Mind,&Food::_handleMind_Buff));
	mCommandMap.insert(std::make_pair(BString::CRC("mask_scent"),&Food::_handleMask_Scent_Buff));*/
	
	
}
