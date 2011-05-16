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

#ifndef ANH_ZONESERVER_BUILDING_OBJECT_H
#define ANH_ZONESERVER_BUILDING_OBJECT_H

#include <vector>

#include "PlayerStructure.h"
#include "BuildingEnums.h"

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

	void			prepareDestruction();

		
	uint16			getCellContentCount();

	void			updateCellPermissions(PlayerObject* player, bool access);

	uint64			getMinCellId(){ return mMaxCellId; }
	void			setMinCellId(uint64 count){ mMaxCellId = count; }

	bool			getPublic(){ return mPublic; }
	void			setPublic(bool value){ mPublic = value; }

	bool			checkCapacity(uint8 amount){return ((int32)(mMaxStorage - getCellContentCount())> amount);}

private:
	CellObjectList	mCells;
	bool			mPublic;

	uint32			mTotalLoadCount;
		
	//max capacity of a house - this is *not* objectcontainer capacity as that holds the cells
	uint32			mMaxStorage;

	SpawnPoints		mSpawnPoints;
	BuildingFamily	mBuildingFamily;

	uint64			mMaxCellId;
};

//=============================================================================

#endif
