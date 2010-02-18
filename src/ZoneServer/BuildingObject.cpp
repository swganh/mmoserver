/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "BuildingObject.h"
#include "CellObject.h"
#include "SpawnPoint.h"
#include "MathLib/Quaternion.h"
#include "Utils/rand.h"

//=============================================================================

BuildingObject::BuildingObject() : PlayerStructure()
{
	mType = ObjType_Building;

}

//=============================================================================

BuildingObject::~BuildingObject()									 
{
	SpawnPoints::iterator it = mSpawnPoints.begin();

	while(it != mSpawnPoints.end())
	{
		delete(*it);

		it = mSpawnPoints.erase(it);
	}
	//destroy the cells 
	CellObjectList::iterator cellIt = mCells.begin();

	while(cellIt != mCells.end())
	{
		gWorldManager->destroyObject((*cellIt));
		//cellIt++;
		cellIt = mCells.erase(cellIt);
	}

}

//=============================================================================



//=============================================================================

SpawnPoint* BuildingObject::getRandomSpawnPoint()
{
	if(mSpawnPoints.size())
	{
		return(mSpawnPoints[gRandom->getRand()%mSpawnPoints.size()]);
	}

	return(NULL);
}

//=============================================================================

bool BuildingObject::removeCell(CellObject* cellObject)
{
	CellObjectList::iterator it = mCells.begin();

	while(it != mCells.end())
	{
		if((*it) == cellObject)
		{
			mCells.erase(it);
			return(true);
		}
		++it;
	}
	return(false);
}

//=============================================================================

bool BuildingObject::checkForCell(CellObject* cellObject)
{
	CellObjectList::iterator it = mCells.begin();

	while(it != mCells.end())
	{
		if((*it) == cellObject)
			return(true);
		++it;
	}
	return(false);
}

ObjectList BuildingObject::getAllCellChilds()
{
	ObjectIDList*	tmpList;
	ObjectList	resultList;
	ObjectIDList::iterator childIt;

	CellObjectList::iterator cellIt = mCells.begin();

	while(cellIt != mCells.end())
	{
		tmpList = (*cellIt)->getObjects();
		childIt = tmpList->begin();

		while(childIt != tmpList->end())
		{
			Object* childObject = gWorldManager->getObjectById((*childIt));
			resultList.push_back(childObject);
			++childIt;
		}
		++cellIt;
	}
	return(resultList);
}

