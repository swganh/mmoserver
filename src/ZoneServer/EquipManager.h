/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_CREATURE_EQUIPMANAGER_H
#define ANH_ZONESERVER_CREATURE_EQUIPMANAGER_H

//=============================================================================

#include "CreatureEnums.h"
#include "Utils/typedefs.h"
#include "Object.h"
#include "ObjectContainer.h"
#include <map>

//=============================================================================

typedef std::map<CreatureEquipSlot,Object*> SlotMap;

//=============================================================================

class CreatureObject;
class Weapon;

//=============================================================================

class EquipManager
{
	public:

		EquipManager();
		~EquipManager();

		// the slot map
		SlotMap*			getSlotMap(){ return &mSlotMap; }

		// all equipped objects, kept to save filtering of the slotmap, since items may occupy multiple slots
		ObjectList*			getEquippedObjects(){ return &mEquippedObjects; }

		// creature parent reference
		CreatureObject*		getParent(){ return mParent; }
		void				setParent(CreatureObject* creature){ mParent = creature; }

		// get an object by a slot given
		Object*				getEquippedObject(CreatureEquipSlot slot);

		// handle equipping by items equip mask
		bool				addEquippedObject(Object* object);
		uint8				removeEquippedObject(Object* object);

		// manually equip by slots given
		uint8				addEquippedObject(uint32 slotMask,Object* object);
		uint8				removeEquippedObject(uint32 slotMask);

		bool				addtoEquipList(Object* object);

		// clear the map
		void				clearSlotMap(){ mSlotMap.clear(); }
		
		// flow control
		uint32				getEquippedObjectsUpdateCounter(){ return mEquippedObjectsUpdateCounter; }
		uint32				advanceEquippedObjectsUpdateCounter(uint32 c){ mEquippedObjectsUpdateCounter += c; return mEquippedObjectsUpdateCounter; }
		void				setEquippedObjectsUpdateCounter(uint32 c){ mEquippedObjectsUpdateCounter = c;}
		void				resetEquippedObjectsUpdateCounter(){ mEquippedObjectsUpdateCounter = mEquippedObjects.size(); }

		// default weapon handling
		Weapon*				getDefaultWeapon(){ return mDefaultWeapon; }
		void				setDefaultWeapon(Weapon* weapon){ mDefaultWeapon = weapon; }
		bool				equipDefaultWeapon();

		// quick slot checks
		uint32				getEquipSlots(){ return mEquipSlots; }
		void				setEquipSlots(uint32 slots){ mEquipSlots = slots; }
		void				occupyEquipSlot(CreatureEquipSlot slot){ mEquipSlots = mEquipSlots | slot; }
		void				freeEquipSlot(CreatureEquipSlot slot){ mEquipSlots = mEquipSlots & ~slot; }
		void				toggleEquipSlot(CreatureEquipSlot slot){ mEquipSlots = mEquipSlots ^ slot; }
		bool				checkEquipSlot(CreatureEquipSlot slot){ return((mEquipSlots & slot) == slot); }
		bool				checkEquipSlots(uint32 slots){ return((mEquipSlots & slots) == slots); }

	private:

		// slot / object references
		SlotMap				mSlotMap;
		ObjectList			mEquippedObjects;

		// reference for checking race/gender restrictions
		CreatureObject*		mParent;

		// slot bitmask for fast checks
		uint32				mEquipSlots;

		// flow control
		uint32				mEquippedObjectsUpdateCounter;

		// default weapon
		Weapon*				mDefaultWeapon;
};

//=============================================================================

#endif