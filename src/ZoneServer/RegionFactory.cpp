/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "RegionFactory.h"
#include "ObjectFactoryCallback.h"
#include "LogManager/LogManager.h"
#include "WorldManager.h"
#include "Utils/utils.h"

#include <assert.h>

//=============================================================================

bool				RegionFactory::mInsFlag    = false;
RegionFactory*		RegionFactory::mSingleton  = NULL;

//======================================================================================================================

RegionFactory*	RegionFactory::Init(Database* database)
{
	if(!mInsFlag)
	{
		mSingleton = new RegionFactory(database);
		mInsFlag = true;
		return mSingleton;
	}
	else
		return mSingleton;
}

//=============================================================================

RegionFactory::RegionFactory(Database* database) : FactoryBase(database)
{
	mCityFactory			= CityFactory::Init(mDatabase);
	mBadgeRegionFactory		= BadgeRegionFactory::Init(mDatabase);
	mSpawnRegionFactory		= SpawnRegionFactory::Init(mDatabase);
	mQTRegionFactory		= QTRegionFactory::Init(mDatabase);
}

//=============================================================================

RegionFactory::~RegionFactory()
{
	mInsFlag = false;
	delete(mSingleton);
}

//=============================================================================

void RegionFactory::requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client)
{
	switch(subGroup)
	{
		case Region_City:	mCityFactory->requestObject(ofCallback,id,subGroup,subType,client);	break;
		case Region_Badge:	mBadgeRegionFactory->requestObject(ofCallback,id,subGroup,subType,client); break;
		case Region_Spawn:  mSpawnRegionFactory->requestObject(ofCallback,id,subGroup,subType,client); break;
		case Region_Zone:	mQTRegionFactory->requestObject(ofCallback,id,subGroup,subType,client); break;
	
		default:
			gLogger->logMsg("RegionFactory::requestObject Unknown Group\n");
		break;
	}
}

//=============================================================================

void RegionFactory::releaseAllPoolsMemory()
{
	mCityFactory->releaseQueryContainerPoolMemory();
	mBadgeRegionFactory->releaseQueryContainerPoolMemory();
	mSpawnRegionFactory->releaseQueryContainerPoolMemory();
	mQTRegionFactory->releaseQueryContainerPoolMemory();
}

//=============================================================================

