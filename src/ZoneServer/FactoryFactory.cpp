/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "FactoryFactory.h"
#include "FactoryCrate.h"

#include "Deed.h"
#include "FactoryObject.h"
#include "PlayerObject.h"
#include "ResourceContainer.h"
#include "TangibleFactory.h"
#include "WorldManager.h"
#include "LogManager/LogManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include "MessageLib/MessageLib.h"
#include "Utils/utils.h"
#include <cassert>

//itemtypes
/*
weapons = 1
armor = 2
food 4
clothing 8
vehicle 16
droid 32
chemical 64
tissue 128

creatures 256
furniture 512
installation 1024
lightsaber 2048
generic 4096
genetics   8192
starshipcomponents131072
mand droid		65536
mand armor 32768
mand tailor	 16384
ship tools 262144
misc   524288
 */
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
		case FFQuery_HopperItemAttributeUpdate:
		{

			Type1_QueryContainer queryContainer;

			DataBinding*	binding = mDatabase->CreateDataBinding(1);
			binding->addField(DFT_bstring,offsetof(Type1_QueryContainer,mString),64,0);
			
			uint64 count;
			count = result->getRowCount();

			// should be 1 result featuring the value as string
			// note that we can get MySQL to give us the resContainersVolume as string as we just tell it to not cast the value
			// MySQL just uses strings in its interface!! so we just wait and cast the value in the virtual function
			// this way we dont have to differentiate between resourceContainers and FactoryCrates and save ourselves alot of unecessary work
			for(uint64 i = 0;i < count;i++)
			{
	
				TangibleObject* tangible = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById(asyncContainer->mId));
				if(!tangible)
				{
					gLogger->logMsg("FactoryFactory::FFQuery_HopperItemAttributeUpdate No tangible :(");
					mDatabase->DestroyDataBinding(binding);
					return;
				}
				result->GetNextRow(binding,&queryContainer);							
				tangible->upDateFactoryVolume(queryContainer.mString); //virtual function present in tangible, resourceContainer and factoryCrate
				
			}
			InLoadingContainer* ilc = _getObject(asyncContainer->mHopper);
			
			if((--ilc->mLoadCounter)== 0)
			{
				gLogger->logMsg("FactoryFactory: FFQuery_HopperItemAttributeUpdate attribute load ended item refresh!");
				if(!(_removeFromObjectLoadMap(asyncContainer->mHopper)))
					gLogger->logMsg("FactoryFactory: Failed removing object from loadmap");

				ilc->mOfCallback->handleObjectReady(asyncContainer->mObject,ilc->mClient,asyncContainer->mHopper);

				mILCPool.free(ilc);
			}
		
			mDatabase->DestroyDataBinding(binding);

		}
		break;

		case FFQuery_HopperUpdate:
		{
			//the player openened a factories hopper.
			//we now asynchronically read the hopper and its content and update them when necessary
			Type1_QueryContainer queryContainer;

			DataBinding*	binding = mDatabase->CreateDataBinding(2);
			binding->addField(DFT_bstring,offsetof(Type1_QueryContainer,mString),64,0);
			binding->addField(DFT_uint64,offsetof(Type1_QueryContainer,mId),8,1);


			uint64 count;
			count = result->getRowCount();
			if(!count)
			{
				asyncContainer->mOfCallback->handleObjectReady(asyncContainer->mObject,asyncContainer->mClient,asyncContainer->mHopper);
				mDatabase->DestroyDataBinding(binding);
				return;
			}

			//asyncContainer->mId == HOPPER!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1
			mObjectLoadMap.insert(std::make_pair(asyncContainer->mId,new(mILCPool.ordered_malloc()) InLoadingContainer(asyncContainer->mObject,asyncContainer->mOfCallback,asyncContainer->mClient,(uint32)count)));

			for(uint64 i = 0;i < count;i++)
			{
				result->GetNextRow(binding,&queryContainer);				
				
				//read in the ID - find the item in the world or load it newly
				if(strcmp(queryContainer.mString.getAnsi(),"item") == 0)
				{
					Item* item = dynamic_cast<Item*>(gWorldManager->getObjectById(queryContainer.mId));
					if(!item)
					{
						//the item is new - load it over the itemfactory
						gTangibleFactory->requestObject(this,queryContainer.mId,TanGroup_Item,0,asyncContainer->mClient);
											
					}
					//else update relevant attributes
					else
					{
						QueryContainerBase* asynContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,FFQuery_HopperItemAttributeUpdate,asyncContainer->mClient,queryContainer.mId);
						asynContainer->mObject = asyncContainer->mObject;
						asynContainer->mHopper = asyncContainer->mHopper;
						
						mDatabase->ExecuteSqlAsync(this,asynContainer,
								//"(SELECT \'item\',id FROM items WHERE parent_id = %"PRIu64")"
								"SELECT value FROM item_attributes WHERE item_id = %"PRIu64" AND attribute_id = 400"
								,queryContainer.mId);
					}
				}

				if(strcmp(queryContainer.mString.getAnsi(),"resource") == 0)
				{
					ResourceContainer* rc = dynamic_cast<ResourceContainer*>(gWorldManager->getObjectById(queryContainer.mId));
					if(!rc)
					{
						//the container is new - load it over the itemfactory
						gTangibleFactory->requestObject(this,queryContainer.mId,TanGroup_ResourceContainer,0,asyncContainer->mClient);
											
					}
					else
					{
						QueryContainerBase* asynContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,FFQuery_HopperItemAttributeUpdate,asyncContainer->mClient,queryContainer.mId);
						asynContainer->mObject = asyncContainer->mObject;
						asynContainer->mHopper = asyncContainer->mHopper;
						
						mDatabase->ExecuteSqlAsync(this,asynContainer,
								//"(SELECT \'item\',id FROM items WHERE parent_id = %"PRIu64")"
								"SELECT amount FROM resource_containers WHERE id= %"PRIu64""
								,queryContainer.mId);
					}
				}
			}
			mDatabase->DestroyDataBinding(binding);
		}
		break;
		
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

			mObjectLoadMap.insert(std::make_pair(factory->getId(),new(mILCPool.ordered_malloc()) InLoadingContainer(factory,asyncContainer->mOfCallback,asyncContainer->mClient,2)));

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

	result->GetNextRow(mFactoryBinding,factory);

	factory->setLoadState(LoadState_Loaded);
	factory->setType(ObjType_Structure);
	factory->mCustomName.convert(BSTRType_Unicode16);
	factory->setCapacity(2); // we want to load 2 hoppers!
}

//=============================================================================

void FactoryFactory::requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client)
{
	//request the harvesters Data first
	
	int8 hmm[1024];
	sprintf(hmm,	"SELECT s.id,s.owner,s.oX,s.oY,s.oZ,s.oW,s.x,s.y,s.z,"
					"std.type,std.object_string,std.stf_name, std.stf_file, s.name,"
					"std.lots_used, f.active, std.maint_cost_wk, std.power_used, std.schematicMask, s.condition, std.max_condition, f.ManSchematicId "
					"FROM structures s INNER JOIN structure_type_data std ON (s.type = std.type) INNER JOIN factories f ON (s.id = f.id) " 
					"WHERE (s.id = %"PRIu64")",id);
	QueryContainerBase* asynContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(ofCallback,FFQuery_MainData,client,id);

	mDatabase->ExecuteSqlAsync(this,asynContainer,hmm);
}

//the factories hopper is accessed - update the hoppers contents
void FactoryFactory::upDateHopper(ObjectFactoryCallback* ofCallback,uint64 hopperId, DispatchClient* client, FactoryObject* factory )
{
	QueryContainerBase* asynContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(ofCallback,FFQuery_HopperUpdate,client,hopperId);
	asynContainer->mObject = factory;
	asynContainer->mHopper = hopperId;

	mDatabase->ExecuteSqlAsync(this,asynContainer,
			"(SELECT \'item\',id FROM items WHERE parent_id = %"PRIu64")"
			" UNION (SELECT \'resource\',id FROM resource_containers WHERE parent_id = %"PRIu64")"
			,hopperId,hopperId);
}
//=============================================================================

void FactoryFactory::_setupDatabindings()
{
	mFactoryBinding = mDatabase->CreateDataBinding(22);
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
	
	mFactoryBinding->addField(DFT_uint32,offsetof(FactoryObject,mSchematicMask),4,18);

	mFactoryBinding->addField(DFT_uint32,offsetof(FactoryObject,mDamage),4,19);
	mFactoryBinding->addField(DFT_uint32,offsetof(FactoryObject,mMaxCondition),4,20);
	mFactoryBinding->addField(DFT_uint64,offsetof(FactoryObject,mManSchematicID),8,21);
}

//=============================================================================

void FactoryFactory::_destroyDatabindings()
{
	mDatabase->DestroyDataBinding(mFactoryBinding);

}

//=============================================================================

void FactoryFactory::handleObjectReady(Object* object,DispatchClient* client)
{
	//On serverstartup or factory create used to load in and out put hopper on factory create and to load hopper content
	//On runtime used to load hopper content when we access a hopper or to create a new factory
	//ILC ID on hoppercontent load is the hoppers ID

	InLoadingContainer* ilc = _getObject(object->getParentId());
	FactoryObject*		factory = dynamic_cast<FactoryObject*>(ilc->mObject);
	if(!factory)
	{
		factory = NULL;

	}
	
	//add hopper / new item to worldObjectlist, but NOT to the SI
	gWorldManager->addObject(object,true);

	//do we have a valid Object?
	TangibleObject* tangible = dynamic_cast<TangibleObject*>(object);
	if(!tangible)
	{		   	
		gLogger->logMsg("FactoryFactory: No Tangible on handleObjectReady!!!!!!!!!!!!!!!!");
		return;
	}

	uint64 parent = 0;
	if(strcmp(tangible->getName().getAnsi(),"ingredient_hopper")==0)
	{
		factory->setIngredientHopper(object->getId());
		factory->addObject(object);
	}
	else
	if(strcmp(tangible->getName().getAnsi(),"output_hopper")==0)
	{
		factory->setOutputHopper(object->getId());
		factory->addObject(object);
	}
	else
	{
		//its a tangible item of a hopper read in during runtime
		TangibleObject* hopper;
		if(factory->getOutputHopper() == tangible->getParentId())
		{
			hopper = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById(factory->getOutputHopper()));
		}
		else
		{
			hopper = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById(factory->getIngredientHopper()));
		}

		if(!hopper)
		{
			gLogger->logMsg("FactoryFactory: outputHopper not found on item load !!!!!!!");
			assert(false && "FactoryFactory::handleObjectReady WorldManager could not find output hopper");	
		}

		parent = hopper->getId();

		//add to hopper / create for players
		hopper->handleObjectReady(object,NULL);		
	}
	
	
	
	if(( --ilc->mLoadCounter) == 0)
	{
		if(!(_removeFromObjectLoadMap(object->getParentId())))
			gLogger->logMsg("FactoryFactory: Failed removing object from loadmap");

		factory->setLoadState(LoadState_Loaded);
		if(!parent)			   //factories dont have a parent! main cell is 0!!!
			ilc->mOfCallback->handleObjectReady(factory,ilc->mClient);
		else
			ilc->mOfCallback->handleObjectReady(factory,ilc->mClient,parent);	//only hoppers have a parent (the factory)

		mILCPool.free(ilc);
	}

}

//=============================================================================

void FactoryFactory::releaseAllPoolsMemory()
{
	releaseQueryContainerPoolMemory();
	releaseILCPoolMemory();

}
