/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/
#include "MissionFactory.h"
#include "ObjectFactoryCallback.h"
#include "LogManager/LogManager.h"
#include "Utils/utils.h"


//=============================================================================

bool				MissionFactory::mInsFlag    = false;
MissionFactory*		MissionFactory::mSingleton  = NULL;

//=============================================================================

MissionFactory*	MissionFactory::Init(Database* database)
{
	if(!mInsFlag)
	{
		mSingleton = new MissionFactory();
		mInsFlag = true;
		return mSingleton;
	}
	else
		return mSingleton;
}

//=============================================================================
//MissionFactory::MissionFactory(Database* database) : FactoryBase(database)
MissionFactory::MissionFactory()
{
	//init memeber objects
}

//=============================================================================

MissionFactory::~MissionFactory()
{
	mInsFlag = false;
	delete(mSingleton);
}

//=============================================================================