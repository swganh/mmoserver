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

#ifndef ANH_ZONESERVER_CREATURE_EQUIPMANAGER_H
#define ANH_ZONESERVER_CREATURE_EQUIPMANAGER_H

//=============================================================================

#include "CreatureEnums.h"
#include "Object.h"
#include "Utils/typedefs.h"
#include <map>

//=============================================================================

typedef std::map<const CreatureEquipSlot,Object*> SlotMap;
typedef std::multimap<Object*, const CreatureEquipSlot> ObjectSlotMap;

//=============================================================================

class CreatureObject;
class Weapon;

//=============================================================================

class EquipManager
{
public:

    EquipManager();
    ~EquipManager();

    ObjectList* getEquippedObjects();

    // the slot map
    SlotMap* getSlotMap() {
        return &mSlotMap;
    }

    // creature parent reference
    CreatureObject*		getParent() {
        return mParent;
    }
    void				setParent(CreatureObject* creature) {
        mParent = creature;
    }

    // get an object by a slot given
    Object*				getEquippedObject(CreatureEquipSlot slot);

    // handle equipping by items equip mask
    bool				addEquippedObject(Object* object);
    uint8				removeEquippedObject(Object* object);

    // manually equip by slots given
    uint8				addEquippedObject(uint64 slotMask,Object* object);
    uint8				removeEquippedObject(uint64 slotMask);

    bool				addtoEquipList(Object* object);

    // clear the map
    void				clearSlotMap() {
        mSlotMap.clear();
    }

    // flow control
    uint32				getEquippedObjectsUpdateCounter() {
        return mEquippedObjectsUpdateCounter;
    }
    uint32				advanceEquippedObjectsUpdateCounter(uint32 c) {
        mEquippedObjectsUpdateCounter += c;
        return mEquippedObjectsUpdateCounter;
    }
    void				setEquippedObjectsUpdateCounter(uint32 c) {
        mEquippedObjectsUpdateCounter = c;
    }

    // default weapon handling
    Weapon*				getDefaultWeapon() {
        return mDefaultWeapon;
    }
    void				setDefaultWeapon(Weapon* weapon) {
        mDefaultWeapon = weapon;
    }
    bool				equipDefaultWeapon();

    // quick slot checks
    uint64				getEquipSlots() {
        return mEquipSlots;
    }
    void				setEquipSlots(uint64 slots) {
        mEquipSlots = slots;
    }
    void				occupyEquipSlot(CreatureEquipSlot slot) {
        mEquipSlots = mEquipSlots | slot;
    }
    void				freeEquipSlot(CreatureEquipSlot slot) {
        mEquipSlots = mEquipSlots & ~slot;
    }
    void				toggleEquipSlot(CreatureEquipSlot slot) {
        mEquipSlots = mEquipSlots ^ slot;
    }
    bool				checkEquipSlot(CreatureEquipSlot slot) {
        return((mEquipSlots & slot) == static_cast<uint32>(slot));
    }
    bool				checkEquipSlots(uint64 slots) {
        return((mEquipSlots & slots) == slots);
    }

    bool				EquipItem(Object* object);
    bool				unEquipItem(Object* object);
    bool				CheckEquipable(Object* object);

private:

    // slot / object references
    SlotMap					mSlotMap;
    ObjectSlotMap			mObjectMap;
    //ObjectList			mEquippedObjects;

    // reference for checking race/gender restrictions
    CreatureObject*		mParent; //- we have 35 Slots

    // slot bitmask for fast checks
    uint64				mEquipSlots;

    // flow control
    uint32				mEquippedObjectsUpdateCounter;

    // default weapon
    Weapon*				mDefaultWeapon;
};

//=============================================================================

#endif
