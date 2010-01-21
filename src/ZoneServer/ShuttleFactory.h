/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_SHUTTLE_OBJECT_FACTORY_H
#define ANH_ZONESERVER_SHUTTLE_OBJECT_FACTORY_H

#include "DatabaseManager/DatabaseCallback.h"
#include "FactoryBase.h"

#define 	gShuttleFactory	ShuttleFactory::getSingletonPtr()

//=============================================================================

class Database;
class DataBinding;
class DispatchClient;
class ObjectFactoryCallback;
class Shuttle;

//=============================================================================

enum SHFQuery
{
	SHFQuery_MainData	= 1
};

//=============================================================================

class ShuttleFactory : public FactoryBase
{
	public:

		static ShuttleFactory*	getSingletonPtr() { return mSingleton; }
		static ShuttleFactory*	Init(Database* database);

		~ShuttleFactory();

		void			handleDatabaseJobComplete(void* ref,DatabaseResult* result);
		void			requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client);

	private:

		ShuttleFactory(Database* database);

		void				_setupDatabindings();
		void				_destroyDatabindings();

		Shuttle*			_createShuttle(DatabaseResult* result);

		static ShuttleFactory*	mSingleton;
		static bool				mInsFlag;

		DataBinding*			mShuttleBinding;
};

//=============================================================================


#endif



