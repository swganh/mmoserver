/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2014 The SWG:ANH Team
---------------------------------------------------------------------------------------
Use of this source code is governed by the GPL v3 license that can be found
in the COPYING file or at http://www.gnu.org/licenses/gpl-3.0.html

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
---------------------------------------------------------------------------------------
*/

#include "ZoneServer/GameSystemManagers/Resource Manager/ResourceContainerFactory.h"


#include "anh/logger.h"
#include "ZoneServer\Objects\Object\ObjectManager.h"
#include "ZoneServer/Objects/Object/ObjectFactoryCallback.h"

#include "Resource.h"
#include "ZoneServer/GameSystemManagers/Resource Manager/ResourceContainer.h"
#include "ZoneServer/GameSystemManagers/Resource Manager/ResourceManager.h"
#include "ZoneServer/GameSystemManagers/Resource Manager/ResourceType.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"

#include "Utils/utils.h"

//=============================================================================

bool						ResourceContainerFactory::mInsFlag    = false;
ResourceContainerFactory*	ResourceContainerFactory::mSingleton  = NULL;

//======================================================================================================================

ResourceContainerFactory*	ResourceContainerFactory::Init(swganh::app::SwganhKernel*	kernel)
{
    if(!mInsFlag)
    {
        mSingleton = new ResourceContainerFactory(kernel);
        mInsFlag = true;
        return mSingleton;
    }
    else
        return mSingleton;
}

void ResourceContainerFactory::saveLocation(Object* object)
{
	std::stringstream sql;
	sql << "UPDATE " << mDatabase->galaxy() << ".resource_containers SET parent_id ='" << object->getParentId() << "', "
		<< "oX='" << object->mDirection.x << "', oY='" << object->mDirection.y << "', oZ='" << object->mDirection.z << "', "
		<< "oW='" << object->mDirection.w << "', x='" << object->mPosition.x << "', y='" << object->mPosition.x << "', z='" << object->mPosition.x << "' WHERE id='" << object->getId() << "'";

	mDatabase->executeSqlAsync(0,0, sql.str());
}

//=============================================================================

ResourceContainerFactory::ResourceContainerFactory(swganh::app::SwganhKernel*	kernel) : FactoryBase(kernel)
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

void ResourceContainerFactory::handleDatabaseJobComplete(void* ref,swganh::database::DatabaseResult* result)
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

			std::stringstream sql;

			sql << "SELECT attributes.name,object_attributes.value,attributes.internal FROM " << mDatabase->galaxy() << ".object_attributes"
				<< " INNER JOIN " << mDatabase->galaxy() << ".attributes ON (object_attributes.attribute_id = attributes.id)"
				<< " WHERE object_attributes.object_id = " << container->getId() << " ORDER BY object_attributes.order";                       

            mDatabase->executeSqlAsync(this,asContainer, sql.str());
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

    default:
        break;
    }

    mQueryContainerPool.free(asyncContainer);
}

//=============================================================================

void ResourceContainerFactory::requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client)
{
    mDatabase->executeSqlAsync(this,new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(ofCallback,RCFQuery_MainData,client),"SELECT * FROM %s.resource_containers WHERE id=%"PRIu64"",mDatabase->galaxy(),id);
    
}

//=============================================================================>>

ResourceContainer* ResourceContainerFactory::_createResourceContainer(swganh::database::DatabaseResult* result)
{
    if (!result->getRowCount()) {
    	return nullptr;
    }

    ResourceContainer*	resourceContainer = new ResourceContainer();

    result->getNextRow(mResourceContainerBinding,(void*)resourceContainer);

    Resource* resource = gResourceManager->getResourceById(resourceContainer->mResourceId);

    if(resource != nullptr)
    {
        resourceContainer->setResource(resource);
        resourceContainer->SetTemplate((resource->getType())->GetTemplate());
    } else {
    	LOG(warning) << "Resource not found [" << resourceContainer->mResourceId << "]";
    }

	gObjectManager->LoadSlotsForObject(resourceContainer);
	
	auto permissions_objects_ = gObjectManager->GetPermissionsMap();
	resourceContainer->SetPermissions(permissions_objects_.find(swganh::object::DEFAULT_PERMISSION)->second.get());//CREATURE_CONTAINER_PERMISSION

    resourceContainer->mMaxCondition = 100;

    resourceContainer->setLoadState(LoadState_Attributes);

    return resourceContainer;
}

//=============================================================================

void ResourceContainerFactory::_setupDatabindings()
{
    mResourceContainerBinding = mDatabase->createDataBinding(11);
    mResourceContainerBinding->addField(swganh::database::DFT_uint64,offsetof(ResourceContainer,mId),8,0);
    mResourceContainerBinding->addField(swganh::database::DFT_uint64,offsetof(ResourceContainer,mParentId),8,1);
    mResourceContainerBinding->addField(swganh::database::DFT_uint64,offsetof(ResourceContainer,mResourceId),8,2);
    mResourceContainerBinding->addField(swganh::database::DFT_uint32,offsetof(ResourceContainer,mAmount),4,11);
    mResourceContainerBinding->addField(swganh::database::DFT_float,offsetof(ResourceContainer,mDirection.x),4,3);
    mResourceContainerBinding->addField(swganh::database::DFT_float,offsetof(ResourceContainer,mDirection.y),4,4);
    mResourceContainerBinding->addField(swganh::database::DFT_float,offsetof(ResourceContainer,mDirection.z),4,5);
    mResourceContainerBinding->addField(swganh::database::DFT_float,offsetof(ResourceContainer,mDirection.w),4,6);
    mResourceContainerBinding->addField(swganh::database::DFT_float,offsetof(ResourceContainer,mPosition.x),4,7);
    mResourceContainerBinding->addField(swganh::database::DFT_float,offsetof(ResourceContainer,mPosition.y),4,8);
    mResourceContainerBinding->addField(swganh::database::DFT_float,offsetof(ResourceContainer,mPosition.z),4,9);
}

//=============================================================================

void ResourceContainerFactory::_destroyDatabindings()
{
    mDatabase->destroyDataBinding(mResourceContainerBinding);
}

//=============================================================================

