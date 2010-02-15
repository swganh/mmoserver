/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_HOUSE_OBJECT_FACTORY_H
#define ANH_ZONESERVER_HOUSE_OBJECT_FACTORY_H

#include "ObjectFactoryCallback.h"
#include "FactoryBase.h"
#include "CellFactory.h"

#define gHouseFactory HouseFactory::getSingletonPtr()


//=============================================================================

class Database;
class DataBinding;
class DispatchClient;
class HouseObject;

//=============================================================================

enum HOFQuery
{
	HOFQuery_MainData					= 1,
    HOFQuery_AttributeData				= 2,
	HOFQuery_AdminData					= 3,
	HOFQuery_CellData					= 4
	
	

};

//=============================================================================

class HouseFactory : public FactoryBase, public ObjectFactoryCallback
{
	public:

		static HouseFactory*	getSingletonPtr() { return mSingleton; }
		static HouseFactory*	Init(Database* database);

		~HouseFactory();

		virtual void	handleObjectReady(Object* object,DispatchClient* client);
		void			handleDatabaseJobComplete(void* ref,DatabaseResult* result);
		void			requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client);

		void			releaseAllPoolsMemory();

	private:

		HouseFactory(Database* database);

		void			_setupDatabindings();
		void			_destroyDatabindings();

		void			_createHouse(DatabaseResult* result, HouseObject* house);

		static HouseFactory*		mSingleton;
		static bool					mInsFlag;


		CellFactory*				mCellFactory;
		DataBinding*				mHouseBinding;

};

//=============================================================================


#endif


