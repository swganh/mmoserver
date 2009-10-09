/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_BUILDING_OBJECT_FACTORY_H
#define ANH_ZONESERVER_BUILDING_OBJECT_FACTORY_H

#include "ObjectFactoryCallback.h"
#include "FactoryBase.h"

#define 	gBuildingFactory	BuildingFactory::getSingletonPtr()


//=============================================================================

class BuildingObject;
class CellFactory;
class Database;
class DataBinding;
class DispatchClient;

//=============================================================================

enum BFQuery
{
	BFQuery_MainData	= 1,
	BFQuery_Cells		= 2,
	BFQuery_CloneData	= 3,
};

//=============================================================================

class BuildingFactory : public FactoryBase, public ObjectFactoryCallback
{
	public:

		static BuildingFactory*	getSingletonPtr() { return mSingleton; }
		static BuildingFactory*	Init(Database* database);

		~BuildingFactory();

		virtual void	handleObjectReady(Object* object,DispatchClient* client);
		void			handleDatabaseJobComplete(void* ref,DatabaseResult* result);
		void			requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client);

		void			releaseAllPoolsMemory();

	private:

		BuildingFactory(Database* database);

		void			_setupDatabindings();
		void			_destroyDatabindings();

		BuildingObject*	_createBuilding(DatabaseResult* result);

		static BuildingFactory*		mSingleton;
		static bool					mInsFlag;

		CellFactory*				mCellFactory;

		DataBinding*				mBuildingBinding;
		DataBinding*				mSpawnBinding;
};

//=============================================================================


#endif


