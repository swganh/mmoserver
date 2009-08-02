/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "ContainerObjectFactory.h"
#include "ObjectFactoryCallback.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include "LogManager/LogManager.h"
#include "Utils/utils.h"
#include "WorldManager.h"
#include "WorldConfig.h"
#include <assert.h>

//=============================================================================

ContainerObjectFactory*	ContainerObjectFactory::mSingleton  = NULL;

//======================================================================================================================

ContainerObjectFactory*	ContainerObjectFactory::Init(Database* database)
{
	if(!mSingleton)
	{
		mSingleton = new ContainerObjectFactory(database);
	}
	return mSingleton;
}

//=============================================================================

ContainerObjectFactory::ContainerObjectFactory(Database* database) : FactoryBase(database)
{
	_setupDatabindings();
}

//=============================================================================

ContainerObjectFactory::~ContainerObjectFactory()
{
	_destroyDatabindings();
}

//=============================================================================

void ContainerObjectFactory::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
	QueryContainerBase* asyncContainer = reinterpret_cast<QueryContainerBase*>(ref);
	
	switch(asyncContainer->mQueryType)
	{
		case CFQuery_MainData:
		{
			Container* container = _createContainer(result);
			
			// If not a player as parent, we will not have to send any create updates.
			PlayerObject* player = NULL;
			if (asyncContainer->mClient)
			{
				player = gWorldManager->getPlayerByAccId(asyncContainer->mClient->getAccountId());
			}
			container->setParent(player);

			QueryContainerBase* asContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,CFQuery_ObjectCount,asyncContainer->mClient);
			asContainer->mObject = container;

			mDatabase->ExecuteSqlAsync(this,asContainer,"SELECT sf_getInventoryObjectCount(%lld)",container->getId());
		}
		break;

		case CFQuery_ObjectCount:
		{
			Container* container  = dynamic_cast<Container*>(asyncContainer->mObject);

			uint32 objectCount;
			DataBinding* binding = mDatabase->CreateDataBinding(1);

			binding->addField(DFT_uint32,0,4);
			result->GetNextRow(binding,&objectCount);

			container->setObjectLoadCounter(objectCount);

			if (objectCount != 0)
			{
				uint64 containerId = container->getId();
	
				container->setLoadState(LoadState_Loading);

				// query contents				
				QueryContainerBase* asContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,CFQuery_Objects,asyncContainer->mClient);
				asContainer->mObject = container;

				mDatabase->ExecuteSqlAsync(this,asContainer,
						"(SELECT \'containers\',containers.id FROM containers INNER JOIN container_types ON (containers.container_type = container_types.id)"
						" WHERE (container_types.name NOT LIKE 'unknown') AND (containers.parent_id = %lld))"
						" UNION (SELECT \'items\',items.id FROM items WHERE (parent_id=%lld))"
						" UNION (SELECT \'resource_containers\',resource_containers.id FROM resource_containers WHERE (parent_id=%lld))",
						containerId,containerId,containerId);
				
			}
			else
			{
				container->setLoadState(LoadState_Loaded);
				asyncContainer->mOfCallback->handleObjectReady(container,asyncContainer->mClient);
			}

			mDatabase->DestroyDataBinding(binding);
		}
		break;

		case CFQuery_Objects:
		{
			Container* container = dynamic_cast<Container*>(asyncContainer->mObject);

			Type1_QueryContainer queryContainer;

			DataBinding* binding = mDatabase->CreateDataBinding(2);
			binding->addField(DFT_bstring,offsetof(Type1_QueryContainer,mString),64,0);
			binding->addField(DFT_uint64,offsetof(Type1_QueryContainer,mId),8,1);

			uint64 count = result->getRowCount();

			mObjectLoadMap.insert(std::make_pair(container->getId(),new(mILCPool.ordered_malloc()) InLoadingContainer(container,asyncContainer->mOfCallback,asyncContainer->mClient,count)));

			for(uint32 i = 0;i < count;i++)
			{
				result->GetNextRow(binding,&queryContainer);

				if(strcmp(queryContainer.mString.getAnsi(),"containers") == 0)
					gTangibleFactory->requestObject(this,queryContainer.mId,TanGroup_Container,0,asyncContainer->mClient);
				else if(strcmp(queryContainer.mString.getAnsi(),"items") == 0)
					gTangibleFactory->requestObject(this,queryContainer.mId,TanGroup_Item,0,asyncContainer->mClient);
				else if(strcmp(queryContainer.mString.getAnsi(),"resource_containers") == 0)
					gTangibleFactory->requestObject(this,queryContainer.mId,TanGroup_ResourceContainer,0,asyncContainer->mClient);
			}
			
			mDatabase->DestroyDataBinding(binding);
		}
		break;

		default:break;
	}

	mQueryContainerPool.free(asyncContainer);
}

//=============================================================================

void ContainerObjectFactory::requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client)
{
	// gLogger->logMsg("ContainerObjectFactory::requestObject:");

	if (!gWorldManager->getObjectById(id))
	{
		// The container does not exist.
		mDatabase->ExecuteSqlAsync(this,new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(ofCallback,CFQuery_MainData,client),
								"SELECT containers.id,containers.parent_id,containers.oX,containers.oY,containers.oZ,containers.oW,containers.x,"
								"containers.y,containers.z,containers.container_type,container_types.object_string,container_types.name,container_types.file,container_types.details_file"
								" FROM containers INNER JOIN container_types ON (containers.container_type = container_types.id)"
								" WHERE (containers.id = %lld)",id);
	}
	else
	{
		// Just update the contence, if needed.
		// gLogger->logMsg("Doing an update...");

		// If not a player as parent, we will not have to send any create updates.
		PlayerObject* player = NULL;
		if (client)
		{
			player = gWorldManager->getPlayerByAccId(client->getAccountId());
		}
		Object* object = gWorldManager->getObjectById(id);
		if (object->getType() == ObjType_Tangible)
		{
			TangibleObject* tangObj = dynamic_cast<TangibleObject*>(object);
			if (tangObj->getTangibleGroup() == TanGroup_Container)
			{
				Container* container = dynamic_cast<Container*>(tangObj);
				container->setParent(player);

				QueryContainerBase* asContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(ofCallback,CFQuery_ObjectCount,client);
				asContainer->mObject = container;

				mDatabase->ExecuteSqlAsync(this,asContainer,"SELECT sf_getInventoryObjectCount(%lld)",container->getId());
			}
		}
	}
}
//=============================================================================

Container* ContainerObjectFactory::_createContainer(DatabaseResult* result)
{
	Container*	container = new Container();

	uint64 count = result->getRowCount();
	assert(count == 1);

	result->GetNextRow(mContainerBinding,(void*)container);

	// container->setLoadState(LoadState_Loaded);

	return container;
}

//=============================================================================

void ContainerObjectFactory::_setupDatabindings()
{
	mContainerBinding = mDatabase->CreateDataBinding(14);
	mContainerBinding->addField(DFT_uint64,offsetof(Container,mId),8,0);
	mContainerBinding->addField(DFT_uint64,offsetof(Container,mParentId),8,1);
	mContainerBinding->addField(DFT_float,offsetof(Container,mDirection.mX),4,2);
	mContainerBinding->addField(DFT_float,offsetof(Container,mDirection.mY),4,3);
	mContainerBinding->addField(DFT_float,offsetof(Container,mDirection.mZ),4,4);
	mContainerBinding->addField(DFT_float,offsetof(Container,mDirection.mW),4,5);
	mContainerBinding->addField(DFT_float,offsetof(Container,mPosition.mX),4,6);
	mContainerBinding->addField(DFT_float,offsetof(Container,mPosition.mY),4,7);
	mContainerBinding->addField(DFT_float,offsetof(Container,mPosition.mZ),4,8);
	mContainerBinding->addField(DFT_uint32,offsetof(Container,mTanType),4,9);
	mContainerBinding->addField(DFT_bstring,offsetof(Container,mModel),256,10);
	mContainerBinding->addField(DFT_bstring,offsetof(Container,mName),64,11);
	mContainerBinding->addField(DFT_bstring,offsetof(Container,mNameFile),64,12);
	mContainerBinding->addField(DFT_bstring,offsetof(Container,mDetailFile),64,13);
}

//=============================================================================

void ContainerObjectFactory::_destroyDatabindings()
{
	mDatabase->DestroyDataBinding(mContainerBinding);
}

//=============================================================================

void ContainerObjectFactory::handleObjectReady(Object* object,DispatchClient* client)
{
	// gLogger->logMsg("ContainerObjectFactory::handleObjectReady");
	InLoadingContainer* ilc	= _getObject(object->getParentId());
	ilc->mLoadCounter--;

	Container* container = dynamic_cast<Container*>(ilc->mObject);

	// reminder: objects are owned by the global map, containers only keeps references

	// If object with same key already exist in world map, this object will be invalid.
	if (!gWorldManager->existObject(object))
	{
		gWorldManager->addObject(object,true);
		container->addObject(object);
	}
	
	// if (container->getObjectLoadCounter() == (container->getObjects())->size())
	if(!ilc->mLoadCounter)
	{
		// gLogger->logMsg("ContainerObjectFactory::handleObjectReady: DONE!!!");
		container->setLoadState(LoadState_Loaded);
	
		if (!(_removeFromObjectLoadMap(container->getId())))
			gLogger->logMsg("ContainerObjectFactory: Failed removing object from loadmap");

		ilc->mOfCallback->handleObjectReady(container,ilc->mClient);

		mILCPool.free(ilc);
	}
}


