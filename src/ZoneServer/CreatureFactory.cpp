/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "CreatureFactory.h"
#include "CreatureEnums.h"
#include "ObjectFactoryCallback.h"
#include "PersistentNpcFactory.h"
#include "ShuttleFactory.h"
#include "LogManager/LogManager.h"
#include "Utils/utils.h"

#include <assert.h>

//=============================================================================

bool					CreatureFactory::mInsFlag    = false;
CreatureFactory*		CreatureFactory::mSingleton  = NULL;

//======================================================================================================================

CreatureFactory*	CreatureFactory::Init(Database* database)
{
	if(!mInsFlag)
	{
		mSingleton = new CreatureFactory(database);
		mInsFlag = true;
		return mSingleton;
	}
	else
		return mSingleton;
}

//=============================================================================

CreatureFactory::CreatureFactory(Database* database) : FactoryBase(database)
{
	mPersistentNpcFactory	= PersistentNpcFactory::Init(mDatabase);
	mShuttleFactory			= ShuttleFactory::Init(mDatabase);
}

//=============================================================================

CreatureFactory::~CreatureFactory()
{
	mInsFlag = false;
	delete(mSingleton);
}

//=============================================================================

void CreatureFactory::requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client)
{
	switch(subGroup)
	{
		case CreoGroup_PersistentNpc:	mPersistentNpcFactory->requestObject(ofCallback,id,subGroup,subType,client);	break;
		case CreoGroup_Shuttle:			mShuttleFactory->requestObject(ofCallback,id,subGroup,subType,client);			break;

		default:
			gLogger->logMsg("CreatureFactory::requestObject Unknown Group\n");
		break;
	}
}

//=============================================================================

void CreatureFactory::releaseAllPoolsMemory()
{
	mPersistentNpcFactory->releaseQueryContainerPoolMemory()	;
	mShuttleFactory->releaseQueryContainerPoolMemory();
}

//=============================================================================

