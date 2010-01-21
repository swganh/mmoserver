 /*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "GroupManager.h"

#include "Datapad.h"
#include "GroupObject.h"
#include "MissionObject.h"
#include "ObjectFactory.h"
#include "UIManager.h"
#include "WaypointObject.h"
#include "WorldConfig.h"
#include "WorldManager.h"
#include "ZoneOpcodes.h"

#include "MessageLib/MessageLib.h"

#include "LogManager/LogManager.h"

#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"

#include "Common/DispatchClient.h"
#include "Common/Message.h"
#include "Common/MessageDispatch.h"
#include "Common/MessageFactory.h"
#include "Common/MessageOpcodes.h"

#include "Utils/utils.h"


bool						GroupManager::mInsFlag    = false;
GroupManager*		GroupManager::mSingleton  = NULL;


//======================================================================================================================

GroupManager::GroupManager(Database* database, MessageDispatch* dispatch)
{	
	mDatabase = database;
	mMessageDispatch = dispatch;
	
	mMessageDispatch->RegisterMessageCallback(opIsmGroupInviteRequest,this);  
	mMessageDispatch->RegisterMessageCallback(opIsmGroupCREO6deltaGroupId,this);  
	mMessageDispatch->RegisterMessageCallback(opIsmGroupLootModeResponse,this); 
	mMessageDispatch->RegisterMessageCallback(opIsmGroupLootMasterResponse,this); 
}


//======================================================================================================================

GroupManager::~GroupManager()
{
	mInsFlag = false;
	delete(mSingleton);
}

//======================================================================================================================

GroupManager*	GroupManager::Init(Database* database, MessageDispatch* dispatch)
{
	if(!mInsFlag)
	{
		mSingleton = new GroupManager(database,dispatch);
		mInsFlag = true;
		return mSingleton;
	}
	else
		return mSingleton;
	
}

//======================================================================================================================

void GroupManager::Shutdown()
{
	mMessageDispatch->UnregisterMessageCallback(opIsmGroupInviteRequest);
	mMessageDispatch->UnregisterMessageCallback(opIsmGroupCREO6deltaGroupId);
	mMessageDispatch->UnregisterMessageCallback(opIsmGroupLootModeResponse);
	mMessageDispatch->UnregisterMessageCallback(opIsmGroupLootMasterResponse);
}

//======================================================================================================================

void GroupManager::handleDispatchMessage(uint32 opcode, Message* message, DispatchClient* client)
{
	switch(opcode)
	{
	
		case opIsmGroupInviteRequest:
		{
			_processIsmInviteRequest(message);
		}
		break;

		case opIsmGroupCREO6deltaGroupId:
		{
			_processIsmGroupCREO6deltaGroupId(message);
		}
		break;

		case opIsmGroupLootModeResponse:
		{
			_processIsmGroupLootModeResponse(message);
		}
		break;

		case opIsmGroupLootMasterResponse:
		{
			_processIsmGroupLootMasterResponse(message);
		}
		break;

		default:
			gLogger->logMsgF("GroupManagerMessage::handleDispatchMessage: Unhandled opcode %u",MSG_NORMAL,opcode);
		break;
	} 
}

//=======================================================================================================================


void GroupManager::_processIsmInviteRequest(Message* message)
{
	
	PlayerObject* sender = gWorldManager->getPlayerByAccId(message->getUint32()); // the player who sent the invite
	PlayerObject* target = gWorldManager->getPlayerByAccId(message->getUint32());  // the player who will recieve it

	if(sender == NULL || target == NULL)
	{
		gLogger->logMsg("GroupManager::_processIsmInviteRequest PlayerAccId not found");
		return;	
	}

	//target->setGroupId(message->getUint64()); // the group id provided by the chatserver

	gMessageLib->sendInviteSenderUpdateDeltasCreo6(sender->getId(),target);
	
}

//=======================================================================================================================

void GroupManager::_processIsmGroupCREO6deltaGroupId(Message* message)
{
	// this packet is sent by the chatserver to update the group_id in the CREO6
	// it concerns the player itself and all the inrange players

	PlayerObject* const player = gWorldManager->getPlayerByAccId(message->getUint32());  // the player whos group_id has changed
	if(player == NULL)
	{
		gLogger->logMsg("GroupManager::_processIsmGroupCREO6deltaGroupId PlayerAccId not found");
		return;
	}
	
	//this makes the player a member of the group
	uint64 id = message->getUint64();

	//did he just leave the group - is the id zero ?
	if(id == 0)
	{
		GroupObject* group = getGroupObject(player->getGroupId());
		if(group != NULL)
		{
			//got it - just remove him from the group
			group->removePlayer(player->getId());

			//if the group is now empty we need to delete it
			Uint64List* groupList = group->getPlayerList();
			if(groupList->size() == 0)
			{
				deleteGroupObject(player->getGroupId());
				delete(group);
			}
		}
	}
	
	player->setGroupId(id);
	
	if(id != 0)
	{
		//check if a group with that Id already exists
		GroupObject* group;
		group = getGroupObject(id);

		if(group != NULL)
		{
			//yes - just add the player
			group->addPlayer(player->getId());
		}
		else
		{
			//add a new group
			group = new GroupObject(id);
			addGroupObject(group);
			group->addPlayer(player->getId());
		}
	}

	// to in-range folks
	const PlayerObjectSet*	const inRangePlayers	= player->getKnownPlayers();
	PlayerObjectSet::const_iterator	it				= inRangePlayers->begin();
	
	while(it != inRangePlayers->end())
	{
		const PlayerObject* const targetObject = (*it);

		if(targetObject->isConnected())
		{
			gMessageLib->sendGroupIdUpdateDeltasCreo6(player->getGroupId(),player,targetObject);
		}

		++it;
	}

	// to self
	gMessageLib->sendGroupIdUpdateDeltasCreo6(player->getGroupId(), player, player);

}

//=======================================================================================================================

void GroupManager::_processIsmGroupLootModeResponse(Message* message)
{
	gLogger->logMsg("_processIsmGroupLootModeResponse");
	PlayerObject* playerObject = gWorldManager->getPlayerByAccId(message->getUint32());  // the player whos group_id has changed
	if(playerObject == NULL)
	{
		gLogger->logMsg("GroupManager::processIsmGroupLootModeResponse PlayerAccId not found");
		return;
	}

	//send the SUI

	BStringVector availableLootModes;
	availableLootModes.push_back("Free for all");
	availableLootModes.push_back("Master Looter");
	availableLootModes.push_back("Lottery");
	availableLootModes.push_back("Random");

	gUIManager->createNewListBox(playerObject,"handleSetLootMode","@group:set_loot_type_title","@group:set_loot_type_text",availableLootModes,playerObject,SUI_Window_SelectGroupLootMode_Listbox);
}

//=======================================================================================================================

void GroupManager::_processIsmGroupLootMasterResponse(Message* message)
{
	gLogger->logMsg("_processIsmGroupLootMasterResponse");
	PlayerObject* playerObject = gWorldManager->getPlayerByAccId(message->getUint32());  // the player whos group_id has changed
	if(playerObject == NULL)
	{
		gLogger->logMsg("GroupManager::_processIsmGroupLootMasterResponse PlayerAccId not found");
		return;
	}

	//send the SUI
	gLogger->logMsg("ok\n");

	PlayerList inRangeMembers	= playerObject->getInRangeGroupMembers(true);
	PlayerList::iterator it		= inRangeMembers.begin();

	BStringVector namesArray;

	while(it != inRangeMembers.end())
	{
		namesArray.push_back((*it)->getFirstName().getAnsi());
		++it;
	}

	gUIManager->createNewPlayerSelectListBox(playerObject,"handleSetLootMaster","@group:master_looter_sui_title","@group:set_loot_type_text",namesArray,inRangeMembers,playerObject);
}

//=======================================================================================================================
// we want to update the group waypoint to the nearest Mission!


void GroupManager::sendGroupMissionUpdate(GroupObject* group)
{
	// this procedure ensures, that in case of a change in the mission pool of the group
	// all players get updated Mission waypoints
	// it concerns all players of the group on the zone, but not on other zones


	//get us the mission nearest to the most players on the Zone
	MissionObject* mission = getZoneGroupMission(group->getPlayerList());

	if(!mission)
		return;

	//now set the GroupWaypoint for all onZone groupmembers
	Uint64List::iterator playerListIt = group->getPlayerList()->begin();
	while(playerListIt != group->getPlayerList()->end())
	{
		PlayerObject*	player		= dynamic_cast<PlayerObject*> (gWorldManager->getObjectById((*playerListIt)));
		Datapad*		datapad		= dynamic_cast<Datapad*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Datapad));
		WaypointObject*	waypoint	= datapad->getWaypointByName("@group:groupwaypoint");

		// remove the old one
		if(waypoint)
		{
			gMessageLib->sendUpdateWaypoint(waypoint,ObjectUpdateAdd,player);
			datapad->removeWaypoint(waypoint);
			gObjectFactory->deleteObjectFromDB(waypoint);

		}
		else
		// create a new one
		if(datapad->getCapacity())
		{
			datapad->requestNewWaypoint("@group:groupwaypoint",mission->getDestination().Coordinates,static_cast<uint16>(gWorldManager->getZoneId()),Waypoint_blue);
			gMessageLib->sendSystemMessage(player,L"","group","groupwaypoint");
		}

		
		playerListIt++;
	}
}

//=======================================================================================================================
// we want to find the mission we set the waypoint to
// this function gets called everytime we accept/finish/delete a mission
// and should be called every few seconds while were moving

MissionObject* GroupManager::getZoneGroupMission(std::vector<uint64>* members)
{

	// we will iterate through all onZone groupmembers and compile a list of missions
	// we will find the mission nearest to the most players
	// and set the waypoints accordingly

	MissionObject*	chosenMission		= NULL;
	uint32			chosenRubberPoints	= 0;

	if(!members->size())
		return NULL;

	MissionGroupRangeList missionRangeList;

	//compile the missionlist
	Uint64List::iterator playerListIt = members->begin();
	while(playerListIt != members->end())
	{
		PlayerObject*	groupMember = dynamic_cast<PlayerObject*> (gWorldManager->getObjectById((*playerListIt)));
		
		if(!groupMember)
		{
			playerListIt++;
			continue;
		}

		Datapad* datapad = dynamic_cast<Datapad*>(groupMember->getEquipManager()->getEquippedObject(CreatureEquipSlot_Datapad));

		MissionList* missionList = datapad->getMissions();
		MissionList::iterator missionListIt = missionList->begin();

		//add all Missions of the player to our List
		while(missionListIt != missionList->end())
		{
			//the rubber point counter will be set to 0 initially
			missionRangeList.push_back(std::make_pair((*missionListIt),0));
			missionListIt++;
		}
	

		playerListIt++;
	}

	//do we have missions in the first place ?
	if(!missionRangeList.size())
		return NULL;

	//now iterate through all players and determine their nearest mission
	playerListIt = members->begin();

	while(playerListIt != members->end())
	{
		PlayerObject*	groupMember = dynamic_cast<PlayerObject*> (gWorldManager->getObjectById((*playerListIt)));
		//now check for the nearest mission - increase its counter
		
		MissionGroupRangeList::iterator missionRangeListIt = missionRangeList.begin();
		MissionGroupRangeList::iterator missionRangeListStorage = missionRangeList.begin();

		while(missionRangeListIt != missionRangeList.end())
		{
			MissionObject* nearestMission = NULL;
			float	nearestDistance = 88000.0;
			
			MissionObject* currentMission = (*missionRangeListIt).first;
			float currentDistance = groupMember->mPosition.distance2D(currentMission->getDestination().Coordinates);

			//store the nearest Mission
			if(currentDistance < nearestDistance)
			{
				nearestDistance = currentDistance;
				nearestMission = currentMission;
				
				//store the lists iterator for future reference
				missionRangeListStorage = missionRangeListIt;

			}

			
			missionRangeListIt++;
		}
		
		//now take the nearest Mission and give it a rubberpoint
		(*missionRangeListStorage).second++;

		//does our mission have enough rubberpoints?
		if((*missionRangeListStorage).second > (uint32)(members->size() /2))
		{
			//jupp :)
			return (*missionRangeListStorage).first;
		}

		//remember the mission with the most rubberpoints anyway in case we dont have a clear winner
		if((*missionRangeListStorage).second > chosenRubberPoints)
		{
			chosenRubberPoints	= (*missionRangeListStorage).second;
			chosenMission		= (*missionRangeListStorage).first;
		}

		playerListIt++;
	}

	return chosenMission;
}


GroupObject* GroupManager::getGroupObject(uint64 id)
{
	GroupList::iterator it = mGroupList.begin();
	while(it != mGroupList.end())
	{
		if((*it)->getId() == id)
			return (*it);
			
		it++;
	}
	return NULL;
}

void GroupManager::deleteGroupObject(uint64 id)
{
	GroupList::iterator it = mGroupList.begin();
	while(it != mGroupList.end())
	{
		if((*it)->getId() == id)
			it = mGroupList.erase(it);
		if(it != mGroupList.end())
			it++;
	}
}