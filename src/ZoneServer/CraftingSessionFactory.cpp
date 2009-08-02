/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "CraftingSessionFactory.h"
#include "DatabaseManager/Database.h"
#include "LogManager/LogManager.h"
#include "PlayerObject.h"
#include "WorldManager.h"
#include "Utils/utils.h"

//=============================================================================

bool						CraftingSessionFactory::mInsFlag    = false;
CraftingSessionFactory*		CraftingSessionFactory::mSingleton  = NULL;

//======================================================================================================================

CraftingSessionFactory*	CraftingSessionFactory::Init(Database* database)
{
	if(!mInsFlag)
	{
		mSingleton = new CraftingSessionFactory(database);
		mInsFlag = true;
		return mSingleton;
	}
	else
		return mSingleton;
}

//=============================================================================

CraftingSessionFactory::CraftingSessionFactory(Database* database) :
mDatabase(database),
mSessionPool(sizeof(CraftingSession))
{
}

//=============================================================================

CraftingSessionFactory::~CraftingSessionFactory()
{
	mInsFlag = false;
	delete(mSingleton);
}

//=============================================================================

CraftingSession* CraftingSessionFactory::createSession(Anh_Utils::Clock* clock,PlayerObject* player,CraftingTool* tool,CraftingStation* station,uint32 expFlag)
{
	CraftingSession* session = new(mSessionPool.ordered_malloc()) CraftingSession(clock,mDatabase,player,tool,station,expFlag);

	return(session);
}

//=============================================================================

void CraftingSessionFactory::destroySession(CraftingSession* session)
{
	if(session)
	{
		session->~CraftingSession();
		mSessionPool.free(session);
	}
}

//=============================================================================


