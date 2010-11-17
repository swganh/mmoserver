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

#ifndef ANH_ZONESERVER_ITEM_H
#define ANH_ZONESERVER_ITEM_H

#include "TangibleObject.h"
#include "Item_Enums.h"

//=============================================================================

class PlayerObject;

class Item : public TangibleObject
{
    friend class ItemFactory;
    friend class NonPersistantObjectFactory;

public:

    Item();
    virtual ~Item();

    //----------------------------------------
    //family is the items family it belongs to
    uint32	getItemFamily() {
        return mItemFamily;
    }
    void	setItemFamily(uint32 family) {
        mItemFamily = family;
    }

    //----------------------------------------
    //thats the specific type!
    uint32	getItemType() {
        return mItemType;
    }
    void	setItemType(uint32 type) {
        mItemType = type;
    }

    uint32	getDynamicInt32() {
        return mDynamicInt32;
    }
    void	setDynamicInt32(uint32 value) {
        mDynamicInt32 = value;
    }

    virtual void	handleObjectMenuSelect(uint8 messageType,Object* srcObject);
    virtual BString	getBazaarName() {
        int8 name[128];
        sprintf(name,"@%s:%s",getNameFile().getAnsi(),getName().getAnsi());
        return name;
    }

    virtual void	prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount);
    //virtual void	prepareCustomRadialMenuInCell(CreatureObject* creatureObject, uint8 itemCount);
    virtual void	ResetRadialMenu() {
        mRadialMenu.reset();
        RadialMenu* radial	= NULL;
        RadialMenuPtr radialPtr(radial);
        mRadialMenu = radialPtr;
    }
    virtual void	updateWorldPosition();

    //temporary Instruments
    bool	getPlaced() {
        return mPlaced;
    }
    void	setPlaced(bool placed) {
        mPlaced = placed;
    }

    uint64	getNonPersistantCopy() {
        return mNonPersistantCopy;
    }
    void	setNonPersistantCopy(uint64 copy) {
        mNonPersistantCopy = copy;
    }

    //gets the id of the persistant copy
    uint64	getPersistantCopy() {
        return mPersistantCopy;
    }
    void	setPersistantCopy(uint64 copy) {
        mPersistantCopy = copy;
    }

    uint64	getOwner() {
        return mOwner;
    }
    void	setOwner(uint64 owner) {
        mOwner = owner;
    }

    uint32	getLoadCount() {
        return mLoadCount;
    }
    void	setLoadCount(uint32 c) {
        mLoadCount = c;
    }


protected:


    uint32	mItemFamily;
    uint32	mItemType;
    uint32	mDynamicInt32;

    //temporary Instruments
    uint64  mNonPersistantCopy;
    uint64	mPersistantCopy;
    uint64	mOwner;
    bool	mPlaced;

    //sitable objects flag 1 = sit. 0 = no sit
    bool	mIsSitable;

    uint32	mLoadCount;

};

//=============================================================================

#endif

