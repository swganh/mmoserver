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

#include "ZoneServer/EquipManager.h"



#include "Utils/logger.h"

#include "MessageLib/MessageLib.h"

#include "ZoneServer/PlayerObject.h"
#include "ZoneServer/SpatialIndexManager.h"
#include "ZoneServer/Inventory.h"
#include "ZoneServer/Object.h"
#include "ZoneServer/Weapon.h"
#include "ZoneServer/WorldManager.h"


EquipManager::EquipManager() :
    mDefaultWeapon(NULL)
{
    mEquipSlots = 0;
}

//=============================================================================

EquipManager::~EquipManager()
{
    ObjectList* objList = getEquippedObjects();
    ObjectList::iterator it = objList->begin();

    while(it != objList->end())
    {
        if(gWorldManager->existObject((*it)))
        {
            gWorldManager->destroyObject((*it));
        }
        else
        {
            SAFE_DELETE(*it);
        }
        it++;
    }

    delete objList;

    mSlotMap.clear();
    mObjectMap.clear();
}

ObjectList* EquipManager::getEquippedObjects()
{
    ObjectList* result = new ObjectList();
    ObjectSlotMap::iterator it = mObjectMap.begin();

    while(it != mObjectMap.end())
    {
        ObjectList::iterator ot = result->begin();

        bool hasObject = false;
        while(ot != result->end())
        {
            if((*ot) == (*it).first)
            {
                hasObject = true;
            }
            ot++;
        }

        if(!hasObject)
            result->push_front(it->first);


        it++;
    }

    return result;
}




//=============================================================================
//
// remove an object from all slots it occupies, returns nr of occurances removed
//

uint8 EquipManager::removeEquippedObject(Object* object)
{
    return removeEquippedObject(object->getEquipSlotMask());
}

//=============================================================================
//
// remove object(s) from the slotmask given
//

uint8 EquipManager::removeEquippedObject(uint64 slotMask)
{
    uint8 occurance = 0;

    for(uint64 slot = 1; slot < CREATURE_MAX_SLOT; slot = slot << 1)
    {
        if((slotMask & slot) == slot)
        {
            SlotMap::iterator	it = mSlotMap.find((CreatureEquipSlot)slot);

            if(it != mSlotMap.end())
            {
                Object*	object = (*it).second;

                freeEquipSlot((CreatureEquipSlot)slot);

                mSlotMap.erase(it);

                ObjectSlotMap::iterator ot = mObjectMap.find(object);
                if(ot != mObjectMap.end())
                    mObjectMap.erase(ot);

                ++occurance;
            }

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
    if(!slot || mSlotMap.empty()) { //Crash bug patch: http://paste.swganh.org/viewp.php?id=20100627004133-026ea7b07136cfad7a5463216da5ab96
        //gLogger->log(LogManager::WARNING,"EquipManager::getEquippedObject could not locate slot given(%u), or the SlotMap was empty.",slot);
        //slotmap is regularly empty when we test for the left hand slot in the playerobjectfactory
        return(NULL);
    }
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
// the return is NOT irrelevant as it might not have a valid slot attached

bool EquipManager::addEquippedObject(Object* object)
{
    int8 result = addEquippedObject(object->getEquipSlotMask(), object);
    return(result != 0);
}

//=============================================================================
//
// add an object manually to the slots given, remove objects with slot conflicts
//

uint8 EquipManager::addEquippedObject(uint64 slotMask,Object* object)
{
    SlotMap::iterator	it;
    ObjectSlotMap::iterator ot;
    uint8			occurance = 0;

    for(uint64 slot = 1; slot < CREATURE_MAX_SLOT; slot = slot << 1)
    {
        if((slotMask & slot) == slot)
        {
            // free the occupied slot(s)
            it = mSlotMap.find((CreatureEquipSlot)slot);
            if(it != mSlotMap.end())
            {
                ot = mObjectMap.find(it->second);

                //Correct the First Map
                it->second = object;

                //Correct th Second Map
                mObjectMap.erase(ot);
                mObjectMap.insert(std::make_pair(object, (CreatureEquipSlot)slot));

                ++occurance;
            }
            else
            {
                mSlotMap.insert(std::make_pair((CreatureEquipSlot)slot, object));
                mObjectMap.insert(std::make_pair(object, (CreatureEquipSlot)slot));
            }
        }
    }

    return(occurance);
}

//=============================================================================
bool EquipManager::equipDefaultWeapon()
{
    // make sure slot is empty
    if(getEquippedObject(CreatureEquipSlot_Hold_Left))
    {
        return(false);
        //removeEquippedObject(CreatureEquipSlot_Weapon);
    }

    // equip the default weapon
    addEquippedObject(CreatureEquipSlot_Hold_Left,mDefaultWeapon);
    return(true);
}

//=============================================================================

bool EquipManager::EquipItem(Object* object)
{
    if(!CheckEquipable(object))
        return false;

    PlayerObject*	owner		= dynamic_cast<PlayerObject*> (getParent());
    Item* item = dynamic_cast<Item*>(object);

    if(!item || !owner)
        return false;

    addEquippedObject(object);

    //Update the Equipped List
    gMessageLib->sendEquippedListUpdate_InRange(owner);

    // weapon update
    if(item->getItemFamily() == ItemFamily_Weapon)
    {
        gMessageLib->sendWeaponIdUpdate(owner);
    }

    return true;
}

//=============================================================================

bool EquipManager::unEquipItem(Object* object)
{

    Item* item = dynamic_cast<Item*>(object);
    if(!item)
    {
        DLOG(WARNING) << "Inventory::unEquipItem : No Item object ID : " << object->getId();
        return false;
    }

    PlayerObject*	owner		= dynamic_cast<PlayerObject*> (this->getParent());
    if(!owner)
    {
        DLOG(WARNING) << "Inventory::unEquipItem : No one has it equipped";
        return false;
    }

    //client forces us to stop performing at this point as he unequips the instrument if he is not dancing
    if((item->getItemFamily() == ItemFamily_Instrument) && (owner->getPerformingState() != PlayerPerformance_Dance))
    {
        gEntertainerManager->stopEntertaining(owner);
    }

    //equipped objects are always contained by the Player
    //unequipped ones by the inventory!

    Inventory*		inventory		=	dynamic_cast<Inventory*>(getEquippedObject(CreatureEquipSlot_Inventory));
    uint64			parentId		=	inventory->getId();


    //gMessageLib->sendDestroyObject_InRange(object->getId(),owner,false);
    gMessageLib->sendEquippedListUpdate_InRange(owner);

    removeEquippedObject(object);

    //check whether the hairslot is now free
    TangibleObject*				playerHair		= dynamic_cast<TangibleObject*>(owner->getHair());//dynamic_cast<TangibleObject*>(customer->getEquipManager()->getEquippedObject(CreatureEquipSlot_Hair));
    TangibleObject*				playerHairSlot	= dynamic_cast<TangibleObject*>(getEquippedObject(CreatureEquipSlot_Hair));
    if((!playerHairSlot)&&playerHair)
    {
        //if we have hair equip it
        addEquippedObject(CreatureEquipSlot_Hair,playerHair);
    }

    // if we unequiped our weapon, set the unarmed default weapon
    if(item->getItemFamily() == ItemFamily_Weapon && (item->getEquipSlotMask() & CreatureEquipSlot_Hold_Left) == CreatureEquipSlot_Hold_Left)
    {
        equipDefaultWeapon();
    }

    if(item->getItemFamily() == ItemFamily_Weapon)
    {
        gMessageLib->sendWeaponIdUpdate(owner);
    }

    return true;
}
//=============================================================================

bool EquipManager::CheckEquipable(Object* object)
{
    Item* item = dynamic_cast<Item*>(object);
    PlayerObject*	owner		= dynamic_cast<PlayerObject*> (getParent());

    if(!item)
    {
        return(false);
    }

    if(!owner)
    {
        return(false);
    }

    // don't equip music instruments or weapons while performing

    if((owner->getPerformingState())&&((item->getItemFamily() == ItemFamily_Instrument) || (item->getItemFamily() == ItemFamily_Weapon)))
    {
        return(false);
    }

    // check items equip restrictions, first check race and gender
    uint64 filter1 = item->getEquipRestrictions() & 0xFFF;
    uint64 filter2 = owner->getRaceGenderMask() & 0xFFF;

    if((filter1 & filter2) != filter2)
    {
        gMessageLib->SendSystemMessage(L"You can't equip this item.", owner);
        return(false);
    }

    // then check, if we need to be jedi
    filter1 = item->getEquipRestrictions() & 0xF000;
    filter2 = owner->getRaceGenderMask() & 0xF000;

    if(filter1 && !filter2)
    {
        gMessageLib->SendSystemMessage(L"You can't equip this item.", owner);
        return(false);
    }

    // then faction
    filter1 = item->getEquipRestrictions() & 0xF0000;

    if((filter1 == 0x10000 && strcmp(owner->getFaction().getAnsi(),"rebel") != 0)
            || (filter1 == 0x20000 && strcmp(owner->getFaction().getAnsi(),"imperial") != 0))
    {
        gMessageLib->SendSystemMessage(L"You can't equip this item.", owner);
        return(false);
    }

    uint64 filter3 = CreatureEquipSlot_Datapad & CreatureEquipSlot_Bank & CreatureEquipSlot_Inventory & CreatureEquipSlot_Mission;
    if(filter3 && item->getEquipSlotMask())
    {
        gMessageLib->SendSystemMessage(L"Attention!!! the Equip - BitMask is messedup.", owner);
        return(false);
    }
    return true;
}
