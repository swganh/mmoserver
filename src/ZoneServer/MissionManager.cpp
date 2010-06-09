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

#include "MissionManager.h"

#include "Bank.h"
#include "Buff.h"
#include "CreatureObject.h"
#include "Datapad.h"
#include "GroupManager.h"
#include "MissionBag.h"
#include "MissionObject.h"
#include "NPCObject.h"
#include "PlayerObject.h"
#include "ResourceCollectionManager.h"
#include "ResourceManager.h"
#include "ResourceType.h"
#include "Terminal.h"
#include "TreasuryManager.h"
#include "UIManager.h"
#include "WaypointObject.h"
#include "WorldManager.h"
#include "ZoneTree.h"

#include "LogManager/LogManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/DatabaseResult.h"
#include "MessageLib/MessageLib.h"

#include "ConfigManager/ConfigManager.h"

#include "Utils/rand.h"
#include <cstdio>
#include <glm/gtx/random.hpp>

//======================================================================================================================

bool MissionManager::mInsFlag = false;
MissionManager* MissionManager::mSingleton = NULL;

//======================================================================================================================

//TEMPORARY
//This is here only for testing purposes

    //Generic
	struct missionData
	{
		char* mSTF;
		int num;
	};

	struct missionTargets
	{
		unsigned int crc;
		char* name;
	};

	char md[250];
	char mo[250];
	char mt[50];

	const char* creators[9] =
	{
	 "The Office of the Governor of Naboo",
	 "The Royal Security Forces",
	 "Meanmon13 Inc.",
	 "SWG: A New Hope",
	 "The Royal Advisory Council",
	 "The Portmaster of Kwilaan Starport",
	 "Thanus March, Freelance Spacer",
	 "Hipan Voge, Textile Merchant",
	 "Kanlee Fentetta, Prominent Naboo Citizen",
	};

//END TEMPORARY


MissionManager::MissionManager(Database* database, uint32 zone) :
mDatabase(database)
{
	bool mDebug;
	try
	{
		mDebug = gConfig->read<bool>("LoadReduceDebug");
	}
	catch (...)
	{
		mDebug = false;
	}
	if(mDebug)
		return;
	MissionManagerAsyncContainer* asyncContainer;
	asyncContainer = new MissionManagerAsyncContainer(MissionQuery_Load_Types, 0);
	mDatabase->ExecuteSqlAsync(this,asyncContainer,"SELECT id, type, content, name FROM swganh.mission_types");

	asyncContainer = new MissionManagerAsyncContainer(MissionQuery_Load_Names, 0);
	mDatabase->ExecuteSqlAsync(this,asyncContainer,"SELECT name FROM swganh.mission_names WHERE planet = %u", zone);

}

//======================================================================================================================

MissionManager* MissionManager::Init(Database* database, uint32 zone)
{
    if(mInsFlag == false)
    {
        mSingleton = new MissionManager(database, zone);
        mInsFlag = true;
        return mSingleton;
    }
    else
        return mSingleton;
}

//======================================================================================================================

MissionManager::~MissionManager()
{
    mInsFlag = false;
    delete(mSingleton);
}

//======================================================================================================================
//static bool printed = false;
static bool failed = false;
void MissionManager::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
    MissionManagerAsyncContainer* asynContainer = (MissionManagerAsyncContainer*)ref;

    switch(asynContainer->mQueryType)
    {

		case MissionQuery_Load_Names_File:
		{
			// m_t.mission_type, m_t.mission_name, m_t.mission_text FROM swganh.mission_text m_t INNER JOIN swganh.mission_types mty ON mty.id = m_t.mission_type WHERE mission_name like 'm%o' AND (mty.type NOT like 'mission_npc_%')", zone);

			DataBinding* binding = mDatabase->CreateDataBinding(3);
			binding->addField(DFT_uint32,offsetof(Mission_Names,type),4,0);
			binding->addField(DFT_bstring,offsetof(Mission_Names,mission_name),64,1);
			binding->addField(DFT_bstring,offsetof(Mission_Names,name),64,2);


			uint64 count;
			count = result->getRowCount();
			Mission_Names* names;

			MissionMap::iterator it;
			for(uint64 i = 0;i < count;i++)
			{
				names = new(Mission_Names);
				result->GetNextRow(binding,names);

				if(names->name.getLength() > 2)
				{
					names->id = names->mission_name.getCrc();

					it = mMissionMap.find(names->type);
					if(it != mMissionMap.end())
					{
						(*it).second->names.insert(std::make_pair(names->id,names));
					}
					else
					{
						failed=true;
					}
				}
				//mNameMap.insert(std::make_pair(i,names));
			}

			// not all missions have associated names ...
			if(result->getRowCount())
				gLogger->log(LogManager::NOTICE,"Loaded mission STFs.");

		}
		break;

		case MissionQuery_Load_Names:
		{
			DataBinding* binding = mDatabase->CreateDataBinding(1);
			binding->addField(DFT_bstring,offsetof(Mission_Names,name),64,0);


			uint64 count;
			count = result->getRowCount();
			Mission_Names* names;

			for(uint64 i = 0;i < count;i++)
			{
				names = new(Mission_Names);
				result->GetNextRow(binding,names);
				names->id = static_cast<uint32>(i);
				mNameMap.insert(std::make_pair(static_cast<uint32>(i),names));
			}

			if(result->getRowCount())
				gLogger->log(LogManager::NOTICE,"Loaded mission names.");


		}
		break;

		case MissionQuery_Load_Types:
		{
			//these is the list of the stf files together with the amount of entries they have

			DataBinding* binding = mDatabase->CreateDataBinding(4);
			binding->addField(DFT_uint32,offsetof(Mission_Types,id),4,0);
			binding->addField(DFT_bstring,offsetof(Mission_Types,stf),128,1);
			binding->addField(DFT_uint32,offsetof(Mission_Types,content),4,2);
			binding->addField(DFT_uint32,offsetof(Mission_Types,nameprovided),4,3);

			uint64 count;
			count = result->getRowCount();
			Mission_Types* mission;

			for(uint64 i = 0;i < count;i++)
			{
				mission = new(Mission_Types);
				result->GetNextRow(binding,mission);
				mMissionMap.insert(std::make_pair(static_cast<uint32>(mission->id),mission));
			}

			MissionManagerAsyncContainer*  asyncContainer = new MissionManagerAsyncContainer(MissionQuery_Load_Terminal_Type, 0);
			mDatabase->ExecuteSqlAsync(this,asyncContainer,"SELECT mtmt.id, mtmt.terminal, mtmt.mission_type,mt.content, mt.name FROM swganh.mission_terminal_mission_types mtmt INNER JOIN swganh.mission_types mt ON (mt.id = mtmt.mission_type)");

			asyncContainer = new MissionManagerAsyncContainer(MissionQuery_Load_Names_File, 0);
			mDatabase->ExecuteSqlAsyncNoArguments(this,asyncContainer,"SELECT m_t.mission_type, m_t.mission_name, m_t.mission_text FROM swganh.mission_text m_t INNER JOIN swganh.mission_types mty ON mty.id = m_t.mission_type WHERE mission_name like 'm%o' AND (mty.type NOT like 'mission_npc_%')");

			if(result->getRowCount())
				gLogger->log(LogManager::NOTICE,"Loaded mission types.");

		}
		break;

		case MissionQuery_Load_Terminal_Type:
		{
			//this links the stffiles for missions to the respective missionterminals
			DataBinding* binding = mDatabase->CreateDataBinding(5);
			binding->addField(DFT_uint32,offsetof(Terminal_Mission_Link,id),4,0);
			binding->addField(DFT_uint64,offsetof(Terminal_Mission_Link,terminal),8,1);
			binding->addField(DFT_uint32,offsetof(Terminal_Mission_Link,mission_type),4,2);
			binding->addField(DFT_uint32,offsetof(Terminal_Mission_Link,content),4,3);
			binding->addField(DFT_uint32,offsetof(Terminal_Mission_Link,name),4,4);

			uint64 count;
			count = result->getRowCount();
			Terminal_Mission_Link* terminalMissionLink;

			for(uint64 i = 0;i < count;i++)
			{
				terminalMissionLink = new(Terminal_Mission_Link);
				result->GetNextRow(binding,terminalMissionLink);

				MissionMap::iterator it = mMissionMap.find(terminalMissionLink->mission_type);

				if(it != mMissionMap.end())
					terminalMissionLink->missiontype = (*it).second;
				else
				{
					continue;
				}

				if(strstr(terminalMissionLink->missiontype->stf.getAnsi(),"creature")!= NULL)
				{
					terminalMissionLink->target= MissionTarget_Creature;
				}
				else
					terminalMissionLink->target= MissionTarget_NPC;

				if(strstr(terminalMissionLink->missiontype->stf.getAnsi(),"from_npc")!= NULL)
				{
					terminalMissionLink->giver= MissionGiver_NPC;
				}
				else
					terminalMissionLink->giver = MissionGiver_NameProvided;



				if(strstr(terminalMissionLink->missiontype->stf.getAnsi(),"destroy")!= NULL)
				{
					terminalMissionLink->type = MissionTypeDestroy;
				}
				else
				if(strstr(terminalMissionLink->missiontype->stf.getAnsi(),"bounty")!= NULL)
				{
					terminalMissionLink->type = MissionTypeBounty;
				}
				else
				if(strstr(terminalMissionLink->missiontype->stf.getAnsi(),"deliver")!= NULL)
				{
					terminalMissionLink->type = MissionTypeDeliver;
				}
				else
				if(strstr(terminalMissionLink->missiontype->stf.getAnsi(),"dancer")!= NULL)
				{
					terminalMissionLink->type = MissionTypeDancer;
				}
				else
				if(strstr(terminalMissionLink->missiontype->stf.getAnsi(),"escorttocreator")!= NULL)
				{
					terminalMissionLink->type = MissionTypeEscorttoCreator;
				}
				else
				if(strstr(terminalMissionLink->missiontype->stf.getAnsi(),"escort")!= NULL)
				{
					terminalMissionLink->type = MissionTypeEscort;
				}
				else
				if(strstr(terminalMissionLink->missiontype->stf.getAnsi(),"musician")!= NULL)
				{
					terminalMissionLink->type = MissionTypeMusician;
				}
				else
				if(strstr(terminalMissionLink->missiontype->stf.getAnsi(),"hunting")!= NULL)
				{
					terminalMissionLink->type = MissionTypeHunting;
				}
				else
				if(strstr(terminalMissionLink->missiontype->stf.getAnsi(),"recon")!= NULL)
				{
					terminalMissionLink->type = MissionTypeRecon;
				}
				else
				if(strstr(terminalMissionLink->missiontype->stf.getAnsi(),"survey")!= NULL)
				{
					terminalMissionLink->type = MissionTypeSurvey;
				}


				if(strstr(terminalMissionLink->missiontype->stf.getAnsi(),"easy")!= NULL)
				{
					terminalMissionLink->difficulty = MissionDifficulty_Easy;
				}
				else
				if(strstr(terminalMissionLink->missiontype->stf.getAnsi(),"medium")!= NULL)
				{
					terminalMissionLink->difficulty = MissionDifficulty_Medium;
				}
				else
				if(strstr(terminalMissionLink->missiontype->stf.getAnsi(),"hard")!= NULL)
				{
					terminalMissionLink->difficulty = MissionDifficulty_Hard;
				}


				if(strstr(terminalMissionLink->missiontype->stf.getAnsi(),"rebel")!= NULL)
				{
					terminalMissionLink->faction = MissionFaction_Rebel;
				}
				else
				if(strstr(terminalMissionLink->missiontype->stf.getAnsi(),"neutral")!= NULL)
				{
					terminalMissionLink->faction = MissionFaction_Neutral;
				}
				else
				if(strstr(terminalMissionLink->missiontype->stf.getAnsi(),"imperial")!= NULL)
				{
					terminalMissionLink->faction = MissionFaction_Imperial;
				}


				//do we have the terminal already in our list?

				TerminalMap::iterator terminalMapIt = mTerminalMap.find(terminalMissionLink->terminal);
				if(terminalMapIt != mTerminalMap.end())
				{
					Terminal_Type* terminal = (*terminalMapIt).second;
					terminal->list.push_back(terminalMissionLink);

				}
				else
				{
					Terminal_Type* terminal = new(Terminal_Type);
					mTerminalMap.insert(std::make_pair(terminalMissionLink->terminal,terminal));
					terminal->id = terminalMissionLink->terminal;
					terminal->list.push_back(terminalMissionLink);

				}
			}

			if(result->getRowCount())
				gLogger->log(LogManager::NOTICE,"Loading %u mission terminal links...",result->getRowCount());


		}
		break;

        default:
            break;
    }

    delete(asynContainer);
}

//======================================================================================================================

/*
 * Player opened a mission terminal or pressed refresh
*/
void MissionManager::listRequest(PlayerObject* player, uint64 terminal_id,uint8 refresh_count)
{
	Terminal*	terminal		= dynamic_cast<Terminal*> (gWorldManager->getObjectById(terminal_id));
    //uint32		terminal_type	= terminal->getTerminalType();

	int8		terminal_name[255];
	strcpy(terminal_name,terminal->getName().getAnsi());

	gLogger->log(LogManager::DEBUG,"Terminal id %"PRIu64" is type '%s'", terminal_id, terminal_name);

 	int count = 0;
	int len = strlen(terminal_name);
	MissionBag* mission_bag = dynamic_cast<MissionBag*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Mission));

	MissionList::iterator it = mission_bag->getMissions()->begin();
	while(it != mission_bag->getMissions()->end())
	{
		MissionObject* mission = dynamic_cast<MissionObject*>(*it);
		mission->clear();
		mission->setIssuingTerminal(terminal);
		mission->setRefreshCount(refresh_count);
		switch(len)
		{
			case 16: //terminal_mission
				count < 5 ? generateDestroyMission(mission,terminal_id) : generateDeliverMission(mission);
			break;
			case 22: //terminal_mission_scout
				if (count < 5) {
					mission->setRefreshCount(0);
				} else {
					generateReconMission(mission);
				}
			break;
			case 24: //terminal_mission_artisan
				count < 5 ? generateCraftingMission(mission) : generateSurveyMission(mission);

			break;
			case 28: //terminal_mission_entertainer
				generateEntertainerMission(mission,count);
			break;
			default:
				gLogger->log(LogManager::DEBUG,"Terminal id %"PRIu64" is type '%s'", terminal_id, terminal_name);
				mission->setRefreshCount(0);
		}

		gMessageLib->sendMISO_Delta(mission, player);

		count++;
		++it;
	}


}

//======================================================================================================================

void MissionManager::detailsRequest(PlayerObject* player)
{
    gLogger->log(LogManager::DEBUG,"Player id %"PRIu64" requested mission details", player->getId());

    // this request likely requires a MissionDetailsResponse (000000F8) packet response
}

//======================================================================================================================

void MissionManager::createRequest(PlayerObject* player)
{
    gLogger->log(LogManager::DEBUG,"Player id %"PRIu64" accepted mission", player->getId());
}

//======================================================================================================================

/*
 * Player Accepted Mission
*/
void MissionManager::missionRequest(PlayerObject* player, uint64 mission_id)
{
	Datapad* datapad			= player->getDataPad();

	//Move the mission from the player's mission bag to his datapad.
	MissionBag* mission_bag = dynamic_cast<MissionBag*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Mission));
	MissionObject* mission =  mission_bag->getMissionById(mission_id);
	if(mission == NULL)
	{
		gLogger->log(LogManager::NOTICE,"ERROR: Failed to retrieve mission with id %"PRIu64". Unable to accept mission!", mission_id);
		return;
	}

	//automatically checks the datapads capacity
	if(!datapad->addMission(mission))
	{
		gMessageLib->sendSystemMessage(player,L"","mission/mission_generic","too_many_missions");
		return;
	}
	mission_bag->removeMission(mission);
	gMessageLib->sendContainmentMessage(mission->getId(), datapad->getId(), 0xffffffff, player);


	//Replace it with a new mission.
	mission_bag->spawnNAdd();

	//Send MissionGenericResponse to tell the client its been accepted
	gMessageLib->sendMissionGenericResponse(mission,player);

	//Update the name and mission waypoint
	MissionObject* updater = new MissionObject();
	updater->clear();
	updater->setId(mission->getId());
	updater->setNameFile(mission->getTitleFile().getRawData());
	updater->setName(mission->getTitle().getRawData());
	if(mission->getMissionType() != survey)
	{
		updater->getWaypoint()->setId(mission->getId()+1);
		if(mission->getMissionType() == deliver || mission->getMissionType() == crafting)
		{
			updater->getWaypoint()->setCoords(mission->getStart().Coordinates);
			updater->getWaypoint()->setPlanetCRC(mission->getStart().PlanetCRC);
		}
		else
		{
			updater->getWaypoint()->setCoords(mission->getDestination().Coordinates);
			updater->getWaypoint()->setPlanetCRC(mission->getDestination().PlanetCRC);

		}
		char name[150];
		sprintf(name, "@%s:%s",mission->getTitleFile().getRawData(),mission->getTitle().getRawData());
		updater->getWaypoint()->setName(name);
		updater->getWaypoint()->setActive(true);
	}
	else
	{
		gMessageLib->sendSystemMessage(player,L"","mission/mission_generic","survey_start");
	}

	//Accept the mission let the player know
	gMessageLib->sendPlayMusicMessage(WMSound_Mission_Accepted,player); //3887, 'sound/music_mission_accepted.snd'
	gMessageLib->sendMISO_Delta(updater,player);

	mission->sendAttributes(mission->getOwner());

	delete updater;

	if(mission->getMissionType() == recon)	mission->setTaskId(gWorldManager->addMissionToProcess(mission));


	//check if we need to inform our group
	if((mission->getMissionType() == destroy)&&(player->getGroupId() != 0))
	{
		// we are in a group and just accepted a destroy mission
		// check the missions and update the nearest waypoint
		GroupObject* group = gGroupManager->getGroupObject(player->getGroupId());
		gGroupManager->sendGroupMissionUpdate(group);

	}

return;
}
//======================================================================================================================


void MissionManager::missionComplete(PlayerObject* player, MissionObject* mission)
{
	gMessageLib->sendSystemMessage(player,L"","mission/mission_generic","success_w_amount","", "", L"",mission->getReward());

	//remove mission
	gMessageLib->sendSetWaypointActiveStatus(mission->getWaypoint(),false,player);
	gMessageLib->sendDestroyObject(mission->getId(),player);

	//Give the player the credit reward
	gMessageLib->sendPlayMusicMessage(2501,player); //sound/music_mission_complete.snd
	gMessageLib->sendMissionComplete(player);
	Bank* bank = dynamic_cast<Bank*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Bank));
	bank->setCredits(bank->getCredits() + mission->getReward());
	gTreasuryManager->saveAndUpdateBankCredits(player);

return;
}
//======================================================================================================================


void MissionManager::missionCompleteEntertainer(PlayerObject* player,Buff* timer)
{
	Datapad* datapad			= player->getDataPad();

	if(datapad->hasMission()) //player has a mission
	{
		MissionList::iterator it = datapad->getMissions()->begin();
		while(it != datapad->getMissions()->end())
		{
			MissionObject* mission = dynamic_cast<MissionObject*>(*it);
			if(mission->getInProgress() == false) { ++it; continue; }			
			if(mission->getMissionType() == dancer || mission->getMissionType() == musician)
			{
				//if(mission->getEntertainingTimer() != timer) { ++it; continue; } //check to see if this is the mission who's timer is expiring
                if(glm::distance(player->mPosition, mission->getDestination().Coordinates) < 20)
				{
						missionComplete(player,mission);
						datapad->removeMission(mission);
						delete mission;
						return;
				}
			}
			++it;
		}
	}

return;
}
//======================================================================================================================


/*
 * Player aborted the mission
*/
void MissionManager::missionAbort(PlayerObject* player, uint64 mission_id)
{
	gLogger->log(LogManager::DEBUG,"ABORT MISSION");
	Datapad* datapad			= player->getDataPad();
	
	MissionObject* mission = datapad->getMissionById(mission_id);
	if(mission)
	{
		//If we failed an entertainer mission then we need to remove the timer associated with it.
		if(mission->getMissionType() == dancer || mission->getMissionType() == musician)
		{
			if(mission->getInProgress())
			{
				Buff* timer = mission->getEntertainingTimer();
				player->RemoveBuff(timer);
				mission->setInProgress(false);			
				SAFE_DELETE(timer);
				mission->setEntertainingTimer(NULL);
			}
		}

		datapad->removeMission(mission);
		gMessageLib->sendSystemMessage(player,L"","mission/mission_generic","incomplete");
		gMessageLib->sendSetWaypointActiveStatus(mission->getWaypoint(),false,player);
		gMessageLib->sendMissionAbort(mission,player);
		gMessageLib->sendContainmentMessage(mission->getId(), datapad->getId(), 4, player);
		gMessageLib->sendDestroyObject(mission_id,player);

		delete mission;
	}
	else
	{
		gLogger->log(LogManager::WARNING,"ERROR: Attempt to abort an invalid mission, with id %.8X, from the datapad.", static_cast<int>(mission_id));
	}

return;
}
//======================================================================================================================


void MissionManager::missionFailed(PlayerObject* player, MissionObject* mission)
{
	gMessageLib->sendSystemMessage(player,L"","mission/mission_generic","failed");


	//If we failed an entertainer mission then we need to remove the timer associated with it.
	if(mission->getMissionType() == dancer || mission->getMissionType() == musician)
	{
		if(mission->getInProgress())
		{
			Buff* timer = mission->getEntertainingTimer();
			
			//help the mission is a success if its still in progress
			//as the bufftimer executes when its stopped
			//so set progress to false
			//argh
			mission->setInProgress(false);			
			player->RemoveBuff(timer);			
			SAFE_DELETE(timer);
			mission->setEntertainingTimer(NULL);
		}
	}

	//remove mission
	gMessageLib->sendSetWaypointActiveStatus(mission->getWaypoint(),false,player);
	gMessageLib->sendDestroyObject(mission->getId(),player);

}
//======================================================================================================================


void MissionManager::missionFailedEntertainer(PlayerObject* player)
{
	Datapad* datapad			= player->getDataPad();

	if(datapad->hasMission()) //player has a mission
	{
		MissionList::iterator it = datapad->getMissions()->begin();
		while(it != datapad->getMissions()->end())
		{
			MissionObject* mission = dynamic_cast<MissionObject*>(*it);
			if(mission->getMissionType() == dancer || mission->getMissionType() == musician)
			{
				if(!mission->getInProgress()) { ++it; continue; }
                if(glm::distance(player->mPosition, mission->getDestination().Coordinates) < 20)
				{
						missionFailed(player,mission);
						it = datapad->removeMission(it);
						delete mission;
						return;
				}
			}
			++it;
		}
	}
}
//======================================================================================================================


bool MissionManager::checkDeliverMission(PlayerObject* player,NPCObject* npc)
{
	Datapad* datapad			= player->getDataPad();
	
	if(datapad->hasMission()) //player has a mission
	{
		MissionList::iterator it = datapad->getMissions()->begin();
		while(it != datapad->getMissions()->end())
		{
			MissionObject* mission = dynamic_cast<MissionObject*>(*it);
			if(mission->getMissionType() == deliver)
			{
				if(mission->getStartNPC() == npc)
				{
					//This is the start npc for the deliver mission
					char mp[10];
					sprintf(mp,"m%dp",mission->getNum());
					gMessageLib->sendSpatialChat(npc,player,L"",mission->getTitleFile(),mp);
					mission->setStartNPC(NULL);
					gMessageLib->sendSystemMessage(player,L"","mission/mission_generic","deliver_received_data");
					MissionObject* updater = new MissionObject();
					updater->clear();
					updater->setId(mission->getId());
					updater->getWaypoint()->setId(mission->getId()+1);
					updater->getWaypoint()->setCoords(mission->getDestination().Coordinates);
					updater->getWaypoint()->setPlanetCRC(mission->getDestination().PlanetCRC);
					char name[150];
					sprintf(name, "@%s:%s",mission->getTitleFile().getRawData(),mission->getTitle().getRawData());
					updater->getWaypoint()->setName(name);
					updater->getWaypoint()->setActive(true);
					gMessageLib->sendMISO_Delta(updater,player);
						delete updater;
					return true;
				}
				else if(mission->getDestinationNPC() == npc && mission->getStartNPC() == NULL)
				{
					//This is the end npc for the deliver mission.
					char mr[10];
					sprintf(mr,"m%dr",mission->getNum());
					gMessageLib->sendSpatialChat(npc,player,L"",mission->getTitleFile(),mr);
					missionComplete(player,mission);
					mission->setDestinationNPC(NULL);
					it = datapad->removeMission(it);
					delete mission;
					return true;
				}
			}

			++it;
		}
	}

return false;
}
//======================================================================================================================

void MissionManager::checkMusicianMission(PlayerObject* player)
{
	Datapad* datapad			= player->getDataPad();

	if(datapad->hasMission()) //player has a mission
	{
		MissionList::iterator it = datapad->getMissions()->begin();
		while(it != datapad->getMissions()->end())
		{
			MissionObject* mission = dynamic_cast<MissionObject*>(*it);
			if(mission->getMissionType() == musician)
			{
				if(mission->getInProgress()) 
				{ 
					++it; 
					continue; 
				}

                if(glm::distance(player->mPosition, mission->getDestination().Coordinates) < 20)
				{
					BuffAttribute* performance_timer = new BuffAttribute(time_remaining, 0,0,0);
					Buff* timer = Buff::SimpleBuff(player, player, 60000, 0, gWorldManager->GetCurrentGlobalTick());
					timer->AddAttribute(performance_timer);
					player->AddBuff(timer);
					mission->setEntertainingTimer(timer);
					mission->setInProgress(true);
				}
			}
			++it;
		}
	}
}
//======================================================================================================================


void MissionManager::checkDancerMission(PlayerObject* player)
{
	Datapad* datapad			= player->getDataPad();

	if(datapad->hasMission()) //player has a mission
	{
		MissionList::iterator it = datapad->getMissions()->begin();
		while(it != datapad->getMissions()->end())
		{
			MissionObject* mission = dynamic_cast<MissionObject*>(*it);
			if(mission->getMissionType() == dancer)
			{
				if(mission->getInProgress()) { ++it; continue; }
                if(glm::distance(player->mPosition, mission->getDestination().Coordinates) < 20)
				{
					BuffAttribute* performance_timer = new BuffAttribute(time_remaining, 0,0,0);
					Buff* timer = Buff::SimpleBuff(player, player, 600000, 0, gWorldManager->GetCurrentGlobalTick());
					timer->AddAttribute(performance_timer);
					player->AddBuff(timer);
					mission->setInProgress(true);
				}
			}
			++it;
		}
	}

return;
}
//======================================================================================================================


void MissionManager::checkSurveyMission(PlayerObject* player,CurrentResource* resource,ResourceLocation highestDist)
{
	Datapad* datapad			= player->getDataPad();

	if(datapad->hasMission()) //player has a mission
	{
		MissionList::iterator it = datapad->getMissions()->begin();
		while(it != datapad->getMissions()->end())
		{
			MissionObject* mission = dynamic_cast<MissionObject*>(*it);
			if(mission->getMissionType() == survey)
			{
				if(mission->getTargetResource() == resource->getType())
				{
					if(mission->getDifficulty() <= (highestDist.ratio*100))
					{
                        if(glm::distance(mission->getIssuingTerminal()->mPosition, highestDist.position) > 1024)
						{
							gLogger->log(LogManager::DEBUG,"PE > 500: ready to apply new BF/wound dmg");
							missionComplete(player,mission);
							it = datapad->removeMission(it);
							delete mission;
							return;
						}
						else
						{


							int8 sm[500];
							sprintf(sm,"That resource pocket is too close (%"PRIu32" meters) to the mission giver to be useful to them. Go find one at least %"PRIu32" meters away to complete your survey mission. ",
                                static_cast<uint32>(glm::distance(mission->getIssuingTerminal()->mPosition, highestDist.position)),
                                    (1024 - (int)glm::distance(mission->getIssuingTerminal()->mPosition, highestDist.position)));

							string s = BString(sm);
							s.convert(BSTRType_Unicode16);
              gMessageLib->sendSystemMessage(player,s.getUnicode16());
						}
					}
				}
			}
			++it;
		}

	}

}
//======================================================================================================================


bool MissionManager::checkCraftingMission(PlayerObject* player,NPCObject* npc)
{
	Datapad* datapad			= player->getDataPad();
	if(datapad->hasMission()) //player has a mission
	{
		MissionList::iterator it = datapad->getMissions()->begin();
		while(it != datapad->getMissions()->end())
		{
			MissionObject* mission = dynamic_cast<MissionObject*>(*it);
			if(mission->getMissionType() == crafting)
			{
				if(mission->getStartNPC() == npc)
				{
					//This is the start npc for the deliver mission
					char mp[10];
					sprintf(mp,"m%dp",mission->getNum());
					gMessageLib->sendSpatialChat(npc,player,L"",mission->getTitleFile(),mp);
					mission->setStartNPC(NULL);
					gMessageLib->sendSystemMessage(player,L"","mission/mission_generic","deliver_received_data");
					MissionObject* updater = new MissionObject();
					updater->clear();
					updater->setId(mission->getId());
					updater->getWaypoint()->setId(mission->getId()+1);
					updater->getWaypoint()->setCoords(mission->getDestination().Coordinates);
					updater->getWaypoint()->setPlanetCRC(mission->getDestination().PlanetCRC);
					char name[150];
					sprintf(name, "@%s:%s",mission->getTitleFile().getRawData(),mission->getTitle().getRawData());
					updater->getWaypoint()->setName(name);
					updater->getWaypoint()->setActive(true);
					gMessageLib->sendMISO_Delta(updater,player);
					delete updater;
					return true;
				}
				else if(mission->getDestinationNPC() == npc && mission->getStartNPC() == NULL)
				{
					//This is the end npc for the deliver mission.
					char mr[10];
					sprintf(mr,"m%dr",mission->getNum());
					gMessageLib->sendSpatialChat(npc,player,L"",mission->getTitleFile(),mr);
					missionComplete(player,mission);
					mission->setDestinationNPC(NULL);
					it = datapad->removeMission(it);
					delete mission;
					return true;
				}
			}

			++it;
		}
	}

return false;
}
//======================================================================================================================


bool MissionManager::checkReconMission(MissionObject* mission)
{
	if (mission->getMissionType() != recon) return false;

    if(glm::distance(mission->getOwner()->mPosition, mission->getDestination().Coordinates) < 20)
	{
		Datapad* datapad			= mission->getOwner()->getDataPad();
		missionComplete(mission->getOwner(),mission);
		gWorldManager->removeMissionFromProcess(mission->getTaskId());
		datapad->removeMission(mission);
		delete mission;
	}



return true;
}
//======================================================================================================================


MissionObject* MissionManager::generateDestroyMission(MissionObject* mission, uint64 terminal)
{


	mission->setMissionType(destroy); //crc = destroy

	//find the missiondata for the respective terminal
	Terminal_Mission_Link* link = NULL;

	TerminalMap::iterator terminalMapIt = mTerminalMap.find(terminal);
	if(terminalMapIt != mTerminalMap.end())
	{
		gLogger->log(LogManager::DEBUG,"MissionManager : found the terminal");
					
		Terminal_Type* terminal = (*terminalMapIt).second;

		//now get the amount of stfs and get one per chance
		uint32 amount = terminal->list.size();
		uint32 chosen = gRandom->getRand() % amount;

		gLogger->log(LogManager::DEBUG,"MissionManager : random : %u", chosen);

		bool found = false;
		uint32 counter = 0;
		while(!found)
		{
			gLogger->log(LogManager::DEBUG,"MissionManager : != found ");

			MissionLinkList::iterator it = 	terminal->list.begin();
			while(it != terminal->list.end())
			{
				if(counter >= chosen)
				{
					if((*it)->type == MissionTypeDestroy)
					{
						found = true;
						link = (*it);
						break;
					}
				}
				counter++;
				it++;

			}
			it = terminal->list.begin();

		}

	}

	int mission_num;

	if(link)
	{
		gLogger->log(LogManager::DEBUG,"MissionManager : found db destroy missions");
					

		//now set the stf
		char s[255];
		sprintf(s,"mission/%s",link->missiontype->stf.getAnsi());
		mission->setTitleFile(s);

		//the mission within the stf
		mission_num = (gRandom->getRand() % link->content)+1;
		mission->setNum(mission_num);

		//Mission Title
		sprintf(mt,"m%dt",mission_num);
		mission->setTitle(mt);


		//Mission Description
		sprintf(md,"m%dd",mission_num);

		mission->setDetailFile(s);
		mission->setDetail(md);

		//still have to sort out the names of the mission givers
	}
	else
	{
		gLogger->log(LogManager::DEBUG,"MissionManager : No mission file associated");
					
		return NULL;

	}


	missionTargets missionTarget[4] =
	{
			{0xB9BA5440, "@lair_n:naboo_otolla_gungan_camp_neutral_large_theater"},
			{0x6D4C33E5, "@lair_n:naboo_capper_spineflap_nest_neutral_large"},
			{0xA0057DAE, "@lair_n:naboo_kaadu_lair_neutral_medium_boss_01"},
			{0xFA6FD53A, "@lair_n:lair_base"},
	};


	//Randomly choose a target
	int target = gRandom->getRand() % 4;

	//END TEMP

	//Position
	//int radius = 500; //500m radius
	Location destination;
    
    glm::vec3 new_vector = glm::gtx::random::vecRand3(50.0f, 500.0f);
    new_vector.y = 0;

	destination.Coordinates = mission->getOwner()->mPosition + new_vector;
	destination.CellID = 0;
	destination.PlanetCRC = BString(gWorldManager->getPlanetNameThis()).getCrc();

	mission->setDestination(destination);


	//Creator

	sprintf(mo,"m%do",mission_num);
	string moS(mo);
	gLogger->log(LogManager::DEBUG,"MissionManager : creator :%s",moS.getAnsi());
	NameMap nameMap = link->missiontype->names;
	NameMap::iterator NameMapIt = nameMap.find(moS.getCrc());
	if(NameMapIt != nameMap.end())
	{
		sprintf(mo,"%s",(*NameMapIt).second->name.getAnsi());
		mission->setCreator(mo);
		//mo = (*NameMapIt).second->name.getAnsi();
	}
	else
	{
		uint32 selected = gRandom->getRand() % this->mNameMap.size();

		NameMap::iterator it = mNameMap.find(selected);
		mission->setCreator((*it).second->name.getAnsi());
	}

   //Difficulty
	mission->setDifficulty((gRandom->getRand() % 90)+5);

	//Payment
	mission->setReward((gRandom->getRand() % 10000)+1000);

	//Target
	mission->setTargetModel(missionTarget[target].crc);
	mission->setTarget(missionTarget[target].name);

return mission;
}
//=============================================================================


MissionObject* MissionManager::generateDeliverMission(MissionObject* mission)
{
	mission->setMissionType(deliver);

	//TEMP

	missionData mission_deliver_hard[2] =
	{
		{"mission/mission_deliver_neutral_hard",25},
		{"mission/mission_deliver_neutral_hard_non_persistent_from_npc",15},
	};

	//Randomly choose a stf file
	int stf_file = gRandom->getRand() % 2;

	//Randomly choose a mission in that file
	int mission_num = (gRandom->getRand() % mission_deliver_hard[stf_file].num)+1;
	mission->setNum(mission_num);

    //END TEMP

	ObjectSet inRangeNPCs;
	gWorldManager->getSI()->getObjectsInRange(mission->getOwner(),&inRangeNPCs,ObjType_NPC,1500);

	//Start & End
	bool found = false;
	Location mission_start;
    Location mission_dest;
	ObjectSet::iterator it = inRangeNPCs.begin();

	//we may stall the main thread with the way it was done ???? however often enough the mission generation never finished!!!!!!!!!!!!!!!


	//get a list containing all suitable npcs and generate a random number corresponding to one of the npcs

	if(inRangeNPCs.size() < 2)
		return NULL;


	uint32 count = 0;

	while(!found && !inRangeNPCs.empty())
	{
		count ++;
		++it;
		if(it == inRangeNPCs.end())
			it = inRangeNPCs.begin();

		NPCObject* npc = dynamic_cast<NPCObject*>(*it);
		if(npc->getNpcFamily() == NpcFamily_Filler)
		{
			uint32 roll		= (gRandom->getRand() / (RAND_MAX  + 1ul) * (9 - 1) + 1);
			if((roll = 5)||(count > inRangeNPCs.size()))
			{
				if(mission_dest.Coordinates.x == 0)
				{
					mission->setDestinationNPC(npc);
					mission_dest.Coordinates = npc->mPosition;
					mission_dest.CellID = 0;
					mission_dest.PlanetCRC = BString(gWorldManager->getPlanetNameThis()).getCrc();
					mission->setDestination(mission_dest);
				}
				else if(mission_start.Coordinates.x == 0 && mission->getDestinationNPC() != npc)
				{
					mission->setStartNPC(npc);
					mission_start.Coordinates = npc->mPosition;
					mission_start.CellID = 0;
					mission_start.PlanetCRC = BString(gWorldManager->getPlanetNameThis()).getCrc();
					mission->setStart(mission_start);
					found = true;
				}

			}
		}
	}

	//Creator
	mission->setCreator(creators[gRandom->getRand() % 9]);

    //Mission Title
	sprintf(mt,"m%dt",mission_num);

	mission->setTitleFile(mission_deliver_hard[stf_file].mSTF);
	mission->setTitle(mt);

	//Mission Description
	sprintf(md,"m%dd",mission_num);

	mission->setDetailFile(mission_deliver_hard[stf_file].mSTF);
	mission->setDetail(md);

   //Diffaculty
	mission->setDifficulty((gRandom->getRand() % 10)+3);

	//Payment
	mission->setReward((gRandom->getRand() % 300)+50);

	//Mission Target
	mission->setTargetModel(0xE191DBAB); //crc = object/tangible/mission/shared_mission_datadisk.iff

return mission;
}
//=============================================================================


MissionObject* MissionManager::generateEntertainerMission(MissionObject* mission,int count)
{

	count < 5 ?
		mission->setMissionType(musician):
	    mission->setMissionType(dancer);

	//Randomly choose an entertainer mission
	int mission_num = (gRandom->getRand() % 50)+1;
	mission->setNum(mission_num);

	ObjectSet inRangeNPCs;
	gWorldManager->getSI()->getObjectsInRange(mission->getOwner(),&inRangeNPCs,ObjType_NPC,3000);
	//Start
	uint32 cntLoop = 0;
	bool found = false;
	Location mission_dest;
	ObjectSet::iterator it = inRangeNPCs.begin();
	while(!found && !inRangeNPCs.empty())
	{
		cntLoop++;
		++it;
		if(it == inRangeNPCs.end())
			it = inRangeNPCs.begin();

		NPCObject* npc = dynamic_cast<NPCObject*>(*it);
		if(npc->getNpcFamily() == NpcFamily_Filler)
		{
			uint32 roll		= (gRandom->getRand() / (RAND_MAX  + 1ul) * (9 - 1) + 1);
			if((roll = 5)||(cntLoop > inRangeNPCs.size()))
			{
				if(mission_dest.Coordinates.x == 0 && mission->getDestinationNPC() != npc)
				{
					mission->setStartNPC(npc);
					mission_dest.Coordinates = npc->mPosition;
					mission_dest.CellID = 0;
					mission_dest.PlanetCRC = BString(gWorldManager->getPlanetNameThis()).getCrc();
					mission->setDestination(mission_dest);
					mission->setDestinationNPC(npc);
					found = true;
				}
			}
		}
	}

	//Creator
	mission->setCreator(creators[gRandom->getRand() % 9]);

    //Mission Title
	sprintf(mt,"m%dt",mission_num);

	count < 5 ?
		mission->setTitleFile("mission/mission_npc_musician_neutral_easy") :
		mission->setTitleFile("mission/mission_npc_dancer_neutral_easy");
	mission->setTitle(mt);

	//Mission Description
	sprintf(md,"m%do",mission_num);

	count < 5 ?
		mission->setDetailFile("mission/mission_npc_musician_neutral_easy") :
		mission->setDetailFile("mission/mission_npc_dancer_neutral_easy");
	mission->setDetail(md);

   //Diffaculty
	mission->setDifficulty((gRandom->getRand() % 90)+3);

	//Payment
	mission->setReward((gRandom->getRand() % 2500)+500);

	//Target Name
	mission->setTarget("Entertainer");

	//Mission Target
	mission->setTargetModel(0x491099A6); //crc = object/tangible/instrument/shared_organ_max_rebo.iff

return mission;
}
//=============================================================================


MissionObject* MissionManager::generateSurveyMission(MissionObject* mission)
{

	mission->setMissionType(survey);

	//Randomly choose a survey mission
	int mission_num = (gRandom->getRand() % 50)+1;
	mission->setNum(mission_num);

	//Creator
	mission->setCreator(creators[gRandom->getRand() % 9]);

	//Title
	sprintf(mt,"m%dt",mission_num);
	mission->setTitleFile("mission/mission_npc_survey_neutral_easy");
	mission->setTitle(mt);

	//Details
	sprintf(md,"m%do",mission_num);
	mission->setDetailFile("mission/mission_npc_survey_neutral_easy");
	mission->setDetail(md);

	bool found = false;

	ResourceTypeMap*	rtMap = gResourceManager->getResourceTypeMap();
	ResourceTypeMap::iterator it = rtMap->begin();

	uint32 cntLoop = 0;
	while(!found && !rtMap->empty())//?????
	{
		cntLoop ++;

		++it;
		if(it == rtMap->end())
			it = rtMap->begin();

		if(!strcmp((*it->second).getResourceType().getRawData(),"mineral_resource"))/* ||
		   !strcmp((*(*it).second).getResourceType().getRawData(),"energy_resource")) */
		{
			uint32 roll		= (gRandom->getRand() / (RAND_MAX  + 1ul) * (9 - 1) + 1);

			if((roll == 5)||(cntLoop > rtMap->size()))
			{
				mission->setTarget((*it->second).getName().getRawData());
				mission->setTargetModel((*it->second).getContainerModel().getCrc());
				mission->setTargetResource(((*it).second));
				found = true;
				break;
			}
		}

	}

	//efficiency - A range from 30 - 70 is the best I can do without
	//knowning the min and max concentration of the given resource.
	//The CurrentResouce class needs to be modified to provide these values.
	mission->setDifficulty((gRandom->getRand() % 41) + 30); //value from 30 - 70

	//Reward
	mission->setReward(mission->getDifficulty() * ((gRandom->getRand() % 14) + 15)); //Difficulty * rand: 15-28

	Location destination;
	destination.Coordinates = mission->getOwner()->mPosition;
	destination.CellID = 0;
	destination.PlanetCRC = BString(gWorldManager->getPlanetNameThis()).getCrc();
	mission->setStart(destination);


return mission;
}
//=============================================================================


MissionObject* MissionManager::generateCraftingMission(MissionObject* mission)
{
	mission->setMissionType(crafting);

	//TEMP!
	string targets[8][2] =
	{
		{"@item_n:output_governor","object/tangible/mission/quest_item/shared_attunement_grid.iff"},
		{"@item_n:current_alternator","object/tangible/mission/quest_item/shared_current_alternator.iff"},
		{"@item_n:nym_hard_drive","object/tangible/loot/quest/shared_nym_hard_drive.iff"},
		{"@item_n:hyperdrive_part_s01","object/tangible/loot/misc/shared_hyperdrive_part_s01.iff"},
		{"@item_n:dermal_analyzer","object/tangible/loot/npc_loot/shared_dermal_analyzer_generic.iff"},
		{"@item_n:feedback_controller","object/tangible/mission/quest_item/shared_feedback_controller.iff"},
		{"@item_n:power_regulator","object/tangible/mission/quest_item/shared_power_regulator.iff"},
		{"@item_n:attunement_grid","object/tangible/mission/quest_item/shared_attunement_grid.iff"}
	};

	int target_num = gRandom->getRand() % 8;

	//END TEMP

	//Randomly choose a crafting mission
	int mission_num = (gRandom->getRand() % 50)+1;
	mission->setNum(mission_num);

	//Creator
	mission->setCreator(creators[gRandom->getRand() % 9]);

	//Title
	sprintf(mt,"m%dt",mission_num);
	mission->setTitleFile("mission/mission_npc_crafting_neutral_easy");
	mission->setTitle(mt);

	//Details
	sprintf(md,"m%dd",mission_num);
	mission->setDetailFile("mission/mission_npc_crafting_neutral_easy");
	mission->setDetail(md);

    //END TEMP

	ObjectSet inRangeNPCs;
	gWorldManager->getSI()->getObjectsInRange(mission->getOwner(),&inRangeNPCs,ObjType_NPC,1500);

	uint32 cntLoop = 0;
	//Start & End
	bool found = false;
	Location mission_start;
    Location mission_dest;
	ObjectSet::iterator it = inRangeNPCs.begin();
	while(!found && !inRangeNPCs.empty())
	{
		cntLoop++;
		++it;

		if(it == inRangeNPCs.end())
			it = inRangeNPCs.begin();

		NPCObject* npc = dynamic_cast<NPCObject*>(*it);
		if(npc->getNpcFamily() == NpcFamily_Filler)
		{
			uint32 roll		= (gRandom->getRand() / (RAND_MAX  + 1ul) * (9 - 1) + 1);
			if((roll = 5)||(cntLoop > inRangeNPCs.size()))
			{
				if(mission_dest.Coordinates.x == 0)
				{
					mission->setDestinationNPC(npc);
					mission_dest.Coordinates = npc->mPosition;
					mission_dest.CellID = 0;
					mission_dest.PlanetCRC = BString(gWorldManager->getPlanetNameThis()).getCrc();
					mission->setDestination(mission_dest);
				}
				else if(mission_start.Coordinates.x == 0 && mission->getDestinationNPC() != npc)
				{
					mission->setStartNPC(npc);
					mission_start.Coordinates = npc->mPosition;
					mission_start.CellID = 0;
					mission_start.PlanetCRC = BString(gWorldManager->getPlanetNameThis()).getCrc();
					mission->setStart(mission_start);
				}
				else
				{
					found = true;
				}
			}
		}
	}

   //Difficulty
	mission->setDifficulty((gRandom->getRand() % 41) + 40); //value from 30 - 80

	//Reward
	mission->setReward(mission->getDifficulty() * ((gRandom->getRand() % 14) + 15)); //Difficulty * rand: 15-28

	//Target
	mission->setTarget((targets[target_num][0]).getRawData()); //will need to be located from the db

	//Mission Target
	mission->setTargetModel((targets[target_num][1]).getCrc()); //crc = object/tangible/mission/quest_item/shared_attunement_grid.iff

return mission;
}
//=============================================================================


MissionObject* MissionManager::generateReconMission(MissionObject* mission)
{
	mission->setMissionType(recon);

	//TEMP

	//First is a stf file, second is number of missions in that file
	missionData mission_recon[3] =
	{
			{"mission/mission_npc_recon_neutral_easy",25},
			{"mission/mission_npc_recon_neutral_medium",50},
			{"mission/mission_npc_recon_neutral_hard",25},
	};

	//Randomly choose a stf file
	int stf_file = gRandom->getRand() % 3;

	//Randomly choose a mission in that file
	int mission_num = (gRandom->getRand() % mission_recon[stf_file].num)+1;
	mission->setNum(mission_num);

	//END TEMP

	//Position
	//int radius = 500; //500m radius
	Location destination;

    glm::vec3 new_vector = glm::gtx::random::vecRand3(50.0f, 500.0f);
    new_vector.y = 0;

	destination.Coordinates = mission->getOwner()->mPosition + new_vector;
	destination.CellID = 0;
	destination.PlanetCRC = BString(gWorldManager->getPlanetNameThis()).getCrc();
	mission->setDestination(destination);

	//Creator
	mission->setCreator(creators[gRandom->getRand() % 9]);

    //Mission Title
	sprintf(mt,"m%dt",mission_num);

	mission->setTitleFile(mission_recon[stf_file].mSTF);
	mission->setTitle(mt);

	//Mission Description
	sprintf(md,"m%do",mission_num);

	mission->setDetailFile(mission_recon[stf_file].mSTF);
	mission->setDetail(md);

   //Difficulty
	mission->setDifficulty((gRandom->getRand() % 70)+20);

	//Payment
	mission->setReward((gRandom->getRand() % 10000)+1000);

	//Target
	mission->setTargetModel(0x45788EE2); //crc = object/tangible/mission/shared_mission_recon_target.iff

return mission;
}
//=============================================================================
