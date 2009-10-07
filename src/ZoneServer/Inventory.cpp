/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "MathLib/Quaternion.h"
#include "WorldManager.h"
#include "Inventory.h"
#include "DatabaseManager/Database.h"
#include "PlayerObject.h"
#include "MessageLib/MessageLib.h"
#include "ResourceContainer.h"
#include "CraftingTool.h"


//=============================================================================

Inventory::Inventory() : TangibleObject()
{
	mTanGroup = TanGroup_Inventory;
	mObjectLoadCounter = 1000;
	mObjects.reserve(80);
}

//=============================================================================
//
// just removes all objects, dont need to send destroys to the client, since it always should be erased along with a parent
//
Inventory::~Inventory()
{
	// gLogger->logMsgF("Inventory::~Inventory() No of object = %u", MSG_HIGH, mObjects.size());
	uint32 itemCounter = 0;

	ObjectList::iterator	invObjectIt = mObjects.begin();

	Object* object;

	while(invObjectIt != mObjects.end())
	{
		itemCounter++;	// Used for debugging purpose, to see is to beleive... instead of ignoring the obvious.

		object = (*invObjectIt);
		//this crashes repeatedly when I stop the server - might it be related to our crashes on log out ??

		// Sure it crash!!! Thats because the item in the inventory is owned by the WorldManager,
		// and if the WorldManager already have removed it before we remove the inventory, as we may do when shutting down the server,
		// we will get a crash when attempting to delete a dangling pointer...
		// As simple as that!

		// There is  one good soultion to this: Store handles to objects (the id's) instead of object references (pointers).
		
		// A less good solution is to protects all code that can risk the use of dangling pointers with exception handling,
		// but I doubt that code will be easier to read or more efficient than retrieving the object when we need access to it.

		if(CraftingTool* tool = dynamic_cast<CraftingTool*>(object))
		{
			if(tool->getCurrentItem())
			{
				gWorldManager->removeBusyCraftTool(tool);

				//save status is not necessary as the status already is in the db and updated on status change
				//crafttool saves time to db on update
			
				
			}

			// todo: save left time and status to db
		}
	


		gWorldManager->destroyObject(object);
		
		invObjectIt = mObjects.erase(invObjectIt);
	}


	invObjectIt = mEquippedObjects.begin();

	while(invObjectIt != mEquippedObjects.end())
	{
		Object* object = (*invObjectIt);

		gWorldManager->destroyObject(object);
		
		invObjectIt = mEquippedObjects.erase(invObjectIt);
	}
}

//=============================================================================

void Inventory::removeObject(Object* object)
{
	ObjectList::iterator it = mObjects.begin();
	while(it != mObjects.end())
	{
		if((*it) == object)
		{
			mObjects.erase(it);
			return;
		}
		++it;
	}
	gLogger->logMsgF("cant find object %s",MSG_HIGH,object->getModelString().getAnsi());
}

void Inventory::addObject(Object* object)
{
	ObjectList::iterator it = mObjects.begin();
	while(it != mObjects.end())
	{
		if((*it) == object)
		{
			gLogger->logMsgF(" object %s already in list",MSG_HIGH,object->getModelString().getAnsi());
			return;
		}
		++it;
	}

	mObjects.push_back(object); 
}

//============================================================================
// checks whether we have enough space in the inventory to accomodate
// amount items
bool Inventory::checkSlots(uint8 amount)
{
	// please note that just counting the amount of items is bound to be faulty as certain items
	// can (in theory) occupy more than one slot
	// we should iterate the list and count each items volume!
	// please note too, that Im not certain of the clients involvement of this
	// at this point the inventories max capacity is stored in the db table inventory_types
	if((mMaxSlots - getObjects()->size()) >= amount)
		return true;

	gLogger->logMsgF("Inventory::checkslots(): Inventory full : max Inv capacity :%u, current capacity %u, nr of items we tried to add", MSG_NORMAL, mMaxSlots,getObjects()->size(),amount);
	return false;
}

//=============================================================================

void Inventory::removeEquippedObject(Object* object)
{
	ObjectList::iterator it = mEquippedObjects.begin();
	while(it != mEquippedObjects.end())
	{
		if((*it) == object)
		{
			mEquippedObjects.erase(it);
			return;
		}
		++it;
	}
	gLogger->logMsgF("cant find equipped object %s",MSG_HIGH,object->getModelString().getAnsi());
}

//=============================================================================

void Inventory::deleteObject(Object* object)
{
	ObjectList::iterator it = mObjects.begin();
	
	while(it != mObjects.end())
	{
		if((*it) == object)
		{
			gWorldManager->destroyObject(object);

			mObjects.erase(it);
			return;
		}
		++it;
	}
}

//=============================================================================

Object* Inventory::getObjectById(uint64 objId)
{
	ObjectList::iterator it = mObjects.begin();
	while(it != mObjects.end())
	{
		if((*it)->getId() == objId)
			return(*it);
		++it;
	}
	gLogger->logMsgF("Inventory::getObjectById Item %I64u not found",MSG_HIGH,objId);
	return(NULL);
}

//=============================================================================

bool Inventory::updateCredits(int32 amount)
{
	if(mCredits + amount < 0)
		return(false);

	mCredits += amount;

	if(mParent->getType() == ObjType_Player)
		gMessageLib->sendInventoryCreditsUpdate(dynamic_cast<PlayerObject*>(mParent));

	gWorldManager->getDatabase()->ExecuteSqlAsync(NULL,NULL,"UPDATE inventories set credits=credits+%i WHERE id=%lld",amount,mId);

	return(true);
}

//=============================================================================

void Inventory::handleObjectReady(Object* object,DispatchClient* client)
{
	if(TangibleObject* tangibleObject = dynamic_cast<TangibleObject*>(object))
	{
		// reminder: objects are owned by the global map, inventory only keeps references
		
		
		//generally we presume that objects are created UNEQUIPPED
		gWorldManager->addObject(object,true);

		mObjects.push_back(object);

		// send the creates, if we are owned by a player
		if(PlayerObject* player = dynamic_cast<PlayerObject*>(mParent))
		{
			// could be a resource container, need to check this first, since it inherits from tangible
			if(ResourceContainer* resCont = dynamic_cast<ResourceContainer*>(object))
			{
				//resource containers are stored in the db under the player ID
				//however, it is important that the inventory contains them
				resCont->setParentId(this->getId());
				gMessageLib->sendCreateResourceContainer(resCont,player);
			}
			// or a tangible
			else
			{
				object->setParentId(this->getId());
				gMessageLib->sendCreateTangible(tangibleObject,player);
			}
		}

		//check if we are a busy crafting tool
		if(CraftingTool* tool = dynamic_cast<CraftingTool*>(object))
		{
			if(tool->getCurrentItem())
			{
				PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(this->getParentId()));
				gMessageLib->sendUpdateTimer(tool,player);
			}
		}
	}
}

//=============================================================================

bool Inventory::EquipItem(Object* object)
{
	Item* item = dynamic_cast<Item*>(object);
	
	if(!item)
	{
		gLogger->logMsgF("Inventory::EquipItem : No Item object ID : %I64u", MSG_NORMAL,object->getId());
		return(false);
	}

	if(!object->hasInternalAttribute("equipped"))
	{
		gLogger->logMsgF("Inventory::EquipItem : object not equipable object ID : %I64u", MSG_NORMAL,object->getId());
		return(false);
	}

	if(object->getInternalAttribute<bool>("equipped"))
	{
		gLogger->logMsgF("Inventory::EquipItem : object is already equipped object ID : %I64u", MSG_NORMAL,object->getId());
		return(false);
	}

	PlayerObject*	owner		= dynamic_cast<PlayerObject*> (gWorldManager->getObjectById(this->getParentId()));
	
	if(!owner)
	{
		gLogger->logMsgF("Inventory::EquipItem : No owner Inventory ID : %I64u", MSG_NORMAL,this->getId());
		return(false);
	}

	// don't equip music instruments or weapons while performing
		
	if((owner->getPerformingState())&&((item->getItemFamily() == ItemFamily_Instrument) || (item->getItemFamily() == ItemFamily_Weapon)))
	{
		return(false);
	}

	// check items equip restrictions, first check race and gender
	uint32 filter1 = item->getEquipRestrictions() & 0xFFF;
	uint32 filter2 = owner->getRaceGenderMask() & 0xFFF;

	if((filter1 & filter2) != filter2)
	{
		gMessageLib->sendSystemMessage(owner,L"You can't equip this item.");
		return(false);
	}

	// then check, if we need to be jedi
	filter1 = item->getEquipRestrictions() & 0xF000;
	filter2 = owner->getRaceGenderMask() & 0xF000;

	if(filter1 && !filter2)
	{
		gMessageLib->sendSystemMessage(owner,L"You can't equip this item.");
		return(false);
	}

	// then faction
	filter1 = item->getEquipRestrictions() & 0xF0000;
	
	if((filter1 == 0x10000 && strcmp(owner->getFaction().getAnsi(),"rebel") != 0)
	|| (filter1 == 0x20000 && strcmp(owner->getFaction().getAnsi(),"imperial") != 0))
	{
		gMessageLib->sendSystemMessage(owner,L"You can't equip this item.");
		return(false);
	}
						
	//check whether the slot is already filled
	if(!owner->getEquipManager()->addEquippedObject(item))
	{
	//	gMessageLib->sendSystemMessage(owner,L"You already wear something there you nutter.");
		return(false);
	}

	gLogger->logMsgF("Inventory::EquipItem : owner ID : %I64u\n", MSG_NORMAL,owner->getId());
	//equipped objects are always contained by the Player
	//unequipped ones by the inventory!

	//now remove from inventories unequipped list
	this->removeObject(object);
	//and add to inventories equipped list
	this->addEquippedObject(object);
	
	uint64			parentId	= this->getParentId();

	object->setParentId(parentId);
	object->setInternalAttribute("equipped","1");

	gMessageLib->sendContainmentMessage(object->getId(),parentId,4,owner);
	gMessageLib->sendEquippedListUpdate_InRange(owner);

	PlayerObjectSet*			inRangePlayers	= owner->getKnownPlayers();
	PlayerObjectSet::iterator	it				= inRangePlayers->begin();

	while(it != inRangePlayers->end())
	{
		PlayerObject* targetObject = (*it);
		gMessageLib->sendCreateTangible(item,targetObject);
		++it;
	}

	// weapon update
	if(item->getItemFamily() == ItemFamily_Weapon)
	{
		gMessageLib->sendWeaponIdUpdate(owner);
	}
	
	int8 sql[256];
	//set the equipped attribute to unequipped
	sprintf(sql,"UPDATE swganh.item_attributes ia INNER JOIN swganh.attributes a ON a.id = ia.attribute_id SET ia.value = '1' WHERE ia.item_id= %I64u AND a.name = 'equipped'", object->getId());
	gWorldManager->getDatabase()->ExecuteSqlAsync(NULL,NULL,sql);

	sprintf(sql,"UPDATE swganh.items  SET parent_id = '%I64u' WHERE id= %I64u ", parentId, object->getId());
	gWorldManager->getDatabase()->ExecuteSqlAsync(NULL,NULL,sql);
	
	return(true);
}

//=============================================================================

void Inventory::unEquipItem(Object* object)
{
	if(!object->hasInternalAttribute("equipped"))
	{
		gLogger->logMsgF("Inventory::unEquipItem : object not equipable object ID : %I64u", MSG_NORMAL,object->getId());
		return;
	}

	if(!object->getInternalAttribute<bool>("equipped"))
	{
		gLogger->logMsgF("Inventory::unEquipItem : object is unequiped object ID : %I64u", MSG_NORMAL,object->getId());
		return;
	}

	Item* item = dynamic_cast<Item*>(object);
	
	if(!item)
	{
		gLogger->logMsgF("Inventory::unEquipItem : No Item object ID : %I64u", MSG_NORMAL,object->getId());
		return;
	}

	PlayerObject*	owner		= dynamic_cast<PlayerObject*> (gWorldManager->getObjectById(this->getParentId()));
	
	if(!owner)
	{
		gLogger->logMsgF("Inventory::unEquipItem : No owner Inventory ID : %I64u", MSG_NORMAL,this->getId());
		return;
	}

	//0client forces us to stop performing at this point as he unequips the instrument regardless of what we do
	if((item->getItemFamily() == ItemFamily_Instrument) && (owner->getPerformingState() != PlayerPerformance_None))
	{
		gEntertainerManager->stopEntertaining(owner);
	}


	gLogger->logMsgF("Inventory::unEquipItem : owner ID : %I64u", MSG_NORMAL,owner->getId());
	//equipped objects are always contained by the Player
	//unequipped ones by the inventory!

	Inventory*		inventory		=	dynamic_cast<Inventory*>(owner->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
	uint64			parentId		=	inventory->getId();

	//the object is now in the inventory
	object->setParentId(this->getId());

	//now remove from the equipped list
	this->removeEquippedObject(object);

	//and add to inventories regular (unequipped) list
	this->addObject(object);

	object->setInternalAttribute("equipped","0");
	
	owner->getEquipManager()->removeEquippedObject(object);

	//check whether the hairslot is now free
	TangibleObject*				playerHair		= dynamic_cast<TangibleObject*>(owner->getHair());//dynamic_cast<TangibleObject*>(customer->getEquipManager()->getEquippedObject(CreatureEquipSlot_Hair));
	TangibleObject*				playerHairSlot	= dynamic_cast<TangibleObject*>(owner->getEquipManager()->getEquippedObject(CreatureEquipSlot_Hair));
	if((!playerHairSlot)&&playerHair)
	{
		//if we have hair equip it
		owner->getEquipManager()->addEquippedObject(CreatureEquipSlot_Hair,playerHair);
	}


	gMessageLib->sendContainmentMessage_InRange(object->getId(),parentId,0xffffffff,owner);
	gMessageLib->sendDestroyObject_InRange(object->getId(),owner,false);
	gMessageLib->sendEquippedListUpdate_InRange(owner);

	
	int8 sql[256];
	//set the equipped attribute to unequipped
	sprintf(sql,"UPDATE swganh.item_attributes ia INNER JOIN swganh.attributes a ON a.id = ia.attribute_id SET ia.value = '0' WHERE ia.item_id= %I64u AND a.name = 'equipped'", object->getId());
	gWorldManager->getDatabase()->ExecuteSqlAsync(NULL,NULL,sql);

	sprintf(sql,"UPDATE swganh.items  SET parent_id = '%I64u' WHERE id= %I64u ", parentId, object->getId());
	gWorldManager->getDatabase()->ExecuteSqlAsync(NULL,NULL,sql);

	if(item->getItemFamily() == ItemFamily_Instrument)	
	{
		if(owner->getPerformingState() == PlayerPerformance_Music)
		{
			//unequipping the instrument we play
			gEntertainerManager->stopEntertaining(owner);
		}
	}

	// if we unequiped our weapon, set the unarmed default weapon
	if(item->getItemFamily() == ItemFamily_Weapon && (item->getEquipSlotMask() & CreatureEquipSlot_Weapon) == CreatureEquipSlot_Weapon)
	{
		owner->getEquipManager()->equipDefaultWeapon();
	}

	if(item->getItemFamily() == ItemFamily_Weapon)
	{
		gMessageLib->sendWeaponIdUpdate(owner);
	}
}

//=============================================================================
//
// Build a list with uninsured items. (in inventory or equipped, that are un-insured, but can be insured)
//
// The list is sorted by the "@item_n:survey_tool_wind" names,
// making item of same "family" be grouped toghter.
//

void Inventory::getUninsuredItems(SortedInventoryItemList* insuranceList)
{
	// Clear the insurance list.
	insuranceList->clear();

	ObjectList::iterator invObjectIt = mObjects.begin();

	// Items inside inventory and child objects.
	while (invObjectIt != mObjects.end())
	{
		Object* object = (*invObjectIt);
		if (object->hasInternalAttribute("insured"))
		{
			// gLogger->logMsgF("Inventory::insuranceListCreate: Found item with insurance attribute inside the inventory: %llu", MSG_NORMAL,object->getId());
			if (!object->getInternalAttribute<bool>("insured"))
			{
				// Add the item to the insurance list.
				// gLogger->logMsgF("Inventory::insuranceListCreate: Found an uninsured item inside Inventory: %llu", MSG_NORMAL,object->getId());
				
				// Handle the list.
				if (object->hasAttribute("original_name"))
				{
					SortedInventoryItemList::iterator it = insuranceList->begin();
					string itemName((int8*)object->getAttribute<std::string>("original_name").c_str());
					for (uint32 index = 0; index < insuranceList->size(); index++)
					{
						if (_strcmpi(itemName.getAnsi(), (*it).first.getAnsi()) < 0)
						{
							break;
						}
						it++;
					}
					insuranceList->insert(it, std::make_pair(itemName,object->getId()));
				}
			}
		}
		invObjectIt++;
	}

	// Items equipped by the player.
	invObjectIt = mEquippedObjects.begin();

	while (invObjectIt != mEquippedObjects.end())
	{
		Object* object = (*invObjectIt);
		if (object->hasInternalAttribute("insured"))
		{
			// gLogger->logMsgF("Inventory::insuranceListCreate: Found equipped item with insurance attribute: %llu", MSG_NORMAL,object->getId());
			if (!object->getInternalAttribute<bool>("insured"))
			{
				// Add the item to the insurance list.
				// gLogger->logMsgF("Inventory::insuranceListCreate: Found an uninsured equipped item: %llu", MSG_NORMAL,object->getId());
				
				// Handle the list.
				if (object->hasAttribute("original_name"))
				{
					SortedInventoryItemList::iterator it = insuranceList->begin();
					string itemName((int8*)object->getAttribute<std::string>("original_name").c_str());
					for (uint32 index = 0; index < insuranceList->size(); index++)
					{
						if (_strcmpi(itemName.getAnsi(), (*it).first.getAnsi()) < 0)
						{
							break;
						}
						it++;
					}
					insuranceList->insert(it, std::make_pair(itemName,object->getId()));
				}
			}
		}
		invObjectIt++;
	}
}

//=============================================================================
//
// Build a list with insured items. (in inventory or equipped)
//
// The list is sorted by the "@item_n:survey_tool_wind" names,
// making item of same "family" be grouped toghter.
//

void Inventory::getInsuredItems(SortedInventoryItemList* insuranceList)
{
	// Clear the insurance list.
	insuranceList->clear();

	ObjectList::iterator invObjectIt = mObjects.begin();

	// Items inside inventory and child objects.
	while (invObjectIt != mObjects.end())
	{
		Object* object = (*invObjectIt);
		if (object->hasInternalAttribute("insured"))
		{
			// gLogger->logMsgF("Inventory::insuranceListCreate: Found item with insurance attribute inside the inventory: %llu", MSG_NORMAL,object->getId());
			if (object->getInternalAttribute<bool>("insured"))
			{
				// Add the item to the insurance list.
				// gLogger->logMsgF("Inventory::insuranceListCreate: Found an insured item inside Inventory: %llu", MSG_NORMAL,object->getId());
				
				// Handle the list.
				if (object->hasAttribute("original_name"))
				{
					SortedInventoryItemList::iterator it = insuranceList->begin();
					string itemName((int8*)object->getAttribute<std::string>("original_name").c_str());
					for (uint32 index = 0; index < insuranceList->size(); index++)
					{
						if (_strcmpi(itemName.getAnsi(), (*it).first.getAnsi()) < 0)
						{
							break;
						}
						it++;
					}
					insuranceList->insert(it, std::make_pair(itemName,object->getId()));
				}
			}
		}
		invObjectIt++;
	}

	// Items equipped by the player.
	invObjectIt = mEquippedObjects.begin();

	while (invObjectIt != mEquippedObjects.end())
	{
		Object* object = (*invObjectIt);
		if (object->hasInternalAttribute("insured"))
		{
			// gLogger->logMsgF("Inventory::insuranceListCreate: Found equipped item with insurance attribute: %llu", MSG_NORMAL,object->getId());
			if (object->getInternalAttribute<bool>("insured"))
			{
				// Add the item to the insurance list.
				// gLogger->logMsgF("Inventory::insuranceListCreate: Found an insured equipped item: %llu", MSG_NORMAL,object->getId());
				
				// Handle the list.
				if (object->hasAttribute("original_name"))
				{
					SortedInventoryItemList::iterator it = insuranceList->begin();
					string itemName((int8*)object->getAttribute<std::string>("original_name").c_str());
					for (uint32 index = 0; index < insuranceList->size(); index++)
					{
						if (_strcmpi(itemName.getAnsi(), (*it).first.getAnsi()) < 0)
						{
							break;
						}
						it++;
					}
					insuranceList->insert(it, std::make_pair(itemName,object->getId()));
				}
			}
		}
		invObjectIt++;
	}
}

//=============================================================================
bool Inventory::itemExist(uint32 familyId, uint32 typeId)
{
	bool found = false;
	ObjectList::iterator invObjectIt = mObjects.begin();

	// Items inside inventory and child objects.
	while (invObjectIt != mObjects.end())
	{
		Object* object = (*invObjectIt);
		Item* item = dynamic_cast<Item*>(object);
		if (item)
		{
			if ((item->getItemFamily() == familyId) && (item->getItemType() == typeId))
			{
				found = true;
				break;
			}
		}
		invObjectIt++;
	}

	if (!found)
	{
		// Items equipped by the player.
		invObjectIt = mEquippedObjects.begin();

		while (invObjectIt != mEquippedObjects.end())
		{
			Object* object = (*invObjectIt);
			Item* item = dynamic_cast<Item*>(object);
			if (item)
			{
				if ((item->getItemFamily() == familyId) && (item->getItemType() == typeId))
				{
					found = true;
					break;
				}
			}
			invObjectIt++;
		}
	}
	return found;
}

