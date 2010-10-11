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

#include "SpawnRegionFactory.h"
#include "SpawnRegion.h"
#include "ObjectFactoryCallback.h"
#include "Common/LogManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"

#include "Utils/utils.h"

//=============================================================================

bool				SpawnRegionFactory::mInsFlag    = false;
SpawnRegionFactory*	SpawnRegionFactory::mSingleton  = NULL;

//======================================================================================================================

SpawnRegionFactory*	SpawnRegionFactory::Init(Database* database)
{
    if(!mInsFlag)
    {
        mSingleton = new SpawnRegionFactory(database);
        mInsFlag = true;
        return mSingleton;
    }
    else
        return mSingleton;
}

//=============================================================================

SpawnRegionFactory::SpawnRegionFactory(Database* database) : FactoryBase(database)
{
    _setupDatabindings();
}

//=============================================================================

SpawnRegionFactory::~SpawnRegionFactory()
{
    _destroyDatabindings();

    mInsFlag = false;
    delete(mSingleton);
}

//=============================================================================

void SpawnRegionFactory::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
    QueryContainerBase* asyncContainer = reinterpret_cast<QueryContainerBase*>(ref);

    switch(asyncContainer->mQueryType)
    {
    case SpawnFQuery_MainData:
    {
        SpawnRegion* spawnRegion = _createSpawnRegion(result);

        if(spawnRegion->getLoadState() == LoadState_Loaded && asyncContainer->mOfCallback)
            asyncContainer->mOfCallback->handleObjectReady(spawnRegion,asyncContainer->mClient);
        else
        {

        }
    }
    break;

    default:
        break;
    }

    mQueryContainerPool.free(asyncContainer);
}

//=============================================================================

void SpawnRegionFactory::requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client)
{
    mDatabase->ExecuteSqlAsync(this,new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(ofCallback,SpawnFQuery_MainData,client),
                               "SELECT spawn_regions.id,spawn_regions.spawn_type,planet_regions.region_name,planet_regions.region_file,planet_regions.x,planet_regions.z,"
                               "planet_regions.width,planet_regions.height,spawn_regions.parent_id,spawn_regions.mission"
                               " FROM spawn_regions"
                               " INNER JOIN planet_regions ON (spawn_regions.region_id = planet_regions.region_id)"
                               " WHERE (spawn_regions.id = %"PRIu64")",id);
   
}

//=============================================================================

SpawnRegion* SpawnRegionFactory::_createSpawnRegion(DatabaseResult* result)
{
    if (!result->getRowCount()) {
    	return nullptr;
    }

    SpawnRegion*	spawnRegion = new SpawnRegion();

    result->GetNextRow(mSpawnRegionBinding,spawnRegion);

    if(spawnRegion->isMission())
    {
        spawnRegion->setType(Region_MissionSpawn);
    }

    spawnRegion->setLoadState(LoadState_Loaded);

    return spawnRegion;
}

//=============================================================================

void SpawnRegionFactory::_setupDatabindings()
{
    mSpawnRegionBinding = mDatabase->CreateDataBinding(10);
    mSpawnRegionBinding->addField(DFT_uint64,offsetof(SpawnRegion,mId),8,0);
    mSpawnRegionBinding->addField(DFT_uint32,offsetof(SpawnRegion,mSpawnType),4,1);
    mSpawnRegionBinding->addField(DFT_bstring,offsetof(SpawnRegion,mRegionName),64,2);
    mSpawnRegionBinding->addField(DFT_bstring,offsetof(SpawnRegion,mNameFile),64,3);
    mSpawnRegionBinding->addField(DFT_float,offsetof(SpawnRegion,mPosition.x),4,4);
    mSpawnRegionBinding->addField(DFT_float,offsetof(SpawnRegion,mPosition.z),4,5);
    mSpawnRegionBinding->addField(DFT_float,offsetof(SpawnRegion,mWidth),4,6);
    mSpawnRegionBinding->addField(DFT_float,offsetof(SpawnRegion,mHeight),4,7);
    mSpawnRegionBinding->addField(DFT_uint64,offsetof(SpawnRegion,mParentId),8,8);
    mSpawnRegionBinding->addField(DFT_uint32,offsetof(SpawnRegion,mMission),4,9);
}

//=============================================================================

void SpawnRegionFactory::_destroyDatabindings()
{
    mDatabase->DestroyDataBinding(mSpawnRegionBinding);
}

//=============================================================================

