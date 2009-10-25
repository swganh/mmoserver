/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "ResourceContainerFactory.h"
#include "ObjectFactoryCallback.h"
#include "Resource.h"
#include "ResourceContainer.h"
#include "ResourceManager.h"
#include "ResourceType.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include "LogManager/LogManager.h"
#include "Utils/utils.h"
#include <assert.h>

//=============================================================================

bool						ResourceContainerFactory::mInsFlag    = false;
ResourceContainerFactory*	ResourceContainerFactory::mSingleton  = NULL;

//======================================================================================================================

ResourceContainerFactory*	ResourceContainerFactory::Init(Database* database)
{
	if(!mInsFlag)
	{
		mSingleton = new ResourceContainerFactory(database);
		mInsFlag = true;
		return mSingleton;
	}
	else
		return mSingleton;
}

//=============================================================================

ResourceContainerFactory::ResourceContainerFactory(Database* database) : FactoryBase(database)
{
	_setupDatabindings();
}

//=============================================================================

ResourceContainerFactory::~ResourceContainerFactory()
{
	_destroyDatabindings();

	mInsFlag = false;
	delete(mSingleton);
}

//=============================================================================

void ResourceContainerFactory::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
	QueryContainerBase* asyncContainer = reinterpret_cast<QueryContainerBase*>(ref);

	switch(asyncContainer->mQueryType)
	{
		case RCFQuery_MainData:
		{
			ResourceContainer* container = _createResourceContainer(result);

			if(container->getLoadState() == LoadState_Loaded && asyncContainer->mOfCallback)
				asyncContainer->mOfCallback->handleObjectReady(container,asyncContainer->mClient);
			
			else if(container->getLoadState() == LoadState_Attributes)
			{
				QueryContainerBase* asContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,RCFQuery_Attributes,asyncContainer->mClient);
				asContainer->mObject = container;

				mDatabase->ExecuteSqlAsync(this,asContainer,"SELECT attributes.name,object_attributes.value,attributes.internal"
															" FROM object_attributes"
															" INNER JOIN attributes ON (object_attributes.attribute_id = attributes.id)"
															" WHERE object_attributes.object_id = %"PRId64" ORDER BY object_attributes.order",container->getId());
			}
		}
		break;

		case RCFQuery_Attributes:
		{
			_buildAttributeMap(asyncContainer->mObject,result);

			if(asyncContainer->mObject->getLoadState() == LoadState_Loaded && asyncContainer->mOfCallback)
				asyncContainer->mOfCallback->handleObjectReady(asyncContainer->mObject,asyncContainer->mClient);
		}
		break;

		default:break;
	}

	mQueryContainerPool.free(asyncContainer);
}

//=============================================================================

void ResourceContainerFactory::requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client)
{
	mDatabase->ExecuteSqlAsync(this,new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(ofCallback,RCFQuery_MainData,client),"SELECT * FROM resource_containers WHERE id=%"PRId64"",id);
}

//=============================================================================>>

ResourceContainer* ResourceContainerFactory::_createResourceContainer(DatabaseResult* result)
{
	ResourceContainer*	resourceContainer = new ResourceContainer();

	uint64 count = result->getRowCount();
	assert(count == 1);

	result->GetNextRow(mResourceContainerBinding,(void*)resourceContainer);

	Resource* resource = gResourceManager->getResourceById(resourceContainer->mResourceId);

	if(resource != NULL)
	{
		resourceContainer->setResource(resource);
		resourceContainer->setModelString((resource->getType())->getContainerModel().getAnsi());
	}
	else
		gLogger->logMsgF("ResourceContainerFactory::_createResourceContainer: Resource not found %"PRId64"",MSG_HIGH,resourceContainer->mResourceId);

	resourceContainer->mMaxCondition = 100;

	resourceContainer->setLoadState(LoadState_Attributes);

	return resourceContainer;
}

//=============================================================================

void ResourceContainerFactory::_setupDatabindings()
{
	mResourceContainerBinding = mDatabase->CreateDataBinding(11);
	mResourceContainerBinding->addField(DFT_uint64,offsetof(ResourceContainer,mId),8,0);
	mResourceContainerBinding->addField(DFT_uint64,offsetof(ResourceContainer,mParentId),8,1);
	mResourceContainerBinding->addField(DFT_uint64,offsetof(ResourceContainer,mResourceId),8,2);
	mResourceContainerBinding->addField(DFT_uint32,offsetof(ResourceContainer,mAmount),4,11);
	mResourceContainerBinding->addField(DFT_float,offsetof(ResourceContainer,mDirection.mX),4,3);
	mResourceContainerBinding->addField(DFT_float,offsetof(ResourceContainer,mDirection.mY),4,4);
	mResourceContainerBinding->addField(DFT_float,offsetof(ResourceContainer,mDirection.mZ),4,5);
	mResourceContainerBinding->addField(DFT_float,offsetof(ResourceContainer,mDirection.mW),4,6);
	mResourceContainerBinding->addField(DFT_float,offsetof(ResourceContainer,mPosition.mX),4,7);
	mResourceContainerBinding->addField(DFT_float,offsetof(ResourceContainer,mPosition.mY),4,8);
	mResourceContainerBinding->addField(DFT_float,offsetof(ResourceContainer,mPosition.mZ),4,9);
}

//=============================================================================

void ResourceContainerFactory::_destroyDatabindings()
{
	mDatabase->DestroyDataBinding(mResourceContainerBinding);
}

//=============================================================================

