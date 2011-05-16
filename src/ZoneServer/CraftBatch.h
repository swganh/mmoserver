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

#ifndef ANH_ZONESERVER_CRAFTBATCH_H
#define ANH_ZONESERVER_CRAFTBATCH_H

#include "Utils/bstring.h"
#include "Utils/typedefs.h"
#include <vector>

class CraftWeight;
class CraftAttribute;
class CraftAttributeWeight;

typedef std::vector<CraftWeight*>		CraftWeights;
typedef std::vector<CraftAttribute*>	CraftAttributes;
typedef std::vector<CraftAttributeWeight*>	CraftAttributeWeights;

//=============================================================================

class CraftBatch
{
    friend class SchematicManager;

public:

    CraftBatch();
    ~CraftBatch();

    uint32					getId() {
        return mId;
    }
    uint32					getListId() {
        return mListId;
    }
    uint32					getExpGroup() {
        return mExpGroup;
    }
    CraftWeights*			getCraftWeights() {
        return &mWeights;
    }
    CraftAttributes*		getCraftAttributes() {
        return &mAttributes;
    }
    uint32  getSchemWeightBatch() {
        return mSchemWeightBatch;
    }
private:

    uint32					mId;
    uint32					mListId;
    uint32					mExpGroup;
    CraftWeights			mWeights;
    CraftAttributes			mAttributes;
    uint32  mSchemWeightBatch;
};

//=============================================================================

class CraftWeight
{
    friend class SchematicManager;

public:

    CraftWeight() {}
    ~CraftWeight() {}

    //returns the resource attribute like oq/pe etc
    uint8	getDataType() {
        return mDataType;
    }
    float	getDistribution() {
        return mDistribution;
    }
    uint32  getSchemWeightBatch() {
        return mSchemWeightBatch;
    }
    uint32  getBatchListId() {
        return mBatchListId;
    }
private:

    uint8	mDataType;
    float	mDistribution;
    uint32  mSchemWeightBatch;
    uint32  mBatchListId;
};

//=============================================================================

class CraftAttribute
{
    friend class SchematicManager;

public:

    CraftAttribute() {}
    ~CraftAttribute() {}

    uint32	getAttributeId() {
        return mAttributeId;
    }
    BString	getAttributeKey() {
        return mAttributeKey;
    }
    float	getMin() {
        return mMin;
    }
    float	getMax() {
        return mMax;
    }
    uint32  getSchemWeightBatch() {
        return mSchemWeightBatch;
    }
    //determines whether the attributes value is integer or float ()draft_craft_item_attribute_link attribute_type
    uint8	getType() {
        return mType;
    }
    uint32  getListId() {
        return mListId;
    }
private:
    uint32  mSchemWeightBatch;
    uint32  mListId;
    uint32	mAttributeId;
    BString	mAttributeKey;
    uint8	mType;
    float	mMin;
    float	mMax;
};

enum AttributePPME_Enum
{
    AttributePPME_AddValue = 1,
    AttributePPME_AddAttribute = 2
};

class CraftAttributeWeight
{
    friend class SchematicManager;

public:

    CraftAttributeWeight() {}
    ~CraftAttributeWeight() {}

    uint32	getAttributeId() {
        return mAttributeId;
    }
    BString	getAttributeKey() {
        return mAttributeKey;
    }

    uint32	getAffectedAttributeId() {
        return mAffectedAttributeId;
    }
    BString	getAffectedAttributeKey() {
        return mAffectedAttributeKey;
    }

    uint32	getManipulation() {
        return mManipulation;
    }

    uint32  getSchemWeightBatch() {
        return mSchemWeightBatch;
    }
private:
    uint32  mSchemWeightBatch;
    uint32	mAttributeId;
    BString	mAttributeKey;

    uint32	mAffectedAttributeId;
    BString	mAffectedAttributeKey;

    AttributePPME_Enum	mManipulation;
    uint8	mType;

};

//=============================================================================

#endif


