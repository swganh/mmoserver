/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "CellFactory.h"
#include "CellObject.h"
#include "CreatureEnums.h"
#include "CreatureObject.h"
#include "ObjectFactory.h"
#include "Shuttle.h"
#include "WorldManager.h"
#include "LogManager/LogManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include "Utils/utils.h"
#include <assert.h>

//=============================================================================

bool			CellFactory::mInsFlag    = false;
CellFactory*	CellFactory::mSingleton  = NULL;

//======================================================================================================================

CellFactory*	CellFactory::Init(Database* database)
{
	if(!mInsFlag)
	{
		mSingleton = new CellFactory(database);
		mInsFlag = true;
		return mSingleton;
	}
	else
		return mSingleton;
}

//=============================================================================

CellFactory::CellFactory(Database* database) : FactoryBase(database)
{
	_setupDatabindings();
}

//=============================================================================

CellFactory::~CellFactory()
{
	_destroyDatabindings();

	mInsFlag = false;
	delete(mSingleton);
}

//=============================================================================

void CellFactory::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
	QueryContainerBase* asyncContainer = reinterpret_cast<QueryContainerBase*>(ref);

	switch(asyncContainer->mQueryType)
	{
		case CellFQuery_MainData:
		{
			CellObject* cell = _createCell(result);
			uint64		cellId = cell->getId();

			QueryContainerBase* asContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,CellFQuery_Objects,asyncContainer->mClient);
			asContainer->mObject = cell;

			mDatabase->ExecuteSqlAsync(this,asContainer,"(SELECT \'terminals\',id FROM terminals WHERE parent_id = %lld)"
														" UNION (SELECT \'containers\',id FROM containers WHERE parent_id = %lld)"
														" UNION (SELECT \'ticket_collectors\',id FROM ticket_collectors WHERE (parent_id=%lld))"
														" UNION (SELECT \'persistent_npcs\',id FROM persistent_npcs WHERE parentId=%lld)"
														" UNION (SELECT \'shuttles\',id FROM shuttles WHERE parentId=%lld)"
														" UNION (SELECT \'items\',id FROM items WHERE parent_id=%lld)"
														" UNION (SELECT \'resource_containers\',id FROM resource_containers WHERE parent_id=%lld)",
														cellId,cellId,cellId,cellId,cellId,cellId,cellId);
		}
		break;

		case CellFQuery_Objects:
		{
			CellObject* cell = dynamic_cast<CellObject*>(asyncContainer->mObject);
			Type1_QueryContainer queryContainer;

			DataBinding*	binding = mDatabase->CreateDataBinding(2);
			binding->addField(DFT_bstring,offsetof(Type1_QueryContainer,mString),64,0);
			binding->addField(DFT_uint64,offsetof(Type1_QueryContainer,mId),8,1);

			uint64 count = result->getRowCount();

			if(count > 0)
			{
				// store us for later lookup
				mObjectLoadMap.insert(std::make_pair(cell->getId(),new(mILCPool.ordered_malloc()) InLoadingContainer(cell,asyncContainer->mOfCallback,asyncContainer->mClient)));
				cell->setLoadCount(static_cast<uint32>(count));

				for(uint32 i = 0;i < count;i++)
				{
					result->GetNextRow(binding,&queryContainer);

					if(strcmp(queryContainer.mString.getAnsi(),"terminals") == 0)	
						gObjectFactory->requestObject(ObjType_Tangible,TanGroup_Terminal,0,this,queryContainer.mId,asyncContainer->mClient);
					else if(strcmp(queryContainer.mString.getAnsi(),"containers") == 0)
						gObjectFactory->requestObject(ObjType_Tangible,TanGroup_Container,0,this,queryContainer.mId,asyncContainer->mClient);
					else if(strcmp(queryContainer.mString.getAnsi(),"ticket_collectors") == 0)
						gObjectFactory->requestObject(ObjType_Tangible,TanGroup_TicketCollector,0,this,queryContainer.mId,asyncContainer->mClient);
					else if(strcmp(queryContainer.mString.getAnsi(),"persistent_npcs") == 0)
						gObjectFactory->requestObject(ObjType_NPC,CreoGroup_PersistentNpc,0,this,queryContainer.mId,asyncContainer->mClient);
					else if(strcmp(queryContainer.mString.getAnsi(),"shuttles") == 0)
						gObjectFactory->requestObject(ObjType_Creature,CreoGroup_Shuttle,0,this,queryContainer.mId,asyncContainer->mClient);
					else if(strcmp(queryContainer.mString.getAnsi(),"items") == 0)
						gObjectFactory->requestObject(ObjType_Tangible,TanGroup_Item,0,this,queryContainer.mId,asyncContainer->mClient);
					else if(strcmp(queryContainer.mString.getAnsi(),"resource_containers") == 0)
						gObjectFactory->requestObject(ObjType_Tangible,TanGroup_ResourceContainer,0,this,queryContainer.mId,asyncContainer->mClient);
				}
			}
			else
				asyncContainer->mOfCallback->handleObjectReady(cell,asyncContainer->mClient);

			mDatabase->DestroyDataBinding(binding);
		}
		break;

		default:break;
	}

	mQueryContainerPool.free(asyncContainer);
}

//=============================================================================

void CellFactory::requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client)
{
	mDatabase->ExecuteSqlAsync(this,new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(ofCallback,CellFQuery_MainData,client),"SELECT id,parent_id FROM cells WHERE id = %lld",id);
}

//=============================================================================

CellObject* CellFactory::_createCell(DatabaseResult* result)
{
	CellObject* cellObject = new CellObject();

	uint64 count = result->getRowCount();
	assert(count == 1);

	result->GetNextRow(mCellBinding,(void*)cellObject);

	return cellObject;
}

//=============================================================================

void CellFactory::_setupDatabindings()
{
	mCellBinding = mDatabase->CreateDataBinding(2);
	mCellBinding->addField(DFT_uint64,offsetof(CellObject,mId),8,0);
	mCellBinding->addField(DFT_uint64,offsetof(CellObject,mParentId),8,1);
}

//=============================================================================

void CellFactory::_destroyDatabindings()
{
	mDatabase->DestroyDataBinding(mCellBinding);
}

//=============================================================================

void CellFactory::handleObjectReady(Object* object,DispatchClient* client)
{
	InLoadingContainer* ilc = _getObject(object->getParentId());
	CellObject*			cell = dynamic_cast<CellObject*>(ilc->mObject);
	
	gWorldManager->addObject(object,true);

	switch(object->getType())
	{
		case ObjType_NPC:
		case ObjType_Creature:
		{
			CreatureObject* creature = dynamic_cast<CreatureObject*>(object);

			if(creature->getCreoGroup() == CreoGroup_Shuttle)
				gWorldManager->addShuttle(dynamic_cast<Shuttle*>(creature));
		}
		break;
	}
	
	cell->addChild(object);

	if(cell->getLoadCount() == (cell->getChilds())->size())
	{
		if(!(_removeFromObjectLoadMap(cell->getId())))
			gLogger->logMsg("CellFactory: Failed removing object from loadmap");

		ilc->mOfCallback->handleObjectReady(cell,ilc->mClient);

		mILCPool.free(ilc);
	}
}

//=============================================================================

