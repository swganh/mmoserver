/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_SCHEMATICMANAGER_H
#define ANH_ZONESERVER_SCHEMATICMANAGER_H

#define	gSchematicManager	SchematicManager::getSingletonPtr()

#include "Utils/typedefs.h"
#include <map>
#include "DatabaseManager/DatabaseCallback.h"
#include <boost/pool/pool.hpp>



//======================================================================================================================

class Database;
class DatabaseCallback;
class DatabaseResult;
class DraftSchematic;
class SchematicGroup;
class ScMAsyncContainer;

typedef std::vector<SchematicGroup*>			SchematicGroupList;
typedef std::map<uint32,DraftSchematic*>		SchematicMap;

//======================================================================================================================

enum ScMQueryType
{
	ScMQuery_SchematicGroups				= 1,
	ScMQuery_GroupSchematics				= 2,
	ScMQuery_DraftSchematic					= 3,
	ScMQuery_SchematicSlots					= 4,
	ScMQuery_SchematicAssemblyBatches		= 5,
	ScMQuery_SchematicExperimentBatches		= 6,
	ScMQuery_SchematicAssemblyWeights		= 7,
	ScMQuery_SchematicExperimentWeights		= 8,
	ScMQuery_ExperimentationGroups			= 9,
	ScMQuery_SchematicCraftBatches			= 10,
	ScMQuery_SchematicCraftWeights			= 11,
	ScMQuery_SchematicCraftAttributeLinks	= 12,
	ScMQuery_SchematicCraftAttributeWeights	= 13
};

//======================================================================================================================

class ScMAsyncContainer
{
	public:

		ScMAsyncContainer(ScMQueryType qt){ mQueryType = qt;}
		~ScMAsyncContainer(){}

		ScMQueryType	mQueryType;
		uint32			mGroupId;
		DraftSchematic*	mSchematic;
		uint32			mBatchId;
};

//======================================================================================================================

class SchematicManager : public DatabaseCallback
{
	public:

		~SchematicManager();
		static SchematicManager*	Init(Database* database);
		static SchematicManager*	getSingletonPtr() { return mSingleton; }

		SchematicGroupList*			getSkillSchematicGroupList(){ return &mSchematicGroupList; }

		virtual void				handleDatabaseJobComplete(void* ref,DatabaseResult* result);

		SchematicGroup*				getSchematicGroupById(uint32 id){ return mSchematicGroupList[id-1]; }
		SchematicMap*				getSlotMap(){ return &mSchematicSlotMap; }
		SchematicMap*				getWeightMap(){ return &mSchematicWeightMap; }
		DraftSchematic*				getSchematicBySlotId(uint32 slotId);
		DraftSchematic*				getSchematicByWeightId(uint32 weightId);

		string						getExpGroup(uint32 groupId){ return mvExpGroups[groupId - 1]; }

		void						releaseAllPoolsMemory(){ mDBAsyncPool.release_memory(); }

	private:

		SchematicManager(Database* database);

		static bool					mInsFlag;
		static SchematicManager*	mSingleton;


		boost::pool<boost::default_user_allocator_malloc_free>				mDBAsyncPool;
		BStringVector				mvExpGroups;
		Database*					mDatabase;

		SchematicGroupList			mSchematicGroupList;
		SchematicMap				mSchematicSlotMap;
		SchematicMap				mSchematicWeightMap;

		uint32						mGroupCount;
		uint32						mGroupLoadCount;
		uint32						mSchematicCount;
};

#endif

