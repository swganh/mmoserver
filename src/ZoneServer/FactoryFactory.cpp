/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "FactoryFactory.h"
#include "Deed.h"
#include "FactoryObject.h"
#include "TangibleFactory.h"
#include "WorldManager.h"
#include "LogManager/LogManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include "Utils/utils.h"
#include <assert.h>

//=============================================================================

bool				FactoryFactory::mInsFlag    = false;
FactoryFactory*		FactoryFactory::mSingleton  = NULL;

//======================================================================================================================

FactoryFactory*	FactoryFactory::Init(Database* database)
{
	if(!mInsFlag)
	{
		mSingleton = new FactoryFactory(database);
		mInsFlag = true;
		return mSingleton;
	}
	else
		return mSingleton;
}

//=============================================================================

FactoryFactory::FactoryFactory(Database* database) : FactoryBase(database)
{

	_setupDatabindings();
}

//=============================================================================

FactoryFactory::~FactoryFactory()
{
	_destroyDatabindings();

	mInsFlag = false;
	delete(mSingleton);
}

//=============================================================================

void FactoryFactory::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
	QueryContainerBase* asyncContainer = reinterpret_cast<QueryContainerBase*>(ref);

	switch(asyncContainer->mQueryType)
	{
		
		case FFQuery_AttributeData:
		{
			
			FactoryObject* factory = dynamic_cast<FactoryObject*>(asyncContainer->mObject);
			//_buildAttributeMap(harvester,result);

			Attribute_QueryContainer	attribute;
			uint64						count = result->getRowCount();
			//int8						str[256];
			//BStringVector				dataElements;

			for(uint64 i = 0;i < count;i++)
			{
				result->GetNextRow(mAttributeBinding,(void*)&attribute);				
				factory->addInternalAttribute(attribute.mKey,std::string(attribute.mValue.getAnsi()));
			}


			QueryContainerBase* asContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,FFQuery_Hopper,asyncContainer->mClient);
			asContainer->mObject = factory;

			mDatabase->ExecuteSqlAsync(this,asContainer,
						"(SELECT \'input\',id FROM items WHERE parent_id = %"PRIu64" AND item_type = 2773)"
						" UNION (SELECT \'output\',id FROM items WHERE parent_id = %"PRIu64" AND item_type = 2774)"
						,factory->getId(),factory->getId());

			
		}
		break;

		case FFQuery_Hopper:
		{

			FactoryObject* factory = dynamic_cast<FactoryObject*>(asyncContainer->mObject);

			Type1_QueryContainer queryContainer;

			DataBinding*	binding = mDatabase->CreateDataBinding(2);
			binding->addField(DFT_bstring,offsetof(Type1_QueryContainer,mString),64,0);
			binding->addField(DFT_uint64,offsetof(Type1_QueryContainer,mId),8,1);

			uint64 count = result->getRowCount();

			//InLoadingContainer* ilc = new(mILCPool.ordered_malloc()) InLoadingContainer(inventory,asyncContainer->mOfCallback,asyncContainer->mClient);
			//ilc->mLoadCounter = count;

			//mObjectLoadMap.insert(std::make_pair(datapad->getId(),new(mILCPool.ordered_malloc()) InLoadingContainer(datapad,asyncContainer->mOfCallback,asyncContainer->mClient,static_cast<uint32>(count))));

			mObjectLoadMap.insert(std::make_pair(factory->getId(),new(mILCPool.ordered_malloc()) InLoadingContainer(factory,asyncContainer->mOfCallback,NULL,2)));

			for(uint32 i = 0;i < count;i++)
			{
				result->GetNextRow(binding,&queryContainer);

				if(strcmp(queryContainer.mString.getAnsi(),"input") == 0)
				{
					gTangibleFactory->requestObject(this,queryContainer.mId,TanGroup_Hopper,0,NULL);
				}

				else if(strcmp(queryContainer.mString.getAnsi(),"output") == 0)
				{
					gTangibleFactory->requestObject(this,queryContainer.mId,TanGroup_Hopper,0,NULL);
				}
			

			}

			mDatabase->DestroyDataBinding(binding);



				//factory->setLoadState(LoadState_Loaded);
			
		  
		}
		break;
		
		case FFQuery_MainData:
		{
			QueryContainerBase* asynContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,FFQuery_AttributeData,asyncContainer->mClient,asyncContainer->mId);
	
			
			FactoryObject* factory = new(FactoryObject);
			_createFactory(result,factory);

			asynContainer->mObject = factory;
			asynContainer->mClient = asyncContainer->mClient;
			asynContainer->mId		= factory->getId();
			

			mDatabase->ExecuteSqlAsync(this,asynContainer,"SELECT attributes.name,sa.value,attributes.internal"
															 " FROM structure_attributes sa"
															 " INNER JOIN attributes ON (sa.attribute_id = attributes.id)"
															 " WHERE sa.structure_id = %"PRIu64" ORDER BY sa.order",factory->getId());

			
		}
		break;

		default:break;
	}

	mQueryContainerPool.free(asyncContainer);
}

//=============================================================================


//=============================================================================

void FactoryFactory::_createFactory(DatabaseResult* result, FactoryObject* factory)
{

	uint64 count = result->getRowCount();
	assert(count == 1);

	result->GetNextRow(mFactoryBinding,factory);

	factory->setLoadState(LoadState_Loaded);
	factory->setType(ObjType_Structure);
	factory->mCustomName.convert(BSTRType_Unicode16);
}

//=============================================================================

void FactoryFactory::requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client)
{
	//request the harvesters Data first
	
	int8 hmm[1024];
	sprintf(hmm,	"SELECT s.id,s.owner,s.oX,s.oY,s.oZ,s.oW,s.x,s.y,s.z,"
					"std.type,std.object_string,std.stf_name, std.stf_file, s.name,"
					"std.lots_used, f.active, std.maint_cost_wk, std.power_used, s.condition, std.max_condition, f.ManSchematicId "
					"FROM structures s INNER JOIN structure_type_data std ON (s.type = std.type) INNER JOIN factories f ON (s.id = f.id) " 
					"WHERE (s.id = %"PRIu64")",id);
	QueryContainerBase* asynContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(ofCallback,FFQuery_MainData,client,id);

	mDatabase->ExecuteSqlAsync(this,asynContainer,hmm);
}


//=============================================================================

void FactoryFactory::_setupDatabindings()
{
	mFactoryBinding = mDatabase->CreateDataBinding(21);
	mFactoryBinding->addField(DFT_uint64,offsetof(FactoryObject,mId),8,0);
	mFactoryBinding->addField(DFT_uint64,offsetof(FactoryObject,mOwner),8,1);
	mFactoryBinding->addField(DFT_float,offsetof(FactoryObject,mDirection.mX),4,2);
	mFactoryBinding->addField(DFT_float,offsetof(FactoryObject,mDirection.mY),4,3);
	mFactoryBinding->addField(DFT_float,offsetof(FactoryObject,mDirection.mZ),4,4);
	mFactoryBinding->addField(DFT_float,offsetof(FactoryObject,mDirection.mW),4,5);
	mFactoryBinding->addField(DFT_float,offsetof(FactoryObject,mPosition.mX),4,6);
	mFactoryBinding->addField(DFT_float,offsetof(FactoryObject,mPosition.mY),4,7);
	mFactoryBinding->addField(DFT_float,offsetof(FactoryObject,mPosition.mZ),4,8);

	mFactoryBinding->addField(DFT_uint32,offsetof(FactoryObject,mFactoryFamily),4,9);//thats the structure_type_data ID
	mFactoryBinding->addField(DFT_bstring,offsetof(FactoryObject,mModel),256,10);
	mFactoryBinding->addField(DFT_bstring,offsetof(FactoryObject,mName),256,11);
	mFactoryBinding->addField(DFT_bstring,offsetof(FactoryObject,mNameFile),256,12);
	mFactoryBinding->addField(DFT_bstring,offsetof(FactoryObject,mCustomName),256,13);
	
	mFactoryBinding->addField(DFT_uint8,offsetof(FactoryObject,mLotsUsed),1,14);
	mFactoryBinding->addField(DFT_uint8,offsetof(FactoryObject,mActive),1,15);
	mFactoryBinding->addField(DFT_uint32,offsetof(FactoryObject,maint_cost_wk),4,16);
	mFactoryBinding->addField(DFT_uint32,offsetof(FactoryObject,mPowerUsed),4,17);
	mFactoryBinding->addField(DFT_uint32,offsetof(FactoryObject,mDamage),4,18);
	mFactoryBinding->addField(DFT_uint32,offsetof(FactoryObject,mMaxCondition),4,19);
	mFactoryBinding->addField(DFT_uint64,offsetof(FactoryObject,mManSchematicID),8,20);
}

//=============================================================================

void FactoryFactory::_destroyDatabindings()
{
	mDatabase->DestroyDataBinding(mFactoryBinding);

}

//=============================================================================

void FactoryFactory::handleObjectReady(Object* object,DispatchClient* client)
{
	//*ONLY* used to load in and out put hopper
	InLoadingContainer* ilc = _getObject(object->getParentId());
	FactoryObject*		factory = dynamic_cast<FactoryObject*>(ilc->mObject);
	if(!factory)
	{
			gLogger->logMsg("FactoryFactory::handleObjectReady No factory :(");
	}

	//add hopper to worldObjectlist, but NOT to the SI
	gWorldManager->addObject(object,true);

	Item* item = dynamic_cast<Item*>(object);
	if(!item)
	{
			gLogger->logMsg("FactoryFactory::handleObjectReady No hopper :(");
	}

	if(strcmp(item->getName().getAnsi(),"ingredient_hopper")==0)
	{
		gLogger->logMsg("FactoryFactory: IngredientHopper!!!!!!!!!!!!!!!!");
		factory->setIngredientHopper(object->getId());
	}
	else
	if(strcmp(item->getName().getAnsi(),"output_hopper")==0)
	{
		gLogger->logMsg("FactoryFactory: outputHopper!!!!!!!!!!!!!!!!");
		factory->setOutputHopper(object->getId());
	}
	
	if(( --ilc->mLoadCounter) == 0)
	{
		if(!(_removeFromObjectLoadMap(factory->getId())))
			gLogger->logMsg("FactoryFactory: Failed removing object from loadmap");

		factory->setLoadState(LoadState_Loaded);
		ilc->mOfCallback->handleObjectReady(factory,ilc->mClient);

		mILCPool.free(ilc);
	}

}

//=============================================================================

void FactoryFactory::releaseAllPoolsMemory()
{
	releaseQueryContainerPoolMemory();
	releaseILCPoolMemory();

}
