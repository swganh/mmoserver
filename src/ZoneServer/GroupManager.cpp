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

#include "ZoneServer/GroupManager.h"

#include "Utils/logger.h"

#include "Utils/utils.h"

#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"

#include "NetworkManager/DispatchClient.h"
#include "NetworkManager/Message.h"
#include "NetworkManager/MessageDispatch.h"
#include "NetworkManager/MessageFactory.h"
#include "NetworkManager/MessageOpcodes.h"

#include "MessageLib/MessageLib.h"

#include "ZoneServer/Datapad.h"
#include "ZoneServer/GroupManagerCallbackContainer.h"
#include "ZoneServer/GroupObject.h"
#include "ZoneServer/MissionObject.h"
#include "ZoneServer/ObjectFactory.h"
#include "ZoneServer/PlayerObject.h"
#include "ZoneServer/SpatialIndexManager.h"
#include "ZoneServer/UIManager.h"
#include "ZoneServer/WaypointObject.h"
#include "ZoneServer/WorldConfig.h"
#include "ZoneServer/WorldManager.h"
#include "ZoneServer/ZoneOpcodes.h"

bool						GroupManager::mInsFlag    = false;
GroupManager*		GroupManager::mSingleton  = NULL;

//======================================================================================================================

GroupManager::GroupManager(Database* database, MessageDispatch* dispatch)
{
    LOG(INFO) << "Beginning GroupManager initialization";
    mDatabase = database;
    mMessageDispatch = dispatch;

    mMessageDispatch->RegisterMessageCallback(opIsmGroupInviteRequest,std::bind(&GroupManager::_processIsmInviteRequest, this, std::placeholders::_1, std::placeholders::_2));
    mMessageDispatch->RegisterMessageCallback(opIsmGroupCREO6deltaGroupId,std::bind(&GroupManager::_processIsmGroupCREO6deltaGroupId, this, std::placeholders::_1, std::placeholders::_2));
    mMessageDispatch->RegisterMessageCallback(opIsmGroupLootModeResponse,std::bind(&GroupManager::_processIsmGroupLootModeResponse, this, std::placeholders::_1, std::placeholders::_2));
    mMessageDispatch->RegisterMessageCallback(opIsmGroupLootMasterResponse,std::bind(&GroupManager::_processIsmGroupLootMasterResponse, this, std::placeholders::_1, std::placeholders::_2));
    mMessageDispatch->RegisterMessageCallback(opIsmGroupInviteInRangeRequest, std::bind(&GroupManager::_processIsmGroupInviteInRangeRequest, this, std::placeholders::_1, std::placeholders::_2));
    mMessageDispatch->RegisterMessageCallback(opIsmIsGroupLeaderResponse, std::bind(&GroupManager::_processIsmIsGroupLeaderResponse, this, std::placeholders::_1, std::placeholders::_2));

    LOG(INFO) << "GroupManager initialization complete";
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
    mMessageDispatch->UnregisterMessageCallback(opIsmGroupInviteInRangeRequest);
}

void GroupManager::_processIsmIsGroupLeaderResponse(Message* message, DispatchClient* client)
{
    std::map<uint64, GroupManagerCallbackContainer*>::iterator it = mLeaderRequests.find(message->getUint64());
    std::map<uint64, GroupManagerCallbackContainer*>::iterator end = mLeaderRequests.end();

    if(it != end)
    {
        uint64 playerId = message->getUint64();

        if(message->getUint8() == 1)
            (*it).second->isLeader = true;
        else
            (*it).second->isLeader = false;

        (*it).second->callback->handleGroupManagerCallback(playerId, (*it).second);

        delete (*it).second;
        mLeaderRequests.erase(it);
    }
}

uint64 GroupManager::_insertLeaderRequest(GroupManagerCallbackContainer* container)
{
    uint64 requestId = mLeaderRequestInc;

    uint32 maxCount=0;//To insure we don't infinite loop.
    std::map<uint64, GroupManagerCallbackContainer*>::iterator it = mLeaderRequests.find(requestId);
    while(it != mLeaderRequests.end() && requestId != 0)
    {
        requestId = mLeaderRequestInc++;
        it = mLeaderRequests.find(requestId);

        if(maxCount >= 50)
            return 0; //Lets not continue this little ordeal.
    }

    mLeaderRequests.insert(std::make_pair(requestId, container));

    return requestId;
}

void GroupManager::getGroupLeader(PlayerObject* requester, uint64 groupId, uint32 operation, GroupManagerCallback* callback, BString arg)
{
    GroupManagerCallbackContainer* container = new GroupManagerCallbackContainer();
    container->requestingPlayer = requester->getId();
    container->arg = arg;
    container->callback = callback;
    container->operation = operation;

    uint64 requestId = _insertLeaderRequest(container);

    if(requestId != 0)
        gMessageLib->sendGroupLeaderRequest(requester, requestId, operation, groupId);
}

void GroupManager::getGroupLeader(PlayerObject* requester, uint64 groupId, uint32 operation, GroupManagerCallback* callback)
{
    GroupManagerCallbackContainer* container = new GroupManagerCallbackContainer();
    container->requestingPlayer = requester->getId();
    container->callback = callback;
    container->operation = operation;

    uint64 requestId = _insertLeaderRequest(container);

    if(requestId != 0)
        gMessageLib->sendGroupLeaderRequest(requester, requestId, operation, groupId);
}

void GroupManager::getGroupLeader(PlayerObject* requester, uint64 groupId, uint32 operation, GroupManagerCallback* callback, uint32 flourishId)
{
    GroupManagerCallbackContainer* container = new GroupManagerCallbackContainer();
    container->requestingPlayer = requester->getId();
    container->flourishId = flourishId;
    container->callback = callback;
    container->operation = operation;

    uint64 requestId = _insertLeaderRequest(container);

    if(requestId != 0)
        gMessageLib->sendGroupLeaderRequest(requester, requestId, operation, groupId);
}

//=======================================================================================================================

void GroupManager::_processIsmInviteRequest(Message* message, DispatchClient* client)
{

    PlayerObject* sender = gWorldManager->getPlayerByAccId(message->getUint32()); // the player who sent the invite
    PlayerObject* target = gWorldManager->getPlayerByAccId(message->getUint32());  // the player who will recieve it

    if(sender == NULL || target == NULL)
    {
        return;
    }

    //target->setGroupId(message->getUint64()); // the group id provided by the chatserver

    gMessageLib->sendInviteSenderUpdateDeltasCreo6(sender->getId(),target);

}

//=======================================================================================================================

void GroupManager::_processIsmGroupCREO6deltaGroupId(Message* message, DispatchClient* client)
{
    // this packet is sent by the chatserver to update the group_id in the CREO6
    // it concerns the player itself and all the inrange players

    PlayerObject* const player = gWorldManager->getPlayerByAccId(message->getUint32());  // the player whos group_id has changed
    if(player == NULL)
    {
        DLOG(WARNING) << "GroupManager::_processIsmGroupCREO6deltaGroupId PlayerAccId not found";
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
    ObjectSet result_set;

    gSpatialIndexManager->getObjectsInRange(player,&result_set,ObjType_Creature,30.0,true);

    std::for_each(result_set.begin(), result_set.end(), [player] (Object* object) {
        if (object->getType() != ObjType_Player) {
            return;
        }

        PlayerObject* target = static_cast<PlayerObject*>(object);

        if(target->isConnected()) {
            gMessageLib->sendGroupIdUpdateDeltasCreo6(player->getGroupId(), player, target);
        }
    });

    // to self
    gMessageLib->sendGroupIdUpdateDeltasCreo6(player->getGroupId(), player, player);
}

//=======================================================================================================================

void GroupManager::_processIsmGroupLootModeResponse(Message* message, DispatchClient* client)
{
    PlayerObject* playerObject = gWorldManager->getPlayerByAccId(message->getUint32());  // the player whos group_id has changed
    if(playerObject == NULL)
    {
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

void GroupManager::_processIsmGroupLootMasterResponse(Message* message, DispatchClient* client)
{
    PlayerObject* playerObject = gWorldManager->getPlayerByAccId(message->getUint32());  // the player whos group_id has changed
    if(playerObject == NULL)
    {
        return;
    }

    //send the SUI
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
        //Datapad*		datapad		= dynamic_cast<Datapad*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Datapad));
        Datapad* datapad			= player->getDataPad();
        WaypointObject*	waypoint	= datapad->getWaypointByName("@group:groupwaypoint");

        // remove the old one
        if(waypoint)
        {

            // now update the DB
            datapad->updateWaypoint(waypoint->getId(), waypoint->getName(), mission->getDestination().Coordinates,
                                    static_cast<uint16>(gWorldManager->getZoneId()), player->getId(), WAYPOINT_ACTIVE);
            gMessageLib->sendUpdateWaypoint(waypoint,ObjectUpdateChange,player);
            gMessageLib->SendSystemMessage(::common::OutOfBand("group","groupwaypoint"), player);
        }
        else
        {
            // create a new one
            if(datapad->getCapacity())
            {
                datapad->requestNewWaypoint("@group:groupwaypoint",mission->getDestination().Coordinates,static_cast<uint16>(gWorldManager->getZoneId()),Waypoint_blue);
                gMessageLib->SendSystemMessage(::common::OutOfBand("group","groupwaypoint"), player);
            }
        }


        playerListIt++;
    }
}

//=======================================================================================================================
// we want to find the mission we set the waypoint to
// this function gets called everytime we accept/finish/delete a mission
// and should be called every few seconds while were moving

MissionObject* GroupManager::getZoneGroupMission(std::list<uint64>* members)
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

        //Datapad* datapad = dynamic_cast<Datapad*>(groupMember->getEquipManager()->getEquippedObject(CreatureEquipSlot_Datapad));
        Datapad* datapad			= groupMember->getDataPad();

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
            float	nearestDistance = 88000.0;

            MissionObject* currentMission = (*missionRangeListIt).first;
            float currentDistance = glm::distance(groupMember->mPosition, currentMission->getDestination().Coordinates);

            //store the nearest Mission
            if(currentDistance < nearestDistance)
            {
                nearestDistance = currentDistance;

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

//======================================================================================================================

void GroupManager::_processIsmGroupInviteInRangeRequest(Message *message, DispatchClient* client)
{
    PlayerObject* sender = gWorldManager->getPlayerByAccId(message->getUint32());
    PlayerObject* target = gWorldManager->getPlayerByAccId(message->getUint32());

    if( sender == NULL || target == NULL )
    {
        return;
    }

    if (glm::distance(sender->getWorldPosition(), target->getWorldPosition()) < 90)
    {
        gMessageLib->sendIsmGroupInviteInRangeResponse(sender, target, true);
    }
    else
    {
        gMessageLib->sendIsmGroupInviteInRangeResponse(sender, target, false);
    }

}
