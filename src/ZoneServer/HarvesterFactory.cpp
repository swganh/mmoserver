/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "HarvesterFactory.h"
#include "Deed.h"
#include "HarvesterObject.h"
#include "ResourceContainerFactory.h"
#include "WorldManager.h"
#include "LogManager/LogManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
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

			uint64 count = result->getRowCount();

			HarvesterObject* harvester = dynamic_cast<HarvesterObject*>(asyncContainer->mObject);

			if(count)
			{	
				DataBinding* binding = mDatabase->CreateDataBinding(2);
				binding->addField(DFT_uint64,offsetof(HarvesterHopperItem,ResourceID),8,0);
				binding->addField(DFT_float,offsetof(HarvesterHopperItem,Quantity),4,1);
				
				HResourceList*	hRList = harvester->getResourceList();
				
				HResourceList::iterator it = hRList->begin();

				HarvesterHopperItem hopperTemp;
				for(uint64 i=0;i <count;i++)
				{
					result->GetNextRow(binding,&hopperTemp);
					hRList->push_back(std::make_pair(hopperTemp.ResourceID,hopperTemp.Quantity));
				}

			}

			QueryContainerBase* asynContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,HFQuery_AttributeData,asyncContainer->mClient,asyncContainer->mId);
			asynContainer->mId		= harvester->getId();
			asynContainer->mObject	= harvester;

			mDatabase->ExecuteSqlAsync(this,asynContainer,"SELECT attributes.name,sa.value,attributes.internal"
															 " FROM structure_attributes sa"
															 " INNER JOIN attributes ON (sa.attribute_id = attributes.id)"
															 " WHERE sa.structure_id = %"PRIu64" ORDER BY sa.order",harvester->getId());

			
		}
		break;

		case HFQuery_AttributeData:
		{
			HarvesterObject* harvester = dynamic_cast<HarvesterObject*>(asyncContainer->mObject);
			_buildAttributeMap(harvester,result);

			gLogger->logMsgF("HarvesterFactory: loaded Harvester %I64u", MSG_HIGH, harvester->getId());
			asyncContainer->mOfCallback->handleObjectReady(harvester,asyncContainer->mClient);
			
		}
		break;

		case HFQuery_MainData:
		{
			QueryContainerBase* asynContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,HFQuery_ResourceData,asyncContainer->mClient,asyncContainer->mId);

			HarvesterObject* harvester = new(HarvesterObject);
			_createHarvester(result,harvester);

			asynContainer->mObject = harvester;
			asynContainer->mClient = asyncContainer->mClient;

			//gLogger->logMsgF("HarvesterFactory: loaded Harvester %I64u", MSG_HIGH, harvester->getId());
			//asyncContainer->mOfCallback->handleObjectReady(harvester,asyncContainer->mClient);

			//now request the associated resource container count

			int8 sql[250];
			sprintf(sql,"SELECT hr.resourceID, hr.quantity FROM harvester_resources hr WHERE hr.ID = '%"PRIu64"' ",harvester->getId());
			mDatabase->ExecuteSqlAsync(this,asynContainer,sql);


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
	sprintf(hmm, "SELECT s.id,s.owner,s.oX,s.oY,s.oZ,s.oW,s.x,s.y,s.z,std.type,std.object_string,std.stf_name, std.stf_file, s.name, std.lots_used, std.resource_Category, h.ResourceID, h.active, h.rate, std.maint_cost_wk, std.power_used, s.condition, std.max_condition FROM structures s INNER JOIN structure_type_data std ON (s.type = std.type) INNER JOIN harvesters h ON (s.id = h.id) WHERE (s.id = %"PRIu64")",id);
	QueryContainerBase* asynContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(ofCallback,HFQuery_MainData,client,id);

	mDatabase->ExecuteSqlAsync(this,asynContainer,hmm);
}


//=============================================================================

void HarvesterFactory::_setupDatabindings()
{
	mHarvesterBinding = mDatabase->CreateDataBinding(23);
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
	mHarvesterBinding->addField(DFT_uint8,offsetof(HarvesterObject,mLotsUsed),1,14);
	mHarvesterBinding->addField(DFT_uint32,offsetof(HarvesterObject,mResourceCategory),4,15);

	mHarvesterBinding->addField(DFT_uint64,offsetof(HarvesterObject,mCurrentResource),8,16);
	mHarvesterBinding->addField(DFT_uint8,offsetof(HarvesterObject,mActive),1,17);
	mHarvesterBinding->addField(DFT_float,offsetof(HarvesterObject,mCurrentExtractionRate),4,18);
	
	mHarvesterBinding->addField(DFT_uint32,offsetof(HarvesterObject,maint_cost_wk),4,19);
	mHarvesterBinding->addField(DFT_uint32,offsetof(HarvesterObject,mPowerUsed),4,20);
	mHarvesterBinding->addField(DFT_uint32,offsetof(HarvesterObject,mDamage),4,21);
	mHarvesterBinding->addField(DFT_uint32,offsetof(HarvesterObject,mMaxCondition),4,22);

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
