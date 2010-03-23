/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "VehicleFactory.h"
#include "CreatureObject.h"
#include "Datapad.h"
#include "FactoryBase.h"
#include "IntangibleObject.h"
#include "Object.h"
#include "ObjectFactoryCallback.h"
#include "PlayerObject.h"
#include "Vehicle.h"
#include "WorldManager.h"
#include "DatabaseManager/DatabaseCallback.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include "MessageLib/MessageLib.h"
//#include "LogManager/LogManager.h"
//#include "Utils/utils.h"




//=============================================================================

bool					VehicleFactory::mInsFlag    = false;
VehicleFactory*			VehicleFactory::mSingleton  = NULL;

//=============================================================================

VehicleFactory*	VehicleFactory::Init(Database* database)
{
	if(!mInsFlag)
	{
		mSingleton = new VehicleFactory(database);
		mInsFlag = true;
		return mSingleton;
	}
	else
		return mSingleton;
}

//=============================================================================

VehicleFactory::VehicleFactory(Database* database) : FactoryBase(database)
{
	_setupDatabindings();
}



//=============================================================================

VehicleFactory::~VehicleFactory()
{
	mInsFlag = false;
	delete(mSingleton);
}

//=============================================================================

void VehicleFactory::requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client)
{
	mDatabase->ExecuteSqlAsync(this,new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(ofCallback,VehicleFactoryQuery_TypesId,client,id),
		"SELECT vehicle_types_id FROM vehicles WHERE id = %"PRIu64"",id);

}

//=============================================================================

void VehicleFactory::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
	QueryContainerBase* asyncContainer = reinterpret_cast<QueryContainerBase*>(ref);
	switch(asyncContainer->mQueryType)
	{

		case VehicleFactoryQuery_Create:
		{
			//get id of newly created vehicle
			DataBinding* binding = mDatabase->CreateDataBinding(1);
			binding->addField(DFT_uint64,0,8);
			uint64	id		= 0;
			uint64	count	= result->getRowCount();
			if(!count)
			{
				gLogger->logMsgF("VehicleFactory::createVehicle query without result", MSG_NORMAL);				
				mDatabase->DestroyDataBinding(binding);
				return;
			}
			result->GetNextRow(binding,&id);
			mDatabase->DestroyDataBinding(binding);
			asyncContainer->mId = id;

			if(!id)
			{
				gLogger->logMsgF("VehicleFactory::createVehicle query with invalid result", MSG_NORMAL);				
				mDatabase->DestroyDataBinding(binding);
				return;
			}

			gVehicleFactory->requestObject(asyncContainer->mOfCallback,id,0,0,asyncContainer->mClient);

		}
		break;

		case VehicleFactoryQuery_TypesId:
		{

			uint32 vehicleType = 0;
			DataBinding* binding = mDatabase->CreateDataBinding(1);
			binding->addField(DFT_uint32,0,4);

			uint64	count = result->getRowCount();

			result->GetNextRow(binding,&vehicleType);
			mDatabase->DestroyDataBinding(binding);

			QueryContainerBase* asContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,VehicleFactoryQuery_ItnoData,asyncContainer->mClient,asyncContainer->mId);

			mDatabase->ExecuteSqlAsync(this,asContainer,"SELECT vehicle_object_string, vehicle_itno_object_string, vehicle_name_file, vehicle_detail_file, vehicle_name FROM vehicle_types WHERE id = %u",vehicleType);

		}
		break;

		case VehicleFactoryQuery_ItnoData:
		{

			Vehicle * vehicle = _createVehicle(result);

			result->GetNextRow(mVehicleItno_Binding,vehicle);

			vehicle->setId(asyncContainer->mId);
			vehicle->setDetail(vehicle->getName().getAnsi());

			QueryContainerBase* aContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,VehicleFactoryQuery_MainData,asyncContainer->mClient,asyncContainer->mId);
			aContainer->mObject = (Object*)(IntangibleObject*)vehicle;

			mDatabase->ExecuteSqlAsync(this,aContainer,"SELECT vehicle_types_id, parent, vehicle_hitpoint_loss,vehicle_incline_acceleration,vehicle_flat_acceleration FROM vehicles WHERE id = %"PRIu64"",vehicle->getId());


		}
		break;

		case VehicleFactoryQuery_MainData:
		{

			Vehicle* vehicle = dynamic_cast<Vehicle*>(asyncContainer->mObject);

			uint64	count = result->getRowCount();

			if(count == 1)
			{
				result->GetNextRow(mVehicleCreo_Binding,vehicle);

				QueryContainerBase* asyncrContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,VehicleFactoryQuery_Attributes,asyncContainer->mClient,asyncContainer->mId);
				asyncrContainer->mObject = (Object*)(IntangibleObject*)vehicle;

				mDatabase->ExecuteSqlAsync(this,asyncrContainer,"SELECT attributes.name, vehicle_attributes.attribute_value, attributes.internal"
					" FROM attributes"
					" INNER JOIN vehicle_attributes ON (attributes.id = vehicle_attributes.attribute_id)"
					" WHERE vehicle_attributes.vehicles_id = %"PRIu64" ORDER BY vehicle_attributes.attribute_order",asyncContainer->mId);
			}

		}
		break;

		case VehicleFactoryQuery_Attributes:
		{
			_buildAttributeMap(asyncContainer->mObject,result);

			if(asyncContainer->mOfCallback)
			{
				if(asyncContainer->mOfCallback == this)
					handleObjectReady(asyncContainer->mObject,asyncContainer->mClient);
				else
					asyncContainer->mOfCallback->handleObjectReady(asyncContainer->mObject,asyncContainer->mClient);
			}
		}
		break;

		default:break;
	}

	mQueryContainerPool.free(asyncContainer);
}

//=============================================================================

void VehicleFactory::createVehicle(uint32 vehicle_type,PlayerObject* targetPlayer)
{
	int8 sql[256];
	sprintf(sql,"SELECT sf_DefaultVehicleCreate(%u, %"PRIu64")",vehicle_type,targetPlayer->getId());
	 gLogger->logMsgF("VehicleFactory::createVehicle query %s", MSG_NORMAL, sql);
	mDatabase->ExecuteSqlAsync(this,new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(this,VehicleFactoryQuery_Create,targetPlayer->getClient()),
		sql);
}
//=============================================================================

Vehicle* VehicleFactory::_createVehicle(DatabaseResult* result)
{
	Vehicle* vehicle = new Vehicle();

	result->GetNextRow(mVehicleItno_Binding,vehicle);

	vehicle->setLoadState(LoadState_Loaded);

	return vehicle;
}
//=============================================================================

void VehicleFactory::_setupDatabindings()
{
	//1vehicle_object_string, 1vehicle_itno_object_string, 2vehicle_name_file, 3vehicle_detail_file, 4vehicle_name
	mVehicleItno_Binding = mDatabase->CreateDataBinding(5);
	mVehicleItno_Binding->addField(DFT_bstring,offsetof(Vehicle,mPhysicalModel),256,0); //creo model
	mVehicleItno_Binding->addField(DFT_bstring,offsetof(Vehicle,mModel),256,1);
	mVehicleItno_Binding->addField(DFT_bstring,offsetof(Vehicle,mNameFile),64,2);
	mVehicleItno_Binding->addField(DFT_bstring,offsetof(Vehicle,mDetailFile),64,3);
	mVehicleItno_Binding->addField(DFT_bstring,offsetof(Vehicle,mName),64,4);

	//vehicles_types_id, parent, vehicle_hitpoint_loss,vehicle_incline_acceleration,vehicle_flat_acceleration
	mVehicleCreo_Binding = mDatabase->CreateDataBinding(5);
	mVehicleCreo_Binding->addField(DFT_uint32,offsetof(Vehicle,mTypesId),4,0);
	mVehicleCreo_Binding->addField(DFT_uint64,offsetof(Vehicle,mParentId),8,1);
	mVehicleCreo_Binding->addField(DFT_uint32,offsetof(Vehicle,mHitPointLoss),4,2);
	mVehicleCreo_Binding->addField(DFT_uint32,offsetof(Vehicle,mInclineAcceleration),4,3);
	mVehicleCreo_Binding->addField(DFT_uint32,offsetof(Vehicle,mFlatAcceleration),4,4);


}
//=============================================================================

void VehicleFactory::_destroyDatabindings()
{
	mDatabase->DestroyDataBinding(mVehicleItno_Binding);
	mDatabase->DestroyDataBinding(mVehicleCreo_Binding);
}

//=============================================================================


void VehicleFactory::handleObjectReady(Object* object,DispatchClient* client)
{

	if(Vehicle* vehicle = dynamic_cast<Vehicle*>(object))
	{

		PlayerObject* player = gWorldManager->getPlayerByAccId(client->getAccountId());
		if(player)
		{
			vehicle->setOwner(player);
			if(Datapad* datapad = dynamic_cast<Datapad*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Datapad)))
			{
				datapad->addData(vehicle);
				gWorldManager->addObject(vehicle,true);
				//spawn it in the player's datapad
				gMessageLib->sendCreateInTangible(vehicle, datapad->getId(), player);

				//now spawn it in the world
				vehicle->call();
			}
		}


	}

}
