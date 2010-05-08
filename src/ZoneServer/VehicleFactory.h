/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_VEHICLECONTROLLER_FACTORY_H
#define ANH_ZONESERVER_VEHICLECONTROLLER_FACTORY_H

#include "FactoryBase.h"
#include "ObjectFactoryCallback.h"

//Forward Declerations
class Database;
class DatabaseCallback;
class DataBinding;
class DispatchClient;
class IntangibleObject;
class PlayerObject;
class VehicleController;

#define	 gVehicleControllerFactory	VehicleControllerFactory::getSingletonPtr()

//=============================================================================


//=============================================================================

enum VehicleControllerQuery
{
	VehicleControllerFactoryQuery_Create		= 1,
	VehicleControllerFactoryQuery_TypesId		= 2,
	VehicleControllerFactoryQuery_ItnoData		= 3,
	VehicleControllerFactoryQuery_MainData		= 4,
	VehicleControllerFactoryQuery_Attributes	= 5
};

//=============================================================================

class VehicleControllerFactory : public FactoryBase, public ObjectFactoryCallback
{
	public:

		static	VehicleControllerFactory*	getSingletonPtr() { return mSingleton; }
		static	VehicleControllerFactory*	Init(Database* database);

		~VehicleControllerFactory();

		virtual void			handleDatabaseJobComplete(void* ref,DatabaseResult* result);
		virtual void			handleObjectReady(Object* object,DispatchClient* client);
		void					requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client);
		void					createVehicleController(uint32 Vehicle_type,PlayerObject* targetPlayer);


	private:

		VehicleControllerFactory(Database* database);

		void				_setupDatabindings();
		void				_destroyDatabindings();

		VehicleController*			_createVehicleController(DatabaseResult* result);

		static VehicleControllerFactory*	mSingleton;
		static bool					mInsFlag;

		DataBinding*				mVehicleControllerItno_Binding;
		DataBinding*				mVehicleControllerCreo_Binding;

};

//=============================================================================


#endif
