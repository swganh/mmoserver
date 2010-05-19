/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "VehicleControllerFactory.h"
#include "CreatureObject.h"
#include "Datapad.h"
#include "FactoryBase.h"
#include "IntangibleObject.h"
#include "Object.h"
#include "ObjectFactoryCallback.h"
#include "PlayerObject.h"
#include "VehicleController.h"
#include "WorldManager.h"
#include "DatabaseManager/DatabaseCallback.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include "MessageLib/MessageLib.h"
//#include "LogManager/LogManager.h"
//#include "Utils/utils.h"




//=============================================================================

bool							VehicleControllerFactory::mInsFlag    = false;
VehicleControllerFactory*		VehicleControllerFactory::mSingleton  = NULL;

//=============================================================================

VehicleControllerFactory*	VehicleControllerFactory::Init(Database* database)
{
	if(!mInsFlag)
	{
		mSingleton = new VehicleControllerFactory(database);
		mInsFlag = true;
		return mSingleton;
	}
	else
		return mSingleton;
}

//=============================================================================

VehicleControllerFactory::VehicleControllerFactory(Database* database) : FactoryBase(database)
{
	_setupDatabindings();
}



//=============================================================================

VehicleControllerFactory::~VehicleControllerFactory()
{
	mInsFlag = false;
	delete(mSingleton);
}

//=============================================================================

void VehicleControllerFactory::requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client)
{

	mDatabase->ExecuteSqlAsync(this,new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(ofCallback,VehicleControllerFactoryQuery_TypesId,client,id),
		"SELECT vehicle_types_id FROM vehicles WHERE id = %"PRIu64"",id);


}

//=============================================================================

void VehicleControllerFactory::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
	QueryContainerBase* asyncContainer = reinterpret_cast<QueryContainerBase*>(ref);
	switch(asyncContainer->mQueryType)
	{

		case VehicleControllerFactoryQuery_Create:
		{
			//get id of newly created vehicle
			DataBinding* binding = mDatabase->CreateDataBinding(1);
			binding->addField(DFT_uint64,0,8);
			uint64	id		= 0;
			uint64	count	= result->getRowCount();
			if(!count)
			{
				gLogger->logMsgF("VehicleControllerFactory::createVehicle query without result", MSG_NORMAL);				
				mDatabase->DestroyDataBinding(binding);
				return;
			}
			result->GetNextRow(binding,&id);
			mDatabase->DestroyDataBinding(binding);
			asyncContainer->mId = id;

			if(!id)
			{
				gLogger->logMsgF("VehicleControllerFactory::createVehicle query with invalid result", MSG_NORMAL);				
				mDatabase->DestroyDataBinding(binding);
				return;
			}

			gVehicleControllerFactory->requestObject(asyncContainer->mOfCallback,id,0,0,asyncContainer->mClient);

		}
		break;

		case VehicleControllerFactoryQuery_TypesId:
		{

			uint32 vehicleType = 0;
			DataBinding* binding = mDatabase->CreateDataBinding(1);
			binding->addField(DFT_uint32,0,4);

			uint64	count = result->getRowCount();

			result->GetNextRow(binding,&vehicleType);
			mDatabase->DestroyDataBinding(binding);

			QueryContainerBase* asContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,VehicleControllerFactoryQuery_ItnoData,asyncContainer->mClient,asyncContainer->mId);

			mDatabase->ExecuteSqlAsync(this,asContainer,"SELECT vehicle_object_string, vehicle_itno_object_string, vehicle_name_file, vehicle_detail_file, vehicle_name FROM vehicle_types WHERE id = %u",vehicleType);

		}
		break;

		case VehicleControllerFactoryQuery_ItnoData:
		{

			VehicleController* vehicleController = _createVehicle(result);

			result->GetNextRow(mVehicleItno_Binding,vehicleController);

			vehicleController->setId(asyncContainer->mId);
			vehicleController->setDetail(vehicleController->getName().getAnsi());

			QueryContainerBase* aContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,VehicleControllerFactoryQuery_MainData,asyncContainer->mClient,asyncContainer->mId);
			aContainer->mObject = (Object*)(IntangibleObject*)vehicleController;


			mDatabase->ExecuteSqlAsync(this,aContainer,"SELECT vehicle_types_id, parent, vehicle_hitpoint_loss, vehicle_incline_acceleration, vehicle_flat_acceleration FROM vehicles WHERE id = %"PRIu64"",vehicleController->getId());


		}
		break;

		case VehicleControllerFactoryQuery_MainData:
		{

			VehicleController* vehicle = dynamic_cast<VehicleController*>(asyncContainer->mObject);

			uint64	count = result->getRowCount();

			if(count == 1)
			{
				result->GetNextRow(mVehicleCreo_Binding,vehicle);

				QueryContainerBase* asyncrContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,VehicleControllerFactoryQuery_Attributes,asyncContainer->mClient,asyncContainer->mId);
				asyncrContainer->mObject = (Object*)(IntangibleObject*)vehicle;

				mDatabase->ExecuteSqlAsync(this,asyncrContainer,"SELECT attributes.name, vehicle_attributes.attribute_value, attributes.internal"
					" FROM attributes"
					" INNER JOIN vehicle_attributes ON (attributes.id = vehicle_attributes.attribute_id)"
					" WHERE Vehicle_attributes.vehicles_id = %"PRIu64" ORDER BY vehicle_attributes.attribute_order", asyncContainer->mId);
			}

		}
		break;

		case VehicleControllerFactoryQuery_Attributes:
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

void VehicleControllerFactory::createVehicle(uint32 vehicle_type,PlayerObject* targetPlayer)
{
	int8 sql[256];

	sprintf(sql,"SELECT sf_DefaultVehicleCreate(%u, %"PRIu64")",vehicle_type,targetPlayer->getId());
	 gLogger->logMsgF("VehicleControllerFactory::createVehicle query %s", MSG_NORMAL, sql);
	mDatabase->ExecuteSqlAsync(this,new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(this,VehicleControllerFactoryQuery_Create,targetPlayer->getClient()),sql);
}
//=============================================================================

VehicleController* VehicleControllerFactory::_createVehicle(DatabaseResult* result)
{
	VehicleController* vehicle = new VehicleController();

	result->GetNextRow(mVehicleItno_Binding,vehicle);

	vehicle->setLoadState(LoadState_Loaded);

	return vehicle;
}
//=============================================================================

void VehicleControllerFactory::_setupDatabindings()
{
	//1vehicle_object_string, 1vehicle_itno_object_string, 2vehicle_name_file, 3vehicle_detail_file, 4vehicle_name
	mVehicleItno_Binding = mDatabase->CreateDataBinding(5);
	mVehicleItno_Binding->addField(DFT_bstring,offsetof(VehicleController,mPhysicalModel),256,0); //creo model
	mVehicleItno_Binding->addField(DFT_bstring,offsetof(VehicleController,mModel),256,1);
	mVehicleItno_Binding->addField(DFT_bstring,offsetof(VehicleController,mNameFile),64,2);
	mVehicleItno_Binding->addField(DFT_bstring,offsetof(VehicleController,mDetailFile),64,3);
	mVehicleItno_Binding->addField(DFT_bstring,offsetof(VehicleController,mName),64,4);

	//vehicles_types_id, parent, vehicle_hitpoint_loss,vehicle_incline_acceleration,vehicle_flat_acceleration
	mVehicleCreo_Binding = mDatabase->CreateDataBinding(5);
	mVehicleCreo_Binding->addField(DFT_uint32,offsetof(VehicleController,mTypesId),4,0);
	mVehicleCreo_Binding->addField(DFT_uint64,offsetof(VehicleController,mParentId),8,1);
	mVehicleCreo_Binding->addField(DFT_uint32,offsetof(VehicleController,mHitPointLoss),4,2);
	mVehicleCreo_Binding->addField(DFT_uint32,offsetof(VehicleController,mInclineAcceleration),4,3);
	mVehicleCreo_Binding->addField(DFT_uint32,offsetof(VehicleController,mFlatAcceleration),4,4);


}
//=============================================================================

void VehicleControllerFactory::_destroyDatabindings()
{
	mDatabase->DestroyDataBinding(mVehicleItno_Binding);
	mDatabase->DestroyDataBinding(mVehicleCreo_Binding);
}

//=============================================================================


void VehicleControllerFactory::handleObjectReady(Object* object,DispatchClient* client)
{

	if(VehicleController* vehicle = dynamic_cast<VehicleController*>(object))
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
