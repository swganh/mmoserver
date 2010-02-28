/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_BUILDING_OBJECT_H
#define ANH_ZONESERVER_BUILDING_OBJECT_H

#include "PlayerStructure.h"
#include "BuildingEnums.h"
#include <vector>

class CellObject;
class SpawnPoint;


//=============================================================================


typedef std::vector<CellObject*>	CellObjectList;
typedef std::vector<SpawnPoint*>	SpawnPoints;

//=============================================================================

class BuildingObject :	public PlayerStructure
{
	friend class BuildingFactory;
	friend class HouseFactory;

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

		CellObjectList*	getCellList(){ return &mCells; }
		void			addCell(CellObject* cellObject){ mCells.push_back(cellObject); }
		bool			removeCell(CellObject* cellObject);
		bool			checkForCell(CellObject* cellObject);
		ObjectList		getAllCellChilds();

		void			updateCellPermissions(PlayerObject* player, bool access);

		uint64			getMinCellId(){ return mMaxCellId; }
		void			setMinCellId(uint64 count){ mMaxCellId = count; }

		bool			getPublic(){ return mPublic; }
		void			setPublic(bool value){ mPublic = value; }
	private:
		CellObjectList	mCells;
		bool			mPublic;

		uint32			mTotalLoadCount;
		SpawnPoints		mSpawnPoints;
		BuildingFamily	mBuildingFamily;

		uint64			mMaxCellId;
};

//=============================================================================

#endif