/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_CELL_OBJECT_FACTORY_H
#define ANH_ZONESERVER_CELL_OBJECT_FACTORY_H

#include "ObjectFactoryCallback.h"
#include "FactoryBase.h"

#define 	gCellFactory	CellFactory::getSingletonPtr()

//=============================================================================

class CellObject;
class Database;
class DataBinding;
class DispatchClient;

//=============================================================================

enum CellFQuery
{
	CellFQuery_MainData	= 1,
	CellFQuery_Objects	= 2
};

//=============================================================================

class CellFactory : public FactoryBase, public ObjectFactoryCallback
{
	public:

		static CellFactory*	getSingletonPtr() { return mSingleton; }
		static CellFactory*	Init(Database* database);

		~CellFactory();

		virtual void	handleObjectReady(Object* object,DispatchClient* client);
		void			handleDatabaseJobComplete(void* ref,DatabaseResult* result);
		void			requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client);
		void			requestStructureCell(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client);

	private:

		CellFactory(Database* database);

		void			_setupDatabindings();
		void			_destroyDatabindings();

		CellObject*		_createCell(DatabaseResult* result);

		static CellFactory*		mSingleton;
		static bool				mInsFlag;

		DataBinding*			mCellBinding;
};

//=============================================================================


#endif



