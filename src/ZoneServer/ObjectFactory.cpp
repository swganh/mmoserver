/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "ObjectFactory.h"
#include "ObjectFactoryCallback.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include "LogManager/LogManager.h"
#include "WorldManager.h"
#include "ResourceManager.h"
#include "Utils/utils.h"
#include "DraftSchematic.h"
#include "StructureManager.h"
#include "TravelMapHandler.h"



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
		case OFQuery_Harvester:
		{
			if(!result->getRowCount())
			{
				gLogger->logMsg("ObjFactory::handleDatabaseJobComplete   :  create Harvester failed");
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
			}
			else
				gLogger->logMsg("ObjFactory::handleDatabaseJobComplete   :  create Harvester failed");
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
				gLogger->logMsg("ObjFactory::createWaypoint failed");
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
				gLogger->logMsg("ObjFactory::createItem failed");
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
				gLogger->logMsg("ObjFactory::createResourceContainer failed");
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

	mDatabase->ExecuteSqlAsync(this,asyncContainer,"SELECT sf_DefaultManufactureSchematicCreate(%u,%lld)",schemCrc,parentId);
}

//=============================================================================
//
// create a new item by schematic crc with default attributes
//
void ObjectFactory::requestNewDefaultItem(ObjectFactoryCallback* ofCallback,uint32 schemCrc,uint64 parentId,uint16 planetId,Anh_Math::Vector3 position,string customName)
{
	OFAsyncContainer* asyncContainer = new(mDbAsyncPool.ordered_malloc()) OFAsyncContainer(ofCallback,OFQuery_Item,NULL);

	mDatabase->ExecuteSqlAsync(this,asyncContainer,"SELECT sf_DefaultItemCreateBySchematic(%u,%lld,%u,%f,%f,%f,'%s')",schemCrc,parentId,planetId,position.mX,position.mY,position.mZ,customName.getAnsi());
}

//=============================================================================
//
// create a new item with default attributes
//
void ObjectFactory::requestNewDefaultItem(ObjectFactoryCallback* ofCallback,uint32 familyId,uint32 typeId,uint64 parentId,uint16 planetId,Anh_Math::Vector3 position,string customName)
{
	OFAsyncContainer* asyncContainer = new(mDbAsyncPool.ordered_malloc()) OFAsyncContainer(ofCallback,OFQuery_Item,NULL);

	mDatabase->ExecuteSqlAsync(this,asyncContainer,"SELECT sf_DefaultItemCreate(%u,%u,%lld,%lld,%u,%f,%f,%f,'%s')",familyId,typeId,parentId,(uint64) 0,planetId,position.mX,position.mY,position.mZ,customName.getAnsi());
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
	sprintf(dstPlanetIdStr,"','%s','",gWorldManager->getPlanetNameById(ticketProperties.dstPlanetId));
	sprintf(restStr,"',%lld,%f,%f,%f,%u)",parentId,0.0f,0.0f,0.0f,planetId);
	sprintf(sql,"SELECT sf_TravelTicketCreate('%s','",gWorldManager->getPlanetNameById(ticketProperties.srcPlanetId));
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
	mDatabase->ExecuteSqlAsync(this,asyncContainer,"SELECT sf_ResourceContainerCreate(%lld,%lld,0,0,0,%u,%u)",resourceId,parentId,planetId,amount);
}

void ObjectFactory::requestnewHarvesterbyDeed(ObjectFactoryCallback* ofCallback,Deed* deed,DispatchClient* client, float x, float y, float z, float dir, string customName, PlayerObject* player)
{
	//create a new Harvester Object with the attributes as specified by the deed
	OFAsyncContainer* asyncContainer = new(mDbAsyncPool.ordered_malloc()) OFAsyncContainer(ofCallback,OFQuery_Harvester,NULL);

	asyncContainer->DeedId = deed->getId();
	asyncContainer->OwnerId = deed->getOwner();
	int8 sql[512];
	//sf_DefaultHarvesterCreate`(type_id INT(11),parent_id BIGINT(20),privateowner_id BIGINT(20),inPlanet INT,oX FLOAT,oY FLOAT,oZ FLOAT, oW FLOAT,inX FLOAT,inY FLOAT,inZ FLOAT,custom_name CHAR(255)) RETURNS bigint(20)
	
	StructureDeedLink* deedLink;
	deedLink = gStructureManager->getDeedData(deed->getItemType());

	float oX, oY, oZ, oW;
	if(dir== 0.0)
	{		
		gLogger->logMsgF("dir is %f",MSG_HIGH,dir);
		oX = 0;
		oY = 0;
		oZ = 0;
		oW = 1;
	}
	if(dir== 1.0)	 //ok
	{
		oX = 0.0;
		oY = 0.71;
		oZ = 0.0;
		oW = 0.71;
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
		oY = -0.71;
		oZ = 0;
		oW = 0.71;
	}

	
	gLogger->logMsgF("dir is %f, x:%f, y:%f, z:%f, w:%f",MSG_HIGH,dir,oX, oY, oZ, oW);
	
	sprintf(sql,"SELECT sf_DefaultHarvesterCreate(%u,0,%I64u,%u,%f,%f,%f,%f,%f,%f,%f,'%s')",deedLink->structure_type, player->getPlayerObjId(), gWorldManager->getZoneId(),oX,oY,oZ,oW,x,y,z,customName.getAnsi());
	mDatabase->ExecuteSqlAsync(this,asyncContainer,sql);
	gLogger->logMsgF(sql,MSG_HIGH);

}

//=============================================================================
//
// create a new waypoint
// never call this directly - always go over the datapad!!!!!  we need to check the capacity
//
void ObjectFactory::requestNewWaypoint(ObjectFactoryCallback* ofCallback,string name,const Anh_Math::Vector3 coords,uint16 planetId,uint64 ownerId,uint8 wpType)
{

	OFAsyncContainer* asyncContainer = new(mDbAsyncPool.ordered_malloc()) OFAsyncContainer(ofCallback,OFQuery_WaypointCreate,NULL);
	int8 sql[512],*sqlPointer;
	int8 restStr[128];

	sprintf(sql,"SELECT sf_WaypointCreate('");
	sqlPointer = sql + strlen(sql);
	sqlPointer += mDatabase->Escape_String(sqlPointer,name.getAnsi(),name.getLength());
	sprintf(restStr,"',%lld,%f,%f,%f,%u,%u)",ownerId,coords.mX,coords.mY,coords.mZ,planetId,wpType);
	strcat(sql,restStr);

	mDatabase->ExecuteSqlAsync(this,asyncContainer,sql);

}

//=============================================================================

void ObjectFactory::requestTanoNewParent(ObjectFactoryCallback* ofCallback,uint64 ObjectId,uint64 parentID, TangibleGroup Group)
{
	OFAsyncContainer* asyncContainer;
	int8 sql[512];
	asyncContainer = new(mDbAsyncPool.ordered_malloc()) OFAsyncContainer(ofCallback,OFQuery_Default,NULL);
	asyncContainer->Id = ObjectId;
	asyncContainer->Group = Group;

	switch(Group)
	{
		case TanGroup_Item:
		{
			sprintf(sql,"UPDATE items SET parent_id = '%I64u' WHERE id = '%I64u' ",parentID,ObjectId);
		}
		break;

		case TanGroup_ResourceContainer:
		{
		
			sprintf(sql,"UPDATE resource_containers SET parent_id = '%I64u' WHERE id = '%I64u' ",parentID,ObjectId);
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
					sprintf(sql,"UPDATE items SET parent_id = '%I64u' WHERE id = '%I64u' ",ID,object->getId());
				}
				break;

				case TanGroup_ResourceContainer:
				{
					sprintf(sql,"UPDATE resource_containers SET parent_id = '%I64u' WHERE id = '%I64u' ",ID,object->getId());
				}
				break;

				default:break;
			}
		}
		break;

		case ObjType_Waypoint:
		{
			sprintf(sql,"UPDATE waypoints SET parent_id = '%I64u' WHERE id = '%I64u' ",ID,object->getId());
		}
		break;

		default:break;
	}
	mDatabase->ExecuteSqlAsync(NULL,NULL,sql);
}

//=============================================================================

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
					sprintf(sql,"DELETE FROM items WHERE id = %lld",object->getId());
					mDatabase->ExecuteSqlAsync(NULL,NULL,sql);

					sprintf(sql,"DELETE FROM item_attributes WHERE item_id = %lld",object->getId());
				}
				break;

				case TanGroup_ResourceContainer:
				{
					sprintf(sql,"DELETE FROM resource_containers WHERE id = %lld",object->getId());
				}
				break;

				case TanGroup_ManufacturingSchematic:
				{
					ManufacturingSchematic* schem = dynamic_cast<ManufacturingSchematic*> (object);
					//first associated item
					sprintf(sql,"DELETE FROM items WHERE id = %lld",schem->getItem()->getId());
					mDatabase->ExecuteSqlAsync(NULL,NULL,sql);
					sprintf(sql,"DELETE FROM item_attributes WHERE item_id = %lld",schem->getItem()->getId());
					mDatabase->ExecuteSqlAsync(NULL,NULL,sql);
					
					//now the schematic
					sprintf(sql,"DELETE FROM items WHERE id = %lld",object->getId());
					mDatabase->ExecuteSqlAsync(NULL,NULL,sql);
					sprintf(sql,"DELETE FROM item_attributes WHERE item_id = %lld",object->getId());
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
					sprintf(sql,"DELETE FROM vehicle_cutomization WHERE vehicles_id = %lld",object->getId());
					mDatabase->ExecuteSqlAsync(NULL,NULL,sql);
					sprintf(sql,"DELETE FROM vehicle_attributes WHERE vehicles_id = %lld",object->getId());
					mDatabase->ExecuteSqlAsync(NULL,NULL,sql);
					sprintf(sql,"DELETE FROM vehicles WHERE id = %lld",object->getId());
				}
				break;

				default: break;
			}
		}
		break;

		case ObjType_Waypoint:
		{
			sprintf(sql,"DELETE FROM waypoints WHERE waypoint_id = %lld",object->getId());
		}
		break;


		default:break;
	}
	mDatabase->ExecuteSqlAsync(NULL,NULL,sql);
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
			gLogger->logMsg("ObjectFactory::requestObject Unknown Object type\n");
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
}

//=============================================================================

