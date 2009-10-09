/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_CITY_FACTORY_H
#define ANH_ZONESERVER_CITY_FACTORY_H

#include "FactoryBase.h"

#define 	gCityFactory	CityFactory::getSingletonPtr()

//=============================================================================

class City;
class Database;
class DataBinding;
class DispatchClient;
class ObjectFactoryCallback;

//=============================================================================

enum CityFQuery
{
	CityFQuery_MainData	= 1,
};

//=============================================================================

class CityFactory : public FactoryBase
{
	public:

		static CityFactory*	getSingletonPtr() { return mSingleton; }
		static CityFactory*	Init(Database* database);

		~CityFactory();

		void			handleDatabaseJobComplete(void* ref,DatabaseResult* result);
		void			requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client);

	private:

		CityFactory(Database* database);

		void				_setupDatabindings();
		void				_destroyDatabindings();

		City*				_createCity(DatabaseResult* result);

		static CityFactory*		mSingleton;
		static bool				mInsFlag;

		DataBinding*			mCityBinding;
};


//=============================================================================


#endif

