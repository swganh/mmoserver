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
	//destroy the cells 

	CellObjectList::iterator itC = mCells.begin();
	while(itC != mCells.end())
	{
		itC = mCells.erase(itC);

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


