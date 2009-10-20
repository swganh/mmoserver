/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "EquipManager.h"
#include "CreatureObject.h"
#include "Inventory.h"
#include "Object.h"
#include "Weapon.h"
#include "LogManager/LogManager.h"

//=============================================================================

EquipManager::EquipManager() :
mDefaultWeapon(NULL)
{
	mEquipSlots = 0;
}

//=============================================================================

EquipManager::~EquipManager()
{
	mSlotMap.clear();
	mEquippedObjects.clear();

	delete(mDefaultWeapon);
}

//=============================================================================
//
// remove an object from all slots it occupies, returns nr of occurances removed
//

uint8 EquipManager::removeEquippedObject(Object* object)
{
	// from slotmap
	uint8 occurance = 0;
	uint32 slotMask = object->getEquipSlotMask();

	SlotMap::iterator it = mSlotMap.begin();

	while(it != mSlotMap.end())
	{
		if((*it).second == object)
		{
			freeEquipSlot((*it).first);
			mSlotMap.erase(it++);

			++occurance;
		}
		else
		{
			++it;
		}
	}

	// from equiplist
	ObjectList::iterator eqIt = mEquippedObjects.begin();

	while(eqIt != mEquippedObjects.end())
	{
		if((*eqIt) == object)
		{
			mEquippedObjects.erase(eqIt);

			break;
		}

		++eqIt;
	}

	return(occurance);
}

//=============================================================================
//
// remove object(s) from the slotmask given
//

uint8 EquipManager::removeEquippedObject(uint32 slotMask)
{
	// from slotmask
	SlotMap::iterator	it;
	uint8				occurance	= 0;
	Object*				object		= NULL;

	for(uint32 slot = 1; slot < CREATURE_MAX_SLOT;slot = slot << 1)
	{
		if((slotMask & slot) == slot)
		{
			it = mSlotMap.find((CreatureEquipSlot)slot);

			if(it != mSlotMap.end())
			{
				// save it to remove from equiplist later
				object = (*it).second;
				freeEquipSlot((CreatureEquipSlot)slot);

				mSlotMap.erase(it);

				++occurance;
			}

		}
	}

	// from equiplist
	if(object)
	{
		ObjectList::iterator eqIt = mEquippedObjects.begin();

		while(eqIt != mEquippedObjects.end())
		{
			if((*eqIt) == object)
			{
				mEquippedObjects.erase(eqIt);

				break;
			}

			++eqIt;
		}
	}

	return(occurance);
}

//=============================================================================
//
// return an object by a slot given
//

Object* EquipManager::getEquippedObject(CreatureEquipSlot slot)
{
	SlotMap::iterator it = mSlotMap.find(slot);

	if(it != mSlotMap.end())
	{
		return((*it).second);
	}

	return(NULL);
}

//=============================================================================
//
// add an object according to its slot definitions, remove objects with slot conflicts
//

bool EquipManager::addEquippedObject(Object* object)
{
	SlotMap::iterator	it;
	uint8				occurance	= 0;
	uint32				slotMask	= object->getEquipSlotMask();

	if(Object* object = getEquippedObject(CreatureEquipSlot_Weapon))
	//if we want to equip a weapon we need to remove the default unarmed first
	if(slotMask == CreatureEquipSlot_Weapon)
	{
		removeEquippedObject(CreatureEquipSlot_Weapon);
	}

	//if we want to equip a helmet make sure we unequip hair first
	if(slotMask == CreatureEquipSlot_Hair)
	{
		removeEquippedObject(CreatureEquipSlot_Hair);
	}

	if (checkEquipSlots(slotMask))
	{
		//gLogger->logMsgF("EquipManager::addEquippedObject: Character: %lld Object: %lld : slot already taken!!!",MSG_NORMAL,mParent->getId(),object->getId());
		//gLogger->logMsgF("Inventory::unEquipItem : object slot already filled slotmask : %u", MSG_NORMAL,slotMask);
		//return(false);
	}

	// make sure we have a slot descriptor
	if(!slotMask)
	{
		gLogger->logMsgF("EquipManager::addEquippedObject: Character: %lld Object: %lld : no slot mask set",MSG_NORMAL,mParent->getId(),object->getId());

		return(false);
	}

	for(uint32 slot = 1; slot < CREATURE_MAX_SLOT;slot = slot << 1)
	{
		if((slotMask & slot) == slot)
		{
			// free the occupied slot(s)
			it = mSlotMap.find((CreatureEquipSlot)slot);
			if(it != mSlotMap.end() && slotMask != CreatureEquipSlot_Rider)
			{
				//return false;
				//unequip it! and update the inventory about it
				Inventory*		inventory		=	dynamic_cast<Inventory*>(mParent->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
				inventory->unEquipItem((*it).second);
			}

			// add the object
			mSlotMap.insert(std::make_pair((CreatureEquipSlot)slot,object));

			++occurance;
		}
	}

	setEquipSlots(slotMask);

	// add to equiplist
	addtoEquipList(object);

	return(true);
}

//=============================================================================
//
// add an object manually to the slots given, remove objects with slot conflicts
//

uint8 EquipManager::addEquippedObject(uint32 slotMask,Object* object)
{
	SlotMap::iterator	it;
	uint8				occurance = 0;

	for(uint32 slot = 1; slot < CREATURE_MAX_SLOT;slot = slot << 1)
	{
		if((slotMask & slot) == slot)
		{
			// free the occupied slot(s)
			it = mSlotMap.find((CreatureEquipSlot)slot);

			if(it != mSlotMap.end())
			{
				removeEquippedObject((*it).second);
			}

			// add the object
			mSlotMap.insert(std::make_pair((CreatureEquipSlot)slot,object));

			++occurance;
		}
	}

	// add to equiplist
	addtoEquipList(object);

	return(occurance);
}

//=============================================================================

bool EquipManager::addtoEquipList(Object* object)
{
	ObjectList::iterator eqIt = mEquippedObjects.begin();

	bool found = false;
	while(eqIt != mEquippedObjects.end())
	{
		if((*eqIt) == object)
		{
			found = true;
			return false;
		}

		++eqIt;
	}

	if(!found)
		mEquippedObjects.push_back(object);

	return true;
}
bool EquipManager::equipDefaultWeapon()
{
	// make sure slot is empty
	if(Object* object = getEquippedObject(CreatureEquipSlot_Weapon))
	{
		return(false);
		//removeEquippedObject(CreatureEquipSlot_Weapon);
	}

	// equip the default weapon
	addEquippedObject(CreatureEquipSlot_Weapon,mDefaultWeapon);
	return(true);
}

//=============================================================================


