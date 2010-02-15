/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_BUILDING_OBJECT_H
#define ANH_ZONESERVER_BUILDING_OBJECT_H

#include "HouseObject.h"
#include "BuildingEnums.h"
#include <vector>

class CellObject;
class SpawnPoint;

//=============================================================================

typedef std::vector<SpawnPoint*>	SpawnPoints;

//=============================================================================

class BuildingObject :	public HouseObject
{
	friend class BuildingFactory;

	public:

		BuildingObject();
		~BuildingObject();

		//ObjectList		getAllCellChilds();

		uint32			getLoadCount(){ return mTotalLoadCount; }
		void			setLoadCount(uint32 count){ mTotalLoadCount = count; }

		BuildingFamily	getBuildingFamily(){ return mBuildingFamily; }
		void			setBuildingFamily(BuildingFamily bf){ mBuildingFamily = bf; }

		void			addSpawnPoint(SpawnPoint* sp){ mSpawnPoints.push_back(sp); }
		SpawnPoints*	getSpawnPoints(){ return &mSpawnPoints; }
		SpawnPoint*		getRandomSpawnPoint();

	private:

		uint32			mTotalLoadCount;
		SpawnPoints		mSpawnPoints;
		BuildingFamily	mBuildingFamily;
};

//=============================================================================

#endif