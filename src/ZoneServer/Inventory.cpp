/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "Inventory.h"
#include "CraftingTool.h"
#include "PlayerObject.h"
#include "ResourceContainer.h"
#include "WorldManager.h"
#include "MessageLib/MessageLib.h"
#include "DatabaseManager/Database.h"
#include "MathLib/Quaternion.h"
#include "Utils/utils.h"

//=============================================================================

Inventory::Inventory() : TangibleObject()
{
	mTanGroup = TanGroup_Inventory;
	mObjectLoadCounter = 1000;
}

//=============================================================================
//
// just removes all objects, dont need to send destroys to the client, since it always should be erased along with a parent
//
Inventory::~Inventory()
{

	//TODO:have the equipped Objects move to the player
	ObjectList::iterator	invObjectIt;


	invObjectIt = mEquippedObjects.begin();

	while(invObjectIt != mEquippedObjects.end())
	{
		Object* object = (*invObjectIt);

		gWorldManager->destroyObject(object);

		invObjectIt = mEquippedObjects.erase(invObjectIt);
	}
}

//=============================================================================


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

bool Inventory::updateCredits(int32 amount)
{
	if(mCredits + amount < 0)
		return(false);

	mCredits += amount;

	if(mParent->getType() == ObjType_Player)
		gMessageLib->sendInventoryCreditsUpdate(dynamic_cast<PlayerObject*>(mParent));

	gWorldManager->getDatabase()->ExecuteSqlAsync(NULL,NULL,"UPDATE inventories set credits=credits+%i WHERE id=%"PRIu64"",amount,mId);

	return(true);
}

//=============================================================================

void Inventory::handleObjectReady(Object* object,DispatchClient* client)
{
	TangibleObject* tangibleObject = dynamic_cast<TangibleObject*>(object);
	if(!tangibleObject)
	{
		gLogger->logMsgF("Inventory::handleObjectReady : Not a tangible ???", MSG_NORMAL);
		assert(false);
		return;
	}
	
	// reminder: objects are owned by the global map, inventory only keeps references

	//generally we presume that objects are created UNEQUIPPED
	//equipped objects are handled through the playerfactory on load
	gWorldManager->addObject(object,true);//true means its not added to the si!!

	// send the creates, if we are owned by a player
	if(PlayerObject* player = dynamic_cast<PlayerObject*>(mParent))
	{
		addObject(object,player);
	}

	else
		addObjectSecure(object);
	
}

//=============================================================================

bool Inventory::EquipItemTest(Object* object)
{
	Item* item = dynamic_cast<Item*>(object);
	PlayerObject*	owner		= dynamic_cast<PlayerObject*> (gWorldManager->getObjectById(this->getParentId()));

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

	return(true);
}

bool Inventory::EquipItem(Object* object)
{
	if(!EquipItemTest(object))
		return false;
	

	PlayerObject*	owner		= dynamic_cast<PlayerObject*> (gWorldManager->getObjectById(this->getParentId()));
	Item* item = dynamic_cast<Item*>(object);

	owner->getEquipManager()->addEquippedObject(object);

	gLogger->logMsgF("Inventory::EquipItem : owner ID : %I64u", MSG_NORMAL,owner->getId());
	//equipped objects are always contained by the Player
	//unequipped ones by the inventory!

	//and add to inventories equipped list   - move this to playerobject at one point
	this->addEquippedObject(object);

	uint64			parentId	= this->getParentId();

	//update containment and db
	object->setParentId(parentId,4,owner,true);

	//update the relevant attribute and the db 
	object->setInternalAttributeIncDB("equipped","1");

	gMessageLib->sendEquippedListUpdate_InRange(owner);

	//create it for all nearby players
	PlayerObjectSet*			inRangePlayers	= owner->getKnownPlayers();
	gMessageLib->sendCreateTangible(item,inRangePlayers);

	// weapon update
	if(item->getItemFamily() == ItemFamily_Weapon)
	{
		gMessageLib->sendWeaponIdUpdate(owner);
	}

	return(true);
}

//=============================================================================

bool Inventory::unEquipItem(Object* object)
{
	if(!object->hasInternalAttribute("equipped"))
	{
		gLogger->logMsgF("Inventory::unEquipItem : object not equipable object ID : %"PRIu64"", MSG_NORMAL,object->getId());
		gLogger->logMsgF("Inventory::unEquipItem : likely playerHair", MSG_NORMAL,object->getId());
		return false;
	}

	if(!object->getInternalAttribute<bool>("equipped"))
	{
		gLogger->logMsgF("Inventory::unEquipItem : object is unequiped object ID : %"PRIu64"", MSG_NORMAL,object->getId());
		return false;
	}

	Item* item = dynamic_cast<Item*>(object);

	if(!item)
	{
		gLogger->logMsgF("Inventory::unEquipItem : No Item object ID : %"PRIu64"", MSG_NORMAL,object->getId());
		return false;
	}

	PlayerObject*	owner		= dynamic_cast<PlayerObject*> (gWorldManager->getObjectById(this->getParentId()));

	if(!owner)
	{
		gLogger->logMsgF("Inventory::unEquipItem : No owner Inventory ID : %"PRIu64"", MSG_NORMAL,this->getId());
		return false;
	}

	//0client forces us to stop performing at this point as he unequips the instrument regardless of what we do
	if((item->getItemFamily() == ItemFamily_Instrument) && (owner->getPerformingState() != PlayerPerformance_None))
	{
		gEntertainerManager->stopEntertaining(owner);
	}


	gLogger->logMsgF("Inventory::unEquipItem : owner ID : %"PRIu64"", MSG_NORMAL,owner->getId());
	//equipped objects are always contained by the Player
	//unequipped ones by the inventory!

	Inventory*		inventory		=	dynamic_cast<Inventory*>(owner->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
	uint64			parentId		=	inventory->getId();

	//the object is now in the inventory
	//update the containment for owner and db
	object->setParentId(inventory->getId(), 0xffffffff, owner, true);
	
	//destroy for everyone in range
	gMessageLib->sendDestroyObject_InRange(object->getId(),owner,false);
	gMessageLib->sendEquippedListUpdate_InRange(owner);

	//now remove from the equipped list
	this->removeEquippedObject(object);

	//and add to inventories regular (unequipped) list
	//this->addObjectSecure(object); the transferhandler will put it wherever necessary

	object->setInternalAttributeIncDB("equipped","0");

	owner->getEquipManager()->removeEquippedObject(object);

	//check whether the hairslot is now free
	TangibleObject*				playerHair		= dynamic_cast<TangibleObject*>(owner->getHair());//dynamic_cast<TangibleObject*>(customer->getEquipManager()->getEquippedObject(CreatureEquipSlot_Hair));
	TangibleObject*				playerHairSlot	= dynamic_cast<TangibleObject*>(owner->getEquipManager()->getEquippedObject(CreatureEquipSlot_Hair));
	if((!playerHairSlot)&&playerHair)
	{
		//if we have hair equip it
		owner->getEquipManager()->addEquippedObject(CreatureEquipSlot_Hair,playerHair);
	}

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

	return true;
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

	ObjectIDList::iterator invObjectIt = getObjects()->begin();

	// Items inside inventory and child objects.
	while (invObjectIt != getObjects()->end())
	{
		Object* object = gWorldManager->getObjectById((*invObjectIt));
		if (object&&object->hasInternalAttribute("insured"))
		{
			// gLogger->logMsgF("Inventory::insuranceListCreate: Found item with insurance attribute inside the inventory: %"PRIu64"", MSG_NORMAL,object->getId());
			if (!object->getInternalAttribute<bool>("insured"))
			{
				// Add the item to the insurance list.
				// gLogger->logMsgF("Inventory::insuranceListCreate: Found an uninsured item inside Inventory: %"PRIu64"", MSG_NORMAL,object->getId());

				// Handle the list.
				if (object->hasAttribute("original_name"))
				{
					SortedInventoryItemList::iterator it = insuranceList->begin();
					string itemName((int8*)object->getAttribute<std::string>("original_name").c_str());
					for (uint32 index = 0; index < insuranceList->size(); index++)
					{
						if (Anh_Utils::cmpistr(itemName.getAnsi(), (*it).first.getAnsi()) < 0)
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
	ObjectList::iterator equippedObjectIt = mEquippedObjects.begin();

	while (equippedObjectIt != mEquippedObjects.end())
	{
		Object* object = (*equippedObjectIt);
		if (object&&object->hasInternalAttribute("insured"))
		{
			// gLogger->logMsgF("Inventory::insuranceListCreate: Found equipped item with insurance attribute: %"PRIu64"", MSG_NORMAL,object->getId());
			if (!object->getInternalAttribute<bool>("insured"))
			{
				// Add the item to the insurance list.
				// gLogger->logMsgF("Inventory::insuranceListCreate: Found an uninsured equipped item: %"PRIu64"", MSG_NORMAL,object->getId());

				// Handle the list.
				if (object->hasAttribute("original_name"))
				{
					SortedInventoryItemList::iterator it = insuranceList->begin();
					string itemName((int8*)object->getAttribute<std::string>("original_name").c_str());
					for (uint32 index = 0; index < insuranceList->size(); index++)
					{
						if (Anh_Utils::cmpistr(itemName.getAnsi(), (*it).first.getAnsi()) < 0)
						{
							break;
						}
						it++;
					}
					insuranceList->insert(it, std::make_pair(itemName,object->getId()));
				}
			}
		}
		equippedObjectIt++;
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

	ObjectIDList::iterator invObjectIt = getObjects()->begin();

	// Items inside inventory and child objects.
	while (invObjectIt != getObjects()->end())
	{
		Object* object = gWorldManager->getObjectById((*invObjectIt));
		if (object&&object->hasInternalAttribute("insured"))
		{
			// gLogger->logMsgF("Inventory::insuranceListCreate: Found item with insurance attribute inside the inventory: %"PRIu64"", MSG_NORMAL,object->getId());
			if (object->getInternalAttribute<bool>("insured"))
			{
				// Add the item to the insurance list.
				// gLogger->logMsgF("Inventory::insuranceListCreate: Found an insured item inside Inventory: %"PRIu64"", MSG_NORMAL,object->getId());

				// Handle the list.
				if (object->hasAttribute("original_name"))
				{
					SortedInventoryItemList::iterator it = insuranceList->begin();
					string itemName((int8*)object->getAttribute<std::string>("original_name").c_str());
					for (uint32 index = 0; index < insuranceList->size(); index++)
					{
						if (Anh_Utils::cmpistr(itemName.getAnsi(), (*it).first.getAnsi()) < 0)
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
	ObjectList::iterator equippedObjectIt = mEquippedObjects.begin();

	while (equippedObjectIt != mEquippedObjects.end())
	{
		Object* object = (*equippedObjectIt);
		if (object->hasInternalAttribute("insured"))
		{
			// gLogger->logMsgF("Inventory::insuranceListCreate: Found equipped item with insurance attribute: %"PRIu64"", MSG_NORMAL,object->getId());
			if (object->getInternalAttribute<bool>("insured"))
			{
				// Add the item to the insurance list.
				// gLogger->logMsgF("Inventory::insuranceListCreate: Found an insured equipped item: %"PRIu64"", MSG_NORMAL,object->getId());

				// Handle the list.
				if (object->hasAttribute("original_name"))
				{
					SortedInventoryItemList::iterator it = insuranceList->begin();
					string itemName((int8*)object->getAttribute<std::string>("original_name").c_str());
					for (uint32 index = 0; index < insuranceList->size(); index++)
					{
						if (Anh_Utils::cmpistr(itemName.getAnsi(), (*it).first.getAnsi()) < 0)
						{
							break;
						}
						it++;
					}
					insuranceList->insert(it, std::make_pair(itemName,object->getId()));
				}
			}
		}
		equippedObjectIt++;
	}
}

//=============================================================================
bool Inventory::itemExist(uint32 familyId, uint32 typeId)
{
	bool found = false;
	ObjectIDList::iterator invObjectIt = getObjects()->begin();

	// Items inside inventory and child objects.
	while (invObjectIt != getObjects()->end())
	{
		Object* object = getObjectById(*invObjectIt);
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
		ObjectList::iterator equippedObjectIt = mEquippedObjects.begin();

		while (equippedObjectIt != mEquippedObjects.end())
		{
			Object* object = (*equippedObjectIt);
			Item* item = dynamic_cast<Item*>(object);
			if (item)
			{
				if ((item->getItemFamily() == familyId) && (item->getItemType() == typeId))
				{
					found = true;
					break;
				}
			}
			equippedObjectIt++;
		}
	}
	return found;
}

//=============================================================================
//check for the containers capacity and return a fitting error message if necessary
//

bool Inventory::checkCapacity(uint8 amount, PlayerObject* player, bool sendMsg)
{
	if(player&&(getCapacity()-getObjects()->size() < amount))
	{
		if(sendMsg)
			gMessageLib->sendSystemMessage(player,L"","error_message","inv_full");
		return false;
	}

	return true;
}