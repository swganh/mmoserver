/*
 This file is part of MMOServer. For more information, visit http://swganh.com
 
 Copyright (c) 2006 - 2011 The SWG:ANH Team

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef ANH_ZONESERVER_RESOURCEMANAGER_H
#define ANH_ZONESERVER_RESOURCEMANAGER_H

#define	 gResourceManager	ResourceManager::getSingletonPtr()

#include "Utils/typedefs.h"
#include <map>
#include <boost/pool/pool.hpp>
#include "DatabaseManager/DatabaseCallback.h"


//======================================================================================================================

class Database;
class DatabaseCallback;
class DatabaseResult;
class DataBinding;
class Resource;
class ResourceCategory;
class ResourceType;

//======================================================================================================================

enum RMQueryType
{
    RMQuery_ResourceTypes		= 1,
    RMQuery_OldResources		= 2,
    RMQuery_CurrentResources	= 3,
    RMQuery_Categories			= 4,
    RMQuery_DepleteResources	= 5
};

//======================================================================================================================

typedef std::map<uint32,ResourceCategory*>	ResourceCategoryMap;
typedef std::map<uint32,ResourceType*>		ResourceTypeMap;
typedef std::map<uint64,Resource*>			ResourceIdMap;
typedef std::map<uint32,Resource*>			ResourceCRCNameMap;

//======================================================================================================================

class RMAsyncContainer
{
public:

    RMAsyncContainer(RMQueryType qt) {
        mQueryType = qt;
    }
    ~RMAsyncContainer() {}

    RMQueryType	mQueryType;
    Resource*	mCurrentResource;
};

//======================================================================================================================

class ResourceManager : public DatabaseCallback
{
public:

    ~ResourceManager();
    static ResourceManager*		Init(Database* database,uint32 zoneId, bool writeResourceMaps, std::string zoneName);
    static ResourceManager*		getSingletonPtr() {
        return mSingleton;
    }

    virtual void				handleDatabaseJobComplete(void* ref,DatabaseResult* result);

    Resource*					getResourceById(uint64 id);
    Resource*					getResourceByNameCRC(uint32 crc);
    ResourceType*				getResourceTypeById(uint32 id);
    ResourceCategory*			getResourceCategoryById(uint32 id);

    bool						setResourceDepletion(Resource* resource, int32 amt);
    // get map references
    ResourceIdMap*				getResourceIdMap();
    ResourceCRCNameMap*			getResourceCRCNameMap();
    ResourceTypeMap*			getResourceTypeMap();
    ResourceCategoryMap*		getResourceCategoryMap();

    void						releaseAllPoolsMemory();

private:

    ResourceManager(Database* database,uint32 zoneId, bool writeResourceMaps, std::string zoneName);

    void						_setupDatabindings();
    void						_destroyDatabindings();

    static bool					mInsFlag;
    static ResourceManager*		mSingleton;

    Database*					mDatabase;

    ResourceIdMap				mResourceIdMap;
    ResourceCRCNameMap			mResourceCRCNameMap;
    ResourceTypeMap				mResourceTypeMap;
    ResourceCategoryMap			mResourceCategoryMap;

    uint32						mZoneId;
	std::string					mZoneName;
	bool						mWriteResourceMaps;

    boost::pool<boost::default_user_allocator_malloc_free>				mDBAsyncPool;

    DataBinding*				mResourceTypebinding;
    DataBinding*				mResourceCategorybinding;
    DataBinding*				mResourceBinding;
    DataBinding*				mCurrentResourceBinding;
};

//======================================================================================================================

#endif

