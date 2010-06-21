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
#include "WorldConfig.h"
#include "nonPersistantObjectFactory.h"
#include "StructureManager.h"
#include "HarvesterObject.h"
#include "HouseObject.h"
#include "FactoryObject.h"
//#include "CellObject.h"
#include "Inventory.h"
#include "ObjectFactory.h"
#include "HouseObject.h"
#include "PlayerObject.h"
#include "PlayerStructure.h"
#include "WorldManager.h"
#include "MessageLib/MessageLib.h"

#include "LogManager/LogManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"




//=======================================================================================================================
//handles callbacks of db creation of items
//=======================================================================================================================

void StructureManager::handleObjectReady(Object* object,DispatchClient* client)
{
	PlayerStructure* structure = dynamic_cast<PlayerStructure*>(object);

	if(!structure)
	{
			gLogger->log(LogManager::DEBUG,"StructureManager::handleObjectReady: No structure");
	}

	if(gWorldManager->getWMState() == WMState_Running)
	{
		// set timer for deletion of building fence

		uint32 account = client->getAccountId();
		PlayerObject* player = gWorldManager->getPlayerByAccId(account);

		PlayerStructure* fence =  gNonPersistantObjectFactory->requestBuildingFenceObject(structure->mPosition.x,structure->mPosition.y,structure->mPosition.z, player);
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


//==================================================================================================
// handles the hoppers permission list
// it will be read in for the list to display when the player displays it to make changes
// please note that the list is purely db based and *not* kept in memory
// it will be cleared once the changes are made
//
void StructureManager::_HandleQueryHopperPermissionData(StructureManagerAsyncContainer* asynContainer,DatabaseResult* result)
{

	PlayerStructure* structure = dynamic_cast<PlayerStructure*>(gWorldManager->getObjectById(asynContainer->mStructureId));

	string playerName;
	DataBinding* binding = gWorldManager->getDatabase()->CreateDataBinding(1);
	binding->addField(DFT_bstring,0,64);

	uint64 count;
	count = result->getRowCount();

	for(uint64 i = 0;i < count;i++)
	{
		result->GetNextRow(binding,&playerName);

		structure->addStructureHopperListEntry(playerName);

	}

	structure->sendStructureHopperList(asynContainer->mPlayerId);

	gWorldManager->getDatabase()->DestroyDataBinding(binding);
}

//==================================================================================================
// handles the admin permission list
// it will be read in for the list to display when the player displays it to make changes
// please note that the list is purely db based and *not* kept in memory
// it will be cleared once the changes are made
//
void StructureManager::_HandleQueryAdminPermissionData(StructureManagerAsyncContainer* asynContainer,DatabaseResult* result)
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

		structure->addStructureAdminListEntry(playerName);

	}

	structure->sendStructureAdminList(asynContainer->mPlayerId);

	mDatabase->DestroyDataBinding(binding);
}

//==================================================================================================
// handles the admin permission list
// it will be read in for the list to display when the player displays it to make changes
// please note that the list is purely db based and *not* kept in memory
// it will be cleared once the changes are made
//
void StructureManager::_HandleQueryEntryPermissionData(StructureManagerAsyncContainer* asynContainer,DatabaseResult* result)
{

	PlayerStructure* structure = dynamic_cast<PlayerStructure*>(gWorldManager->getObjectById(asynContainer->mStructureId));

	string playerName;
	DataBinding* binding = mDatabase->CreateDataBinding(1);
	binding->addField(DFT_bstring,0,64);

	uint64 count;
	count = result->getRowCount();
	structure->resetStructureEntryList();
	for(uint64 i = 0;i < count;i++)
	{
		result->GetNextRow(binding,&playerName);

		structure->addStructureEntryListEntry(playerName);

	}

	structure->sendStructureEntryList(asynContainer->mPlayerId);

	mDatabase->DestroyDataBinding(binding);
	
}

//==================================================================================================
// handles the admin permission list
// it will be read in for the list to display when the player displays it to make changes
// please note that the list is purely db based and *not* kept in memory
// it will be cleared once the changes are made
//
void StructureManager::_HandleQueryBanPermissionData(StructureManagerAsyncContainer* asynContainer,DatabaseResult* result)
{
	PlayerStructure* structure = dynamic_cast<PlayerStructure*>(gWorldManager->getObjectById(asynContainer->mStructureId));

	string playerName;
	DataBinding* binding = mDatabase->CreateDataBinding(1);
	binding->addField(DFT_bstring,0,64);

	uint64 count;
	count = result->getRowCount();
	structure->resetStructureBanList();

	for(uint64 i = 0;i < count;i++)
	{
		result->GetNextRow(binding,&playerName);

		structure->addStructureBanListEntry(playerName);

	}

	structure->sendStructureBanList(asynContainer->mPlayerId);

	mDatabase->DestroyDataBinding(binding);
}

//==================================================================================================
// 
// updates the characters lots
//
void StructureManager::_HandleUpdateCharacterLots(StructureManagerAsyncContainer* asynContainer,DatabaseResult* result)
{
	PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(asynContainer->mPlayerId));

	uint8 lotCount;
	DataBinding* binding = mDatabase->CreateDataBinding(1);
	binding->addField(DFT_uint8,0,1);

	uint64 count;
	count = result->getRowCount();

	if (!count)
	{
		gLogger->log(LogManager::DEBUG,"StructureManager::add Permission no return value...");
		return;
	}
	result->GetNextRow(binding,&lotCount);

	if(player)
	{
		//update the lots
		uint32 maxLots = gWorldConfig->getConfiguration<uint32>("Player_Max_Lots",(uint32)10);

		maxLots -= static_cast<uint8>(lotCount);
		player->setLots((uint8)maxLots);
		
		//mmmmh a bit of a hack ... maybe it works ?
		gMessageLib->sendCharacterSheetResponse(player);

		
	}
	mDatabase->DestroyDataBinding(binding);
	
}

//==================================================================================================
// 
// this is the callback from updating the structures deed in case the structure redeeded
// we use it to create the deed in the inventory and get all the other necessary stuff underway 
// to delete the playerstructure

void StructureManager::_HandleStructureRedeedCallBack(StructureManagerAsyncContainer* asynContainer,DatabaseResult* result)
{
	PlayerStructure* structure = dynamic_cast<PlayerStructure*>(gWorldManager->getObjectById(asynContainer->mStructureId));

	//if its a playerstructure boot all players and pets inside
	HouseObject* house = dynamic_cast<HouseObject*>(structure);
	if(house)
	{
		house->prepareDestruction();
	}
	

	//destroy the structure here so the sf can still access the relevant data
	gObjectFactory->deleteObjectFromDB(structure);
	gMessageLib->sendDestroyObject_InRangeofObject(structure);

	gWorldManager->destroyObject(structure);
	

	PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(asynContainer->mPlayerId));

	uint64 deedId;
	DataBinding* binding = mDatabase->CreateDataBinding(1);
	binding->addField(DFT_uint64,0,8);

	uint64 count;
	count = result->getRowCount();

	if (!count)
	{
		gLogger->log(LogManager::DEBUG,"StructureManager::create deed no result...");
		mDatabase->DestroyDataBinding(binding);
		return;
	}
	result->GetNextRow(binding,&deedId);

	//return value of 0 means something wasnt found
	if(!deedId)
	{
		gLogger->log(LogManager::DEBUG,"StructureManager::create deed no valid return value...");
		mDatabase->DestroyDataBinding(binding);
		return;
	}
	//returnvalue of 1 means that there wasnt enough money on the deed
	if(deedId == 1)
	{
		gLogger->log(LogManager::DEBUG,"StructureManager::create deed with not enough maintenance...");
		gMessageLib->sendSysMsg(player, "player_structure","structure_destroyed ");	
		mDatabase->DestroyDataBinding(binding);
		return;
	}

	if(player)
	{
		//load the deed into the inventory
		Inventory* inventory = dynamic_cast<Inventory*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
		if(inventory)
		{
			//15 is itemfamily for deeds
			gObjectFactory->createIteminInventory(inventory,deedId,TanGroup_Item);
		}
	}

	UpdateCharacterLots(asynContainer->mPlayerId);

	mDatabase->DestroyDataBinding(binding);
}

//==================================================================================================
// 
// handles the callback of the destruction of structures when condition wears of.
// 

void StructureManager::_HandleStructureDestruction(StructureManagerAsyncContainer* asynContainer,DatabaseResult* result)
{
	struct structData
	{
		uint64 id;
		uint32 condition;
	};

	structData sd;

	DataBinding* binding = mDatabase->CreateDataBinding(2);
	binding->addField(DFT_uint64,offsetof(structData,id),8,0);
	binding->addField(DFT_uint32,offsetof(structData,condition),4,1);

	uint64 count;
	count = result->getRowCount();

	for(uint64 i = 0;i < count;i++)
	{
		result->GetNextRow(binding,&sd);

		PlayerStructure* structure = dynamic_cast<PlayerStructure*>(gWorldManager->getObjectById(sd.id));
		if(structure)
		{
			gLogger->log(LogManager::DEBUG,"StructureManager::delete structure due to zero condition %I64u",structure->getId());
			//delete the deed in the db
			//the parent is the structure and the item family is 15
			int8 sql[100];
			sprintf(sql,"DELETE FROM items WHERE parent_id = %"PRIu64" AND item_family = 15",structure->getId());
			mDatabase->ExecuteSqlAsync(NULL,NULL,sql);

			//delete harvester db side with all power and all resources
			gObjectFactory->deleteObjectFromDB(structure);
			UpdateCharacterLots(structure->getOwner());

			//delete it in the world
			gMessageLib->sendDestroyObject_InRangeofObject(structure);
			gWorldManager->destroyObject(structure);


								
		}
	}

	mDatabase->DestroyDataBinding(binding);
}

//==================================================================================================
// 
// handles the callback of the destruction of structures when condition wears of.
// 

void StructureManager::_HandleGetInactiveHarvesters(StructureManagerAsyncContainer* asynContainer,DatabaseResult* result)
{

	struct structData
	{
		uint64 id;
		uint32 condition;
	};

	structData sd;

	DataBinding* binding = mDatabase->CreateDataBinding(2);
	binding->addField(DFT_uint64,offsetof(structData,id),8,0);
	binding->addField(DFT_uint32,offsetof(structData,condition),4,1);

	uint64 count;
	count = result->getRowCount();

	for(uint64 i = 0;i < count;i++)
	{
		result->GetNextRow(binding,&sd);

		HarvesterObject* harvester = dynamic_cast<HarvesterObject*>(gWorldManager->getObjectById(sd.id));
		if(harvester)
		{
			//if the harvesters status is changed we need to alter it
			if(harvester->getActive())
			{
				harvester->setActive(false);
				harvester->setDamage(sd.condition);
				
				gMessageLib->sendHarvesterActive(harvester);

			}
			//Now update the condition
			gMessageLib->sendHarvesterCurrentConditionUpdate(harvester);
		}

	}

	mDatabase->DestroyDataBinding(binding);
}

//==================================================================================================
// 
// tests the amount of lots for the recipient of a structure during a structure transfer
// 

void StructureManager::_HandleStructureTransferLotsRecipient(StructureManagerAsyncContainer* asynContainer,DatabaseResult* result)
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
		gLogger->log(LogManager::DEBUG,"StructureManager::Transfer Structure Admin List Callback couldnt get recipients lots");
		mDatabase->DestroyDataBinding(binding);			
		return;
	}

	
	//thats lots in use
	result->GetNextRow(binding,&lots);

	uint8 requiredLots = structure->getLotCount();

	uint32 freelots = gWorldConfig->getConfiguration<uint32>("Player_Max_Lots",(uint32)10) - lots;
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
		//say something
	}
	
	mDatabase->DestroyDataBinding(binding);												   	
}

//==================================================================================================
// 
// basic structuretype information for the structuremanager	loaded on startup
// 

void StructureManager::_HandleQueryLoadDeedData(StructureManagerAsyncContainer* asynContainer,DatabaseResult* result)
{
	StructureDeedLink* deedLink;

	DataBinding* binding = mDatabase->CreateDataBinding(12);
	binding->addField(DFT_uint32,offsetof(StructureDeedLink,structure_type),4,0);
	binding->addField(DFT_uint32,offsetof(StructureDeedLink,item_type),4,1);
	binding->addField(DFT_uint32,offsetof(StructureDeedLink,skill_Requirement),4,2);
	binding->addField(DFT_bstring,offsetof(StructureDeedLink,structureObjectString),128,3);
	binding->addField(DFT_uint8,offsetof(StructureDeedLink,requiredLots),1,4);
	binding->addField(DFT_bstring,offsetof(StructureDeedLink,stf_name),64,5);
	binding->addField(DFT_bstring,offsetof(StructureDeedLink,stf_file),64,6);
	binding->addField(DFT_float,offsetof(StructureDeedLink,healing_modifier),4,7);
	binding->addField(DFT_uint32,offsetof(StructureDeedLink,repair_cost),4,8);
	binding->addField(DFT_uint32,offsetof(StructureDeedLink,length), 4, 9);
	binding->addField(DFT_uint32,offsetof(StructureDeedLink,width), 4, 10);
	binding->addField(DFT_uint64,offsetof(StructureDeedLink,placementMask), 8, 11);

	uint64 count;
	count = result->getRowCount();

	for(uint64 i = 0;i < count;i++)
	{
		deedLink	= new(StructureDeedLink);
		result->GetNextRow(binding,deedLink);
		mDeedLinkList.push_back(deedLink);
	}

	if(result->getRowCount())
		gLogger->log(LogManager::NOTICE,"Loaded structures.");

	mDatabase->DestroyDataBinding(binding);
}

//==================================================================================================
// 
// a player has been removed from the permission list of a structure
// 

void StructureManager::_HandleRemovePermission(StructureManagerAsyncContainer* asynContainer,DatabaseResult* result)
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
		gLogger->log(LogManager::DEBUG,"StructureManager::Structure_Query_Remove_Permission no return value...");
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
		name.convert(BSTRType_Unicode16);
		gMessageLib->sendSystemMessage(player,L"","player_structure","player_removed","","",name.getUnicode16());

		if(HouseObject*	house = dynamic_cast<HouseObject*>(gWorldManager->getObjectById(asynContainer->mStructureId)))
		{
			updateKownPlayerPermissions(house);
			StructureManagerAsyncContainer* asContainer = new StructureManagerAsyncContainer(Structure_Query_UpdateAdminPermission,NULL);
			asContainer->mStructureId = asynContainer->mStructureId;

			gWorldManager->getDatabase()->ExecuteSqlAsync(this,asContainer,"SELECT PlayerID FROM structure_admin_data WHERE StructureID = %"PRIu64" AND AdminType like 'ADMIN';",asContainer->mStructureId);
		}
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

//==================================================================================================
// 
// after adding a name to the admin list we read it in again (as ID though)
// so dropping / picking up items in cells works

void StructureManager::_HandleUpdateAdminPermission(StructureManagerAsyncContainer* asynContainer,DatabaseResult* result)
{
	HouseObject*	house = dynamic_cast<HouseObject*>(gWorldManager->getObjectById(asynContainer->mStructureId));

	struct adminStruct
	{
		uint64 playerID;
	};

	adminStruct adminData;

	DataBinding*	adminBinding = mDatabase->CreateDataBinding(1);
	adminBinding->addField(DFT_uint64,offsetof(adminStruct,playerID),8,0);

	uint64 count = result->getRowCount();
	house->resetHousingAdminList();

	for(uint32 j = 0;j < count;j++)
	{
		result->GetNextRow(adminBinding,&adminData);
		house->addHousingAdminEntry(adminData.playerID);

		//now that we added it to the list we need to update the players delta
	}

	mDatabase->DestroyDataBinding(adminBinding);

}

//==================================================================================================
// 
// A name has been added to (one of the) permission / ban lists of a structure
// if it was a playerhouse make sure to update our in memory admin list
// so pickup/drop works

void StructureManager::_HandleAddPermission(StructureManagerAsyncContainer* asynContainer,DatabaseResult* result)
{
	PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(asynContainer->mPlayerId));

	uint32 returnValue;
	DataBinding* binding = mDatabase->CreateDataBinding(1);
	binding->addField(DFT_uint32,0,4);

	uint64 count;
	count = result->getRowCount();

	if (!count)
	{
		gLogger->log(LogManager::DEBUG,"StructureManager::add Permission no return value...");
	}
	result->GetNextRow(binding,&returnValue);
	// 0 is sucess
	// 1 name doesnt exist
	// 2 name already on list
	// 3 list is full (more than 36 entries)
	// 4 owner tried to add him/herselve to the ban list

	if(returnValue == 0)
	{
		string name;
		name = asynContainer->name;
		//gMessageLib->sendSystemMessage(player,L"","player_structure","player_added","",name.getAnsi());
		name.convert(BSTRType_Unicode16);
		gMessageLib->sendSystemMessage(player,L"","player_structure","player_added","","",name.getUnicode16());
		
		//now read in the (admin) list again if its a playerHouse
		//we need to keep them in memory to handle drop/pickup in cells
		if(HouseObject*	house = dynamic_cast<HouseObject*>(gWorldManager->getObjectById(asynContainer->mStructureId)))
		{
			updateKownPlayerPermissions(house);
			StructureManagerAsyncContainer* asContainer = new StructureManagerAsyncContainer(Structure_Query_UpdateAdminPermission,NULL);
			asContainer->mStructureId = asynContainer->mStructureId;

			gWorldManager->getDatabase()->ExecuteSqlAsync(this,asContainer,"SELECT PlayerID FROM structure_admin_data WHERE StructureID = %"PRIu64" AND AdminType like 'ADMIN';",asContainer->mStructureId);
		}
	}

	//no valid name
	if(returnValue == 1)
	{
		string name;
		name = asynContainer->name;
		name.convert(BSTRType_ANSI);
		gLogger->log(LogManager::DEBUG,"StructurManager add %s failed ",name.getAnsi());
		name.convert(BSTRType_Unicode16);
		
		gMessageLib->sendSystemMessage(player,L"","player_structure","modify_list_invalid_player","","",name.getUnicode16());
	}

	//name already on the list
	if(returnValue == 2)
	{
		string name;
		name = asynContainer->name;
		name.convert(BSTRType_ANSI);
		name << " is already on the list";
		name.convert(BSTRType_Unicode16);
		gMessageLib->sendSystemMessage(player,name.getUnicode16());
	}

	//no more than 36 entries on the list
	if(returnValue == 3)
	{
		gMessageLib->sendSystemMessage(player,L"","player_structure","too_many_entries");
	}

	//dont ban the owner
	if(returnValue == 4)
	{
		gMessageLib->sendSystemMessage(player,L"You cannot Ban the structure's Owner");
	}

	mDatabase->DestroyDataBinding(binding);

}

//==================================================================================================
// 
// this loads basic (nonpersistant) structure item information for structure types
// used with camps -	loaded on startup

void StructureManager::_HandleNonPersistantLoadStructureItem(StructureManagerAsyncContainer* asynContainer,DatabaseResult* result)
{
	StructureItemTemplate* itemTemplate;

	DataBinding* binding = mDatabase->CreateDataBinding(14);
	binding->addField(DFT_uint32,offsetof(StructureItemTemplate,structure_id),4,0);
	binding->addField(DFT_uint32,offsetof(StructureItemTemplate,CellNr),4,1);
	binding->addField(DFT_uint32,offsetof(StructureItemTemplate,item_type),4,2);
	binding->addField(DFT_float,offsetof(StructureItemTemplate,mPosition.x),4,3);
	binding->addField(DFT_float,offsetof(StructureItemTemplate,mPosition.y),4,4);
	binding->addField(DFT_float,offsetof(StructureItemTemplate,mPosition.z),4,5);
	binding->addField(DFT_float,offsetof(StructureItemTemplate,mDirection.x),4,6);
	binding->addField(DFT_float,offsetof(StructureItemTemplate,mDirection.y),4,7);
	binding->addField(DFT_float,offsetof(StructureItemTemplate,mDirection.z),4,8);
	binding->addField(DFT_float,offsetof(StructureItemTemplate,mDirection.w),4,9);
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
		gLogger->log(LogManager::NOTICE,"Loaded structure items.");

	mDatabase->DestroyDataBinding(binding);

}

//==================================================================================================
// 
// asynchronously checks whether the player is on the permissionlist list provided in the query
// after succesful verifying the entry the processVerification procedure is called
// to execute the provided command
// a special case is checking the ban / entry list when we send a structure create
// we then update the cell permission per delta depending on check outcome

void StructureManager::_HandleCheckPermission(StructureManagerAsyncContainer* asynContainer,DatabaseResult* result)
{
	PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(asynContainer->mPlayerId));

	uint32 returnValue;
	DataBinding* binding = mDatabase->CreateDataBinding(1);
	binding->addField(DFT_uint32,0,4);

	uint64 count;
	count = result->getRowCount();

	if (!count)
	{
		gLogger->log(LogManager::DEBUG,"StructureManager::check Permission no return value...");
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
		
		gLogger->log(LogManager::DEBUG,"StructurManager check Permission name %s doesnt exist ",name.getAnsi());
		
	}

	if(returnValue == 2)
	{
		if(asynContainer->command.Command == Structure_Command_CellEnter )
		{
			//the structure is private - we are not on the access list :(
			if(BuildingObject* building = dynamic_cast<BuildingObject*>(gWorldManager->getObjectById(asynContainer->command.StructureId)))
				building->updateCellPermissions(player,false);
		}
		else
		if(asynContainer->command.Command == Structure_Command_CellEnterDenial)
		{
			//in case the structure was private before we are now be allowed to enter
			//as we are not banned
			if(BuildingObject* building = dynamic_cast<BuildingObject*>(gWorldManager->getObjectById(asynContainer->command.StructureId)))
				building->updateCellPermissions(player,true);
		}
		else
			gMessageLib->sendSystemMessage(player,L"", "player_structure","not_admin");
	}

	mDatabase->DestroyDataBinding(binding);
}

//==================================================================================================
// 
// asynchronously updates a provided attribute and calls a specified handler
// based on the command option

void StructureManager::_HandleUpdateAttributes(StructureManagerAsyncContainer* asynContainer,DatabaseResult* result)
{
	BString value;
	Type_QueryContainer container;

	DataBinding*	binding = mDatabase->CreateDataBinding(2);
	binding->addField(DFT_bstring,offsetof(Type_QueryContainer,mString),128,0);
	binding->addField(DFT_bstring,offsetof(Type_QueryContainer,mValue),128,1);
	
	PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(asynContainer->mPlayerId));
	
	PlayerStructure* structure = dynamic_cast<PlayerStructure*>(gWorldManager->getObjectById(asynContainer->mStructureId));

	uint64 count;
	count = result->getRowCount();

	for(uint64 i = 0;i < count;i++)
	{
		result->GetNextRow(binding,&container);

		if(strcmp(container.mString.getAnsi(),"schematicCustom") == 0)
		{	
			FactoryObject* factory = dynamic_cast<FactoryObject*>(structure);
			if(factory)
				factory->setSchematicCustomName(container.mValue);
			gLogger->log(LogManager::DEBUG,"StructureManager::GetCustomName : %s", container.mValue.getAnsi());
		}

		if(strcmp(container.mString.getAnsi(),"schematicName") == 0)
		{	
			FactoryObject* factory = dynamic_cast<FactoryObject*>(structure);
			if(factory)
				factory->setSchematicName(container.mValue);
			gLogger->log(LogManager::DEBUG,"StructureManager::GetName : %s", container.mValue.getAnsi());
		}

		if(strcmp(container.mString.getAnsi(),"schematicFile") == 0)
		{	
			FactoryObject* factory = dynamic_cast<FactoryObject*>(structure);
			if(factory)
				factory->setSchematicFile(container.mValue);
			gLogger->log(LogManager::DEBUG,"StructureManager::GetNameFile : %s", container.mValue.getAnsi());
		}

		if(strcmp(container.mString.getAnsi(),"maintenance") == 0)
		{

			if(structure->hasAttribute("examine_maintenance"))
			{
				structure->setAttribute("examine_maintenance",container.mValue.getAnsi());
			}
			else
			{
				structure->addAttribute("examine_maintenance",container.mValue.getAnsi());
			}
		}

		if(strcmp(container.mString.getAnsi(),"power") == 0)
		{

			if(structure->hasAttribute("examine_power"))
			{
				structure->setAttribute("examine_power",container.mValue.getAnsi());
			}
			else
			{
				structure->addAttribute("examine_power",container.mValue.getAnsi());
			}

		}

		if(strcmp(container.mString.getAnsi(),"condition") == 0)
		{

			
			container.mValue.setLength(4);
			structure->setDamage(boost::lexical_cast<uint32>(container.mValue.getAnsi()));
			gLogger->log(LogManager::DEBUG,"StructureManager::GetConditionData : %u", structure->getDamage());
		}

		if(strcmp(container.mString.getAnsi(),"name") == 0)
		{

			structure->setOwnersName(container.mValue);
			
		}
	}

	switch(asynContainer->command.Command)
	{
		case Structure_Command_AccessSchem:
		{
			FactoryObject* factory = dynamic_cast<FactoryObject*>(structure);
			if(factory)
				createNewFactorySchematicBox(player, factory);
		}
		break;

		case Structure_Command_Destroy:
		{
			structure->deleteStructureDBDataRead(player->getId());
		}
		break;

		case Structure_Command_DepositPower:
		{
			createPowerTransferBox(player,structure);
		}
		break;

		case Structure_Command_PayMaintenance:
		{
			createPayMaintenanceTransferBox(player,structure);
		}
		break;

		case Structure_Command_ViewStatus:
		{
			createNewStructureStatusBox(player, structure);
		}
		break;

		default:
			break;
	}		
	
	mDatabase->DestroyDataBinding(binding);												   	


}

void StructureManager::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
	StructureManagerAsyncContainer* asynContainer = (StructureManagerAsyncContainer*)ref;

	StructureManagerCommandMap::iterator it = gStructureManagerCmdMap.find(asynContainer->mQueryType);

	if(it != gStructureManagerCmdMap.end())
	{
		(this->*((*it).second))(asynContainer, result);
	}

	SAFE_DELETE(asynContainer);
}
