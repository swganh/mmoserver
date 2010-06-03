/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_DRAFTWEIGHT_H
#define ANH_ZONESERVER_DRAFTWEIGHT_H

#include "Utils/typedefs.h"


//=============================================================================

class DraftWeight
{
	friend class SchematicManager;

	public:

		DraftWeight();
		~DraftWeight();

		uint8	getDataType(){ return mDataType; }
		void	setDataType(uint8 type){ mDataType = type; }
		uint8	getDistribution(){ return mDistribution; }
		void	setDistribution(uint8 dist){mDistribution = dist; }

		void	prepareData();
		uint8	getData(){ return mData; }
		uint32  getSchemWeightBatch(){return mSchemWeightBatch;}
		uint32  getBatchListId(){return mBatchListId;}

	private:
		uint32  mSchemWeightBatch;
		uint8	mDataType;
		uint8	mDistribution;
		uint8	mData;
		uint32  mBatchListId;
};

#endif


