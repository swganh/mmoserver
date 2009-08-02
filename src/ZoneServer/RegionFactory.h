/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_REGION_FACTORY_H
#define ANH_ZONESERVER_REGION_FACTORY_H

#include "RegionObject.h"
#include "CityFactory.h"
#include "BadgeRegionFactory.h"
#include "SpawnRegionFactory.h"
#include "QTRegionFactory.h"
#include "FactoryBase.h"

#define	 gRegionFactory	RegionFactory::getSingletonPtr()

//=============================================================================

class Database;
class DataBinding;
class DispatchClient;
class ObjectFactoryCallback;

//=============================================================================

class RegionFactory : public FactoryBase
{
	public:

		static RegionFactory*	getSingletonPtr() { return mSingleton; }
		static RegionFactory*	Init(Database* database);

		~RegionFactory();

		virtual void			handleDatabaseJobComplete(void* ref,DatabaseResult* result){}
		void					requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client);

		void					releaseAllPoolsMemory();

	private:

		RegionFactory(Database* database);

		static RegionFactory*	mSingleton;
		static bool				mInsFlag;

		CityFactory*			mCityFactory;
		BadgeRegionFactory*		mBadgeRegionFactory;
		SpawnRegionFactory*		mSpawnRegionFactory;
		QTRegionFactory*		mQTRegionFactory;
};

//=============================================================================

#endif

