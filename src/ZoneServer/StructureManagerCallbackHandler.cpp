 /*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/
#include "WorldConfig.h"
#include "nonPersistantObjectFactory.h"
#include "StructureManager.h"
#include "HarvesterObject.h"
#include "FactoryObject.h"
#include "Inventory.h"
#include "ObjectFactory.h"
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



void StructureManager::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
	StructureManagerAsyncContainer* asynContainer = (StructureManagerAsyncContainer*)ref;

	switch(asynContainer->mQueryType)
	{

		case Structure_Query_Hopper_Data:
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

				structure->addStructureHopper(playerName);

			}

			structure->sendStructureHopperList(asynContainer->mPlayerId);

			gWorldManager->getDatabase()->DestroyDataBinding(binding);
		}
		break;

		//asynchronously updates the playerlots for a character
		case Structure_UpdateCharacterLots:
		{
			PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(asynContainer->mPlayerId));

			uint8 lotCount;
			DataBinding* binding = mDatabase->CreateDataBinding(1);
			binding->addField(DFT_uint8,0,1);

			uint64 count;
			count = result->getRowCount();

			if (!count)
			{
				gLogger->logMsgLoadFailure("StructureManager::add Permission no return value...",MSG_NORMAL);
				return;
			}
			result->GetNextRow(binding,&lotCount);

			if(player)
			{
				//update the lots
				uint8 maxLots = gWorldConfig->getConfiguration("Player_Max_Lots",(uint8)10);

				maxLots -= static_cast<uint8>(lotCount);
				player->setLots((uint8)maxLots);

				
			}
			mDatabase->DestroyDataBinding(binding);
		}
		break;

		//this is the callback from updating the structures deed in case the structure redeeded
		//we use it to create the deed in the inventory
		case Structure_UpdateStructureDeed:
		{
			PlayerStructure* structure = dynamic_cast<PlayerStructure*>(gWorldManager->getObjectById(asynContainer->mStructureId));

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
				gLogger->logMsgLoadFailure("StructureManager::create deed no returned values...",MSG_NORMAL);
				mDatabase->DestroyDataBinding(binding);
				return;
			}
			result->GetNextRow(binding,&deedId);

			//return value of 0 means something wasnt found
			if(!deedId)
			{
				gLogger->logMsgLoadFailure("StructureManager::create deed no return value...",MSG_NORMAL);
				mDatabase->DestroyDataBinding(binding);
				return;
			}
			//returnvalue of 1 means that there wasnt enough money on the deed
			if(deedId == 1)
			{
				gLogger->logMsgLoadFailure("StructureManager::create deed with not enough maintenance...",MSG_NORMAL);
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
		break;

		//this is a list of the structures that has zero condition and needs to get destroyed
		case Structure_GetDestructionStructures:
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
					gLogger->logMsgF("StructureManager::delete structure due to zero condition %I64u",MSG_NORMAL,structure->getId());
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
		break;

		case Structure_GetInactiveHarvesters:
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
		break;

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

		//queries all entries of a structures admin list
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

		// basic structure information for the structuremanager
		case Structure_Query_LoadDeedData:
		{
			StructureDeedLink* deedLink;

			DataBinding* binding = mDatabase->CreateDataBinding(9);
			binding->addField(DFT_uint32,offsetof(StructureDeedLink,structure_type),4,0);
			binding->addField(DFT_uint32,offsetof(StructureDeedLink,item_type),4,1);
			binding->addField(DFT_uint32,offsetof(StructureDeedLink,skill_Requirement),4,2);
			binding->addField(DFT_bstring,offsetof(StructureDeedLink,structureObjectString),128,3);
			binding->addField(DFT_uint8,offsetof(StructureDeedLink,requiredLots),1,4);
			binding->addField(DFT_bstring,offsetof(StructureDeedLink,stf_name),64,5);
			binding->addField(DFT_bstring,offsetof(StructureDeedLink,stf_file),64,6);
			binding->addField(DFT_float,offsetof(StructureDeedLink,healing_modifier),4,7);
			binding->addField(DFT_uint32,offsetof(StructureDeedLink,repair_cost),4,8);

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

		// a player has been removed from the permission list of a structure
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
				name.convert(BSTRType_Unicode16);
				gMessageLib->sendSystemMessage(player,L"","player_structure","player_added","","",name.getUnicode16());
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


		// a player has been added to the permission list of a structure
		case Structure_Query_Add_Permission:
		{

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
				//gMessageLib->sendSystemMessage(player,L"","player_structure","player_added","",name.getAnsi());
				name.convert(BSTRType_Unicode16);
				gMessageLib->sendSystemMessage(player,L"","player_structure","player_added","","",name.getUnicode16());
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

		//this loads basic structure information for structure types
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

		// =========================================
		// loads attribute data and calls a taskspecific handler
		// as set through the command option
		case Structure_UpdateAttributes:
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
					gLogger->logMsgF("StructureManager::GetCustomName : %s",MSG_HIGH, container.mValue.getAnsi());
				}

				if(strcmp(container.mString.getAnsi(),"schematicName") == 0)
				{	
					FactoryObject* factory = dynamic_cast<FactoryObject*>(structure);
					if(factory)
						factory->setSchematicName(container.mValue);
					gLogger->logMsgF("StructureManager::GetName : %s",MSG_HIGH, container.mValue.getAnsi());
				}

				if(strcmp(container.mString.getAnsi(),"schematicFile") == 0)
				{	
					FactoryObject* factory = dynamic_cast<FactoryObject*>(structure);
					if(factory)
						factory->setSchematicFile(container.mValue);
					gLogger->logMsgF("StructureManager::GetNameFile : %s",MSG_HIGH, container.mValue.getAnsi());
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
					gLogger->logMsgF("StructureManager::GetConditionData : %u",MSG_HIGH, structure->getDamage());
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
		break;


		default:break;

	}

	SAFE_DELETE(asynContainer);
}
