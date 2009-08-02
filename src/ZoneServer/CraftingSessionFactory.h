/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_CRAFTINGSESSION_FACTORY_H
#define ANH_ZONESERVER_CRAFTINGSESSION_FACTORY_H

//#include "Utils/typedefs.h"
#include "CraftingSession.h"
#include <boost/pool/pool.hpp>


//=============================================================================

#define 	gCraftingSessionFactory	CraftingSessionFactory::getSingletonPtr()

class Database;
class PlayerObject;

//=============================================================================

class CraftingSessionFactory
{
	public:

		static CraftingSessionFactory*	getSingletonPtr() { return mSingleton; }
		static CraftingSessionFactory*	Init(Database* database);

		~CraftingSessionFactory();

		CraftingSession*		createSession(Anh_Utils::Clock* clock,PlayerObject* player,CraftingTool* tool,CraftingStation* station,uint32 expFlag);
		void					destroySession(CraftingSession* session);

	private:

		CraftingSessionFactory(Database* database);

		static CraftingSessionFactory*	mSingleton;
		static bool						mInsFlag;

		Database*						mDatabase;

		boost::pool<boost::default_user_allocator_malloc_free>	mSessionPool;
};

//=============================================================================

#endif


