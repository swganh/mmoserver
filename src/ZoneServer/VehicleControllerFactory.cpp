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

bool					VehicleControllerFactory::mInsFlag    = false;
VehicleControllerFactory*			VehicleControllerFactory::mSingleton  = NULL;

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
		"SELECT Vehicle_types_id FROM Vehicles WHERE id = %"PRIu64"",id);

}

//=============================================================================

void VehicleControllerFactory::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
	QueryContainerBase* asyncContainer = reinterpret_cast<QueryContainerBase*>(ref);
	switch(asyncContainer->mQueryType)
	{

		case VehicleControllerFactoryQuery_Create:
		{
			//get id of newly created VehicleController
			DataBinding* binding = mDatabase->CreateDataBinding(1);
			binding->addField(DFT_uint64,0,8);
			uint64	id		= 0;
			uint64	count	= result->getRowCount();
			if(!count)
			{
				gLogger->logMsgF("VehicleControllerFactory::createVehicleController query without result", MSG_NORMAL);				
				mDatabase->DestroyDataBinding(binding);
				return;
			}
			result->GetNextRow(binding,&id);
			mDatabase->DestroyDataBinding(binding);
			asyncContainer->mId = id;

			if(!id)
			{
				gLogger->logMsgF("VehicleControllerFactory::createVehicleController query with invalid result", MSG_NORMAL);				
				mDatabase->DestroyDataBinding(binding);
				return;
			}

			gVehicleControllerFactory->requestObject(asyncContainer->mOfCallback,id,0,0,asyncContainer->mClient);

		}
		break;

		case VehicleControllerFactoryQuery_TypesId:
		{

			uint32 VehicleControllerType = 0;
			DataBinding* binding = mDatabase->CreateDataBinding(1);
			binding->addField(DFT_uint32,0,4);

			uint64	count = result->getRowCount();

			result->GetNextRow(binding,&VehicleControllerType);
			mDatabase->DestroyDataBinding(binding);

			QueryContainerBase* asContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,VehicleControllerFactoryQuery_ItnoData,asyncContainer->mClient,asyncContainer->mId);

			mDatabase->ExecuteSqlAsync(this,asContainer,"SELECT Vehicle_object_string, Vehicle_itno_object_string, Vehicle_name_file, Vehicle_detail_file, Vehicle_name FROM Vehicle_types WHERE id = %u",VehicleControllerType);

		}
		break;

		case VehicleControllerFactoryQuery_ItnoData:
		{

			VehicleController * VehicleController = _createVehicleController(result);

			result->GetNextRow(mVehicleControllerItno_Binding,VehicleController);

			VehicleController->setId(asyncContainer->mId);
			VehicleController->setDetail(VehicleController->getName().getAnsi());

			QueryContainerBase* aContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,VehicleControllerFactoryQuery_MainData,asyncContainer->mClient,asyncContainer->mId);
			aContainer->mObject = (Object*)(IntangibleObject*)VehicleController;

			mDatabase->ExecuteSqlAsync(this,aContainer,"SELECT Vehicle_types_id, parent, Vehicle_hitpoint_loss,Vehicle_incline_acceleration,Vehicle_flat_acceleration FROM Vehicles WHERE id = %"PRIu64"",VehicleController->getId());


		}
		break;

		case VehicleControllerFactoryQuery_MainData:
		{

			VehicleController* controller = dynamic_cast<VehicleController*>(asyncContainer->mObject);

			uint64	count = result->getRowCount();

			if(count == 1)
			{
				result->GetNextRow(mVehicleControllerCreo_Binding,controller);

				QueryContainerBase* asyncrContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,VehicleControllerFactoryQuery_Attributes,asyncContainer->mClient,asyncContainer->mId);
				asyncrContainer->mObject = (Object*)(IntangibleObject*)controller;

				mDatabase->ExecuteSqlAsync(this,asyncrContainer,"SELECT attributes.name, Vehicle_attributes.attribute_value, attributes.internal"
					" FROM attributes"
					" INNER JOIN Vehicle_attributes ON (attributes.id = Vehicle_attributes.attribute_id)"
					" WHERE Vehicle_attributes.Vehicle_id = %"PRIu64" ORDER BY Vehicle_attributes.attribute_order",asyncContainer->mId);
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

void VehicleControllerFactory::createVehicleController(uint32 VehicleController_type,PlayerObject* targetPlayer)
{
	int8 sql[256];
	sprintf(sql,"SELECT sf_DefaultVehicleCreate(%u, %"PRIu64")",VehicleController_type,targetPlayer->getId());
	 gLogger->logMsgF("VehicleFactory::createVehicle query %s", MSG_NORMAL, sql);
	mDatabase->ExecuteSqlAsync(this,new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(this,VehicleControllerFactoryQuery_Create,targetPlayer->getClient()),
		sql);
}
//=============================================================================

VehicleController* VehicleControllerFactory::_createVehicleController(DatabaseResult* result)
{
	VehicleController* controller = new VehicleController();

	result->GetNextRow(mVehicleControllerItno_Binding,controller);

	controller->setLoadState(LoadState_Loaded);

	return controller;
}
//=============================================================================

void VehicleControllerFactory::_setupDatabindings()
{
	//1VehicleController_object_string, 1VehicleController_itno_object_string, 2VehicleController_name_file, 3VehicleController_detail_file, 4VehicleController_name
	mVehicleControllerItno_Binding = mDatabase->CreateDataBinding(5);
	mVehicleControllerItno_Binding->addField(DFT_bstring,offsetof(VehicleController,mPhysicalModel),256,0); //creo model
	mVehicleControllerItno_Binding->addField(DFT_bstring,offsetof(VehicleController,mModel),256,1);
	mVehicleControllerItno_Binding->addField(DFT_bstring,offsetof(VehicleController,mNameFile),64,2);
	mVehicleControllerItno_Binding->addField(DFT_bstring,offsetof(VehicleController,mDetailFile),64,3);
	mVehicleControllerItno_Binding->addField(DFT_bstring,offsetof(VehicleController,mName),64,4);

	//VehicleControllers_types_id, parent, VehicleController_hitpoint_loss,VehicleController_incline_acceleration,VehicleController_flat_acceleration
	mVehicleControllerCreo_Binding = mDatabase->CreateDataBinding(5);
	mVehicleControllerCreo_Binding->addField(DFT_uint32,offsetof(VehicleController,mTypesId),4,0);
	mVehicleControllerCreo_Binding->addField(DFT_uint64,offsetof(VehicleController,mParentId),8,1);
	mVehicleControllerCreo_Binding->addField(DFT_uint32,offsetof(VehicleController,mHitPointLoss),4,2);
	mVehicleControllerCreo_Binding->addField(DFT_uint32,offsetof(VehicleController,mInclineAcceleration),4,3);
	mVehicleControllerCreo_Binding->addField(DFT_uint32,offsetof(VehicleController,mFlatAcceleration),4,4);


}
//=============================================================================

void VehicleControllerFactory::_destroyDatabindings()
{
	mDatabase->DestroyDataBinding(mVehicleControllerItno_Binding);
	mDatabase->DestroyDataBinding(mVehicleControllerCreo_Binding);
}

//=============================================================================


void VehicleControllerFactory::handleObjectReady(Object* object,DispatchClient* client)
{

	if(VehicleController* controller = dynamic_cast<VehicleController*>(object))
	{

		PlayerObject* player = gWorldManager->getPlayerByAccId(client->getAccountId());
		if(player)
		{
			controller->setOwner(player);
			if(Datapad* datapad = dynamic_cast<Datapad*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Datapad)))
			{
				datapad->addData(controller);
				gWorldManager->addObject(controller,true);
				//spawn it in the player's datapad
				gMessageLib->sendCreateInTangible(controller, datapad->getId(), player);

				//now spawn it in the world
				controller->call();
			}
		}


	}

}
