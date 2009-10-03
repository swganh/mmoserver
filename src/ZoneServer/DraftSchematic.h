/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_DRAFTSCHEMATIC_H
#define ANH_ZONESERVER_DRAFTSCHEMATIC_H

#include "Utils/typedefs.h"
#include "DraftSlot.h"
#include "WeightsBatch.h"
#include "CraftBatch.h"

typedef std::vector<DraftSlot*>		DraftSlots;
typedef std::vector<WeightsBatch*>	WeightsBatches;
typedef std::vector<CraftBatch*>	CraftBatches;

//=============================================================================

enum SchematicLoadState
{
	DSLoadState_Loading				= 1,
	DSLoadState_Loaded				= 2,
	DSLoadState_Slots				= 3,
	DSLoadState_Assembly_Weights	= 4,
	DSLoadState_Experiment_Weights	= 5,
};

//=============================================================================

class DraftSchematic
{
	friend class SchematicManager;

	public:

		DraftSchematic();
		~DraftSchematic();

		SchematicLoadState	getLoadState(){ return mLoadState; }
		void				setLoadState(SchematicLoadState ls){ mLoadState = ls; }
		uint64				getId(){ return mId; }
		void				setId(uint64 id){ mId = id; }

		uint32				getCrc(){ return mCrc; }
		void				setCrc(uint32 crc){ mCrc = crc; }

		uint32				getWeightsBatchId(){ return mWeightsBatchId; }
		void				setWeightsBatchId(uint32 id){ mWeightsBatchId = id; }
		string				getModel(){ return mModel; }
		void				setModel(string model){ mModel = model; }
		DraftSlots*			getDraftSlots(){ return &mDraftSlots; }
		WeightsBatches*		getAssemblyWeights(){ return &mAssemblyBatch; }
		WeightsBatches*		getExperimentWeights(){ return &mExperimentBatch; }
		CraftBatches*		getCraftBatches(){ return &mCraftBatch; }
		uint32				getComplexity(){ return mComplexity; }
		void				setComplexity(uint32 comp){ mComplexity = comp; }
		uint32				getDataSize(){ return mDataSize; }
		void				setDataSize(uint32 datasize){ mDataSize = datasize; }
		WeightsBatch*		getAssemblyWeightsBatchByListId(uint32 listId);
		WeightsBatch*		getExperimentWeightsBatchByListId(uint32 listId);
		CraftBatch*			getCraftBatchByListId(uint32 listId);
		uint32				getSubCategory(){ return mSubCategory; }
		void				setSubCategory(uint32 category){ mSubCategory = category; }
		uint32				getDraftSlotsUpdate(){ return(mDraftSlotsUpdate+=1); }

		uint8				isCraftEnabled(){ return mCraftEnabled; }

	private:

		uint64				mId;
		uint32				mCrc;

		uint32				mWeightsBatchId;
		string				mModel;
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
};

//=============================================================================

#endif

