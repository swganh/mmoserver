/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "BuildingFactory.h"
#include "BuildingObject.h"
#include "CellFactory.h"
#include "CellObject.h"
#include "SpawnPoint.h"
#include "WorldManager.h"
#include "LogManager/LogManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/DatabaseResult.h"
#include "Utils/utils.h"
#include <assert.h>

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

				mDatabase->ExecuteSqlAsync(this,asContainer,"SELECT spawn_clone.parentId,spawn_clone.oX,spawn_clone.oY,spawn_clone.oZ,spawn_clone.oW,"
															"spawn_clone.cell_x,spawn_clone.cell_y,spawn_clone.cell_z,spawn_clone.city "
															"FROM  spawn_clone "
															"INNER JOIN cells ON spawn_clone.parentid = cells.id "
															"INNER JOIN buildings ON cells.parent_id = buildings.id "
															"WHERE buildings.id = %lld;",building->getId());
			}
			else
			{
				asContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,BFQuery_Cells,asyncContainer->mClient);
				asContainer->mObject = building;

				mDatabase->ExecuteSqlAsync(this,asContainer,"SELECT id FROM cells WHERE parent_id = %lld;",building->getId());
			}
		}
		break;

		case BFQuery_CloneData:
		{
			BuildingObject*	building = dynamic_cast<BuildingObject*>(asyncContainer->mObject);

			uint64 spawnCount = result->getRowCount();

			if(!spawnCount)
			{
				gLogger->logMsgF("BuildingFactory: Cloning facility %lld has no spawn points",MSG_NORMAL,building->getId());
			}

			for(uint64 i = 0;i < spawnCount;i++)
			{
				SpawnPoint* spawnPoint = new SpawnPoint();

				result->GetNextRow(mSpawnBinding,spawnPoint);

				building->addSpawnPoint(spawnPoint);
			}

			// load cells
			QueryContainerBase* asContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,BFQuery_Cells,asyncContainer->mClient);
			asContainer->mObject = building;

			mDatabase->ExecuteSqlAsync(this,asContainer,"SELECT id FROM cells WHERE parent_id = %lld;",building->getId());
		}
		break;

		case BFQuery_Cells:
		{
			BuildingObject*	building = dynamic_cast<BuildingObject*>(asyncContainer->mObject);
			uint32			cellCount;
			uint64			cellId;

			DataBinding*	cellBinding = mDatabase->CreateDataBinding(1);
			cellBinding->addField(DFT_int64,0,8);

			// store us for later lookup
			mObjectLoadMap.insert(std::make_pair(building->getId(),new(mILCPool.ordered_malloc()) InLoadingContainer(building,asyncContainer->mOfCallback,asyncContainer->mClient)));

			cellCount = static_cast<uint32>(result->getRowCount());

			building->setLoadCount(cellCount);

			for(uint32 j = 0;j < cellCount;j++)
			{
				result->GetNextRow(cellBinding,&cellId);

				mCellFactory->requestObject(this,cellId,0,0,asyncContainer->mClient);
			}

			mDatabase->DestroyDataBinding(cellBinding);
		}
		break;
		
		default:break;
	}

	mQueryContainerPool.free(asyncContainer);
}

//=============================================================================

void BuildingFactory::requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client)
{
	mDatabase->ExecuteSqlAsync(this,new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(ofCallback,BFQuery_MainData,client),
								"SELECT buildings.id,buildings.oX,buildings.oY,buildings.oZ,buildings.oW,buildings.x,"
								"buildings.y,buildings.z,building_types.model,building_types.width,building_types.height,"
								"building_types.file,building_types.name,building_types.family "
								"FROM buildings INNER JOIN building_types ON (buildings.type_id = building_types.id) "
								"WHERE (buildings.id = %lld)",id);
}

//=============================================================================

BuildingObject* BuildingFactory::_createBuilding(DatabaseResult* result)
{
	BuildingObject*	buildingObject = new BuildingObject();

	uint64 count = result->getRowCount();
	assert(count == 1);

	result->GetNextRow(mBuildingBinding,buildingObject);

	buildingObject->setLoadState(LoadState_Loaded);

	return buildingObject;
}

//=============================================================================

void BuildingFactory::_setupDatabindings()
{
	mBuildingBinding = mDatabase->CreateDataBinding(14);
	mBuildingBinding->addField(DFT_uint64,offsetof(BuildingObject,mId),8,0);
	mBuildingBinding->addField(DFT_float,offsetof(BuildingObject,mDirection.mX),4,1);
	mBuildingBinding->addField(DFT_float,offsetof(BuildingObject,mDirection.mY),4,2);
	mBuildingBinding->addField(DFT_float,offsetof(BuildingObject,mDirection.mZ),4,3);
	mBuildingBinding->addField(DFT_float,offsetof(BuildingObject,mDirection.mW),4,4);
	mBuildingBinding->addField(DFT_float,offsetof(BuildingObject,mPosition.mX),4,5);
	mBuildingBinding->addField(DFT_float,offsetof(BuildingObject,mPosition.mY),4,6);
	mBuildingBinding->addField(DFT_float,offsetof(BuildingObject,mPosition.mZ),4,7);
	mBuildingBinding->addField(DFT_bstring,offsetof(BuildingObject,mModel),256,8);
	mBuildingBinding->addField(DFT_float,offsetof(BuildingObject,mWidth),4,9);
	mBuildingBinding->addField(DFT_float,offsetof(BuildingObject,mHeight),4,10);
	mBuildingBinding->addField(DFT_bstring,offsetof(BuildingObject,mNameFile),256,11);
	mBuildingBinding->addField(DFT_bstring,offsetof(BuildingObject,mName),256,12);
	mBuildingBinding->addField(DFT_uint32,offsetof(BuildingObject,mBuildingFamily),4,13);

	mSpawnBinding = mDatabase->CreateDataBinding(9);
	mSpawnBinding->addField(DFT_uint64,offsetof(SpawnPoint,mCellId),8,0);
	mSpawnBinding->addField(DFT_float,offsetof(SpawnPoint,mDirection.mX),4,1);
	mSpawnBinding->addField(DFT_float,offsetof(SpawnPoint,mDirection.mY),4,2);
	mSpawnBinding->addField(DFT_float,offsetof(SpawnPoint,mDirection.mZ),4,3);
	mSpawnBinding->addField(DFT_float,offsetof(SpawnPoint,mDirection.mW),4,4);
	mSpawnBinding->addField(DFT_float,offsetof(SpawnPoint,mPosition.mX),4,5);
	mSpawnBinding->addField(DFT_float,offsetof(SpawnPoint,mPosition.mY),4,6);
	mSpawnBinding->addField(DFT_float,offsetof(SpawnPoint,mPosition.mZ),4,7);
	mSpawnBinding->addField(DFT_bstring,offsetof(SpawnPoint,mName),256,8);
}

//=============================================================================

void BuildingFactory::_destroyDatabindings()
{
	mDatabase->DestroyDataBinding(mBuildingBinding);
	mDatabase->DestroyDataBinding(mSpawnBinding);
}

//=============================================================================

void BuildingFactory::handleObjectReady(Object* object,DispatchClient* client)
{
	InLoadingContainer* ilc = _getObject(object->getParentId());
	BuildingObject*		building = dynamic_cast<BuildingObject*>(ilc->mObject);
	
	gWorldManager->addObject(object,true);

	building->addCell(dynamic_cast<CellObject*>(object));

	if(building->getLoadCount() == (building->getCellList())->size())
	{
		if(!(_removeFromObjectLoadMap(building->getId())))
			gLogger->logMsg("BuildingFactory: Failed removing object from loadmap");

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
