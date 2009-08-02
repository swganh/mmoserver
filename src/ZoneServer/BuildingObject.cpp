/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "MathLib/Quaternion.h"
#include "BuildingObject.h"
#include "Utils/rand.h"

//=============================================================================

BuildingObject::BuildingObject() : TangibleObject()
{
	mType = ObjType_Building;
	mWidth = 128;
	mHeight = 128;
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

//=============================================================================

ObjectList BuildingObject::getAllCellChilds()
{
	ObjectList*	tmpList;
	ObjectList	resultList;
	ObjectList::iterator childIt;

	CellObjectList::iterator cellIt = mCells.begin();

	while(cellIt != mCells.end())
	{
		tmpList = (*cellIt)->getChilds();
		childIt = tmpList->begin();

		while(childIt != tmpList->end())
		{
			resultList.push_back((*childIt));
			++childIt;
		}
		++cellIt;
	}
	return(resultList);
}

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


