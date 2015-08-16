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

#include "BuildingFactory.h"


#include "Utils/logger.h"

#include "BuildingObject.h"
#include "CellFactory.h"
#include "CellObject.h"
#include "SpawnPoint.h"
#include "WorldManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/DatabaseResult.h"
#include "Utils/utils.h"

//=============================================================================

bool				BuildingFactory::mInsFlag    = false;
BuildingFactory*	BuildingFactory::mSingleton  = NULL;

//======================================================================================================================

BuildingFactory*	BuildingFactory::Init(Database* database)
{
    if(!mInsFlag)
    {
        mSingleton = new BuildingFactory(database);
        mInsFlag = true;
        return mSingleton;
    }
    else
        return mSingleton;
}

//=============================================================================

BuildingFactory::BuildingFactory(Database* database) : FactoryBase(database)
{
    mCellFactory = CellFactory::Init(mDatabase);

    _setupDatabindings();
}

//=============================================================================

BuildingFactory::~BuildingFactory()
{
    _destroyDatabindings();

    mInsFlag = false;
    delete(mSingleton);
}

//=============================================================================

void BuildingFactory::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
    QueryContainerBase* asyncContainer = reinterpret_cast<QueryContainerBase*>(ref);

    switch(asyncContainer->mQueryType)
    {
    case BFQuery_MainData:
    {
        BuildingObject* building = _createBuilding(result);

        QueryContainerBase* asContainer;

        // if its a cloning facility, query its spawn points
        if(building->getBuildingFamily() == BuildingFamily_Cloning_Facility)
        {
            asContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,BFQuery_CloneData,asyncContainer->mClient);
            asContainer->mObject = building;

            mDatabase->executeSqlAsync(this,asContainer,"SELECT spawn_clone.parentId,spawn_clone.oX,spawn_clone.oY,spawn_clone.oZ,spawn_clone.oW,"
                                       "spawn_clone.cell_x,spawn_clone.cell_y,spawn_clone.cell_z,spawn_clone.city "
                                       "FROM  %s.spawn_clone "
                                       "INNER JOIN %s.cells ON spawn_clone.parentid = cells.id "
                                       "INNER JOIN %s.buildings ON cells.parent_id = buildings.id "
                                       "WHERE buildings.id = %" PRIu64 ";",
                                       mDatabase->galaxy(),mDatabase->galaxy(),mDatabase->galaxy(),building->getId());
           
        }
        else
        {
            asContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,BFQuery_Cells,asyncContainer->mClient);
            asContainer->mObject = building;

            mDatabase->executeSqlAsync(this,asContainer,"SELECT id FROM %s.cells WHERE parent_id = %" PRIu64 ";",mDatabase->galaxy(),building->getId());
            
        }
    }
    break;

    case BFQuery_CloneData:
    {
        BuildingObject*	building = dynamic_cast<BuildingObject*>(asyncContainer->mObject);

        uint64 spawnCount = result->getRowCount();

        if(!spawnCount)
        {
        	LOG(ERR) << "Cloning facility [" << building->getId() << "] has no spawn points";
        }

        for(uint64 i = 0; i < spawnCount; i++)
        {
            SpawnPoint* spawnPoint = new SpawnPoint();

            result->getNextRow(mSpawnBinding,spawnPoint);

            building->addSpawnPoint(spawnPoint);
        }

        // load cells
        QueryContainerBase* asContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,BFQuery_Cells,asyncContainer->mClient);
        asContainer->mObject = building;

        mDatabase->executeSqlAsync(this,asContainer,"SELECT id FROM %s.cells WHERE parent_id = %" PRIu64 ";",mDatabase->galaxy(),building->getId());
        
    }
    break;

    case BFQuery_Cells:
    {
        BuildingObject*	building = dynamic_cast<BuildingObject*>(asyncContainer->mObject);
        uint32			cellCount;
        uint64			cellId;

        DataBinding*	cellBinding = mDatabase->createDataBinding(1);
        cellBinding->addField(DFT_int64,0,8);

        // store us for later lookup
        mObjectLoadMap.insert(std::make_pair(building->getId(),new(mILCPool.ordered_malloc()) InLoadingContainer(building,asyncContainer->mOfCallback,asyncContainer->mClient)));

        cellCount = static_cast<uint32>(result->getRowCount());

        building->setLoadCount(cellCount);

        for(uint32 j = 0; j < cellCount; j++)
        {
            result->getNextRow(cellBinding,&cellId);

            mCellFactory->requestObject(this,cellId,0,0,asyncContainer->mClient);
        }

        mDatabase->destroyDataBinding(cellBinding);
    }
    break;

    default:
        break;
    }

    mQueryContainerPool.free(asyncContainer);
}

//=============================================================================

void BuildingFactory::requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client)
{
    mDatabase->executeSqlAsync(this,new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(ofCallback,BFQuery_MainData,client),
                               "SELECT buildings.id,buildings.oX,buildings.oY,buildings.oZ,buildings.oW,buildings.x,"
                               "buildings.y,buildings.z,building_types.model,building_types.width,building_types.height,"
                               "building_types.file,building_types.name,building_types.family "
                               "FROM %s.buildings INNER JOIN %s.building_types ON (buildings.type_id = building_types.id) "
                               "WHERE (buildings.id = %" PRIu64 ")",mDatabase->galaxy(),mDatabase->galaxy(),id);
    
}

//=============================================================================

BuildingObject* BuildingFactory::_createBuilding(DatabaseResult* result)
{
    if (!result->getRowCount()) {
    	return nullptr;
    }

	BuildingObject*	buildingObject = new BuildingObject();

    result->getNextRow(mBuildingBinding,buildingObject);

    buildingObject->setLoadState(LoadState_Loaded);
    buildingObject->setPlayerStructureFamily(PlayerStructure_TreBuilding);

    return buildingObject;
}

//=============================================================================

void BuildingFactory::_setupDatabindings()
{
    mBuildingBinding = mDatabase->createDataBinding(14);
    mBuildingBinding->addField(DFT_uint64,offsetof(BuildingObject,mId),8,0);
    mBuildingBinding->addField(DFT_float,offsetof(BuildingObject,mDirection.x),4,1);
    mBuildingBinding->addField(DFT_float,offsetof(BuildingObject,mDirection.y),4,2);
    mBuildingBinding->addField(DFT_float,offsetof(BuildingObject,mDirection.z),4,3);
    mBuildingBinding->addField(DFT_float,offsetof(BuildingObject,mDirection.w),4,4);
    mBuildingBinding->addField(DFT_float,offsetof(BuildingObject,mPosition.x),4,5);
    mBuildingBinding->addField(DFT_float,offsetof(BuildingObject,mPosition.y),4,6);
    mBuildingBinding->addField(DFT_float,offsetof(BuildingObject,mPosition.z),4,7);
    mBuildingBinding->addField(DFT_bstring,offsetof(BuildingObject,mModel),256,8);
    mBuildingBinding->addField(DFT_float,offsetof(BuildingObject,mWidth),4,9);
    mBuildingBinding->addField(DFT_float,offsetof(BuildingObject,mHeight),4,10);
    mBuildingBinding->addField(DFT_bstring,offsetof(BuildingObject,mNameFile),256,11);
    mBuildingBinding->addField(DFT_bstring,offsetof(BuildingObject,mName),256,12);
    mBuildingBinding->addField(DFT_uint32,offsetof(BuildingObject,mBuildingFamily),4,13);

    mSpawnBinding = mDatabase->createDataBinding(9);
    mSpawnBinding->addField(DFT_uint64,offsetof(SpawnPoint,mCellId),8,0);
    mSpawnBinding->addField(DFT_float,offsetof(SpawnPoint,mDirection.x),4,1);
    mSpawnBinding->addField(DFT_float,offsetof(SpawnPoint,mDirection.y),4,2);
    mSpawnBinding->addField(DFT_float,offsetof(SpawnPoint,mDirection.z),4,3);
    mSpawnBinding->addField(DFT_float,offsetof(SpawnPoint,mDirection.w),4,4);
    mSpawnBinding->addField(DFT_float,offsetof(SpawnPoint,mPosition.x),4,5);
    mSpawnBinding->addField(DFT_float,offsetof(SpawnPoint,mPosition.y),4,6);
    mSpawnBinding->addField(DFT_float,offsetof(SpawnPoint,mPosition.z),4,7);
    mSpawnBinding->addField(DFT_bstring,offsetof(SpawnPoint,mName),256,8);
}

//=============================================================================

void BuildingFactory::_destroyDatabindings()
{
    mDatabase->destroyDataBinding(mBuildingBinding);
    mDatabase->destroyDataBinding(mSpawnBinding);
}

//=============================================================================

void BuildingFactory::handleObjectReady(Object* object,DispatchClient* client)
{
    InLoadingContainer* ilc = _getObject(object->getParentId());

    if (! ilc) {//ILC sanity check...
    	LOG(WARNING) << "Could not locate InLoadingContainer for object parent [" << object->getParentId() << "]";
        return;
    }

    BuildingObject*		building = dynamic_cast<BuildingObject*>(ilc->mObject);

    //this happens on load so no reason to update players
    gWorldManager->addObject(object,true);

    building->addCell(dynamic_cast<CellObject*>(object));

    if(building->getLoadCount() == (building->getCellList())->size())
    {
        if(!(_removeFromObjectLoadMap(building->getId())))
            LOG(WARNING) << "Failed removing object from loadmap";

        ilc->mOfCallback->handleObjectReady(building,ilc->mClient);

        mILCPool.free(ilc);
    }
}

//=============================================================================

void BuildingFactory::releaseAllPoolsMemory()
{
    releaseQueryContainerPoolMemory();
    releaseILCPoolMemory();

    mCellFactory->releaseQueryContainerPoolMemory()	;
    mCellFactory->releaseILCPoolMemory();
}
