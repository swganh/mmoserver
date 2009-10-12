 /*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "StructureManager.h"
#include "ObjectFactory.h"
#include "PlayerObject.h"	  
#include "PlayerStructure.h"	 
#include "QuadTree.h"
#include "WorldManager.h"
#include "ZoneTree.h"

#include "LogManager/LogManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"

#include "Common/DispatchClient.h"


bool						StructureManager::mInsFlag    = false;
StructureManager*			StructureManager::mSingleton  = NULL;


//======================================================================================================================

StructureManager::StructureManager(Database* database,MessageDispatch* dispatch)
{	
	mDatabase = database;
	mMessageDispatch = dispatch;
	StructureManagerAsyncContainer* asyncContainer;

	// load our structure data
	//todo load buildings from building table and use appropriate stfs there
	//are harvesters on there too
	asyncContainer = new StructureManagerAsyncContainer(Structure_Query_LoadDeedData, 0);
	mDatabase->ExecuteSqlAsync(this,asyncContainer,"SELECT sdd.id, sdd.DeedType, sdd.SkillRequirement, s_td.object_string, s_td.lots_used, s_td.stf_name, s_td.stf_file, s_td.healing_modifier from swganh.structure_deed_data sdd INNER JOIN structure_type_data s_td ON sdd.id = s_td.type");

	//items
	asyncContainer = new StructureManagerAsyncContainer(Structure_Query_LoadstructureItem, 0);
	mDatabase->ExecuteSqlAsync(this,asyncContainer,"SELECT sit.structure_id, sit.cell, sit.item_type , sit.relX, sit.relY, sit.relZ, sit.dirX, sit.dirY, sit.dirZ, sit.dirW, sit.tan_type, "
													"it.object_string, it.stf_name, it.stf_file from swganh.structure_item_template sit INNER JOIN item_types it ON (it.id = sit.item_type) WHERE sit.tan_type = %u",TanGroup_Item);
	
	//statics
	asyncContainer = new StructureManagerAsyncContainer(Structure_Query_LoadstructureItem, 0);
	mDatabase->ExecuteSqlAsync(this,asyncContainer,"SELECT sit.structure_id, sit.cell, sit.item_type , sit.relX, sit.relY, sit.relZ, sit.dirX, sit.dirY, sit.dirZ, sit.dirW, sit.tan_type,  "
													"st.object_string, st.name, st.file from swganh.structure_item_template sit INNER JOIN static_types st ON (st.id = sit.item_type) WHERE sit.tan_type = %u",TanGroup_Static);
													
	
	//terminals
	asyncContainer = new StructureManagerAsyncContainer(Structure_Query_LoadstructureItem, 0);
	mDatabase->ExecuteSqlAsync(this,asyncContainer,"SELECT sit.structure_id, sit.cell, sit.item_type , sit.relX, sit.relY, sit.relZ, sit.dirX, sit.dirY, sit.dirZ, sit.dirW, sit.tan_type,  "
													"tt.object_string, tt.name, tt.file from swganh.structure_item_template sit INNER JOIN terminal_types tt ON (tt.id = sit.item_type) WHERE sit.tan_type = %u",TanGroup_Terminal);
	
}


//======================================================================================================================
StructureManager::~StructureManager()
{
	mInsFlag = false;
	delete(mSingleton);
	
}
//======================================================================================================================
StructureManager*	StructureManager::Init(Database* database, MessageDispatch* dispatch)
{
	if(!mInsFlag)
	{
		mSingleton = new StructureManager(database,dispatch);
		mInsFlag = true;
		return mSingleton;
	}
	else
		return mSingleton;
	
}

//======================================================================================================================

void StructureManager::Shutdown()
{

}


//=======================================================================================================================
static bool printed = false;
void StructureManager::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
	StructureManagerAsyncContainer* asynContainer = (StructureManagerAsyncContainer*)ref;
	
	switch(asynContainer->mQueryType)
	{

		case Structure_Query_delete:
		{

			PlayerStructure* structure = dynamic_cast<PlayerStructure*>(gWorldManager->getObjectById(asynContainer->mStructureId));
			
			attributeDetail detail;

			DataBinding* binding = mDatabase->CreateDataBinding(2);
			binding->addField(DFT_bstring,offsetof(attributeDetail,value),128,0);
			binding->addField(DFT_uint32,offsetof(attributeDetail,attributeId),4,1);
			
			
			uint64 count;
			count = result->getRowCount();

			if(!count)
			{
				gLogger->logMsgF("StructureManager::Structure %I64u without maintenance attributes",MSG_HIGH,asynContainer->mStructureId);
				break;
			}

			if(!structure)
			{
				gLogger->logMsgF("StructureManager::Structure %I64u not found",MSG_HIGH,asynContainer->mStructureId);
				break;
			}
	
			for(uint64 i = 0;i < count;i++)
			{
				result->GetNextRow(binding,&detail);

				if(detail.attributeId == 322)
				{
					//322 = energy_maintenance
					if (structure->hasAttribute("energy_maintenance"))
					{
						structure->setAttribute("energy_maintenance",detail.value.getAnsi());
						
					}

					structure->addAttribute("energy_maintenance",detail.value.getAnsi());
				}

				if(detail.attributeId == 382)
				{
					//382 = examine_maintenance
					if (structure->hasAttribute("examine_maintenance"))
					{
						structure->setAttribute("examine_maintenance",detail.value.getAnsi());
						
					}

					structure->addAttribute("examine_maintenance",detail.value.getAnsi());
				}

			}	
			structure->deleteStructureDBDataRead(asynContainer->mPlayerId);

			mDatabase->DestroyDataBinding(binding);
		}
		break;

		case Structure_Query_LoadDeedData:
		{
			StructureDeedLink* deedLink;
		
			DataBinding* binding = mDatabase->CreateDataBinding(7);
			binding->addField(DFT_uint32,offsetof(StructureDeedLink,structure_type),4,0);
			binding->addField(DFT_uint32,offsetof(StructureDeedLink,item_type),4,1);
			binding->addField(DFT_uint32,offsetof(StructureDeedLink,skill_Requirement),4,2);
			binding->addField(DFT_bstring,offsetof(StructureDeedLink,structureObjectString),128,3);
			binding->addField(DFT_uint8,offsetof(StructureDeedLink,requiredLots),1,4);
			binding->addField(DFT_bstring,offsetof(StructureDeedLink,stf_name),64,5);
			binding->addField(DFT_bstring,offsetof(StructureDeedLink,stf_file),64,6);
			binding->addField(DFT_float,offsetof(StructureDeedLink,healing_modifier),4,7);

			uint64 count;
			count = result->getRowCount();
	
			for(uint64 i = 0;i < count;i++)
			{
				deedLink	= new(StructureDeedLink);
				result->GetNextRow(binding,deedLink);
				mDeedLinkList.push_back(deedLink);
			}
		
			if(result->getRowCount())
				gLogger->logMsgLoadSuccess("StructureManager::Loading %u Structures...",MSG_NORMAL,result->getRowCount());
			else
				gLogger->logMsgLoadFailure("StructureManager::Loading Structures...",MSG_NORMAL);					
			
			mDatabase->DestroyDataBinding(binding);
		}
		break;

		case Structure_Query_LoadstructureItem:
		{

			StructureItemTemplate* itemTemplate;
		
			DataBinding* binding = mDatabase->CreateDataBinding(14);
			binding->addField(DFT_uint32,offsetof(StructureItemTemplate,structure_id),4,0);
			binding->addField(DFT_uint32,offsetof(StructureItemTemplate,CellNr),4,1);
			binding->addField(DFT_uint32,offsetof(StructureItemTemplate,item_type),4,2);
			binding->addField(DFT_float,offsetof(StructureItemTemplate,mPosition.mX),4,3);
			binding->addField(DFT_float,offsetof(StructureItemTemplate,mPosition.mY),4,4);
			binding->addField(DFT_float,offsetof(StructureItemTemplate,mPosition.mZ),4,5);
			binding->addField(DFT_float,offsetof(StructureItemTemplate,mDirection.mX),4,6);
			binding->addField(DFT_float,offsetof(StructureItemTemplate,mDirection.mY),4,7);
			binding->addField(DFT_float,offsetof(StructureItemTemplate,mDirection.mZ),4,8);
			binding->addField(DFT_float,offsetof(StructureItemTemplate,dw),4,9);
			binding->addField(DFT_uint32,offsetof(StructureItemTemplate,tanType),4,10);
			
			binding->addField(DFT_bstring,offsetof(StructureItemTemplate,structureObjectString),128,11);
			binding->addField(DFT_bstring,offsetof(StructureItemTemplate,name),32,12);
			binding->addField(DFT_bstring,offsetof(StructureItemTemplate,file),32,13);

			uint64 count;
			count = result->getRowCount();
	
			for(uint64 i = 0;i < count;i++)
			{
				itemTemplate = new(StructureItemTemplate);
				result->GetNextRow(binding,itemTemplate);
				mItemTemplate.push_back(itemTemplate);
			}
			
			if(result->getRowCount())
				gLogger->logMsgLoadSuccess("StructureManager::Loading %u Structure Items...",MSG_NORMAL,result->getRowCount());
			else
				gLogger->logMsgLoadFailure("StructureManager::Loading Structure Items...",MSG_NORMAL);					

			mDatabase->DestroyDataBinding(binding);
		}
		break;
		
		default:break;

	}
	SAFE_DELETE(asynContainer);
}

//=======================================================================================================================
//handles callbacks of db creation of items
//=======================================================================================================================

void StructureManager::getDeleteStructureMaintenanceData(uint64 structureId, uint64 playerId)
{
	// load our structures maintenance data
	// that means the maintenance attribute and the energy attribute
	//

	StructureManagerAsyncContainer* asyncContainer;

	asyncContainer = new StructureManagerAsyncContainer(Structure_Query_delete, 0);
	mDatabase->ExecuteSqlAsync(this,asyncContainer,"SELECT sa.value, sa.attribute_id FROM structure_attributes sa where attribute_id = 382 or attribute_id = 322 and structure_id = %I64u",structureId);
	asyncContainer->mStructureId = structureId;
	asyncContainer->mPlayerId = playerId;

	//322 = energy_maintenance
	//382 = examine_maintenance
//mDatabase->ExecuteSqlAsync(0,0,"UPDATE item_attributes SET value='%.2f' WHERE item_id=%lld AND attribute_id=%u",attValue,mItem->getId(),att->getAttributeId());
}


//=======================================================================================================================
//handles callbacks of db creation of items
//=======================================================================================================================

void StructureManager::handleObjectReady(Object* object,DispatchClient* client)
{
}


//======================================================================================================================
//looks up the data for a specific deed
//======================================================================================================================

StructureDeedLink* StructureManager::getDeedData(uint32 type)
{
	DeedLinkList::iterator it = mDeedLinkList.begin();
	bool found = false;
	while(it != mDeedLinkList.end())
	{
		if ((*it)->item_type == type )
		{
			return (*it);
		}
		it++;
	}

	return NULL;
}

//======================================================================================================================
//returns true when we are NOT within 25m of a camp
//======================================================================================================================

bool StructureManager::checkCampRadius(PlayerObject* player)
{
	QTRegion*			mQTRegion = NULL;
	uint32				subZoneId = player->getSubZoneId();
	float				width  = 25.0;
	float				height = 25.0;

	Anh_Math::Rectangle mQueryRect;
	if(!subZoneId)
	{
		mQTRegion	= gWorldManager->getSI()->getQTRegion(player->mPosition.mX,player->mPosition.mZ);
		subZoneId	= (uint32)mQTRegion->getId();
		mQueryRect	= Anh_Math::Rectangle(player->mPosition.mX - width,player->mPosition.mZ - height,width * 2,height * 2);
	}

	RegionObject*	object;
	ObjectSet		objList;
	
	gWorldManager->getSI()->getObjectsInRange(player,&objList,ObjType_Region,width*2);

	if(mQTRegion)
	{
		mQTRegion->mTree->getObjectsInRange(player,&objList,ObjType_Region,&mQueryRect);
	}

	ObjectSet::iterator objIt = objList.begin();

	while(objIt != objList.end())
	{
		object = (RegionObject*)(*objIt);

		if(object->getRegionType() == Region_Camp)
		{
			return false;
		}

		++objIt;
	}

	return true;

}

//======================================================================================================================
//returns true when we are NOT within 5m of a city
//======================================================================================================================

bool StructureManager::checkCityRadius(PlayerObject* player)
{
	QTRegion*			mQTRegion = NULL;
	uint32				subZoneId = player->getSubZoneId();
	float				width  = 5.0;
	float				height = 5.0;

	Anh_Math::Rectangle mQueryRect;
	if(!subZoneId)
	{
		mQTRegion	= gWorldManager->getSI()->getQTRegion(player->mPosition.mX,player->mPosition.mZ);
		subZoneId	= (uint32)mQTRegion->getId();
		mQueryRect	= Anh_Math::Rectangle(player->mPosition.mX - width,player->mPosition.mZ - height,width * 2,height * 2);
	}

	RegionObject*	object;
	ObjectSet		objList;
	
	gWorldManager->getSI()->getObjectsInRangeIntersection(player,&objList,ObjType_Region,width*2);

	if(mQTRegion)
	{
		mQTRegion->mTree->getObjectsInRange(player,&objList,ObjType_Region,&mQueryRect);
	}

	ObjectSet::iterator objIt = objList.begin();

	while(objIt != objList.end())
	{
		object = (RegionObject*)(*objIt);

		if(object->getRegionType() == Region_City)
		{
			return false;
		}

		++objIt;
	}

	return true;

}

//======================================================================================================================
//returns true when we are within 1m of a camp
//======================================================================================================================

bool StructureManager::checkinCamp(PlayerObject* player)
{
	QTRegion*			mQTRegion = NULL;
	uint32				subZoneId = player->getSubZoneId();
	float				width  = 1.0;
	float				height = 1.0;

	Anh_Math::Rectangle mQueryRect;
	if(!subZoneId)
	{
		mQTRegion	= gWorldManager->getSI()->getQTRegion(player->mPosition.mX,player->mPosition.mZ);
		subZoneId	= (uint32)mQTRegion->getId();
		mQueryRect	= Anh_Math::Rectangle(player->mPosition.mX - width,player->mPosition.mZ - height,width * 2,height * 2);
	}

	RegionObject*	object;
	ObjectSet		objList;
	
	gWorldManager->getSI()->getObjectsInRange(player,&objList,ObjType_Region,width*2);

	if(mQTRegion)
	{
		mQTRegion->mTree->getObjectsInRange(player,&objList,ObjType_Region,&mQueryRect);
	}

	ObjectSet::iterator objIt = objList.begin();

	while(objIt != objList.end())
	{
		object = (RegionObject*)(*objIt);

		if(object->getRegionType() == Region_Camp)
		{
			return true;
		}

		++objIt;
	}

	return false;

}


//=========================================================================================0
// gets the code to confirm structure destruction
//
string StructureManager::getCode()
{
	int8	serial[12],chance[9];
	bool	found = false;
	uint8	u;

	for(uint8 i = 0; i < 6; i++)
	{
		while(!found)
		{
			found = true;
			u = static_cast<uint8>(static_cast<double>(gRandom->getRand()) / (RAND_MAX + 1.0f) * (122.0f - 48.0f) + 48.0f);

			//only 1 to 9 or a to z
			if(u >57)
				found = false;

			if(u < 48)
				found = false;
			
		}
		chance[i] = u;
		found = false;
	}
	chance[6] = 0;

	sprintf(serial,"(%s)",chance);

	return(BString(serial));
}
