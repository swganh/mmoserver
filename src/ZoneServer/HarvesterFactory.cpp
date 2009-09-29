/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "HarvesterFactory.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include "LogManager/LogManager.h"
#include "WorldManager.h"
#include "Deed.h"
#include "Utils/utils.h"

#include <assert.h>

//=============================================================================

bool				HarvesterFactory::mInsFlag    = false;
HarvesterFactory*	HarvesterFactory::mSingleton  = NULL;

//======================================================================================================================

HarvesterFactory*	HarvesterFactory::Init(Database* database)
{
	if(!mInsFlag)
	{
		mSingleton = new HarvesterFactory(database);
		mInsFlag = true;
		return mSingleton;
	}
	else
		return mSingleton;
}

//=============================================================================

HarvesterFactory::HarvesterFactory(Database* database) : FactoryBase(database)
{

	_setupDatabindings();
}

//=============================================================================

HarvesterFactory::~HarvesterFactory()
{
	_destroyDatabindings();

	mInsFlag = false;
	delete(mSingleton);
}

//=============================================================================

void HarvesterFactory::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
	QueryContainerBase* asyncContainer = reinterpret_cast<QueryContainerBase*>(ref);

	switch(asyncContainer->mQueryType)
	{
		case HFQuery_ResourceData:
		{
			QueryContainerBase* asynContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,HFQuery_MainData,asyncContainer->mClient);
			
			uint32 containerCount = result->getRowCount();
			
			HarvesterObject* harvester = new(HarvesterObject);

			asynContainer->mObject = harvester;

			mObjectLoadMap.insert(std::make_pair(asyncContainer->mId,new(mILCPool.ordered_malloc()) InLoadingContainer(harvester,asyncContainer->mOfCallback,asyncContainer->mClient,containerCount)));

			DataBinding*	resContBinding = mDatabase->CreateDataBinding(1);
			resContBinding ->addField(DFT_int64,0,8);
			

			harvester->setLoadCount(containerCount);

			uint64 resContId;

			for(uint32 j = 0;j < containerCount;j++)
			{
				result->GetNextRow(resContBinding ,&resContId);

				gResourceContainerFactory->requestObject(this,resContId,0,0,asyncContainer->mClient);
				
			}

			mDatabase->DestroyDataBinding(resContBinding );


			mDatabase->ExecuteSqlAsync(this,asynContainer,
								"SELECT harvesters.id,harvesters.oX,harvesters.oY,harvesters.oZ,harvesters.oW,harvesters.x,"
								"harvesters.y,harvesters.z,structure_type_data.type,structure_type_data.object_string,"
								"harvesters.name"
								"FROM harvesters INNER JOIN structure_type_data ON (harvesters.type = structure_type_data.type) "
								"WHERE (harvesters.id = %lld)",asyncContainer->mId);
		}
		break;

		case HFQuery_MainData:
		{
			HarvesterObject* harvester = dynamic_cast<HarvesterObject*>(asyncContainer->mObject);
			_createHarvester(result,harvester);

		}
		break;
		
		default:break;
	}

	mQueryContainerPool.free(asyncContainer);
}

//=============================================================================

void HarvesterFactory::requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client)
{
	//start by requesting the associated resource container count
	mDatabase->ExecuteSqlAsync(this,new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(ofCallback,HFQuery_ResourceData,client,id),
								"SELECT id FROM swganh.resource_containers WHERE parent_id = %I64u",id);
	
	/*
	mDatabase->ExecuteSqlAsync(this,new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(ofCallback,BFQuery_MainData,client),
								"SELECT buildings.id,buildings.oX,buildings.oY,buildings.oZ,buildings.oW,buildings.x,"
								"buildings.y,buildings.z,building_types.model,building_types.width,building_types.height,"
								"building_types.file,building_types.name,building_types.family "
								"FROM buildings INNER JOIN building_types ON (buildings.type_id = building_types.id) "
								"WHERE (buildings.id = %lld)",id);
								*/
}

//=============================================================================


//=============================================================================

void HarvesterFactory::_createHarvester(DatabaseResult* result, HarvesterObject* harvester)
{

	uint64 count = result->getRowCount();
	assert(count == 1);

	result->GetNextRow(mHarvesterBinding,harvester);

	harvester->setLoadState(LoadState_Loaded);
}

//=============================================================================

void HarvesterFactory::_setupDatabindings()
{
	mHarvesterBinding = mDatabase->CreateDataBinding(14);
	mHarvesterBinding->addField(DFT_uint64,offsetof(HarvesterObject,mId),8,0);
	mHarvesterBinding->addField(DFT_float,offsetof(HarvesterObject,mDirection.mX),4,1);
	mHarvesterBinding->addField(DFT_float,offsetof(HarvesterObject,mDirection.mY),4,2);
	mHarvesterBinding->addField(DFT_float,offsetof(HarvesterObject,mDirection.mZ),4,3);
	mHarvesterBinding->addField(DFT_float,offsetof(HarvesterObject,mDirection.mW),4,4);
	mHarvesterBinding->addField(DFT_float,offsetof(HarvesterObject,mPosition.mX),4,5);
	mHarvesterBinding->addField(DFT_float,offsetof(HarvesterObject,mPosition.mY),4,6);
	mHarvesterBinding->addField(DFT_float,offsetof(HarvesterObject,mPosition.mZ),4,7);
	mHarvesterBinding->addField(DFT_bstring,offsetof(HarvesterObject,mModel),256,8);
	mHarvesterBinding->addField(DFT_bstring,offsetof(HarvesterObject,mNameFile),256,9);
	mHarvesterBinding->addField(DFT_bstring,offsetof(HarvesterObject,mName),256,10);
	mHarvesterBinding->addField(DFT_uint32,offsetof(HarvesterObject,mHarvesterFamily),4,11);

}

//=============================================================================

void HarvesterFactory::_destroyDatabindings()
{
	mDatabase->DestroyDataBinding(mHarvesterBinding);
	
}

//=============================================================================

void HarvesterFactory::handleObjectReady(Object* object,DispatchClient* client)
{
	//*ONLY* used to load resource containers
	InLoadingContainer* ilc = _getObject(object->getParentId());
	HarvesterObject*		harvester = dynamic_cast<HarvesterObject*>(ilc->mObject);
	
	//add res containers
	gWorldManager->addObject(object,true);

	
	if(harvester->decLoadCount() == 0)
	{
		if(!(_removeFromObjectLoadMap(harvester->getId())))
			gLogger->logMsg("HarvesterFactory: Failed removing object from loadmap");

		ilc->mOfCallback->handleObjectReady(harvester,ilc->mClient);

		mILCPool.free(ilc);
	}

}

//=============================================================================

void HarvesterFactory::releaseAllPoolsMemory()
{
	releaseQueryContainerPoolMemory();
	releaseILCPoolMemory();

}
