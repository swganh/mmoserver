/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_DATAPAD_FACTORY_H
#define ANH_ZONESERVER_DATAPAD_FACTORY_H

#include "ObjectFactoryCallback.h"
#include "DatabaseManager/DatabaseCallback.h"
#include "FactoryBase.h"

#define	 gDatapadFactory	DatapadFactory::getSingletonPtr()

//=============================================================================

class Database;
class DataBinding;
class Datapad;
class DispatchClient;
class WaypointFactory;

//=============================================================================

enum DPFQuery
{
	DPFQuery_MainDatapadData		= 1,
	DPFQuery_Objects				= 2,
	DPFQuery_Data					= 3,
	DPFQuery_ManufactureSchematic	= 4,
	DPFQuery_ObjectCount			= 5,
	DPFQuery_ItemId					= 6,
	DPFQuery_Item					= 7,
	DPFQuery_MSParent				= 8,
};

//=============================================================================

class DatapadFactory : public FactoryBase, public ObjectFactoryCallback
{
	public:

		static DatapadFactory*	getSingletonPtr() { return mSingleton; }
		static DatapadFactory*	Init(Database* database);

		~DatapadFactory();

		virtual void	handleObjectReady(Object* object,DispatchClient* client);
		void			handleDatabaseJobComplete(void* ref,DatabaseResult* result);
		void			requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client);
		void			requestManufacturingSchematic(ObjectFactoryCallback* ofCallback, uint64 id);

	private:

		DatapadFactory(Database* database);

		void					_setupDatabindings();
		void					_destroyDatabindings();

		Datapad*				_createDatapad(DatabaseResult* result);

		static DatapadFactory*	mSingleton;
		static bool				mInsFlag;

		WaypointFactory*		mWaypointFactory;

		DataBinding*			mDatapadBinding;
		InLoadingContainer*		mIlc;
};

//=============================================================================


#endif

