/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

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
#include "MathLib/Quaternion.h"

//=============================================================================

Food::Food() : Item()
{
	toDelete = false;
	
}

//=============================================================================

Food::~Food()
{
}

void Food::prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount)
{
	// gLogger->logMsg("Food::prepareCustomRadialMenu(");
	RadialMenu* radial	= new RadialMenu();
		
	CreatureObject* unknownCreature;
	Inventory* creatureInventory;
	if (this->getParentId() &&
		(unknownCreature = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById(this->getParentId() - 1))) &&
		(creatureInventory = dynamic_cast<Inventory*>(unknownCreature->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory))) &&
		(creatureInventory->getId() == this->getParentId()))
	{
		// Its an object in an inventory
		// gLogger->logMsgF("Food::prepareCustomRadialMenu creatureInventory", MSG_NORMAL);

		NPCObject* npcObject = dynamic_cast<NPCObject*>(unknownCreature);
		if (npcObject)
		{
			// gLogger->logMsgF("Food::prepareCustomRadialMenu npcObject", MSG_NORMAL);
			if ((npcObject->getNpcFamily() == NpcFamily_AttackableCreatures) && npcObject->isDead())
			{
				// gLogger->logMsgF("Food::prepareCustomRadialMenu NpcFamily_AttackableCreatures and DEAD", MSG_NORMAL);
				// I'm pretty sure we are a loot item.
				radial->addItem(1,0,radId_itemPickup,radAction_ObjCallback,"@ui_radial:loot");
				radial->addItem(2,0,radId_examine,radAction_Default);
				mRadialMenu = RadialMenuPtr(radial);
				return;
			}
		}
	}

	// gLogger->logMsgF("Food::prepareCustomRadialMenu Standard radial stuff", MSG_NORMAL);
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
					// gLogger->logMsgF("Item::handleObjectMenuSelect: Use food",MSG_NORMAL);
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

	//we need to start by checking whether our stomach isnt full

	//we need to update our stomach
	uint32 filling = 0;

	if(this->hasInternalAttribute("food_icon"))
	{
		mIcon = 0;
		mIcon = this->getInternalAttribute<uint32>("food_icon");					
	}

	if(this->hasAttribute("stomach_food"))
	{
		gMessageLib->sendSystemMessage(playerObject, L"Food Filling applied.");
		filling = this->getAttribute<uint32>("stomach_food");					
		
		//do we still have place for it ?
		if(!playerObject->getStomach()->checkFood(filling))
		{
			gMessageLib->sendSystemMessage(playerObject, L"","error_message","full_food");
			return;
		}
		
		//get a stomach Buff to handle the filling
		BuffAttribute* foodAttribute = new BuffAttribute(Food_Filling, +filling,0,-(int)filling); 
		Buff* foodBuff = Buff::SimpleBuff(playerObject, playerObject, 300000, 0, gWorldManager->GetCurrentGlobalTick());
		foodBuff->AddAttribute(foodAttribute);	
		playerObject->AddBuff(foodBuff,true);

		if(playerObject->getGender())
		{
			gMessageLib->sendPlayMusicMessage(WMSound_Eat_Human_Female,playerObject);
		}
		else
		{
			gMessageLib->sendPlayMusicMessage(WMSound_Eat_Human_Male,playerObject);
		}

	}

	if(this->hasAttribute("stomach_drink"))
	{
		uint32 filling = 0;
		gMessageLib->sendSystemMessage(playerObject, L"Drink Filling applied.");
		filling = this->getAttribute<uint32>("stomach_drink");					
		
		//do we still have place for it ?
		if(!playerObject->getStomach()->checkDrink(filling))
		{
			gMessageLib->sendSystemMessage(playerObject, L"","error_message","full_drink");
			return;
		}

		//get a stomach Buff to handle the filling
		BuffAttribute* foodAttribute = new BuffAttribute(Drink_Filling, +filling,0,-(int)filling); 
		Buff* foodBuff = Buff::SimpleBuff(playerObject, playerObject, 300000, 0, gWorldManager->GetCurrentGlobalTick());
		foodBuff->AddAttribute(foodAttribute);	
		playerObject->AddBuff(foodBuff,true);
		
		if(playerObject->getGender())
		{
			gMessageLib->sendPlayMusicMessage(WMSound_Drink_Human_Female,playerObject);
		}
		else
		{
			gMessageLib->sendPlayMusicMessage(WMSound_Drink_Human_Male,playerObject);
		}

	}


	AttributeOrderList::iterator	orderIt = mAttributeOrderList.begin();

	while(orderIt != mAttributeOrderList.end())
	{
		uint32 key = *orderIt;

		FoodCommandMap::iterator it = gFoodCmdMap.find(key);

		if(it != gFoodCmdMap.end())
		{
			(this->*((*it).second))(playerObject);
		}
		else
		{
			//gLogger->logMsgF("Food::processAttribute: Unhandled Attribute 0x%x for %"PRIu64"",MSG_NORMAL,key,this->getId());
		
		}
		++orderIt;
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
		inventory->deleteObject(this);
		
		return;

	}
		
	return;
}

void Food::_handleHealth_Buff(PlayerObject* playerObject)
{
	uint32 amount = static_cast<uint32>(this->getAttribute<float>("attr_health"));
	uint32 duration = 0;
	if(this->hasAttribute("duration"))
	{
		duration = static_cast<uint32>(this->getAttribute<float>("duration"));
		//get a stomach Buff to handle the filling
		BuffAttribute* foodAttribute = new BuffAttribute(Health, +amount,0,-(int)amount); 
		Buff* foodBuff = Buff::SimpleBuff(playerObject, playerObject, duration*10, mIcon, gWorldManager->GetCurrentGlobalTick());
		foodBuff->AddAttribute(foodAttribute);	
		playerObject->AddBuff(foodBuff,true);
	}
	else
	{
		playerObject->getHam()->updatePropertyValue(HamBar_Health,HamProperty_CurrentHitpoints,amount);
		
	}
}

void Food::_handleMind_Buff(PlayerObject* playerObject)
{
	uint32 amount = static_cast<uint32>(this->getAttribute<float>("attr_health"));
	uint32 duration = 0;
	if(this->hasAttribute("duration"))
	{
		duration = static_cast<uint32>(this->getAttribute<float>("duration"));
		//get a stomach Buff to handle the filling
		BuffAttribute* foodAttribute = new BuffAttribute(Mind, +amount,0,-(int)amount); 
		Buff* foodBuff = Buff::SimpleBuff(playerObject, playerObject, duration*10, mIcon, gWorldManager->GetCurrentGlobalTick());
		foodBuff->AddAttribute(foodAttribute);	
		playerObject->AddBuff(foodBuff,true);
	}
	else
	{
		playerObject->getHam()->updatePropertyValue(HamBar_Health,HamProperty_CurrentHitpoints,amount);
		
	}
}

void Food::_handleUses_Remaining(PlayerObject* playerObject)
{
	gMessageLib->sendSystemMessage(playerObject, L"Uses Remaining updated.");

	uint32 quantity = this->getAttribute<uint32>("counter_uses_remaining");
	quantity--;
	
	if(quantity)
	{
		toDelete = false;

		this->setAttribute("counter_uses_remaining",boost::lexical_cast<std::string>(quantity));
		gWorldManager->getDatabase()->ExecuteSqlAsync(0,0,"UPDATE item_attributes SET value='%u' WHERE item_id=%"PRIu64" AND attribute_id=%u",quantity,this->getId(),AttrType_CounterUsesRemaining);

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
	mCommandMap.insert(std::make_pair(opAttributeUses_Remaining,&Food::_handleUses_Remaining));
	mCommandMap.insert(std::make_pair(opAttributeAttr_Health,&Food::_handleHealth_Buff));
	mCommandMap.insert(std::make_pair(opAttributeAttr_Mind,&Food::_handleMind_Buff));
	
	
}