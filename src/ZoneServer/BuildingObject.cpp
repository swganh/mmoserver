/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "BuildingObject.h"
#include "CellObject.h"
#include "MessageLib/MessageLib.h"
#include "SpawnPoint.h"
#include "MathLib/Quaternion.h"
#include "Utils/rand.h"

//=============================================================================

BuildingObject::BuildingObject() : PlayerStructure()
{
	mType = ObjType_Building;
	mMaxStorage = 0;

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

//=============================================================================
//gets the count of all tangibles in a cell
//how to not count the structure terminal ???? 
// - dont count tangibles marked as ststic :)
uint16 BuildingObject::getCellContentCount()
{
	uint16 count = 0;
	CellObjectList::iterator it = mCells.begin();

	while(it != mCells.end())
	{
		count += (*it)->getHeadCount();
			
		++it;
	}
	return(count);
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

//================================================================================
//
//the cells send an updated permission  to the specified player
//
void BuildingObject::updateCellPermissions(PlayerObject* player, bool access)
{
	//gLogger->logMsg("BuildingObject::updateCellPermissions: Permission set to %u",access);
	//iterate through all the cells - do they need to be deleted ?
	//place players inside a cell in the world
	CellObjectList*				cellList	= getCellList();
	CellObjectList::iterator	cellIt		= cellList->begin();

	while(cellIt != cellList->end())
	{
		CellObject* cell = (*cellIt);
					
		gMessageLib->sendUpdateCellPermissionMessage(cell,access,player);	

		++cellIt;
	}

}