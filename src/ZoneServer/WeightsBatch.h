/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

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

	private:

		uint32			mId;
		uint32			mListId;
		DraftWeights	mWeights;
};

#endif


