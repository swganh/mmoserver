/*
 This file is part of MMOServer. For more information, visit http://swganh.com
 
 Copyright (c) 2006 - 2011 The SWG:ANH Team

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef ANH_ZONESERVER_TANGIBLE_INVENTORY_H
#define ANH_ZONESERVER_TANGIBLE_INVENTORY_H

#include "TangibleObject.h"
#include "ObjectFactoryCallback.h"

#include <vector>

typedef std::vector<std::pair<BString,uint64> >	SortedInventoryItemList;
class CreatureObject;
class PlayerObject;

//=============================================================================

class Inventory : public TangibleObject
{
    friend class ObjectFactory;
    friend class PlayerObjectFactory;
    friend class InventoryFactory;

public:

    Inventory();
    ~Inventory();

    // inherited callback
    virtual void	handleObjectReady(Object* object,DispatchClient* client);

    void			setParent(CreatureObject* creature) {
        mParent = creature;
    }

    uint32			getObjectLoadCounter() {
        return mObjectLoadCounter;
    }
    void			setObjectLoadCounter(uint32 count) {
        mObjectLoadCounter = count;
    }

    int32			getCredits() {
        return mCredits;
    }
    void			setCredits(int32 credits) {
        mCredits = credits;
    }
    bool			updateCredits(int32 amount);

    //depecrated - use check(get/set) capicity instead
    uint8			getMaxSlots() {
        return mMaxSlots;
    }
    bool			checkSlots(uint8 amount);
    void			setMaxSlots(uint8 slots) {
        mMaxSlots = slots;
    }

    virtual bool	checkCapacity(uint8 amount, PlayerObject* player = NULL, bool sendMsg = true);

    void			getUninsuredItems(SortedInventoryItemList* insuranceList);
    void			getInsuredItems(SortedInventoryItemList* insuranceList);
    bool			itemExist(uint32 familyId, uint32 typeId);

private:

    CreatureObject*		mParent;
    uint32				mObjectLoadCounter;

    int32				mCredits;
    uint8				mMaxSlots;
};

//=============================================================================

#endif

