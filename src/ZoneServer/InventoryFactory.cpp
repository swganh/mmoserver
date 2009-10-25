/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "InventoryFactory.h"
#include "Inventory.h"
#include "ObjectFactoryCallback.h"
#include "TangibleFactory.h"
#include "WorldManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include "LogManager/LogManager.h"

#include "Utils/utils.h"

#include <assert.h>

//=============================================================================

bool					InventoryFactory::mInsFlag    = false;
InventoryFactory*		InventoryFactory::mSingleton  = NULL;

//======================================================================================================================

InventoryFactory*	InventoryFactory::Init(Database* database)
{
	if(!mInsFlag)
	{
		mSingleton = new InventoryFactory(database);
		mInsFlag = true;
		return mSingleton;
	}
	else
		return mSingleton;
}

//=============================================================================

InventoryFactory::InventoryFactory(Database* database) : FactoryBase(database)
{
	mTangibleFactory = TangibleFactory::Init(mDatabase);

	_setupDatabindings();
}

//=============================================================================

InventoryFactory::~InventoryFactory()
{
	_destroyDatabindings();

	mInsFlag = false;
	delete(mSingleton);
}

//=============================================================================

void InventoryFactory::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
	QueryContainerBase* asyncContainer = reinterpret_cast<QueryContainerBase*>(ref);

	switch(asyncContainer->mQueryType)
	{
		case IFQuery_MainInventoryData:
		{
			Inventory* inventory = _createInventory(result);

			QueryContainerBase* asContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,IFQuery_ObjectCount,asyncContainer->mClient);
			asContainer->mObject = inventory;

			mDatabase->ExecuteSqlAsync(this,asContainer,"SELECT sf_getInventoryObjectCount(%"PRId64")",inventory->getId());
		}
		break;

		case IFQuery_ObjectCount:
		{
			Inventory* inventory = dynamic_cast<Inventory*>(asyncContainer->mObject);

			uint32 objectCount;
			DataBinding* binding = mDatabase->CreateDataBinding(1);

			binding->addField(DFT_uint32,0,4);
			result->GetNextRow(binding,&objectCount);

			inventory->setObjectLoadCounter(objectCount);

			if(objectCount != 0)
			{
				uint64 invId = inventory->getId();

				inventory->setLoadState(LoadState_Loading);

				// query contents				
				QueryContainerBase* asContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,IFQuery_Objects,asyncContainer->mClient);
				asContainer->mObject = inventory;

				mDatabase->ExecuteSqlAsync(this,asContainer,
						"(SELECT \'containers\',containers.id FROM containers INNER JOIN container_types ON (containers.container_type = container_types.id)"
						" WHERE (container_types.name NOT LIKE 'unknown') AND (containers.parent_id = %"PRId64"))"
						" UNION (SELECT \'items\',items.id FROM items WHERE (parent_id=%"PRId64"))"
						" UNION (SELECT \'resource_containers\',resource_containers.id FROM resource_containers WHERE (parent_id=%"PRId64"))",
						invId,invId,invId);
				
			}
			else
			{
				inventory->setLoadState(LoadState_Loaded);
				asyncContainer->mOfCallback->handleObjectReady(inventory,asyncContainer->mClient);
			}

			mDatabase->DestroyDataBinding(binding);
		}
		break;

		case IFQuery_Objects:
		{
			Inventory* inventory = dynamic_cast<Inventory*>(asyncContainer->mObject);

			Type1_QueryContainer queryContainer;

			DataBinding*	binding = mDatabase->CreateDataBinding(2);
			binding->addField(DFT_bstring,offsetof(Type1_QueryContainer,mString),64,0);
			binding->addField(DFT_uint64,offsetof(Type1_QueryContainer,mId),8,1);

			uint64 count = result->getRowCount();

			//InLoadingContainer* ilc = new(mILCPool.ordered_malloc()) InLoadingContainer(inventory,asyncContainer->mOfCallback,asyncContainer->mClient);
			//ilc->mLoadCounter = count;

			mObjectLoadMap.insert(std::make_pair(inventory->getId(),new(mILCPool.ordered_malloc()) InLoadingContainer(inventory,asyncContainer->mOfCallback,asyncContainer->mClient,static_cast<uint8>(count))));

			for(uint32 i = 0;i < count;i++)
			{
				result->GetNextRow(binding,&queryContainer);

				if(strcmp(queryContainer.mString.getAnsi(),"containers") == 0)
					mTangibleFactory->requestObject(this,queryContainer.mId,TanGroup_Container,0,asyncContainer->mClient);
				else if(strcmp(queryContainer.mString.getAnsi(),"items") == 0)
					mTangibleFactory->requestObject(this,queryContainer.mId,TanGroup_Item,0,asyncContainer->mClient);
				else if(strcmp(queryContainer.mString.getAnsi(),"resource_containers") == 0)
					mTangibleFactory->requestObject(this,queryContainer.mId,TanGroup_ResourceContainer,0,asyncContainer->mClient);
				
					
			
			}
			
			mDatabase->DestroyDataBinding(binding);
		}
		break;

		default:break;
	}

	mQueryContainerPool.free(asyncContainer);
}

//=============================================================================

void InventoryFactory::requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client)
{
	mDatabase->ExecuteSqlAsync(this,new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(ofCallback,IFQuery_MainInventoryData,client),
		"SELECT inventories.id,inventories.credits,inventory_types.object_string,inventory_types.name,inventory_types.file,"
		"inventory_types.slots"
		" FROM inventories INNER JOIN inventory_types ON (inventories.inventory_type = inventory_types.id)"
		" WHERE (inventories.id = %"PRId64")",id);
}

//=============================================================================

Inventory* InventoryFactory::_createInventory(DatabaseResult* result)
{
	Inventory*	inventory = new Inventory();

	uint64 count = result->getRowCount();
	assert(count == 1);

	// get our results
	result->GetNextRow(mInventoryBinding,(void*)inventory);
	inventory->setParentId(inventory->mId - 1);

	return inventory;
}

//=============================================================================

void InventoryFactory::_setupDatabindings()
{
	// inventory binding
	mInventoryBinding = mDatabase->CreateDataBinding(6);
	mInventoryBinding->addField(DFT_uint64,offsetof(Inventory,mId),8,0);
	mInventoryBinding->addField(DFT_int32,offsetof(Inventory,mCredits),4,1);
	mInventoryBinding->addField(DFT_bstring,offsetof(Inventory,mModel),256,2);
	mInventoryBinding->addField(DFT_bstring,offsetof(Inventory,mName),64,3);
	mInventoryBinding->addField(DFT_bstring,offsetof(Inventory,mNameFile),64,4);
	mInventoryBinding->addField(DFT_uint8,offsetof(Inventory,mMaxSlots),1,5);
}

//=============================================================================

void InventoryFactory::_destroyDatabindings()
{
	mDatabase->DestroyDataBinding(mInventoryBinding);
}

//=============================================================================

void InventoryFactory::handleObjectReady(Object* object,DispatchClient* client)
{
	//we either have the ID of the inventory or of the player
	//the inventory of course is stored under its own Id

	InLoadingContainer* ilc	= _getObject(object->getParentId());

	Inventory*			inventory	= dynamic_cast<Inventory*>(ilc->mObject);

	gWorldManager->addObject(object,true);

	//for unequipped items only
	inventory->addObject(object);
	
	if(inventory->getObjectLoadCounter() == (inventory->getObjects())->size())
	{
		inventory->setLoadState(LoadState_Loaded);

		if(!(_removeFromObjectLoadMap(inventory->getId())))
			gLogger->logMsg("InventoryFactory: Failed removing object from loadmap");

		ilc->mOfCallback->handleObjectReady(inventory,ilc->mClient);

		mILCPool.free(ilc);
	}
}

//=============================================================================

