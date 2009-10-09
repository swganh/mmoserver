/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_QTREGION_FACTORY_H
#define ANH_ZONESERVER_QTREGION_FACTORY_H

#include "FactoryBase.h"

#define 	gQTRegionFactory	QTRegionFactory::getSingletonPtr()

//=============================================================================

class Database;
class DataBinding;
class DispatchClient;
class ObjectFactoryCallback;
class QTRegion;

//=============================================================================

enum QTRFQuery
{
	QTRFQuery_MainData	= 1,
};

//=============================================================================

class QTRegionFactory : public FactoryBase
{
	public:

		static QTRegionFactory*	getSingletonPtr() { return mSingleton; }
		static QTRegionFactory*	Init(Database* database);

		~QTRegionFactory();

		void			handleDatabaseJobComplete(void* ref,DatabaseResult* result);
		void			requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client);

	private:

		QTRegionFactory(Database* database);

		void				_setupDatabindings();
		void				_destroyDatabindings();

		QTRegion*			_createRegion(DatabaseResult* result);

		static QTRegionFactory*	mSingleton;
		static bool				mInsFlag;

		DataBinding*			mRegionBinding;
};


//=============================================================================


#endif

