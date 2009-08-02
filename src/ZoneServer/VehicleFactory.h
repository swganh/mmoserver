/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_VEHICLE_FACTORY_H
#define ANH_ZONESERVER_VEHICLE_FACTORY_H

#include "DatabaseManager/DatabaseCallback.h"
#include "FactoryBase.h"
#include "IntangibleObject.h"
#include "Vehicle.h"



#define	 gVehicleFactory	VehicleFactory::getSingletonPtr()

//=============================================================================

class Database;
class DataBinding;
class DispatchClient;
class ObjectFactoryCallback;

//=============================================================================

enum VehicleQuery
{
	VehicleFactoryQuery_Create		= 1,
	VehicleFactoryQuery_TypesId		= 2,
	VehicleFactoryQuery_ItnoData	= 3,
	VehicleFactoryQuery_MainData	= 4,
	VehicleFactoryQuery_Attributes	= 5,
};

//=============================================================================

class VehicleFactory : public FactoryBase, public ObjectFactoryCallback
{
	public:

		static	VehicleFactory*	getSingletonPtr() { return mSingleton; }
		static	VehicleFactory*	Init(Database* database);

		~VehicleFactory();

		virtual void			handleDatabaseJobComplete(void* ref,DatabaseResult* result);
		virtual void			handleObjectReady(Object* object,DispatchClient* client);
		void					requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client);
		void					createVehicle(uint32 vehicle_type,PlayerObject* targetPlayer);


	private:

		VehicleFactory(Database* database);

		void				_setupDatabindings();
		void				_destroyDatabindings();

		Vehicle*			_createVehicle(DatabaseResult* result);

		static VehicleFactory*	mSingleton;
		static bool					mInsFlag;

		DataBinding*				mVehicleItno_Binding;
		DataBinding*				mVehicleCreo_Binding;

};

//=============================================================================


#endif