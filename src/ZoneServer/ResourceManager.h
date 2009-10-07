/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
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

		RMAsyncContainer(RMQueryType qt){ mQueryType = qt;}
		~RMAsyncContainer(){}

		RMQueryType	mQueryType;
};

//======================================================================================================================

class ResourceManager : public DatabaseCallback
{
	public:

		~ResourceManager();
		static ResourceManager*		Init(Database* database,uint32 zoneId);
		static ResourceManager*		getSingletonPtr() { return mSingleton; }

		virtual void				handleDatabaseJobComplete(void* ref,DatabaseResult* result);

		Resource*					getResourceById(uint64 id);
		Resource*					getResourceByNameCRC(uint32 crc);
		ResourceType*				getResourceTypeById(uint32 id);
		ResourceCategory*			getResourceCategoryById(uint32 id);

		// get map references
		ResourceIdMap*				getResourceIdMap();
		ResourceCRCNameMap*			getResourceCRCNameMap();
		ResourceTypeMap*			getResourceTypeMap();
		ResourceCategoryMap*		getResourceCategoryMap();

		void						releaseAllPoolsMemory();

	private:

		ResourceManager(Database* database,uint32 zoneId);

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

		boost::pool<boost::default_user_allocator_malloc_free>				mDBAsyncPool;

		DataBinding*				mResourceTypebinding;
		DataBinding*				mResourceCategorybinding;
		DataBinding*				mResourceBinding;
		DataBinding*				mCurrentResourceBinding;
};

//======================================================================================================================

#endif

