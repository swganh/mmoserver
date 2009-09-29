/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_HARVESTER_OBJECT_H
#define ANH_ZONESERVER_HARVESTER_OBJECT_H

#include "tangibleobject.h"

#include "BuildingEnums.h"
#include <vector>


//=============================================================================



//=============================================================================

class HarvesterObject :	public TangibleObject
{
	friend class HarvesterFactory;

	public:

		HarvesterObject();
		~HarvesterObject();

		
		HarvesterFamily	getHarvesterFamily(){ return mHarvesterFamily; }
		void			setHarvesterFamily(HarvesterFamily hf){ mHarvesterFamily = hf; }

		uint32			getLoadCount(){ return mTotalLoadCount; }
		uint32			decLoadCount(){ return (mTotalLoadCount-1); }
		void			setLoadCount(uint32 count){ mTotalLoadCount = count; }
		

	private:

		uint32			mResourceHarvested;
		uint32			mHoperSize;
		uint64			mResourceType;
		HarvesterFamily	mHarvesterFamily;

		uint32			mTotalLoadCount;
};

//=============================================================================

#endif