 /*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/
#include "WorldConfig.h"
#include "StructureManager.h"
#include "nonPersistantObjectFactory.h"
#include "HarvesterObject.h"
#include "ObjectFactory.h"
#include "PlayerObject.h"
#include "PlayerStructure.h"
#include "QuadTree.h"
#include "WorldManager.h"
#include "ZoneTree.h"
#include "MessageLib/MessageLib.h"

#include "LogManager/LogManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include "UIManager.h"

#include "Common/DispatchClient.h"


bool						StructureManager::mInsFlag    = false;
StructureManager*			StructureManager::mSingleton  = NULL;


//======================================================================================================================

StructureManager::StructureManager(Database* database,MessageDispatch* dispatch)
{
	mBuildingFenceInterval = gWorldConfig->getConfiguration("Zone_BuildingFenceInterval",(uint16)10000);

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
//static bool printed = false;
void StructureManager::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
	StructureManagerAsyncContainer* asynContainer = (StructureManagerAsyncContainer*)ref;

	switch(asynContainer->mQueryType)
	{
		
		//tests the amount of lots for the recipient of a structure during a structure transfer
		case Structure_StructureTransfer_Lots_Recipient:
		{
			PlayerStructure* structure = dynamic_cast<PlayerStructure*>(gWorldManager->getObjectById(asynContainer->mStructureId));

			PlayerObject* donor = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(asynContainer->mPlayerId));
			PlayerObject* recipient = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(asynContainer->mTargetId));

			uint8 lots;
			DataBinding* binding = mDatabase->CreateDataBinding(1);
			binding->addField(DFT_uint8,0,1);

			uint64 count;
			count = result->getRowCount();
			if(!count)
			{
				gLogger->logMsgF("StructureManager::Transfer Structure Admin List Callback couldnt get recipients lots",MSG_HIGH);
				mDatabase->DestroyDataBinding(binding);			
				return;
			}

			
			//thats lots in use
			result->GetNextRow(binding,&lots);

			uint8 requiredLots = structure->getLotCount();

			uint8 freelots = gWorldConfig->getConfiguration("Player_Max_Lots",(uint8)10) - lots;
			if(freelots >= requiredLots)
			{
				//yay we were succesful
				structure->setOwner(asynContainer->mTargetId);
				mDatabase->ExecuteSqlAsync(0,0,"UPDATE structures SET structures.owner = %I64u WHERE structures.id = %I64u",asynContainer->mTargetId,asynContainer->mStructureId);
				mDatabase->ExecuteSqlAsync(0,0,"DELETE FROM structure_admin_data where playerId = %I64u AND StructureID = %I64u",asynContainer->mPlayerId,asynContainer->mStructureId);
				mDatabase->ExecuteSqlAsync(0,0,"INSERT INTO structure_admin_data VALUES (NULL,%I64u,%I64u,'ADMIN')",asynContainer->mStructureId, asynContainer->mTargetId);
				        

				//update the administration list

				if(donor)
				{
					gMessageLib->sendSystemMessage(donor,L"","player_structure","ownership_transferred_out","",asynContainer->name);
				}
				if(recipient)
				{
					gMessageLib->sendSystemMessage(recipient,L"","player_structure","ownership_transferred_in","",donor->getFirstName().getAnsi());
				}
				

			}
			else
			{
			}
			
			mDatabase->DestroyDataBinding(binding);												   	

		}
		break;
		case Structure_Query_Admin_Data:
		{
			PlayerStructure* structure = dynamic_cast<PlayerStructure*>(gWorldManager->getObjectById(asynContainer->mStructureId));

			string playerName;
			DataBinding* binding = mDatabase->CreateDataBinding(1);
			binding->addField(DFT_bstring,0,64);

			uint64 count;
			count = result->getRowCount();

			for(uint64 i = 0;i < count;i++)
			{
				result->GetNextRow(binding,&playerName);

				structure->addStructureAdmin(playerName);

			}

			structure->sendStructureAdminList(asynContainer->mPlayerId);

			mDatabase->DestroyDataBinding(binding);
		}
		break;

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
				//add the attributes so the structure has them and can be deleted
				//322 = energy_maintenance
				//382 = examine_maintenance
				int8 sql[250];
				sprintf(sql,"INSERT INTO item_attributes VALUES(%"PRIu64",384,'%s',2,0)",asynContainer->mStructureId,"5");
				mDatabase->ExecuteSqlAsync(0,0,sql);
				structure->addAttribute("energy_maintenance","5");

				sprintf(sql,"INSERT INTO item_attributes VALUES(%"PRIu64",382,'%s',2,0)",asynContainer->mStructureId,"5");
				mDatabase->ExecuteSqlAsync(0,0,sql);
				structure->addAttribute("examine_maintenance","5");
				
			}

			if(!structure)
			{
				gLogger->logMsgF("StructureManager::Structure %I64u not found",MSG_HIGH,asynContainer->mStructureId);
				break;
			}

			for(uint64 i = 0;i < count;i++)
			{
				result->GetNextRow(binding,&detail);

				if(detail.attributeId == 384)
				{
					//322 = energy_maintenance
					if (structure->hasAttribute("examine_power"))
					{
						structure->setAttribute("examine_power",detail.value.getAnsi());

					}
					else
						structure->addAttribute("examine_power",detail.value.getAnsi());
				}

				if(detail.attributeId == 382)
				{
					//382 = examine_maintenance
					if (structure->hasAttribute("examine_maintenance"))
					{
						structure->setAttribute("examine_maintenance",detail.value.getAnsi());

					}
					else
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

		case Structure_Query_Remove_Permission:
		{
			//PlayerStructure* structure = dynamic_cast<PlayerStructure*>(gWorldManager->getObjectById(asynContainer->mStructureId));

			PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(asynContainer->mPlayerId));

			uint32 returnValue;
			DataBinding* binding = mDatabase->CreateDataBinding(1);
			binding->addField(DFT_uint32,0,4);

			uint64 count;
			count = result->getRowCount();

			if (!count)
			{
				gLogger->logMsgLoadFailure("StructureManager::add Permission no return value...",MSG_NORMAL);
			}
			result->GetNextRow(binding,&returnValue);
			// 0 is sucess
			// 1 name doesnt exist
			// 2 name not on list
			// 3 Owner cannot be removed

			if(returnValue == 0)
			{
				string name;
				name = asynContainer->name;
				//name.convert(BSTRType_Unicode16);
				gMessageLib->sendSystemMessage(player,L"","player_structure","player_removed","",name.getAnsi());
			}

			if(returnValue == 1)
			{
				string name;
				name = asynContainer->name;
				name.convert(BSTRType_Unicode16);

				gMessageLib->sendSystemMessage(player,L"","player_structure","modify_list_invalid_player","","",name.getUnicode16());
			}

			if(returnValue == 2)
			{
				string name;
				name = asynContainer->name;
				//name.convert(BSTRType_Unicode16);
				name.convert(BSTRType_ANSI);
				name << " is not on the list";
				name.convert(BSTRType_Unicode16);
				gMessageLib->sendSystemMessage(player,name.getUnicode16());
			}

			if(returnValue == 3)
			{
				string name;
				name = asynContainer->name;
				name.convert(BSTRType_Unicode16);

				gMessageLib->sendSystemMessage(player,L"","player_structure","cannot_remove_owner","","",name.getUnicode16());
			}


			//sendStructureAdminList(asynContainer->mPlayerId);

			mDatabase->DestroyDataBinding(binding);
		}
		break;



		case Structure_Query_Add_Permission:
		{
			//PlayerStructure* structure = dynamic_cast<PlayerStructure*>(gWorldManager->getObjectById(asynContainer->mStructureId));

			PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(asynContainer->mPlayerId));

			uint32 returnValue;
			DataBinding* binding = mDatabase->CreateDataBinding(1);
			binding->addField(DFT_uint32,0,4);

			uint64 count;
			count = result->getRowCount();

			if (!count)
			{
				gLogger->logMsgLoadFailure("StructureManager::add Permission no return value...",MSG_NORMAL);
			}
			result->GetNextRow(binding,&returnValue);
			// 0 is sucess
			// 1 name doesnt exist
			// 2 name already on list
			// 3 list is full (more than 36 entries)

			if(returnValue == 0)
			{
				string name;
				name = asynContainer->name;
				gMessageLib->sendSystemMessage(player,L"","player_structure","player_added","",name.getAnsi());
			}

			if(returnValue == 1)
			{
				string name;
				name = asynContainer->name;
				name.convert(BSTRType_ANSI);
				gLogger->logMsgF("StructurManager add %s failed ", MSG_HIGH,name.getAnsi());
				name.convert(BSTRType_Unicode16);
				
				gMessageLib->sendSystemMessage(player,L"","player_structure","modify_list_invalid_player","","",name.getUnicode16());
			}

			if(returnValue == 2)
			{
				string name;
				name = asynContainer->name;
				name.convert(BSTRType_ANSI);
				name << " is already on the list";
				name.convert(BSTRType_Unicode16);
				gMessageLib->sendSystemMessage(player,name.getUnicode16());
			}

			if(returnValue == 3)
			{
				gMessageLib->sendSystemMessage(player,L"","player_structure","too_many_entries");
			}

			//sendStructureAdminList(asynContainer->mPlayerId);

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

		// =========================================
		// asynchronously checks whether the player is on the admin list
		case Structure_Query_Check_Permission:
		{
			PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(asynContainer->mPlayerId));

			uint32 returnValue;
			DataBinding* binding = mDatabase->CreateDataBinding(1);
			binding->addField(DFT_uint32,0,4);

			uint64 count;
			count = result->getRowCount();

			if (!count)
			{
				gLogger->logMsgLoadFailure("StructureManager::check Permission no return value...",MSG_NORMAL);
				mDatabase->DestroyDataBinding(binding);
				return;
			}
			result->GetNextRow(binding,&returnValue);
			// 0 is on List
			// 1 name doesnt exist
			// 2 name not on list
			// 3 owner

			if((returnValue == 0)||(returnValue == 3))
			{
				// call processing handler
				// 3 means structure Owner
				processVerification(asynContainer->command,(returnValue == 3));
				
			}

			if(returnValue == 1)
			{
				string name;
				name = asynContainer->name;
				name.convert(BSTRType_ANSI);
				
				gLogger->logMsgF("StructurManager check Permission name %s doesnt exist ", MSG_HIGH,name.getAnsi());
				
			}

			if(returnValue == 2)
			{
				gMessageLib->sendSystemMessage(player,L"You are not an admin of this structure");
			}

			mDatabase->DestroyDataBinding(binding);
		}
		break;

		default:break;

	}

	SAFE_DELETE(asynContainer);
}



//=======================================================================================================================
//checks for a name on a permission list
//=======================================================================================================================
void StructureManager::checkNameOnPermissionList(uint64 structureId, uint64 playerId, string name, string list, StructureAsyncCommand command)
{

	StructureManagerAsyncContainer* asyncContainer;

	asyncContainer = new StructureManagerAsyncContainer(Structure_Query_Check_Permission, 0);
	mDatabase->ExecuteSqlAsync(this,asyncContainer,"select sf_CheckPermissionList(%I64u,'%s','%s')",structureId,name.getAnsi(),list.getAnsi());
	asyncContainer->mStructureId = structureId;
	asyncContainer->mPlayerId = playerId;
	asyncContainer->command = command;
	sprintf(asyncContainer->name,"%s",name.getAnsi());


	// 0 is Name on list
	// 1 name doesnt exist
	// 2 name not on list
	// 3 Owner 
}


//=======================================================================================================================
//removes a name from a permission list
//=======================================================================================================================
void StructureManager::removeNamefromPermissionList(uint64 structureId, uint64 playerId, string name, string list)
{

	StructureManagerAsyncContainer* asyncContainer;

	asyncContainer = new StructureManagerAsyncContainer(Structure_Query_Remove_Permission, 0);
	mDatabase->ExecuteSqlAsync(this,asyncContainer,"select sf_RemovePermissionList(%I64u,'%s','%s')",structureId,name.getAnsi(),list.getAnsi());
	asyncContainer->mStructureId = structureId;
	asyncContainer->mPlayerId = playerId;
	sprintf(asyncContainer->name,"%s",name.getAnsi());


	// 0 is sucess
	// 1 name doesnt exist
	// 2 name not on list
	// 3 Owner cannot be removed
}



//=======================================================================================================================
//adds a name to a permission list
//=======================================================================================================================
void StructureManager::addNametoPermissionList(uint64 structureId, uint64 playerId, string name, string list)
{
	// load our structures maintenance data
	// that means the maintenance attribute and the energy attribute
	//

	StructureManagerAsyncContainer* asyncContainer;

	asyncContainer = new StructureManagerAsyncContainer(Structure_Query_Add_Permission, 0);
	mDatabase->ExecuteSqlAsync(this,asyncContainer,"select sf_AddPermissionList(%"PRIu64",'%s','%s')",structureId,name.getAnsi(),list.getAnsi());
	asyncContainer->mStructureId = structureId;
	asyncContainer->mPlayerId = playerId;
	sprintf(asyncContainer->name,"%s",name.getAnsi());


	// 0 is sucess
	// 1 name doesnt exist
	// 2 name already on list

//mDatabase->ExecuteSqlAsync(0,0,"UPDATE item_attributes SET value='%.2f' WHERE item_id=%"PRIu64" AND attribute_id=%u",attValue,mItem->getId(),att->getAttributeId());
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
	mDatabase->ExecuteSqlAsync(this,asyncContainer,"SELECT sa.value, sa.attribute_id FROM structure_attributes sa where attribute_id = 382 or attribute_id = 384 and structure_id = %I64u",structureId);
	asyncContainer->mStructureId = structureId;
	asyncContainer->mPlayerId = playerId;

	//382 = examine_maintenance
	//384 = examine_power
//mDatabase->ExecuteSqlAsync(0,0,"UPDATE item_attributes SET value='%.2f' WHERE item_id=%"PRIu64" AND attribute_id=%u",attValue,mItem->getId(),att->getAttributeId());
}


//=======================================================================================================================
//handles callbacks of db creation of items
//=======================================================================================================================

void StructureManager::handleObjectReady(Object* object,DispatchClient* client)
{
	PlayerStructure* structure = dynamic_cast<PlayerStructure*>(object);

	if(!structure)
	{
			gLogger->logMsg("StructureManager::handleObjectReady: No structure");
	}

	if(gWorldManager->getWMState() == WMState_Running)
	{
		// set timer for deletion of building fence

		uint32 account = client->getAccountId();
		PlayerObject* player = gWorldManager->getPlayerByAccId(account);

		PlayerStructure* fence =  gNonPersistantObjectFactory->requestBuildingFenceObject(structure->mPosition.mX,structure->mPosition.mY,structure->mPosition.mZ, player);
		structure->getTTS()->todo = ttE_BuildingFence;
		structure->getTTS()->buildingFence = fence->getId();
		structure->getTTS()->playerId = player->getId();
		structure->getTTS()->projectedTime = mBuildingFenceInterval + Anh_Utils::Clock::getSingleton()->getLocalTime();

		gWorldManager->handleObjectReady(structure,player->getClient());

		addStructureforConstruction(structure->getId());
	}
	else
	{
		gWorldManager->handleObjectReady(structure,NULL);
	}




}


//======================================================================================================================
//looks up the data for a specific deed
//======================================================================================================================

StructureDeedLink* StructureManager::getDeedData(uint32 type)
{
	DeedLinkList::iterator it = mDeedLinkList.begin();
	//bool found = false;
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

	sprintf(serial,"%s",chance);

	return(BString(serial));
}

//======================================================================================================================
//
// Handle deletion of destroyed Structures
//

bool StructureManager::_handleStructureObjectTimers(uint64 callTime, void* ref)
{
	//iterate through all harvesters to delete
	ObjectIDList* objectList = gStructureManager->getStrucureDeleteList();
	ObjectIDList::iterator it = objectList->begin();

	while(it != objectList->end())
	{
		PlayerStructure* structure = dynamic_cast<PlayerStructure*>(gWorldManager->getObjectById((*it)));

		if(!structure)
		{
			gLogger->logMsg("StructureManager::_handleStructureObjectTimers: No structure");
			continue;
		}

		if(structure->getTTS()->todo == ttE_UpdateHopper)
		{

			if(Anh_Utils::Clock::getSingleton()->getLocalTime() < structure->getTTS()->projectedTime)
			{
				gLogger->logMsg("StructureManager::_handleStructureObjectTimers: intervall to short - delayed");
				break;
			}

			PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById( structure->getTTS()->playerId ));
			if(!player)
			{
				break;
			}

			HarvesterObject* harvester = dynamic_cast<HarvesterObject*>(structure);

			if(!harvester)
			{
				gLogger->logMsg("StructureManager::_handleStructureObjectTimers: No structure");
				continue;
			}

			// TODO
			// read the current resource hopper contents
			StructureManagerAsyncContainer* asyncContainer = new StructureManagerAsyncContainer(Structure_HopperUpdate,player->getClient());

			asyncContainer->mStructureId	= structure->getId();
			asyncContainer->mPlayerId		= player->getId();
			
			int8 sql[250];
			sprintf(sql,"SELECT hr.resourceID, hr.quantity FROM harvester_resources hr WHERE hr.ID = '%"PRIu64"' ",asyncContainer->mStructureId);
			
			mDatabase->ExecuteSqlAsync(harvester,asyncContainer,sql);	

			//is the structure in Range??? - otherwise stop updating
			float fTransferDistance = gWorldConfig->getConfiguration("Player_Structure_Operate_Distance",(float)20.0);
			if(player->mPosition.inRange2D(structure->mPosition,fTransferDistance))
			{
				structure->getTTS()->projectedTime = Anh_Utils::Clock::getSingleton()->getLocalTime() + 5000;
				addStructureforHopperUpdate(structure->getId());
			}
		

		}

		if(structure->getTTS()->todo == ttE_Delete)
		{
			// TODO
			// get the deed to the inventory
			// update the deeds attributes
			gObjectFactory->deleteObjectFromDB(structure);
			gMessageLib->sendDestroyObject_InRangeofObject(structure);
			gWorldManager->destroyObject(structure);

		}

		if(structure->getTTS()->todo == ttE_BuildingFence)
		{
			if(Anh_Utils::Clock::getSingleton()->getLocalTime() < structure->getTTS()->projectedTime)
			{
				gLogger->logMsg("StructureManager::_handleStructureObjectTimers: intervall to short - delayed");
				break;
			}

			//gLogger->logMsg("StructureManager::_handleStructureObjectTimers: building fence");

			PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById( structure->getTTS()->playerId ));
			PlayerStructure* fence = dynamic_cast<PlayerStructure*>(gWorldManager->getObjectById(structure->getTTS()->buildingFence));

			if(!player)
			{
				gLogger->logMsg("StructureManager::_handleStructureObjectTimers: No Player");
				gMessageLib->sendDestroyObject_InRangeofObject(fence);
				gWorldManager->destroyObject(fence);
				gWorldManager->handleObjectReady(structure,player->getClient());


				return false;
			}

			if(!fence)
			{
				gLogger->logMsg("StructureManager::_handleStructureObjectTimers: No fence");
				return false;
			}

			//delete the fence
			gMessageLib->sendDestroyObject_InRangeofObject(fence);
			gWorldManager->destroyObject(fence);

			PlayerObjectSet*			inRangePlayers	= player->getKnownPlayers();
			PlayerObjectSet::iterator	it				= inRangePlayers->begin();
			while(it != inRangePlayers->end())
			{
				PlayerObject* targetObject = (*it);
				gMessageLib->sendCreateStructure(structure,targetObject);
				targetObject->addKnownObjectSafe(structure);
				structure->addKnownObjectSafe(targetObject);
				++it;
			}

			gMessageLib->sendCreateStructure(structure,player);
			player->addKnownObjectSafe(structure);
			structure->addKnownObjectSafe(player);
			gMessageLib->sendDataTransform(structure);



		}


		it = objectList->erase(it);
		//it = objectList->begin();
	}

	return (false);
}


//=======================================================================================================================
//handles callback of altering the hopper list
//
void StructureManager::OpenStructureHopperList(uint64 structureId, uint64 playerId)
{
	// load our structures Admin data
	//
	HarvesterObject* harvester = dynamic_cast<HarvesterObject*>(gWorldManager->getObjectById(structureId));
	if(!harvester)
	{
		gLogger->logMsgF("OpenStructureHopperList : No harvester :(",MSG_HIGH);
		return;
	}

	StructureManagerAsyncContainer* asyncContainer;
	asyncContainer = new StructureManagerAsyncContainer(Structure_Query_Hopper_Data, 0);
	asyncContainer->mStructureId = structureId;
	asyncContainer->mPlayerId = playerId;

	mDatabase->ExecuteSqlAsync(harvester,asyncContainer,"SELECT c.firstname FROM structure_admin_data sad  INNER JOIN characters c ON (sad.PlayerID = c.ID)where sad.StructureID = %I64u AND sad.AdminType like 'HOPPER'",structureId);

}

//=======================================================================================================================
//handles callback of altering the admin list
//

void StructureManager::OpenStructureAdminList(uint64 structureId, uint64 playerId)
{
	// load our structures Admin data
	//

	StructureManagerAsyncContainer* asyncContainer;
	asyncContainer = new StructureManagerAsyncContainer(Structure_Query_Admin_Data, 0);
	asyncContainer->mStructureId = structureId;
	asyncContainer->mPlayerId = playerId;

	mDatabase->ExecuteSqlAsync(this,asyncContainer,"SELECT c.firstname FROM structure_admin_data sad  INNER JOIN characters c ON (sad.PlayerID = c.ID)where sad.StructureID = %I64u AND sad.AdminType like 'ADMIN'",structureId);


}

//=======================================================================================================================
//processes a succesfull PermissionList verification
//=======================================================================================================================
void StructureManager::processVerification(StructureAsyncCommand command, bool owner)
{

	PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(command.PlayerId));

	if(!player)
	{
		gLogger->logMsg("StructureManager::processVerification : No Player");
		return;
	}

	switch(command.Command)
	{

		case Structure_Command_PayMaintenance:
		{
			HarvesterObject* harvester = dynamic_cast<HarvesterObject*>(gWorldManager->getObjectById(command.StructureId));

			gUIManager->createPayMaintenanceTransferBox(harvester,player,harvester);

		}
		break;

		// callback for retrieving a variable amount of the selected resource
		case Structure_Command_RetrieveResource:
		{
			HarvesterObject* harvester = dynamic_cast<HarvesterObject*>(gWorldManager->getObjectById(command.StructureId));

			StructureManagerAsyncContainer* asyncContainer = new StructureManagerAsyncContainer(Structure_ResourceRetrieve,player->getClient());
			asyncContainer->mStructureId	= command.StructureId;
			asyncContainer->mPlayerId		= command.PlayerId;
			asyncContainer->command 		= command;
	
			//mDatabase->ExecuteSqlAsync(harvester,asyncContainer,"SELECT hr.resourceID, hr.quantity FROM harvester_resources hr WHERE hr.ID = '%"PRIu64"' ",harvester->getId());
			mDatabase->ExecuteSqlAsync(harvester,asyncContainer,"SELECT sf_DiscardResource(%"PRIu64",%"PRIu64",%u) ",harvester->getId(),command.ResourceId,command.Amount);

		}
		break;

		// callback for discarding a variable amount of the selected resource
		case Structure_Command_DiscardResource:
		{
			HarvesterObject* harvester = dynamic_cast<HarvesterObject*>(gWorldManager->getObjectById(command.StructureId));

			StructureManagerAsyncContainer* asyncContainer = new StructureManagerAsyncContainer(Structure_ResourceDiscard,player->getClient());
			asyncContainer->mStructureId	= command.StructureId;
			asyncContainer->mPlayerId		= command.PlayerId;
			asyncContainer->command 		= command;
			
			//mDatabase->ExecuteSqlAsync(harvester,asyncContainer,"SELECT hr.resourceID, hr.quantity FROM harvester_resources hr WHERE hr.ID = '%"PRIu64"' ",harvester->getId());
			mDatabase->ExecuteSqlAsync(harvester,asyncContainer,"SELECT sf_DiscardResource(%"PRIu64",%"PRIu64",%u) ",harvester->getId(),command.ResourceId,command.Amount);

		}
		break;

		case Structure_Command_GetResourceData:
		{
			HarvesterObject* harvester = dynamic_cast<HarvesterObject*>(gWorldManager->getObjectById(command.StructureId));

			StructureManagerAsyncContainer* asyncContainer = new StructureManagerAsyncContainer(Structure_GetResourceData,player->getClient());
			asyncContainer->mStructureId	= command.StructureId;
			asyncContainer->mPlayerId		= command.PlayerId;
			mDatabase->ExecuteSqlAsync(harvester,asyncContainer,"SELECT hr.resourceID, hr.quantity FROM harvester_resources hr WHERE hr.ID = '%"PRIu64"' ",harvester->getId());

		}
		break;

		case Structure_Command_DiscardHopper:
		{
			//send the db update
			StructureManagerAsyncContainer* asyncContainer;

			HarvesterObject* harvester = dynamic_cast<HarvesterObject*>(gWorldManager->getObjectById(command.StructureId));

			asyncContainer = new StructureManagerAsyncContainer(Structure_HopperDiscard, 0);
			asyncContainer->mStructureId	= command.StructureId;
			asyncContainer->mPlayerId		= command.PlayerId;
			mDatabase->ExecuteSqlAsync(harvester,asyncContainer,"select sf_DiscardHopper(%I64u)",command.StructureId);

		}
		break;

		case Structure_Command_RenameStructure:
		{
			if(owner)
			{
				PlayerStructure* structure = dynamic_cast<PlayerStructure*>(gWorldManager->getObjectById(command.StructureId));
				gUIManager->createRenameStructureBox(structure,player, structure);
			}
			else
				gMessageLib->sendSystemMessage(player,L"","player_structure","rename_must_be_owner ");

			
		}
		return;

		case Structure_Command_TransferStructure:
		{
			if(owner)
				gStructureManager->TransferStructureOwnership(command);
			else
				gMessageLib->sendSystemMessage(player,L"You cannot transfer ownership of this structure");
			
		}
		return;

		case Structure_Command_Destroy: 
		{		
			if(owner)
				gStructureManager->getDeleteStructureMaintenanceData(command.StructureId, command.PlayerId);
			else
				gMessageLib->sendSystemMessage(player,L"","player_structure","destroy_must_be_owner");
			
			
		}
		break;

		case Structure_Command_PermissionAdmin:
		{
			player->setStructurePermissionId(command.StructureId);
			OpenStructureAdminList(command.StructureId, command.PlayerId);

		}
		break;

		case Structure_Command_PermissionHopper:
		{
			player->setStructurePermissionId(command.StructureId);
			OpenStructureHopperList(command.StructureId, command.PlayerId);

		}
		break;

		case Structure_Command_AddPermission:
		{
		
			addNametoPermissionList(command.StructureId, command.PlayerId, command.PlayerStr, command.List);

		}
		break;

		case Structure_Command_RemovePermission:
		{

			removeNamefromPermissionList(command.StructureId, command.PlayerId, command.PlayerStr, command.List);
	
		}
	
	}
}


//=======================================================================================================================
//processes a structure transfer
//=======================================================================================================================
void StructureManager::TransferStructureOwnership(StructureAsyncCommand command)
{
	//at this point we have already made sure that the command is issued by the owner
	PlayerStructure* structure = dynamic_cast<PlayerStructure*>(gWorldManager->getObjectById(command.StructureId));

	//if we have no structure that way, see whether we have a structure were we just used the adminlist
	if(!structure)
	{
		gLogger->logMsgF("StructureManager::TransferStructureOwnership structure not found :(",MSG_HIGH);	
		return;
	}
	
	//step 1 make sure the recipient has enough free lots!

	//step 1a -> get the recipients ID

	//the recipient MUST be online !!!!! ???????

	StructureManagerAsyncContainer* asyncContainer;
	asyncContainer = new StructureManagerAsyncContainer(Structure_StructureTransfer_Lots_Recipient, 0);
	asyncContainer->mStructureId = command.StructureId;
	asyncContainer->mPlayerId = command.PlayerId;
	asyncContainer->mTargetId = command.RecipientId;

	mDatabase->ExecuteSqlAsync(this,asyncContainer,"SELECT sf_getLotCount(%I64u)",command.PlayerId);
}