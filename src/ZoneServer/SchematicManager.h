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

#ifndef ANH_ZONESERVER_SCHEMATICMANAGER_H
#define ANH_ZONESERVER_SCHEMATICMANAGER_H

#define	gSchematicManager	SchematicManager::getSingletonPtr()

#include "Utils/bstring.h"
#include "Utils/typedefs.h"
#include <map>
#include "DatabaseManager/DatabaseCallback.h"
#include <boost/pool/pool.hpp>
#include <boost/unordered_map.hpp>


//======================================================================================================================

class Database;
class DatabaseCallback;
class DatabaseResult;
class DraftSchematic;
class SchematicGroup;
class ScMAsyncContainer;

typedef std::vector<SchematicGroup*>			SchematicGroupList;
typedef std::map<uint32,DraftSchematic*>		SchematicMap;
//typedef std::vector<DraftSchematic*>
typedef boost::unordered_map<uint32, DraftSchematic*> SchematicList;

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

    ScMAsyncContainer(ScMQueryType qt) {
        mQueryType = qt;
    }
    ~ScMAsyncContainer() {}

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
    static SchematicManager*	getSingletonPtr() {
        return mSingleton;
    }

    SchematicGroupList*			getSkillSchematicGroupList() {
        return &mSchematicGroupList;
    }

    virtual void				handleDatabaseJobComplete(void* ref,DatabaseResult* result);

    SchematicGroup*				getSchematicGroupById(uint32 id) {
        return mSchematicGroupList[id-1];
    }
    SchematicMap*				getSlotMap() {
        return &mSchematicSlotMap;
    }
    SchematicMap*				getWeightMap() {
        return &mSchematicWeightMap;
    }
    DraftSchematic*				getSchematicBySlotId(uint32 slotId);
    DraftSchematic*				getSchematicByWeightId(uint32 weightId);
    //DraftSchematic*				getSchematicByID(uint64 schematicId);
    //DraftSchematic*				getSchematicByID(uint64 schematicId, uint32 groupid);
    DraftSchematic*				getSchematicByWeightID(uint32 weightsbatch_Id);
    BString						getExpGroup(uint32 groupId) {
        return mvExpGroups[groupId - 1];
    }

    void						releaseAllPoolsMemory() {
        mDBAsyncPool.release_memory();
    }

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
    SchematicList				mSchematicList;

    uint32						mGroupCount;
    uint32						mGroupLoadCount;
    uint32						mSchematicCount;
};

#endif

