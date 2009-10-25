/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_WAYPOINT_OBJECT_FACTORY_H
#define ANH_ZONESERVER_WAYPOINT_OBJECT_FACTORY_H

#include "FactoryBase.h"

#define	 gWaypointFactory	WaypointFactory::getSingletonPtr()

//=============================================================================

class Database;
class DataBinding;
class DispatchClient;
class ObjectFactoryCallback;
class WaypointObject;

//=============================================================================

enum WaypointFQuery
{
	WaypointFQuery_MainData	= 1
};

//=============================================================================

class WaypointFactory : public FactoryBase
{
	public:

		static WaypointFactory*	getSingletonPtr() { return mSingleton; }
		static WaypointFactory*	Init(Database* database);

		~WaypointFactory();

		void			handleDatabaseJobComplete(void* ref,DatabaseResult* result);
		void			requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client);

	private:

		WaypointFactory(Database* database);

		void				_setupDatabindings();
		void				_destroyDatabindings();

		WaypointObject*		_createWaypoint(DatabaseResult* result);

		static WaypointFactory*		mSingleton;
		static bool				mInsFlag;

		DataBinding*				mWaypointBinding;
};

//=============================================================================


#endif

