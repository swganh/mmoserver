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

#include "DraftSchematic.h"
#include "CraftBatch.h"
#include "DraftSlot.h"
#include "DraftWeight.h"
#include "WeightsBatch.h"



//=============================================================================

DraftSchematic::DraftSchematic() :
    mLoadState(DSLoadState_Loading)
{
    mDraftSlots.reserve(8);
    mAssemblyBatch.reserve(16);
    mExperimentBatch.reserve(16);
    mCraftBatch.reserve(16);
    mDraftSlotsUpdate = 0;
}

//=============================================================================

DraftSchematic::~DraftSchematic()
{
    DraftSlots::iterator slotIt = mDraftSlots.begin();

    while(slotIt != mDraftSlots.end())
    {
        delete(*slotIt);
        mDraftSlots.erase(slotIt);
        slotIt = mDraftSlots.begin();
    }

    WeightsBatches::iterator batchIt = mAssemblyBatch.begin();

    while(batchIt != mAssemblyBatch.end())
    {
        WeightsBatch* batch = (*batchIt);
        DraftWeights* weights = batch->getWeights();
        DraftWeights::iterator weightIt = weights->begin();
        while(weightIt != weights->end())
        {
            delete(*weightIt);
            weights->erase(weightIt);
            weightIt = weights->begin();
        }
        ++batchIt;
    }

    batchIt = mExperimentBatch.begin();

    while(batchIt != mExperimentBatch.end())
    {
        WeightsBatch* batch = (*batchIt);
        DraftWeights* weights = batch->getWeights();
        DraftWeights::iterator weightIt = weights->begin();
        while(weightIt != weights->end())
        {
            delete(*weightIt);
            weights->erase(weightIt);
            weightIt = weights->begin();
        }
        ++batchIt;
    }
}

//=============================================================================

WeightsBatch* DraftSchematic::getAssemblyWeightsBatchByListId(uint32 listId)
{
    WeightsBatches::iterator it = mAssemblyBatch.begin();
    while(it != mAssemblyBatch.end())
    {
        if((*it)->getListId() == listId)
            return(*it);
        ++it;
    }
    return nullptr;
}

//=============================================================================

WeightsBatch* DraftSchematic::getExperimentWeightsBatchByListId(uint32 listId)
{
    WeightsBatches::iterator it = mExperimentBatch.begin();
    while(it != mExperimentBatch.end())
    {
        if((*it)->getListId() == listId)
            return(*it);
        ++it;
    }
    return nullptr;
}

//=============================================================================

CraftBatch* DraftSchematic::getCraftBatchByListId(uint32 listId)
{
    CraftBatches::iterator it = mCraftBatch.begin();
    while(it != mCraftBatch.end())
    {
        if((*it)->getListId() == listId)
            return(*it);

        ++it;
    }

    return nullptr;
}

//=============================================================================

