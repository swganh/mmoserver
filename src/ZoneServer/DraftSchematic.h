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

#ifndef ANH_ZONESERVER_DRAFTSCHEMATIC_H
#define ANH_ZONESERVER_DRAFTSCHEMATIC_H

#include "Utils/bstring.h"
#include "Utils/typedefs.h"

class CraftBatch;
class DraftSlot;
class WeightsBatch;
class CraftAttributeWeight;

typedef std::vector<DraftSlot*>		DraftSlots;
typedef std::vector<WeightsBatch*>	WeightsBatches;
typedef std::vector<CraftBatch*>	CraftBatches;
typedef std::vector<CraftAttributeWeight*>	CraftAttributeWeights;

//=============================================================================

enum SchematicLoadState
{
    DSLoadState_Loading				= 1,
    DSLoadState_Loaded				= 2,
    DSLoadState_Slots				= 3,
    DSLoadState_Assembly_Weights	= 4,
    DSLoadState_Experiment_Weights	= 5
};

//=============================================================================

class DraftSchematic
{
    friend class SchematicManager;

public:

    DraftSchematic();
    ~DraftSchematic();

    SchematicLoadState	getLoadState() {
        return mLoadState;
    }
    void				setLoadState(SchematicLoadState ls) {
        mLoadState = ls;
    }
    uint64				getId() {
        return mId;
    }
    void				setId(uint64 id) {
        mId = id;
    }

    uint32				getCrc() {
        return mCrc;
    }
    void				setCrc(uint32 crc) {
        mCrc = crc;
    }

    uint32				getWeightsBatchId() {
        return mWeightsBatchId;
    }
    void				setWeightsBatchId(uint32 id) {
        mWeightsBatchId = id;
    }
    BString				getModel() {
        return mModel;
    }
    void				setModel(BString model) {
        mModel = model;
    }
    DraftSlots*			getDraftSlots() {
        return &mDraftSlots;
    }
    WeightsBatches*		getAssemblyWeights() {
        return &mAssemblyBatch;
    }
    WeightsBatches*		getExperimentWeights() {
        return &mExperimentBatch;
    }
    CraftBatches*		getCraftBatches() {
        return &mCraftBatch;
    }
    uint32				getComplexity() {
        return mComplexity;
    }
    void				setComplexity(uint32 comp) {
        mComplexity = comp;
    }
    uint32				getDataSize() {
        return mDataSize;
    }
    void				setDataSize(uint32 datasize) {
        mDataSize = datasize;
    }
    uint32				getGroupId() {
        return mGroupId;
    }
    void				setGroupId(uint32 value) {
        mGroupId=value;
    }
    WeightsBatch*		getAssemblyWeightsBatchByListId(uint32 listId);
    WeightsBatch*		getExperimentWeightsBatchByListId(uint32 listId);
    CraftBatch*			getCraftBatchByListId(uint32 listId);
    //the subcategory tells us what category the schem is under in the crafting tool
    uint32				getSubCategory() {
        return mSubCategory;
    }
    void				setSubCategory(uint32 category) {
        mSubCategory = category;
    }
    uint32				getDraftSlotsUpdate() {
        return(mDraftSlotsUpdate+=1);
    }

    uint8				isCraftEnabled() {
        return mCraftEnabled;
    }

    CraftAttributeWeights*	getCraftAttributeWeights() {
        return &mAttributeWeights;
    }

private:

    uint64				mId;
    uint32				mCrc;

    uint32				mGroupId;
    uint32				mWeightsBatchId;
    BString				mModel;
    SchematicLoadState	mLoadState;
    DraftSlots			mDraftSlots;
    WeightsBatches		mAssemblyBatch;
    WeightsBatches		mExperimentBatch;
    CraftBatches		mCraftBatch;
    uint32				mComplexity;
    uint32				mDataSize;
    uint32				mSubCategory;
    uint32				mDraftSlotsUpdate;
    uint8				mCraftEnabled;

    CraftAttributeWeights	mAttributeWeights;
};

//=============================================================================

#endif

