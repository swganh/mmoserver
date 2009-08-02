/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "IntangibleFactory.h"
#include "VehicleFactory.h"
#include "ObjectFactoryCallback.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include "LogManager/LogManager.h"
#include "WorldManager.h"
#include "Utils/utils.h"

//=============================================================================

bool					IntangibleFactory::mInsFlag    = false;
IntangibleFactory*		IntangibleFactory::mSingleton  = NULL;

//=============================================================================

IntangibleFactory*	IntangibleFactory::Init(Database* database)
{
	if(!mInsFlag)
	{
		mSingleton = new IntangibleFactory(database);
		mInsFlag = true;
		return mSingleton;
	}
	else
		return mSingleton;
}

//=============================================================================

IntangibleFactory::IntangibleFactory(Database* database) : FactoryBase(database)
{
	mVehicleFactory = VehicleFactory::Init(mDatabase);
}



//=============================================================================

IntangibleFactory::~IntangibleFactory()
{
	mInsFlag = false;
	delete(mSingleton);
}

//=============================================================================

void IntangibleFactory::requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client)
{
	mVehicleFactory->requestObject(ofCallback,id,subGroup,subType,client);
}

//=============================================================================

void IntangibleFactory::releaseAllPoolsMemory()
{
	mVehicleFactory->releaseQueryContainerPoolMemory();
}

//=============================================================================

void IntangibleFactory::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{

}

//=============================================================================
