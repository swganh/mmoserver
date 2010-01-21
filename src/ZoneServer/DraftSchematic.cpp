/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
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
	return(NULL);
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
	return(NULL);
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
	return(NULL);
}

//=============================================================================

