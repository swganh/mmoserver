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

#include "ObjectFactory.h"
#include "BuildingFactory.h"
#include "CreatureFactory.h"
#include "Deed.h"
#include "Datapad.h"
#include "CellObject.h"
#include "DraftSchematic.h"
#include "HarvesterFactory.h"
#include "HouseObject.h"
#include "HouseFactory.h"
#include "FactoryFactory.h"
#include "IntangibleObject.h"
#include "IntangibleFactory.h"
#include "ManufacturingSchematic.h"
#include "ObjectFactoryCallback.h"
#include "PlayerObject.h"
#include "Inventory.h"
#include "PlayerObjectFactory.h"
#include "MessageLib/MessageLib.h"
#include "RegionFactory.h"
#include "ResourceManager.h"
#include "StructureManager.h"
#include "TangibleFactory.h"
#include "TravelMapHandler.h"
#include "WaypointFactory.h"
#include "WorldManager.h"
#include "LogManager/LogManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include "Utils/utils.h"
#include <assert.h>

//=============================================================================

bool				ObjectFactory::mInsFlag    = false;
ObjectFactory*		ObjectFactory::mSingleton  = NULL;

//======================================================================================================================

ObjectFactory*	ObjectFactory::Init(Database* database)
{
	if(!mInsFlag)
	{
		mSingleton = new ObjectFactory(database);
		mInsFlag = true;
		return mSingleton;
	}
	else
		return mSingleton;
}

//=============================================================================

ObjectFactory::ObjectFactory(Database* database) :
mDatabase(database),
mDbAsyncPool(sizeof(OFAsyncContainer))
{
	mPlayerObjectFactory	= PlayerObjectFactory::Init(mDatabase);
	mTangibleFactory		= TangibleFactory::Init(mDatabase);
	mIntangibleFactory		= IntangibleFactory::Init(mDatabase);
	mCreatureFactory		= CreatureFactory::Init(mDatabase);
	mBuildingFactory		= BuildingFactory::Init(mDatabase);
	mRegionFactory			= RegionFactory::Init(mDatabase);
	mWaypointFactory		= WaypointFactory::Init(mDatabase);
	mHarvesterFactory		= HarvesterFactory::Init(mDatabase);
	mFactoryFactory			= FactoryFactory::Init(mDatabase);
	mHouseFactory			= HouseFactory::Init(mDatabase);
}

//=============================================================================

ObjectFactory::~ObjectFactory()
{
	mInsFlag = false;
	delete(mSingleton);
}

//=============================================================================

void ObjectFactory::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
	OFAsyncContainer* asyncContainer = reinterpret_cast<OFAsyncContainer*>(ref);

	switch(asyncContainer->query)
	{
		case OFQuery_House:
		{
			PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(asyncContainer->PlayerId));
			if(!result->getRowCount())
			{
				gLogger->log(LogManager::DEBUG,"ObjFactory::handleDatabaseJobComplete   :  create house failed : no result");
				break;
			}

			uint64 requestId = 0;
			DataBinding* binding = mDatabase->CreateDataBinding(1);
			binding->addField(DFT_uint64,0,8);
			result->GetNextRow(binding,&requestId);
			mDatabase->DestroyDataBinding(binding);

			if(!requestId)
			{
				gLogger->log(LogManager::DEBUG,"ObjFactory::handleDatabaseJobComplete   :  create house failed : result is 0");
			}
			mHouseFactory->requestObject(asyncContainer->ofCallback,requestId,0,0,asyncContainer->client);

			//now we need to update the Owners Lots				
			//cave he might have logged out already - even if thats *very* unlikely (heck of a query that would have been)
			if(player)
			{
				gStructureManager->UpdateCharacterLots(asyncContainer->PlayerId);
				Deed* deed = dynamic_cast<Deed*>(gWorldManager->getObjectById(asyncContainer->DeedId));
				
				TangibleObject* tO = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById(deed->getParentId()));
				//destroy it in the client
				gMessageLib->sendDestroyObject(asyncContainer->DeedId,player);
	
				//delete it out of the inventory
				tO->deleteObject(deed);
				
				Datapad* datapad			= player->getDataPad();
				datapad->requestNewWaypoint("Player House",asyncContainer->coords,gWorldManager->getPlanetIdByName(gWorldManager->getPlanetNameThis()),1);

			}
			
			// now we need to link the deed to the factory in the db and remove it out of the inventory in the db
			int8 sql[250];
			sprintf(sql,"UPDATE items SET parent_id = %I64u WHERE id = %"PRIu64"",requestId, asyncContainer->DeedId);
			mDatabase->ExecuteSqlAsync(NULL,NULL,sql);
				
		}
		break;

		case OFQuery_Factory:
		{
			PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(asyncContainer->PlayerId));
			if(!result->getRowCount())
			{
				gLogger->log(LogManager::DEBUG,"ObjFactory::handleDatabaseJobComplete   :  create Factory failed : no result");
				break;
			}

			uint64 requestId = 0;
			DataBinding* binding = mDatabase->CreateDataBinding(1);
			binding->addField(DFT_uint64,0,8);
			result->GetNextRow(binding,&requestId);
			mDatabase->DestroyDataBinding(binding);

			if(!requestId)
			{
				gLogger->log(LogManager::DEBUG,"ObjFactory::handleDatabaseJobComplete   :  create Factory failed : result is 0");
			}
			mFactoryFactory->requestObject(asyncContainer->ofCallback,requestId,0,0,asyncContainer->client);

			//now we need to update the Owners Lots
				
			//cave he might have logged out already - even if thats *very* unlikely (heck of a query that would have been)
			if(player)
			{
				gStructureManager->UpdateCharacterLots(asyncContainer->PlayerId);
				Deed* deed = dynamic_cast<Deed*>(gWorldManager->getObjectById(asyncContainer->DeedId));
				
				//destroy it in the client
				gMessageLib->sendDestroyObject(asyncContainer->DeedId,player);
	
				//delete it out of the inventory
				TangibleObject* tO = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById(deed->getParentId()));
				tO->deleteObject(deed);

				Datapad* datapad			= player->getDataPad();
				datapad->requestNewWaypoint("Player Factory",asyncContainer->coords,gWorldManager->getPlanetIdByName(gWorldManager->getPlanetNameThis()),1);
			}
			
			// now we need to link the deed to the factory in the db and remove it out of the inventory in the db
			int8 sql[250];
			sprintf(sql,"UPDATE items SET parent_id = %I64u WHERE id = %"PRIu64"",requestId, asyncContainer->DeedId);
			mDatabase->ExecuteSqlAsync(NULL,NULL,sql);
				
		}
		break;

		case OFQuery_Harvester:
		{
			if(!result->getRowCount())
			{
				gLogger->log(LogManager::DEBUG,"ObjFactory::handleDatabaseJobComplete   :  create Harvester failed");
				break;
			}

			uint64 requestId = 0;
			DataBinding* binding = mDatabase->CreateDataBinding(1);
			binding->addField(DFT_uint64,0,8);
			result->GetNextRow(binding,&requestId);
			mDatabase->DestroyDataBinding(binding);

			if(requestId)
			{
				mHarvesterFactory->requestObject(asyncContainer->ofCallback,requestId,0,0,asyncContainer->client);

				//now we need to update the Owners Lots
				PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(asyncContainer->PlayerId));
					
				//cave he might have logged out already - even if thats *very* unlikely (heck of a query that would have been)
				if(player)
				{
					gStructureManager->UpdateCharacterLots(asyncContainer->PlayerId);
					Inventory* inventory = dynamic_cast<Inventory*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
					Deed* deed = dynamic_cast<Deed*>(inventory->getObjectById(asyncContainer->DeedId));
					
					//destroy it in the client
					gMessageLib->sendDestroyObject(asyncContainer->DeedId,player);
		
					//delete it out of the inventory
					TangibleObject* tO = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById(deed->getParentId()));
					tO->deleteObject(deed);

					Datapad* datapad			= player->getDataPad();
					datapad->requestNewWaypoint("Harvester",asyncContainer->coords,gWorldManager->getPlanetIdByName(gWorldManager->getPlanetNameThis()),1);

				}
				
				// now we need to link the deed to the harvester in the db and remove it out of the inventory
				int8 sql[250];
				sprintf(sql,"UPDATE items SET parent_id = %I64u WHERE id = %"PRIu64"",requestId, asyncContainer->DeedId);
				mDatabase->ExecuteSqlAsync(NULL,NULL,sql);
			}
			else
				gLogger->log(LogManager::DEBUG,"ObjFactory::handleDatabaseJobComplete   :  create Harvester failed");
		}
		break;

		case OFQuery_WaypointCreate:
		{
			uint64 requestId = 0;
			DataBinding* binding = mDatabase->CreateDataBinding(1);
			binding->addField(DFT_uint64,0,8);
			result->GetNextRow(binding,&requestId);
			mDatabase->DestroyDataBinding(binding);

			if(requestId)
				mWaypointFactory->requestObject(asyncContainer->ofCallback,requestId,0,0,asyncContainer->client);
			else
				gLogger->log(LogManager::DEBUG,"ObjFactory::createWaypoint failed");
		}
		break;

		case OFQuery_Item:
		{
			uint64 requestId = 0;
			DataBinding* binding = mDatabase->CreateDataBinding(1);
			binding->addField(DFT_uint64,0,8);
			result->GetNextRow(binding,&requestId);
			mDatabase->DestroyDataBinding(binding);

			if(requestId)
				mTangibleFactory->requestObject(asyncContainer->ofCallback,requestId,TanGroup_Item,0,asyncContainer->client);
			else
				gLogger->log(LogManager::DEBUG,"ObjFactory::createItem failed");
		}
		break;

		case OFQuery_ResourceContainerCreate:
		{
			uint64 requestId = 0;
			DataBinding* binding = mDatabase->CreateDataBinding(1);
			binding->addField(DFT_uint64,0,8);
			result->GetNextRow(binding,&requestId);
			mDatabase->DestroyDataBinding(binding);

			if(requestId)
				mTangibleFactory->requestObject(asyncContainer->ofCallback,requestId,TanGroup_ResourceContainer,0,asyncContainer->client);
			else
				gLogger->log(LogManager::DEBUG,"ObjFactory::createResourceContainer failed");
		}
		break;

		default:
		{
			mTangibleFactory->requestObject(asyncContainer->ofCallback,asyncContainer->Id,asyncContainer->Group,0,asyncContainer->client);
		}
		break;
	}

	mDbAsyncPool.free(asyncContainer);
}

//=============================================================================
//
// create a new manufacture schematic with default values
//
void ObjectFactory::requestNewDefaultManufactureSchematic(ObjectFactoryCallback* ofCallback,uint32 schemCrc,uint64 parentId)
{
	OFAsyncContainer* asyncContainer = new(mDbAsyncPool.ordered_malloc()) OFAsyncContainer(ofCallback,OFQuery_Item,NULL);

	mDatabase->ExecuteSqlAsync(this,asyncContainer,"SELECT sf_DefaultManufactureSchematicCreate(%u,%"PRIu64")",schemCrc,parentId);
}

//=============================================================================
//
// create a new item based on a tangible template (clone it)
//
void ObjectFactory::requestNewClonedItem(ObjectFactoryCallback* ofCallback,uint64 templateId,uint64 parentId)
{
	OFAsyncContainer* asyncContainer = new(mDbAsyncPool.ordered_malloc()) OFAsyncContainer(ofCallback,OFQuery_Item,NULL);

	mDatabase->ExecuteSqlAsync(this,asyncContainer,"SELECT sf_DefaultItemCreateByTangibleTemplate(%"PRIu64",%"PRIu64")",parentId,templateId);
}


//=============================================================================
//
// create a new item by schematic crc with default attributes
//
void ObjectFactory::requestNewDefaultItem(ObjectFactoryCallback* ofCallback, uint32 schemCrc, uint64 parentId, uint16 planetId, const glm::vec3& position, const string& customName)
{
	OFAsyncContainer* asyncContainer = new(mDbAsyncPool.ordered_malloc()) OFAsyncContainer(ofCallback,OFQuery_Item,NULL);

	mDatabase->ExecuteSqlAsync(this,asyncContainer,"SELECT sf_DefaultItemCreateBySchematic(%u,%"PRIu64",%u,%f,%f,%f,'%s')",schemCrc,parentId,planetId,position.x,position.y,position.z,customName.getAnsi());
}

//=============================================================================
//
// create a new item with default attributes
//
void ObjectFactory::requestNewDefaultItem(ObjectFactoryCallback* ofCallback,uint32 familyId,uint32 typeId,uint64 parentId,uint16 planetId, const glm::vec3& position, const string& customName)
{
	OFAsyncContainer* asyncContainer = new(mDbAsyncPool.ordered_malloc()) OFAsyncContainer(ofCallback,OFQuery_Item,NULL);

	mDatabase->ExecuteSqlAsync(this,asyncContainer,"SELECT sf_DefaultItemCreate(%u,%u,%"PRIu64",%"PRIu64",%u,%f,%f,%f,'%s')",familyId,typeId,parentId,(uint64) 0,planetId,position.x,position.y,position.z,customName.getAnsi());
}

//=============================================================================
//
// create a new item with default attributes
//
void ObjectFactory::requestNewDefaultItemWithUses(ObjectFactoryCallback* ofCallback,uint32 familyId,uint32 typeId,uint64 parentId,uint16 planetId, const glm::vec3& position, const string& customName, int useCount)
{
	OFAsyncContainer* asyncContainer = new(mDbAsyncPool.ordered_malloc()) OFAsyncContainer(ofCallback,OFQuery_Item,NULL);

	mDatabase->ExecuteProcedureAsync(this,asyncContainer,"CALL sp_CreateForagedItem(%u,%u,%"PRIu64",%"PRIu64",%u,%f,%f,%f,'%s',%d)",familyId,typeId,parentId,(uint64) 0,planetId,position.x,position.y,position.z,customName.getAnsi(), useCount);
}


//=============================================================================
//
// create a new travel ticket
//
void ObjectFactory::requestNewTravelTicket(ObjectFactoryCallback* ofCallback,TicketProperties ticketProperties,uint64 parentId,uint16 planetId)
{
	OFAsyncContainer* asyncContainer = new(mDbAsyncPool.ordered_malloc()) OFAsyncContainer(ofCallback,OFQuery_Item,NULL);
	int8 sql[512],*sqlPointer;
	int8 dstPlanetIdStr[64];
	int8 restStr[128];
	sprintf(dstPlanetIdStr,"','%s','",gWorldManager->getPlanetNameById(static_cast<uint8>(ticketProperties.dstPlanetId)));
	sprintf(restStr,"',%"PRIu64",%f,%f,%f,%u)",parentId,0.0f,0.0f,0.0f,planetId);
	sprintf(sql,"SELECT sf_TravelTicketCreate('%s','",gWorldManager->getPlanetNameById(static_cast<uint8>(ticketProperties.srcPlanetId)));
	sqlPointer = sql + strlen(sql);
	sqlPointer += mDatabase->Escape_String(sqlPointer,ticketProperties.srcPoint->descriptor,strlen(ticketProperties.srcPoint->descriptor));
	strcat(sql,dstPlanetIdStr);
	sqlPointer = sql + strlen(sql);
	sqlPointer += mDatabase->Escape_String(sqlPointer,ticketProperties.dstPoint->descriptor,strlen(ticketProperties.dstPoint->descriptor));
	strcat(sql,restStr);

	mDatabase->ExecuteSqlAsync(this,asyncContainer,sql);
}

//=============================================================================
//
// create a new resource container
//
void ObjectFactory::requestNewResourceContainer(ObjectFactoryCallback* ofCallback,uint64 resourceId,uint64 parentId,uint16 planetId,uint32 amount)
{
	OFAsyncContainer* asyncContainer = new(mDbAsyncPool.ordered_malloc()) OFAsyncContainer(ofCallback,OFQuery_ResourceContainerCreate,NULL);
	mDatabase->ExecuteSqlAsync(this,asyncContainer,"SELECT sf_ResourceContainerCreate(%"PRIu64",%"PRIu64",0,0,0,%u,%u)",resourceId,parentId,planetId,amount);
}

//=============================================================================
//
// creates a harvester based on the supplied deed
//
void ObjectFactory::requestnewHarvesterbyDeed(ObjectFactoryCallback* ofCallback,Deed* deed,DispatchClient* client, float x, float y, float z, float dir, string customName, PlayerObject* player)
{
	//create a new Harvester Object with the attributes as specified by the deed
	OFAsyncContainer* asyncContainer = new(mDbAsyncPool.ordered_malloc()) OFAsyncContainer(ofCallback,OFQuery_Harvester,client);

	asyncContainer->DeedId = deed->getId();
	asyncContainer->OwnerId = deed->getOwner();
	asyncContainer->PlayerId = player->getId();
	asyncContainer->coords.x = x;
	asyncContainer->coords.y = y;
	asyncContainer->coords.z = z;
	int8 sql[512];
	//sf_DefaultHarvesterCreate`(type_id INT(11),parent_id BIGINT(20),privateowner_id BIGINT(20),inPlanet INT,oX FLOAT,oY FLOAT,oZ FLOAT, oW FLOAT,inX FLOAT,inY FLOAT,inZ FLOAT,custom_name CHAR(255)) RETURNS bigint(20)

	StructureDeedLink* deedLink;
	deedLink = gStructureManager->getDeedData(deed->getItemType());

	float oX, oY, oZ, oW;
	if(dir== 0.0)
	{
		oX = 0;
		oY = 0;
		oZ = 0;
		oW = 1;
	}
	if(dir== 1.0)	 //ok
	{
		oX = 0.0;
		oY = static_cast<float>(0.71);
		oZ = 0.0;
		oW = static_cast<float>(0.71);
	}

	if(dir== 2.0)
	{
		oX = 0;
		oY = 1;
		oZ = 0;
		oW = 0;
	}

	if(dir== 3.0)
	{
		oX = 0;
		oY = static_cast<float>(-0.71);
		oZ = 0;
		oW = static_cast<float>(0.71);
	}


	gLogger->log(LogManager::DEBUG,"New Harvester dir is %f, x:%f, y:%f, z:%f, w:%f",dir,oX, oY, oZ, oW);

	sprintf(sql,"SELECT sf_DefaultHarvesterCreate(%u,0,%"PRIu64",%u,%f,%f,%f,%f,%f,%f,%f,'%s',%I64u)",deedLink->structure_type, player->getId(), gWorldManager->getZoneId(),oX,oY,oZ,oW,x,y,z,customName.getAnsi(),deed->getId());
	mDatabase->ExecuteSqlAsync(this,asyncContainer,sql);
	gLogger->log(LogManager::DEBUG,sql);

}

//=============================================================================
//
// creates a fatory based on the supplied deed
//
void ObjectFactory::requestnewFactorybyDeed(ObjectFactoryCallback* ofCallback,Deed* deed,DispatchClient* client, float x, float y, float z, float dir, string customName, PlayerObject* player)
{
	//create a new Harvester Object with the attributes as specified by the deed
	OFAsyncContainer* asyncContainer = new(mDbAsyncPool.ordered_malloc()) OFAsyncContainer(ofCallback,OFQuery_Factory,client);

	asyncContainer->DeedId = deed->getId();
	asyncContainer->OwnerId = deed->getOwner();
	asyncContainer->PlayerId = player->getId();
	asyncContainer->coords.x = x;
	asyncContainer->coords.y = y;
	asyncContainer->coords.z = z;
	int8 sql[512];
	

	StructureDeedLink* deedLink;
	deedLink = gStructureManager->getDeedData(deed->getItemType());

	float oX, oY, oZ, oW;
	if(dir== 0.0)
	{
		oX = 0;
		oY = 0;
		oZ = 0;
		oW = 1;
	}
	if(dir== 1.0)	 //ok
	{
		oX = 0.0;
		oY = static_cast<float>(0.71);
		oZ = 0.0;
		oW = static_cast<float>(0.71);
	}

	if(dir== 2.0)
	{
		oX = 0;
		oY = 1;
		oZ = 0;
		oW = 0;
	}

	if(dir== 3.0)
	{
		oX = 0;
		oY = static_cast<float>(-0.71);
		oZ = 0;
		oW = static_cast<float>(0.71);
	}


	gLogger->log(LogManager::DEBUG,"New Factory dir is %f, x:%f, y:%f, z:%f, w:%f",dir,oX, oY, oZ, oW);

	sprintf(sql,"SELECT sf_DefaultFactoryCreate(%u,0,%"PRIu64",%u,%f,%f,%f,%f,%f,%f,%f,'%s',%I64u)",deedLink->structure_type, player->getId(), gWorldManager->getZoneId(),oX,oY,oZ,oW,x,y,z,customName.getAnsi(),deed->getId());
	mDatabase->ExecuteSqlAsync(this,asyncContainer,sql);
	gLogger->log(LogManager::DEBUG,sql);

}

void ObjectFactory::requestnewHousebyDeed(ObjectFactoryCallback* ofCallback,Deed* deed,DispatchClient* client, float x, float y, float z, float dir, string customName, PlayerObject* player)
{
		//create a new Harvester Object with the attributes as specified by the deed
	OFAsyncContainer* asyncContainer = new(mDbAsyncPool.ordered_malloc()) OFAsyncContainer(ofCallback,OFQuery_House,client);

	asyncContainer->DeedId = deed->getId();
	asyncContainer->OwnerId = deed->getOwner();
	asyncContainer->PlayerId = player->getId();
	asyncContainer->coords.x = x;
	asyncContainer->coords.y = y;
	asyncContainer->coords.z = z;
	int8 sql[512];
	

	StructureDeedLink* deedLink;
	deedLink = gStructureManager->getDeedData(deed->getItemType());

	float oX, oY, oZ, oW;
	if(dir== 0.0)
	{
		oX = 0;
		oY = 0;
		oZ = 0;
		oW = 1;
	}
	if(dir== 1.0)	 //ok
	{
		oX = 0.0;
		oY = 0.707107f;
		oZ = 0.0;
		oW = 0.707107f;
	}

	if(dir== 2.0)
	{
		oX = 0;
		oY = 1;
		oZ = 0;
		oW = 0;
	}

	if(dir== 3.0)
	{
		oX = 0;
		oY = -0.707107f;
		oZ = 0;
		oW = 0.707107f;
	}


	gLogger->log(LogManager::DEBUG,"New House dir is %f, x:%f, y:%f, z:%f, w:%f",dir,oX, oY, oZ, oW);

	sprintf(sql,"SELECT sf_DefaultHouseCreate(%u,0,%"PRIu64",%u,%f,%f,%f,%f,%f,%f,%f,'%s',%I64u)",deedLink->structure_type, player->getId(), gWorldManager->getZoneId(),oX,oY,oZ,oW,x,y,z,customName.getAnsi(),deed->getId());
	mDatabase->ExecuteSqlAsync(this,asyncContainer,sql);
	gLogger->log(LogManager::DEBUG,sql);
}

//=============================================================================
//
// create a new waypoint
// never call this directly - always go over the datapad!!!!!  we need to check the capacity
//
void ObjectFactory::requestNewWaypoint(ObjectFactoryCallback* ofCallback,string name, const glm::vec3& coords,uint16 planetId,uint64 ownerId,uint8 wpType)
{

	OFAsyncContainer* asyncContainer = new(mDbAsyncPool.ordered_malloc()) OFAsyncContainer(ofCallback,OFQuery_WaypointCreate,NULL);
	int8 sql[512],*sqlPointer;
	int8 restStr[128];

	sprintf(sql,"SELECT sf_WaypointCreate('");
	sqlPointer = sql + strlen(sql);
	sqlPointer += mDatabase->Escape_String(sqlPointer,name.getAnsi(),name.getLength());
	sprintf(restStr,"',%"PRIu64",%f,%f,%f,%u,%u)", ownerId, coords.x, coords.y, coords.z, planetId, wpType);
	strcat(sql,restStr);

	mDatabase->ExecuteSqlAsync(this,asyncContainer,sql);

}

//=============================================================================

void ObjectFactory::requestTanoNewParent(ObjectFactoryCallback* ofCallback,uint64 ObjectId,uint64 parentID, TangibleGroup Group)
{
	//this is used to create an item after an auction and to load a Manufacturing schematic into the DataPad after it was removed from the factory
	//After the owner ID was changed the item is requested through the OFQuery_Default request

	OFAsyncContainer* asyncContainer;
	int8 sql[512];

	asyncContainer = new(mDbAsyncPool.ordered_malloc()) OFAsyncContainer(ofCallback,OFQuery_Default,NULL);
	asyncContainer->Id = ObjectId;
	asyncContainer->Group = Group;

	switch(Group)
	{
		case TanGroup_ManufacturingSchematic:
		case TanGroup_Item:
		{

			sprintf(sql,"UPDATE items SET parent_id = '%"PRIu64"' WHERE id = '%"PRIu64"' ",parentID,ObjectId);
		}
		break;

		case TanGroup_ResourceContainer:
		{

			sprintf(sql,"UPDATE resource_containers SET parent_id = '%"PRIu64"' WHERE id = '%"PRIu64"' ",parentID,ObjectId);
		}
		break;

		default:break;
	}
	mDatabase->ExecuteSqlAsync(this,asyncContainer,sql);
}

void ObjectFactory::createIteminInventory(ObjectFactoryCallback* ofCallback,uint64 ObjectId, TangibleGroup Group)
{

	mTangibleFactory->requestObject(ofCallback,ObjectId,Group,0,NULL);


}


//=============================================================================

void ObjectFactory::GiveNewOwnerInDB(Object* object, uint64 ID)
{
	int8 sql[256];

	switch(object->getType())
	{
		case ObjType_Tangible:
		{
			TangibleObject* tangibleObject = dynamic_cast<TangibleObject*>(object);

			switch(tangibleObject->getTangibleGroup())
			{
				case TanGroup_Item:
				{
					sprintf(sql,"UPDATE items SET parent_id = '%"PRIu64"' WHERE id = '%"PRIu64"' ",ID,object->getId());
				}
				break;

				case TanGroup_ResourceContainer:
				{
					sprintf(sql,"UPDATE resource_containers SET parent_id = '%"PRIu64"' WHERE id = '%"PRIu64"' ",ID,object->getId());
				}
				break;

				default:break;
			}
		}
		break;

		case ObjType_Waypoint:
		{
			sprintf(sql,"UPDATE waypoints SET parent_id = '%"PRIu64"' WHERE id = '%"PRIu64"' ",ID,object->getId());
		}
		break;

		default:break;
	}
	mDatabase->ExecuteSqlAsync(NULL,NULL,sql);
}

//=============================================================================

void ObjectFactory::deleteObjectFromDB(uint64 id)
{
	Object* object = dynamic_cast<Object*>(gWorldManager->getObjectById(id));
	deleteObjectFromDB(object );
}

void ObjectFactory::deleteObjectFromDB(Object* object)
{
	int8 sql[256];

	switch(object->getType())
	{
		case ObjType_Tangible:
		{
			TangibleObject* tangibleObject = dynamic_cast<TangibleObject*>(object);

			switch(tangibleObject->getTangibleGroup())
			{
				case TanGroup_Item:
				{
					Item* item = dynamic_cast<Item*>(object);
					if(item->getItemType() == ItemFamily_ManufacturingSchematic)
					{
						ManufacturingSchematic* schem = dynamic_cast<ManufacturingSchematic*> (object);
						//first associated item
						sprintf(sql,"DELETE FROM items WHERE id = %"PRIu64"",schem->getItem()->getId());
						mDatabase->ExecuteSqlAsync(NULL,NULL,sql);
						sprintf(sql,"DELETE FROM item_attributes WHERE item_id = %"PRIu64"",schem->getItem()->getId());
						mDatabase->ExecuteSqlAsync(NULL,NULL,sql);

					}

					ObjectIDList* objectList		= item->getObjects();
					ObjectIDList::iterator objIt	= objectList->begin();

					while(objIt != objectList->end())
					{			
						Object* object = gWorldManager->getObjectById((*objIt));
						deleteObjectFromDB(object);
						
						++objIt;
					}

					sprintf(sql,"DELETE FROM items WHERE id = %"PRIu64"",object->getId());
					mDatabase->ExecuteSqlAsync(NULL,NULL,sql);

					sprintf(sql,"DELETE FROM item_attributes WHERE item_id = %"PRIu64"",object->getId());
					mDatabase->ExecuteSqlAsync(NULL,NULL,sql);
				}
				break;

				case TanGroup_ResourceContainer:
				{
					sprintf(sql,"DELETE FROM resource_containers WHERE id = %"PRIu64"",object->getId());
					mDatabase->ExecuteSqlAsync(NULL,NULL,sql);
				}
				break;

				case TanGroup_Terminal:
				{
					sprintf(sql,"DELETE FROM terminals WHERE id = %"PRIu64"",object->getId());
					mDatabase->ExecuteSqlAsync(NULL,NULL,sql);

				}
				break;

				default:break;
			}
		}
		break;

		case ObjType_Intangible:
		{
			IntangibleObject* itno = dynamic_cast<IntangibleObject*>(object);

			switch(itno->getItnoGroup())
			{
				case ItnoGroup_Vehicle:
				{
					sprintf(sql,"DELETE FROM vehicle_cutomization WHERE vehicles_id = %"PRIu64"",object->getId());
					mDatabase->ExecuteSqlAsync(NULL,NULL,sql);
					sprintf(sql,"DELETE FROM vehicle_attributes WHERE vehicles_id = %"PRIu64"",object->getId());
					mDatabase->ExecuteSqlAsync(NULL,NULL,sql);
					sprintf(sql,"DELETE FROM vehicles WHERE id = %"PRIu64"",object->getId());
					mDatabase->ExecuteSqlAsync(NULL,NULL,sql);
				}
				break;

				default: break;
			}
		}
		break;

		case ObjType_Cell:
		{
			CellObject* cell = dynamic_cast<CellObject*>(object);
			ObjectIDList* cellObjects		= cell->getObjects();
			ObjectIDList::iterator objIt	= cellObjects->begin();

			while(objIt != cellObjects->end())
			{
				Object* childObject = gWorldManager->getObjectById((*objIt));

				if(PlayerObject* player = dynamic_cast<PlayerObject*>(childObject))
				{
					//place the player in the world	and db	- do *NOT* delete him :P
					//player->setParentId(0,0xffffffff,player->getKnownPlayers(),true);
				}
				else
				if(CreatureObject* pet = dynamic_cast<CreatureObject*>(childObject))
				{
					//place the player in the world	and db	- do *NOT* delete him :P
					//pet->setParentId(0,0xffffffff,pet->getKnownPlayers(),true);
				}
				else
				{
					deleteObjectFromDB(childObject);
				}
				
				++objIt;

				sprintf(sql,"UPDATE characters SET parent_id = 0 WHERE parent_id = %"PRIu64"",object->getId());
				mDatabase->ExecuteSqlAsync(NULL,NULL,sql);
			}

			sprintf(sql,"DELETE FROM cells WHERE id = %"PRIu64"",object->getId());
			mDatabase->ExecuteSqlAsync(NULL,NULL,sql);
			sprintf(sql,"DELETE FROM structure_cells WHERE id = %"PRIu64"",object->getId());
			mDatabase->ExecuteSqlAsync(NULL,NULL,sql);
		}
		break;

		case ObjType_Building:
		{
			//only delete when a playerbuilding
			HouseObject* house = dynamic_cast<HouseObject*>(object);
			if(!house)
			{
				//no player building
				return;
			}

			CellObjectList*				cellList	= house->getCellList();
			CellObjectList::iterator	cellIt		= cellList->begin();

			while(cellIt != cellList->end())
			{
				CellObject* cell = (*cellIt);
				deleteObjectFromDB(cell);
				//remove items in the building from world and db

				++cellIt;
			}

			sprintf(sql,"DELETE FROM houses WHERE ID = %"PRIu64"",object->getId());
			mDatabase->ExecuteSqlAsync(NULL,NULL,sql);

			//sprintf(sql,"DELETE FROM terminals WHERE ID = %"PRIu64"",object->getId());
			//mDatabase->ExecuteSqlAsync(NULL,NULL,sql);

			sprintf(sql,"DELETE FROM structures WHERE ID = %"PRIu64"",object->getId());
			mDatabase->ExecuteSqlAsync(NULL,NULL,sql);

			//Admin / Hopper Lists
			sprintf(sql,"DELETE FROM structure_admin_data WHERE StructureID = %"PRIu64"",object->getId());
			mDatabase->ExecuteSqlAsync(NULL,NULL,sql);

			//update attributes cave redeed vs destroy
			sprintf(sql,"DELETE FROM structure_attributes WHERE Structure_id = %"PRIu64"",object->getId());
			mDatabase->ExecuteSqlAsync(NULL,NULL,sql);

		}
		break;
		case ObjType_Structure:
		{			

			//Harvester
			sprintf(sql,"DELETE FROM structures WHERE ID = %"PRIu64"",object->getId());
			mDatabase->ExecuteSqlAsync(NULL,NULL,sql);

			sprintf(sql,"DELETE FROM harvesters WHERE ID = %"PRIu64"",object->getId());
			mDatabase->ExecuteSqlAsync(NULL,NULL,sql);

			sprintf(sql,"DELETE FROM factories WHERE ID = %"PRIu64"",object->getId());
			mDatabase->ExecuteSqlAsync(NULL,NULL,sql);

			//Admin / Hopper Lists
			sprintf(sql,"DELETE FROM structure_admin_data WHERE StructureID = %"PRIu64"",object->getId());
			mDatabase->ExecuteSqlAsync(NULL,NULL,sql);

			//update attributes cave redeed vs destroy
			sprintf(sql,"DELETE FROM structure_attributes WHERE Structure_id = %"PRIu64"",object->getId());
			mDatabase->ExecuteSqlAsync(NULL,NULL,sql);

			//update hopper contents
			sprintf(sql,"DELETE FROM harvester_resources WHERE ID = %"PRIu64"",object->getId());
			mDatabase->ExecuteSqlAsync(NULL,NULL,sql);

		}
		break;

		case ObjType_Waypoint:
		{
			sprintf(sql,"DELETE FROM waypoints WHERE waypoint_id = %"PRIu64"",object->getId());
			mDatabase->ExecuteSqlAsync(NULL,NULL,sql);
		}
		break;


		default:break;
	}
}

//=============================================================================

void ObjectFactory::requestObject(ObjectType objType,uint16 subGroup,uint16 subType,ObjectFactoryCallback* ofCallback,uint64 id,DispatchClient* client)
{
	switch(objType)
	{
		case ObjType_Player:
		{
			mPlayerObjectFactory->requestObject(ofCallback,id,subGroup,subType,client);
		}
		break;

		case ObjType_Tangible:
		{
			mTangibleFactory->requestObject(ofCallback,id,subGroup,subType,client);
		}
		break;
		case ObjType_NPC:
		case ObjType_Creature:
		{
			mCreatureFactory->requestObject(ofCallback,id,subGroup,subType,client);
		}
		break;

		case ObjType_Building:
		{
			mBuildingFactory->requestObject(ofCallback,id,subGroup,subType,client);
		}
		break;

		case ObjType_Region:
		{
			mRegionFactory->requestObject(ofCallback,id,subGroup,subType,client);
		}
		break;

		case ObjType_Waypoint:
		{
			mWaypointFactory->requestObject(ofCallback,id,subGroup,subType,client);
		}
		break;

		default:
			gLogger->log(LogManager::DEBUG,"ObjectFactory::requestObject Unknown Object type");
		break;
	}
}

//=============================================================================

void ObjectFactory::releaseAllPoolsMemory()
{
	mDbAsyncPool.release_memory();

	mPlayerObjectFactory->releaseAllPoolsMemory();
	mTangibleFactory->releaseAllPoolsMemory();
	mIntangibleFactory->releaseAllPoolsMemory();
	mCreatureFactory->releaseAllPoolsMemory();
	mBuildingFactory->releaseAllPoolsMemory();
	mRegionFactory->releaseAllPoolsMemory();
	mWaypointFactory->releaseQueryContainerPoolMemory();
	mHarvesterFactory->releaseAllPoolsMemory();
	mFactoryFactory->releaseAllPoolsMemory();

}

//=============================================================================

