/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "HarvesterFactory.h"
#include "HarvesterObject.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include "LogManager/LogManager.h"
#include "WorldManager.h"
#include "Deed.h"
#include "Utils/utils.h"
#include "ResourceContainerFactory.h"

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
			
			uint32 containerCount = static_cast<uint32>(result->getRowCount());

			HarvesterObject* harvester = dynamic_cast<HarvesterObject*>(asyncContainer->mObject);			

			if(!containerCount)
			{
				//no associated resources
			
				//gLogger->logMsgLoadSuccess("HarvesterFactory::Loaded Harvester %I64u",MSG_NORMAL, harvester->getId());
				gLogger->logMsgF("HarvesterFactory: loaded Harvester %I64u", MSG_HIGH, harvester->getId());
				asyncContainer->mOfCallback->handleObjectReady(harvester,asyncContainer->mClient);				
				mQueryContainerPool.free(asyncContainer);
				return;
			}

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


			
		}
		break;

		case HFQuery_MainData:
		{
			gLogger->logMsgF("HarvesterFactory: HFQuery_MainData ", MSG_HIGH);
			QueryContainerBase* asynContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,HFQuery_ResourceData,asyncContainer->mClient,asyncContainer->mId);
			
			HarvesterObject* harvester = new(HarvesterObject);
			asynContainer->mObject = harvester;			
			
			_createHarvester(result,harvester);

			//now request the associated resource container count
			mDatabase->ExecuteSqlAsync(this,asynContainer,
				"SELECT id FROM swganh.resource_containers WHERE parent_id = %I64u",harvester->getId());

		}
		break;
		
		default:break;
	}

	mQueryContainerPool.free(asyncContainer);
}

//=============================================================================


//=============================================================================

void HarvesterFactory::_createHarvester(DatabaseResult* result, HarvesterObject* harvester)
{

	uint64 count = result->getRowCount();
	assert(count == 1);

	result->GetNextRow(mHarvesterBinding,harvester);

	harvester->setLoadState(LoadState_Loaded);
	harvester->setType(ObjType_Harvester);
	harvester->mCustomName.convert(BSTRType_Unicode16);
}

//=============================================================================

void HarvesterFactory::requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client)
{
	//request the harvesters Data first
	/*
	mDatabase->ExecuteSqlAsync(this,new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(ofCallback,HFQuery_MainData,client,id),
								"SELECT harvesters.id,harvesters.oX,harvesters.oY,harvesters.oZ,harvesters.oW,harvesters.x,"
								"harvesters.y,harvesters.z,structure_type_data.type,structure_type_data.object_string,"
								"structure_type_data.stf_name, structure_type_data.stf_file, harvesters.name"
								"FROM harvesters INNER JOIN structure_type_data ON (harvesters.type = structure_type_data.type) "
								"WHERE (harvesters.id = %I64u)",id);

								*/
	int8 hmm[1024];
	sprintf(hmm, "SELECT harvesters.id,harvesters.owner,harvesters.oX,harvesters.oY,harvesters.oZ,harvesters.oW,harvesters.x,harvesters.y,harvesters.z,structure_type_data.type,structure_type_data.object_string,structure_type_data.stf_name, structure_type_data.stf_file, harvesters.name FROM harvesters INNER JOIN structure_type_data ON (harvesters.type = structure_type_data.type) WHERE (harvesters.id = %I64u)",id);
	QueryContainerBase* asynContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(ofCallback,HFQuery_MainData,client,id);

	mDatabase->ExecuteSqlAsync(this,asynContainer,hmm);
}


//=============================================================================

void HarvesterFactory::_setupDatabindings()
{
	mHarvesterBinding = mDatabase->CreateDataBinding(14);
	mHarvesterBinding->addField(DFT_uint64,offsetof(HarvesterObject,mId),8,0);
	mHarvesterBinding->addField(DFT_uint64,offsetof(HarvesterObject,mOwner),8,1);
	mHarvesterBinding->addField(DFT_float,offsetof(HarvesterObject,mDirection.mX),4,2);
	mHarvesterBinding->addField(DFT_float,offsetof(HarvesterObject,mDirection.mY),4,3);
	mHarvesterBinding->addField(DFT_float,offsetof(HarvesterObject,mDirection.mZ),4,4);
	mHarvesterBinding->addField(DFT_float,offsetof(HarvesterObject,mDirection.mW),4,5);
	mHarvesterBinding->addField(DFT_float,offsetof(HarvesterObject,mPosition.mX),4,6);
	mHarvesterBinding->addField(DFT_float,offsetof(HarvesterObject,mPosition.mY),4,7);
	mHarvesterBinding->addField(DFT_float,offsetof(HarvesterObject,mPosition.mZ),4,8);
	mHarvesterBinding->addField(DFT_uint32,offsetof(HarvesterObject,mHarvesterFamily),4,9);
	mHarvesterBinding->addField(DFT_bstring,offsetof(HarvesterObject,mModel),256,10);
	mHarvesterBinding->addField(DFT_bstring,offsetof(HarvesterObject,mName),256,11);
	mHarvesterBinding->addField(DFT_bstring,offsetof(HarvesterObject,mNameFile),256,12);
	mHarvesterBinding->addField(DFT_bstring,offsetof(HarvesterObject,mCustomName),256,13);
	

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
