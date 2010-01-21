/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_SPAWNREGION_FACTORY_H
#define ANH_ZONESERVER_SPAWNREGION_FACTORY_H

#include "FactoryBase.h"

#define	 gSpawnRegionFactory	SpawnRegionFactory::getSingletonPtr()

//=============================================================================

class Database;
class DataBinding;
class DispatchClient;
class ObjectFactoryCallback;
class SpawnRegion;

//=============================================================================

enum SpawnFQuery
{
	SpawnFQuery_MainData	= 1
};

//=============================================================================

class SpawnRegionFactory : public FactoryBase
{
public:

	static SpawnRegionFactory*	getSingletonPtr() { return mSingleton; }
	static SpawnRegionFactory*	Init(Database* database);

	~SpawnRegionFactory();

	void			handleDatabaseJobComplete(void* ref,DatabaseResult* result);
	void			requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client);

private:

	SpawnRegionFactory(Database* database);

	void				_setupDatabindings();
	void				_destroyDatabindings();

	SpawnRegion*		_createSpawnRegion(DatabaseResult* result);

	static SpawnRegionFactory*		mSingleton;
	static bool						mInsFlag;

	DataBinding*					mSpawnRegionBinding;
};

//=============================================================================


#endif

