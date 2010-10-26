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

#include "CellFactory.h"
#include "CellObject.h"
#include "CreatureEnums.h"
#include "CreatureObject.h"
#include "PlayerStructureTerminal.h"
#include "ObjectFactory.h"
#include "Shuttle.h"
#include "WorldManager.h"
#include "LogManager/LogManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include "Utils/utils.h"

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

			mDatabase->ExecuteSqlAsync(this,asContainer,"(SELECT \'terminals\',id FROM terminals WHERE parent_id = %"PRIu64")"
														" UNION (SELECT \'containers\',id FROM containers WHERE parent_id = %"PRIu64")"
														" UNION (SELECT \'ticket_collectors\',id FROM ticket_collectors WHERE (parent_id=%"PRIu64"))"
														" UNION (SELECT \'persistent_npcs\',id FROM persistent_npcs WHERE parentId=%"PRIu64")"
														" UNION (SELECT \'shuttles\',id FROM shuttles WHERE parentId=%"PRIu64")"
														" UNION (SELECT \'items\',id FROM items WHERE parent_id=%"PRIu64")"
														" UNION (SELECT \'resource_containers\',id FROM resource_containers WHERE parent_id=%"PRIu64")",
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
	mDatabase->ExecuteSqlAsync(this,new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(ofCallback,CellFQuery_MainData,client),"SELECT id,parent_id FROM cells WHERE id = %"PRIu64"",id);
}

//=============================================================================

void CellFactory::requestStructureCell(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client)
{
	mDatabase->ExecuteSqlAsync(this,new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(ofCallback,CellFQuery_MainData,client),"SELECT id,parent_id FROM structure_cells WHERE id = %"PRIu64"",id);
}

//=============================================================================

CellObject* CellFactory::_createCell(DatabaseResult* result)
{
	CellObject* cellObject = new CellObject();
	cellObject->setCapacity(500);

	uint64 count = result->getRowCount();

	result->GetNextRow(mCellBinding,(void*)cellObject);

	//cells are added to the worldmanager in the buildingFactory!!
	
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
		
		case ObjType_Tangible:
		{
			PlayerStructureTerminal* terminal = dynamic_cast<PlayerStructureTerminal*>(object);
			if(terminal)
			{
				terminal->setStructure(cell->getParentId());
			}
		}
		break;

		case ObjType_Building:
		case ObjType_Cell:
		case ObjType_DraftSchematic:
		case ObjType_Structure:
		case ObjType_Intangible:
		case ObjType_Lair:
		case ObjType_Mission:
		case ObjType_None:
		case ObjType_NonPersistant:
		case ObjType_Player:
		case ObjType_Region:
		
		case ObjType_Waypoint:
		default:
			break;
	}

	cell->addObjectSecure(object);

	if(cell->getLoadCount() == cell->getObjects()->size())
	{
		if(!(_removeFromObjectLoadMap(cell->getId())))
			gLogger->log(LogManager::DEBUG,"CellFactory: Failed removing object from loadmap");

		ilc->mOfCallback->handleObjectReady(cell,ilc->mClient);

		mILCPool.free(ilc);
	}
}

//=============================================================================

