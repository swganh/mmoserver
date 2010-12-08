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

#ifndef ANH_ZONESERVER_TANGIBLE_OBJECT_H
#define ANH_ZONESERVER_TANGIBLE_OBJECT_H

#include "Object.h"
#include "TangibleEnums.h"

//=============================================================================

class TangibleObject :	public Object
{
    friend class ObjectFactory;
    friend class PlayerObjectFactory;
    friend class InventoryFactory;
    friend class NonPersistentItemFactory;

public:

    TangibleObject();
    TangibleObject(uint64 id,uint64 parentId,const BString model,TangibleGroup tanGroup = TanGroup_None,TangibleType tanType = TanType_None,const BString name = "",const BString nameFile = "",const BString detailFile = "");
    virtual ~TangibleObject();

    virtual void		prepareCustomRadialMenuInCell(CreatureObject* creatureObject, uint8 itemCount);

    virtual void		upDateFactoryVolume(const std::string& amount) {}
    BString				getName() const {
        return mName;
    }
    void				setName(const int8* name) {
        mName = name;
    }
    BString				getNameFile() const {
        return mNameFile;
    }
    void				setNameFile(const int8* file) {
        mNameFile = file;
    }
    BString				getDetailFile() {
        return mDetailFile;
    }
    void				setDetailFile(const int8* file) {
        mDetailFile = file;
    }
    BString				getColorStr() {
        return mColorStr;
    }
    void				setColorStr(const int8* coloStr) {
        mColorStr = coloStr;
    }

    //============================================================
    //sets ParentId - for item table objects in the db and Object
    virtual void		setParentIdIncDB(uint64 parentId);

    BString				getCustomizationStr() const {
        return mCustomizationStr;
    }
    void				setCustomizationStr(const uint8* custStr) {
        mCustomizationStr = (int8*)custStr;
    }
    void				setCustomization(uint8 index, uint16 val, uint8 length = 73) {
        mCustomization[index] = val;
        buildTanoCustomization(length);
    }
    uint16*				getCustomization() {
        return &mCustomization[0];
    }
    uint16				getCustomization(uint8 index) {
        return mCustomization[index];
    }
    void				buildTanoCustomization(uint8 len);

    BString				getUnknownStr1() const {
        return mUnknownStr1;
    }
    void				setUnknownStr1(const int8* unknownStr) {
        mUnknownStr1 = unknownStr;
    }
    BString				getUnknownStr2() const {
        return mUnknownStr2;
    }
    void				setUnknownStr2(const int8* unknownStr) {
        mUnknownStr2 = unknownStr;
    }

    BString				getCustomName() const {
        return mCustomName;
    }
    void				setCustomName(const int8* name) {
        mCustomName = name;
    }
    void				setCustomNameIncDB(const int8* name);

    TangibleGroup		getTangibleGroup() const {
        return mTanGroup;
    }
    void				setTangibleGroup(TangibleGroup group) {
        mTanGroup = group;
    }
    TangibleType		getTangibleType() const {
        return mTanType;
    }
    void				setTangibleType(TangibleType type) {
        mTanType = type;
    }

    uint32				getMaxCondition() const {
        return mMaxCondition;
    }
    void				setMaxCondition(uint32 maxCondition) {
        mMaxCondition = maxCondition;
    }
    uint32				getDamage() const {
        return mDamage;
    }
    void				setDamage(uint32 damage) {
        mDamage = damage;
    }

    virtual uint32		getCategoryBazaar() {
        return 0;
    }
    BString				getBazaarTang() {
        return getModelString();
    }

    float				getComplexity() {
        return mComplexity;
    }
    void				setComplexity(float complexity) {
        mComplexity = complexity;
    }

    void				initTimer(int32 count,int32 interval,uint64 startTime);
    bool				updateTimer(uint64 callTime);
    int32				getTimer() const {
        return mTimer;
    }
    void				setTimer(int32 timer) {
        mTimer = timer;
    }

    bool				getStatic()const {
        return mStatic;
    }
    void				setStatic(bool isStatic) {
        mStatic = isStatic;
    }
protected:

    BString				mCustomizationStr;
    BString				mCustomName;
    BString				mName;
    BString				mNameFile;
    BString				mColorStr;
    BString				mDetailFile;
    BString				mUnknownStr1;
    BString				mUnknownStr2;
    uint16				mCustomization[256];
    TangibleGroup		mTanGroup;
    TangibleType		mTanType;
    float				mComplexity;
    uint64				mLastTimerUpdate;
    int32				mTimer;
    uint32				mCustomizationSize;
    uint32				mDamage;
    uint32				mMaxCondition;
    uint32				mTimerInterval;
    bool				mStatic;
};

//=============================================================================

#endif

