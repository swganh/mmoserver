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

BuildingObject::BuildingObject() : HouseObject()
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


