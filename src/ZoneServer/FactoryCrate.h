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

#ifndef ANH_ZONESERVER_TANGIBLE_FACTORY_CRATE_H
#define ANH_ZONESERVER_TANGIBLE_FACTORY_CRATE_H

#include "Item.h"

//=============================================================================

class FactoryCrate : public Item
{
    friend class ItemFactory;

public:

    FactoryCrate();
    virtual ~FactoryCrate();


    virtual void	prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount);
    virtual void	handleObjectMenuSelect(uint8 messageType,Object* srcObject);
    void			sendAttributes(PlayerObject* playerObject);

    virtual void	upDateFactoryVolume(const std::string& amount);

    TangibleObject*	getLinkedObject();
    int32			decreaseContent(uint32 amount);

    //the amount of stuff *virtually* in us while iterating through the slots while crafting
    uint32			getTempAmount() {
        return mTempAmount;
    }
    uint32			decTempAmount(uint32 c) {
        if(c>mTempAmount) return 0;
        return mTempAmount-c;
    }
    void			incTempAmount(uint32 c) {
        mTempAmount+=c;
    }
    void			setTempAmount(uint32 c) {
        mTempAmount= c;
    }

    //the amount of slots were linked to
    uint32			getSlotCount() {
        return mTempAmount;
    }
    uint32			decSlotCount(uint32 c) {
        if(c>mTempAmount) {
            mTempAmount = 0;
            return 0;
        }
        mTempAmount-=c;
        return mTempAmount-c;
    }
    void			incSlotCount(uint32 c) {
        mTempAmount+=c;
    }
    void			setSlotCount(uint32 c) {
        mTempAmount= c;
    }

private:
    uint32 mTempAmount;
    uint32 mSlotCount;
};

//=============================================================================

#endif

