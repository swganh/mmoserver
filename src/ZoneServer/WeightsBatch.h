/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_WEIGHTSBATCH_H
#define ANH_ZONESERVER_WEIGHTSBATCH_H

#include "Utils/typedefs.h"

class DraftWeight;

typedef std::vector<DraftWeight*>	DraftWeights;

//=============================================================================

class WeightsBatch
{
	friend class SchematicManager;

	public:

		WeightsBatch();
		~WeightsBatch();

		uint32			getId(){ return mId; }
		uint32			getListId(){ return mListId; }
		DraftWeights*	getWeights(){ return &mWeights; }
		uint64  getSchemId() {return mSchematicId;}
		uint32  getSchemGroupId(){return mSchemGroupId;}
	private:

		uint32			mId;
		uint32			mListId;
		DraftWeights	mWeights;
		uint64  mSchematicId;
		uint32  mSchemGroupId;
};

#endif


