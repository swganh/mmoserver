/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_INTANGIBLE_FACTORY_H
#define ANH_ZONESERVER_INTANGIBLE_FACTORY_H

#include "IntangibleObject.h"
#include "DatabaseManager/DatabaseCallback.h"
#include "FactoryBase.h"
#include "VehicleFactory.h"

#define		gIntangibleFactory	IntangibleFactory::getSingletonPtr()

//=============================================================================

class Database;
class DataBinding;
class DispatchClient;
class ObjectFactoryCallback;

//=============================================================================

class IntangibleFactory : public FactoryBase, public ObjectFactoryCallback
{
	public:

		static	IntangibleFactory*	getSingletonPtr() { return mSingleton; }
		static	IntangibleFactory*	Init(Database* database);

		~IntangibleFactory();

		virtual void			handleDatabaseJobComplete(void* ref,DatabaseResult* result);
		void					requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client);

		void					releaseAllPoolsMemory();


	private:
		IntangibleFactory(Database* database);

		void				_setupDatabindings();
		void				_destroyDatabindings();


		static IntangibleFactory*	mSingleton;
		static bool					mInsFlag;

		VehicleFactory*				mVehicleFactory;

};

//=============================================================================


#endif