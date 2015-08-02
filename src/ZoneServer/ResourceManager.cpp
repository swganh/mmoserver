/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2010 The SWG:ANH Team
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

#include "ResourceManager.h"


#include "Utils/logger.h"

#include "CurrentResource.h"
#include "ResourceCategory.h"
#include "ResourceType.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"

//======================================================================================================================

bool				ResourceManager::mInsFlag	= false;
ResourceManager*	ResourceManager::mSingleton = NULL;

//======================================================================================================================

ResourceManager::ResourceManager(Database* database,uint32 zoneId, bool writeResourceMaps, std::string zoneName) :
    mDatabase(database),
    mZoneId(zoneId),
	mZoneName(zoneName),
	mWriteResourceMaps(writeResourceMaps),
    mDBAsyncPool(sizeof(RMAsyncContainer))
{
    // init our tree with a root
    ResourceCategory* rootCat = new ResourceCategory();
    rootCat->setId(1);
    rootCat->setName("Resource");
    rootCat->setDescriptor("resource");
    rootCat->setParentId(0);
    mResourceCategoryMap.insert(std::make_pair(1,rootCat));

    //the tutorial will not assign us a db so we cannot load resources
    //this way we will save the resources and time but still have a resourcemanager for our lowlevelID lookups
    if(!database)
        return;

    _setupDatabindings();

    // load resource types
    mDatabase->executeSqlAsync(this,new(mDBAsyncPool.ordered_malloc()) RMAsyncContainer(RMQuery_ResourceTypes),
                               "SELECT id,category_id,namefile_name,type_name,type_swg,tang,bazaar_catID,type FROM %s.resource_template ORDER BY id",mDatabase->galaxy());
 
}

//======================================================================================================================

ResourceManager* ResourceManager::Init(Database* database,uint32 zoneId, bool writeResourceMaps, std::string zoneName)
{
    if(mInsFlag == false)
    {
        mSingleton = new ResourceManager(database,zoneId, writeResourceMaps, zoneName);
        mInsFlag = true;
        return mSingleton;
    }
    else
        return mSingleton;
}

//======================================================================================================================

ResourceManager::~ResourceManager()
{
    _destroyDatabindings();

    ResourceCategoryMap::iterator catIt = mResourceCategoryMap.begin();

    while(catIt != mResourceCategoryMap.end())
    {
        delete((*catIt).second);

        mResourceCategoryMap.erase(catIt++);
    }

    ResourceTypeMap::iterator typeIt = mResourceTypeMap.begin();

    while(typeIt != mResourceTypeMap.end())
    {
        delete((*typeIt).second);

        mResourceTypeMap.erase(typeIt++);
    }

    ResourceIdMap::iterator resIt = mResourceIdMap.begin();

    while(resIt != mResourceIdMap.end())
    {
        delete((*resIt).second);

        mResourceIdMap.erase(resIt++);
    }

    mInsFlag = false;
    delete(mSingleton);
}

//======================================================================================================================

void ResourceManager::_setupDatabindings()
{
    mResourceTypebinding = mDatabase->createDataBinding(8);
    mResourceTypebinding->addField(DFT_uint32,offsetof(ResourceType,mId),4,0);
    mResourceTypebinding->addField(DFT_uint16,offsetof(ResourceType,mCatId),2,1);
    mResourceTypebinding->addField(DFT_bstring,offsetof(ResourceType,mTypeName),64,2);
    mResourceTypebinding->addField(DFT_bstring,offsetof(ResourceType,mName),255,3);
    mResourceTypebinding->addField(DFT_bstring,offsetof(ResourceType,mDescriptor),255,4);
    mResourceTypebinding->addField(DFT_bstring,offsetof(ResourceType,mModel),255,5);
    mResourceTypebinding->addField(DFT_uint32,offsetof(ResourceType,mCategoryBazaar),4,6);
    mResourceTypebinding->addField(DFT_bstring,offsetof(ResourceType,mResourceType),255,7);

    mResourceCategorybinding = mDatabase->createDataBinding(4);
    mResourceCategorybinding->addField(DFT_uint32,offsetof(ResourceCategory,mId),4,0);
    mResourceCategorybinding->addField(DFT_bstring,offsetof(ResourceCategory,mName),255,1);
    mResourceCategorybinding->addField(DFT_bstring,offsetof(ResourceCategory,mDescriptor),255,2);
    mResourceCategorybinding->addField(DFT_uint32,offsetof(ResourceCategory,mParentId),4,3);

    mResourceBinding = mDatabase->createDataBinding(14);
    mResourceBinding->addField(DFT_uint64,offsetof(Resource,mId),8,0);
    mResourceBinding->addField(DFT_bstring,offsetof(Resource,mName),255,1);
    mResourceBinding->addField(DFT_uint32,offsetof(Resource,mTypeId),4,2);
    mResourceBinding->addField(DFT_uint16,offsetof(Resource,mAttributes[ResAttr_ER]),2,3);
    mResourceBinding->addField(DFT_uint16,offsetof(Resource,mAttributes[ResAttr_CR]),2,4);
    mResourceBinding->addField(DFT_uint16,offsetof(Resource,mAttributes[ResAttr_CD]),2,5);
    mResourceBinding->addField(DFT_uint16,offsetof(Resource,mAttributes[ResAttr_DR]),2,6);
    mResourceBinding->addField(DFT_uint16,offsetof(Resource,mAttributes[ResAttr_FL]),2,7);
    mResourceBinding->addField(DFT_uint16,offsetof(Resource,mAttributes[ResAttr_HR]),2,8);
    mResourceBinding->addField(DFT_uint16,offsetof(Resource,mAttributes[ResAttr_MA]),2,9);
    mResourceBinding->addField(DFT_uint16,offsetof(Resource,mAttributes[ResAttr_OQ]),2,10);
    mResourceBinding->addField(DFT_uint16,offsetof(Resource,mAttributes[ResAttr_SR]),2,11);
    mResourceBinding->addField(DFT_uint16,offsetof(Resource,mAttributes[ResAttr_UT]),2,12);
    mResourceBinding->addField(DFT_uint16,offsetof(Resource,mAttributes[ResAttr_PE]),2,13);

    mCurrentResourceBinding = mDatabase->createDataBinding(25);
    mCurrentResourceBinding->addField(DFT_uint64,offsetof(CurrentResource,mId),8,0);
    mCurrentResourceBinding->addField(DFT_bstring,offsetof(CurrentResource,mName),255,1);
    mCurrentResourceBinding->addField(DFT_uint32,offsetof(CurrentResource,mTypeId),4,2);
    mCurrentResourceBinding->addField(DFT_uint16,offsetof(CurrentResource,mAttributes[ResAttr_ER]),2,3);
    mCurrentResourceBinding->addField(DFT_uint16,offsetof(CurrentResource,mAttributes[ResAttr_CR]),2,4);
    mCurrentResourceBinding->addField(DFT_uint16,offsetof(CurrentResource,mAttributes[ResAttr_CD]),2,5);
    mCurrentResourceBinding->addField(DFT_uint16,offsetof(CurrentResource,mAttributes[ResAttr_DR]),2,6);
    mCurrentResourceBinding->addField(DFT_uint16,offsetof(CurrentResource,mAttributes[ResAttr_FL]),2,7);
    mCurrentResourceBinding->addField(DFT_uint16,offsetof(CurrentResource,mAttributes[ResAttr_HR]),2,8);
    mCurrentResourceBinding->addField(DFT_uint16,offsetof(CurrentResource,mAttributes[ResAttr_MA]),2,9);
    mCurrentResourceBinding->addField(DFT_uint16,offsetof(CurrentResource,mAttributes[ResAttr_OQ]),2,10);
    mCurrentResourceBinding->addField(DFT_uint16,offsetof(CurrentResource,mAttributes[ResAttr_SR]),2,11);
    mCurrentResourceBinding->addField(DFT_uint16,offsetof(CurrentResource,mAttributes[ResAttr_UT]),2,12);
    mCurrentResourceBinding->addField(DFT_uint16,offsetof(CurrentResource,mAttributes[ResAttr_PE]),2,13);
    mCurrentResourceBinding->addField(DFT_double,offsetof(CurrentResource,mNoiseMapBoundsX1),8,14);
    mCurrentResourceBinding->addField(DFT_double,offsetof(CurrentResource,mNoiseMapBoundsX2),8,15);
    mCurrentResourceBinding->addField(DFT_double,offsetof(CurrentResource,mNoiseMapBoundsY1),8,16);
    mCurrentResourceBinding->addField(DFT_double,offsetof(CurrentResource,mNoiseMapBoundsY2),8,17);
    mCurrentResourceBinding->addField(DFT_uint8,offsetof(CurrentResource,mNoiseMapOctaves),1,18);
    mCurrentResourceBinding->addField(DFT_double,offsetof(CurrentResource,mNoiseMapFrequency),8,19);
    mCurrentResourceBinding->addField(DFT_double,offsetof(CurrentResource,mNoiseMapPersistence),8,20);
    mCurrentResourceBinding->addField(DFT_double,offsetof(CurrentResource,mNoiseMapScale),8,21);
    mCurrentResourceBinding->addField(DFT_double,offsetof(CurrentResource,mNoiseMapBias),8,22);
    mCurrentResourceBinding->addField(DFT_uint64,offsetof(CurrentResource,mUnitsTotal),8,23);
    mCurrentResourceBinding->addField(DFT_uint64,offsetof(CurrentResource,mUnitsLeft),8,24);

}

//======================================================================================================================

void ResourceManager::_destroyDatabindings()
{
    mDatabase->destroyDataBinding(mResourceTypebinding);
    mDatabase->destroyDataBinding(mResourceCategorybinding);
    mDatabase->destroyDataBinding(mResourceBinding);
    mDatabase->destroyDataBinding(mCurrentResourceBinding);
}

//======================================================================================================================

void ResourceManager::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
    RMAsyncContainer* asyncContainer = reinterpret_cast<RMAsyncContainer*>(ref);

    switch(asyncContainer->mQueryType)
    {
    case RMQuery_ResourceTypes:
    {
        ResourceType* resType;

        uint64 count = result->getRowCount();

        for(uint64 i = 0; i < count; i++)
        {
            resType = new ResourceType();

            result->getNextRow(mResourceTypebinding,resType);
            mResourceTypeMap.insert(std::make_pair(resType->mId,resType));
        }

        // query categories
        mDatabase->executeSqlAsync(this,new(mDBAsyncPool.ordered_malloc()) RMAsyncContainer(RMQuery_Categories),"SELECT * FROM %s.resource_categories ORDER BY id",mDatabase->galaxy());
    }
    break;

    case RMQuery_Categories:
    {
        ResourceCategory* category;

        uint64 count = result->getRowCount();

        for(uint64 i = 0; i < count; i++)
        {
            category = new ResourceCategory();

            result->getNextRow(mResourceCategorybinding,category);
            (getResourceCategoryById(category->mParentId))->insertCategory(category);
            mResourceCategoryMap.insert(std::make_pair(category->mId,category));
        }


        mDatabase->executeSqlAsync(this,new(mDBAsyncPool.ordered_malloc()) RMAsyncContainer(RMQuery_CurrentResources),
                                   "SELECT resources.id,resources.name,resources.type_id,"
                                   "resources.er,resources.cr,resources.cd,resources.dr,resources.fl,resources.hr,"
                                   "resources.ma,resources.oq,resources.sr,resources.ut,resources.pe,"
                                   "resources_spawn_config.noiseMapBoundsX1,resources_spawn_config.noiseMapBoundsX2,"
                                   "resources_spawn_config.noiseMapBoundsY1,resources_spawn_config.noiseMapBoundsY2,"
                                   "resources_spawn_config.noiseMapOctaves,resources_spawn_config.noiseMapFrequency,"
                                   "resources_spawn_config.noiseMapPersistence,resources_spawn_config.noiseMapScale,"
                                   "resources_spawn_config.noiseMapBias,"
                                   "resources_spawn_config.unitsTotal,resources_spawn_config.unitsLeft"
                                   " FROM %s.resources"
                                   " INNER JOIN %s.resources_spawn_config ON (resources.id = resources_spawn_config.resource_id)"
                                   " WHERE"
                                   " (resources_spawn_config.planet_id = %u) AND"
                                   " (resources.active = 1)",mDatabase->galaxy(),mDatabase->galaxy(),mZoneId);
        
    }
    break;

    case RMQuery_OldResources:
    {
        Resource* resource;

        uint64 count = result->getRowCount();
        //gLogger->log(LogManager::INFORMATION,"Loading %u Old Resources",count);

        for(uint64 i = 0; i < count; i++)
        {
            resource = new Resource();

            result->getNextRow(mResourceBinding,resource);

            if(getResourceById(resource->mId) == NULL)
            {
                resource->mType = getResourceTypeById(resource->mTypeId);
                resource->mCurrent = 0;
                mResourceIdMap.insert(std::make_pair(resource->mId,resource));
                mResourceCRCNameMap.insert(std::make_pair(resource->mName.getCrc(),resource));
                (getResourceCategoryById(resource->mType->mCatId))->insertResource(resource);
            }
            else
                delete(resource);
        }

		if (count)
		{
			LOG(INFO) << "Loaded " << count << " resources";
		}
    }
    break;

    case RMQuery_CurrentResources:
    {
        CurrentResource* resource;

        uint64 count = result->getRowCount();
        for(uint64 i = 0; i < count; i++)
        {
            resource = new CurrentResource(mWriteResourceMaps, mZoneName);

            result->getNextRow(mCurrentResourceBinding,resource);
            resource->mType = getResourceTypeById(resource->mTypeId);
            resource->mCurrent = 1;
            resource->buildDistributionMap();
			mResourceIdMap.insert(std::make_pair(resource->mId,resource));
            mResourceCRCNameMap.insert(std::make_pair(resource->mName.getCrc(),resource));
            (getResourceCategoryById(resource->mType->mCatId))->insertResource(resource);
        }

		if (count)
		{
			LOG(INFO) << "Generated " << count << " resource maps";
		}

        // query old and current resources not from this planet
        mDatabase->executeSqlAsync(this,new(mDBAsyncPool.ordered_malloc()) RMAsyncContainer(RMQuery_OldResources),
									"SELECT * FROM %s.resources "
									" INNER JOIN %s.resources_spawn_config ON (resources.id = resources_spawn_config.resource_id)"
									" WHERE (NOT ("
									" (resources_spawn_config.planet_id = %u) AND"
									" (resources.active = 1)))",mDatabase->galaxy(),mDatabase->galaxy(),mZoneId);
		// dont load the active resources of this zone again!!!
    }
    break;
    case RMQuery_DepleteResources:
    {
        // do we have a return?
        // if the return is 0 we need to do nothing else
        // if it is 1, we need to execute sql async to set the active resource to 0
        // this means the resource is depleted. (can we do this via a stored proc?)
        uint32 returnId = 0;
        DataBinding* binding = mDatabase->createDataBinding(1);
        binding->addField(DFT_uint32,0,4);
        result->getNextRow(binding,&returnId);
        mDatabase->destroyDataBinding(binding);
        if (returnId == 1)
        {
            // remove from map
            Resource* resource = asyncContainer->mCurrentResource;
            mResourceCRCNameMap.erase(resource->mName.getCrc());
            mResourceIdMap.erase(resource->getId());
            (getResourceCategoryById(resource->mType->mCatId))->removeResource(resource);
        }
    }
    default:
        break;
    }

    mDBAsyncPool.ordered_free(asyncContainer);
}

//======================================================================================================================

ResourceType* ResourceManager::getResourceTypeById(uint32 id)
{
    ResourceTypeMap::iterator it = mResourceTypeMap.find(id);

    if(it != mResourceTypeMap.end())
        return((*it).second);

    return(NULL);
}

//======================================================================================================================

Resource* ResourceManager::getResourceById(uint64 id)
{
    ResourceIdMap::iterator it = mResourceIdMap.find(id);

    if(it != mResourceIdMap.end())
        return((*it).second);

    return(NULL);
}

//======================================================================================================================

Resource* ResourceManager::getResourceByNameCRC(uint32 crc)
{
    ResourceCRCNameMap::iterator it = mResourceCRCNameMap.find(crc);

    if(it != mResourceCRCNameMap.end())
        return((*it).second);

    return(NULL);
}

//======================================================================================================================

ResourceCategory* ResourceManager::getResourceCategoryById(uint32 id)
{
    ResourceCategoryMap::iterator it = mResourceCategoryMap.find(id);

    if(it != mResourceCategoryMap.end())
        return((*it).second);

    return(NULL);
}

bool ResourceManager::setResourceDepletion(Resource* resource, int32 amt)
{
    RMAsyncContainer* asyncContainer = new RMAsyncContainer(RMQuery_DepleteResources);
    // check resource against current map and make sure it's active
    ResourceIdMap::iterator it = mResourceIdMap.find(resource->getId());
    if (it != mResourceIdMap.end() && resource->getCurrent() != 0)
    {
        asyncContainer->mCurrentResource = resource;
        mDatabase->executeSqlAsync(this,asyncContainer,"update resources_spawn_config set unitsLeft = unitsLeft - %u where resource_id = %" PRIu64 "",amt ,resource->getId());
        
    }
    else
    {
        LOG(WARNING) << "Resource " << resource->getName().getAnsi() << " was not found or is inactive";
        return false;
    }

    return true;
}
//======================================================================================================================


ResourceIdMap* ResourceManager::getResourceIdMap()
{
    return &mResourceIdMap;
}


ResourceCRCNameMap* ResourceManager::getResourceCRCNameMap()
{
    return &mResourceCRCNameMap;
}


ResourceTypeMap* ResourceManager::getResourceTypeMap()
{
    return &mResourceTypeMap;
}


ResourceCategoryMap* ResourceManager::getResourceCategoryMap()
{
    return &mResourceCategoryMap;
}


void ResourceManager::releaseAllPoolsMemory()
{
    mDBAsyncPool.release_memory();
}
