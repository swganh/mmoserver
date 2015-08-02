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

#include "ZoneServer/CharacterLoginHandler.h"

#include <iostream>
#include <sstream>




#include "Utils/logger.h"

#include "Common/BuildInfo.h"
#include "Utils/rand.h"

#include "DatabaseManager/Database.h"

#include "NetworkManager/DispatchClient.h"
#include "NetworkManager/Message.h"
#include "NetworkManager/MessageDispatch.h"
#include "NetworkManager/MessageFactory.h"
#include "NetworkManager/MessageOpcodes.h"

#include "MessageLib/MessageLib.h"

#include "ZoneServer/BuffManager.h"
#include "ZoneServer/Inventory.h"
#include "ZoneServer/ObjectFactory.h"
#include "ZoneServer/PlayerObject.h"
#include "ZoneServer/SpatialIndexManager.h"
#include "ZoneServer/StateManager.h"
#include "ZoneServer/TravelMapHandler.h"
#include "ZoneServer/TravelTicket.h"
#include "ZoneServer/Tutorial.h"
#include "ZoneServer/WorldConfig.h"
#include "ZoneServer/WorldManager.h"
#include "ZoneServer/ZoneOpcodes.h"

using namespace std;
//======================================================================================================================

CharacterLoginHandler::CharacterLoginHandler(Database* database, MessageDispatch* dispatch)
    : mDatabase(database)
    , mMessageDispatch(dispatch)
{
    // Register our opcodes
    mMessageDispatch->RegisterMessageCallback(opSelectCharacter, std::bind(&CharacterLoginHandler::_processSelectCharacter, this, std::placeholders::_1, std::placeholders::_2));
    mMessageDispatch->RegisterMessageCallback(opCmdSceneReady, std::bind(&CharacterLoginHandler::_processCmdSceneReady, this, std::placeholders::_1, std::placeholders::_2));
    mMessageDispatch->RegisterMessageCallback(opClusterClientDisconnect, std::bind(&CharacterLoginHandler::_processClusterClientDisconnect, this, std::placeholders::_1, std::placeholders::_2));
    mMessageDispatch->RegisterMessageCallback(opClusterZoneTransferApprovedByTicket,std::bind(&CharacterLoginHandler::_processClusterZoneTransferApprovedByTicket, this, std::placeholders::_1, std::placeholders::_2));
    mMessageDispatch->RegisterMessageCallback(opClusterZoneTransferApprovedByPosition,std::bind(&CharacterLoginHandler::_processClusterZoneTransferApprovedByPosition, this, std::placeholders::_1, std::placeholders::_2));
    mMessageDispatch->RegisterMessageCallback(opClusterZoneTransferDenied,std::bind(&CharacterLoginHandler::_processClusterZoneTransferDenied, this, std::placeholders::_1, std::placeholders::_2));
    mMessageDispatch->RegisterMessageCallback(opNewbieTutorialResponse, std::bind(&CharacterLoginHandler::_processNewbieTutorialResponse, this, std::placeholders::_1, std::placeholders::_2));

    // Load anything we need from the database
    mZoneId = gWorldManager->getZoneId();
}


//======================================================================================================================

CharacterLoginHandler::~CharacterLoginHandler(void)
{
    // Unregister our callbacks
    mMessageDispatch->UnregisterMessageCallback(opSelectCharacter);
    mMessageDispatch->UnregisterMessageCallback(opCmdSceneReady);
    mMessageDispatch->UnregisterMessageCallback(opClusterClientDisconnect);
    mMessageDispatch->UnregisterMessageCallback(opClusterZoneTransferApprovedByTicket);
    mMessageDispatch->UnregisterMessageCallback(opClusterZoneTransferApprovedByPosition);
    mMessageDispatch->UnregisterMessageCallback(opClusterZoneTransferDenied);
    mMessageDispatch->UnregisterMessageCallback(opNewbieTutorialResponse);
}

void CharacterLoginHandler::_processCmdSceneReady(Message* message, DispatchClient* client)
{
    // Client has finished loading, acknowledge
    gMessageLib->sendSceneReady(client);

    // get the according player object
    PlayerObject* player = gWorldManager->getPlayerByAccId(message->getAccountId());
    if (player)
    {
        player->setReady(true);

        if(player->getParentId())
        {
            gMessageLib->sendDataTransformWithParent0B(player);
        }
        else
        {
            gMessageLib->sendDataTransform0B(player);
        }

        // send our message of the day
        std::string motd = gWorldConfig->getConfiguration<std::string>("motD", "Welcome to SWG:ANH");

        if(player && !(player->getMotdReceived()) && motd.length())
        {
            player->setMotdReceived(true);
            gMessageLib->SendSystemMessage(std::wstring(motd.begin(), motd.end()), player);
        }

        // Send newbie info.
        player->newPlayerMessage();

        player->togglePlayerCustomFlagOff(PlayerCustomFlag_LogOut);

        // Fix/workaround for addIgnore (Eruptor)
        // If we send this info to client as soon as we get connected, client will miss the info most of the time.
        // In this case client will end up with an empty "Ignore List" even if the Ignore-list should be populated.

        // Update: The same apply to frindsList.
        gMessageLib->sendFriendListPlay9(player);
        gMessageLib->sendIgnoreListPlay9(player);
        gMessageLib->sendSceneReadyToChat(client);	// will get any mails received when offline. The point is: Notidy about new mails AFTER the user have got the "logged in" message.

        //Initialise the buffs
        gBuffManager->InitBuffs(player);

        // Some info about the current build
        std::stringstream ss;
        ss << "Running build " << GetBuildNumber() << " created " << GetBuildTime();
        std::string tmp(ss.str());

        gMessageLib->SendSystemMessage(std::wstring(tmp.begin(), tmp.end()), player);
    }
}

void	CharacterLoginHandler::_processSelectCharacter(Message* message, DispatchClient* client)
{
	

    PlayerObject*	playerObject;
    uint64			playerId = message->getUint64();

	ObjectIDSet::iterator it = playerZoneList.find(playerId);
	if(it != playerZoneList.end())	{
		return;
	}

	playerZoneList.insert(playerId);

    // player already exists and is in logged state

    playerObject = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(playerId));

	//this exact character is currently being logged out ...
    if((playerObject) && playerObject->isLinkDead())
    {
        // Remove old client, if any.
        delete playerObject->getClient();

        playerObject->setClient(client);

        gWorldManager->addReconnectedPlayer(playerObject);

        gMessageLib->sendChatServerStatus(0x01,0x36,client);
        gMessageLib->sendParameters(900,client);
        gMessageLib->sendStartScene(mZoneId,playerObject);
        gMessageLib->sendServerTime(gWorldManager->getServerTime(),client);

        Weather* weather = gWorldManager->getCurrentWeather();

        gMessageLib->sendWeatherUpdate(weather->mClouds,weather->mWeather,playerObject);

        //initialize us for the world
        //gWorldManager->addObject(playerObject);

        //create us for others dont use create in world - we are still in the cell
        //we just want to reinitialize the grid for us
        gSpatialIndexManager->InitializeObject(playerObject);

        //create ourselves for us
        gSpatialIndexManager->sendCreatePlayer(playerObject,playerObject);



        playerObject->togglePlayerCustomFlagOff(PlayerCustomFlag_LogOut);
        gMessageLib->sendUpdatePlayerFlags(playerObject);

        playerObject->getHam()->checkForRegen();
        playerObject->getStomach()->checkForRegen();

		ObjectIDSet::iterator it = playerZoneList.find(playerId);
		if(it != playerZoneList.end())	{
			it = playerZoneList.erase(it);
		}

    }
    else if(playerObject  && playerObject->isBeingDestroyed())
    {
        //dont quite understand this one - the player is about to be destroyed
        //so just ignore it ????

        //we might want to wait and then reload the character

        // Remove old client, if any.
        delete playerObject->getClient();

		ObjectIDSet::iterator it = playerZoneList.find(playerId);
		if(it != playerZoneList.end())	{
			it = playerZoneList.erase(it);
		}
		return;
    }
    
	// account already logged in with a character - we need to unload that character before loading the requested one
	//make sure that char finished loading before trying to get rid of it
	else if((playerObject = gWorldManager->getPlayerByAccId(client->getAccountId())))
    {

        DLOG(INFO) << "CharacterLoginHandler::_processSelectCharacter same account : new character ";
        // remove old char immidiately
        if(playerObject->getId() == playerId)
        {
            //we need to bail out. If a bot tries to rapidly login it can happen that we get here again even before the character
            //did finish loading or even with a properly logged in player ...
            //loading this player a second time and logging it out at the same time will lead to desaster
            LOG(WARNING) << "CharacterLoginHandler::_processSelectCharacter account " << client->getAccountId() << " is spamming logins";
            return;
        }

        //the old character mustnt be necessarily on the disconnect list
        gWorldManager->removePlayerFromDisconnectedList(playerObject);

        // need to make sure the char is saved and removed, before requesting the new one
        // so doing it sync

        //start async save with command character request and relevant ID
        CharacterLoadingContainer* clContainer = new(CharacterLoadingContainer);

        clContainer->mClient		= client;
        clContainer->mPlayerId		= playerId;
        clContainer->ofCallback		= this;

		//take the old player out of the grid
		gSpatialIndexManager->RemoveObjectFromWorld(playerObject);

        gWorldManager->savePlayer(playerObject->getAccountId(),true, WMLogOut_Char_Load, clContainer);
    }
    // request a load from db
    else
    {
        gObjectFactory->requestObject(ObjType_Player,0,0,this,playerId,client);
    }
}

//======================================================================================================================

void	CharacterLoginHandler::_processNewbieTutorialResponse(Message* message, DispatchClient* client)
{
    // Updated comment (Eru): string depends on tutorial, default is "clientReady" though.
    PlayerObject* player = gWorldManager->getPlayerByAccId(client->getAccountId());
    if (player && player->isConnected())
    {
        BString tutorialEventString;
        message->getStringAnsi(tutorialEventString);
        if (gWorldConfig->isTutorial())
        {
            // Notify tutorial
            player->getTutorial()->tutorialResponse(tutorialEventString);
        }
    }
}

//======================================================================================================================

void CharacterLoginHandler::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
    CharacterLoadingContainer* asyncContainer = reinterpret_cast<CharacterLoadingContainer*>(ref);

    switch(asyncContainer->callBack)
    {
        //were travelling to another zone
    case CLHCallBack_Transfer_Ticket:
    {
        // Next step is save the player this call back goes to the worldmanager which will handle the rest
        CharacterLoadingContainer* newContainer = new(CharacterLoadingContainer);

        newContainer->callBack		= CLHCallBack_Transfer_Position;
        newContainer->destination	= asyncContainer->destination;
        newContainer->planet		= asyncContainer->planet;
        newContainer->player		= asyncContainer->player;
        newContainer->dbCallback	= this;

        //no remove by the save we do it here in the callback
        gWorldManager->savePlayer(asyncContainer->player->getAccountId(),false, WMLogOut_Zone_Transfer, newContainer);
        /////////////////////////////////////////////////////

    }
    break;

    case CLHCallBack_Transfer_Position:
    {
        //the worldmanager just saved the player and updated its position to the new planet
        gMessageLib->sendClusterZoneTransferCharacter(asyncContainer->player, asyncContainer->planet);

        asyncContainer->player->setConnectionState(PlayerConnState_LinkDead);

        gWorldManager->destroyObject(asyncContainer->player);
    }
    break;

    case CLHCallBack_None:
    default:
        break;
    }
}

//======================================================================================================================
//
// ObjectFactory Callback, we got a new player object, so notify the client and add it to the world
//
void CharacterLoginHandler::handleObjectReady(Object* object,DispatchClient* client)
{
    switch(object->getType())
    {
    case ObjType_Player:
    {
        PlayerObject* player = dynamic_cast<PlayerObject*>(object);
        player->setConnectionState(PlayerConnState_Connected);
        player->setClient(client);

        gMessageLib->sendChatServerStatus(0x01,0x36,client);
        gMessageLib->sendParameters(900,client);
        gMessageLib->sendStartScene(mZoneId,player);
        gMessageLib->sendServerTime(gWorldManager->getServerTime(),client);

        //initialize us in the world
        gWorldManager->addObject(player);

        //create us for others
        gSpatialIndexManager->createInWorld(player);

        //create ourselves for us
        gSpatialIndexManager->sendCreatePlayer(player,player);

		//remove us from the loaders list
		ObjectIDSet::iterator it = playerZoneList.find(player->getId());
		if(it != playerZoneList.end())	{
			it = playerZoneList.erase(it);
		}

    }
    break;

    default:
        DLOG(WARNING) << "CharacterLoginHandler::ObjectFactoryCallback: Unhandled object: " << object->getType();
        break;
    }
}

//======================================================================================================================
//
// handles client disconnect, resets players client object and puts him into logged(LD) state
//
void CharacterLoginHandler::_processClusterClientDisconnect(Message* message, DispatchClient* client)
{
    uint32 reason = message->getUint32();
    PlayerObject* playerObject;

    if (reason == 1)
    {
        DLOG(INFO) << "Removed Player: Total Players on zone : " << gWorldManager->getPlayerAccMap()->size();
    }
    else
    {
        // put it to the disconnected list
        if((playerObject = gWorldManager->getPlayerByAccId(client->getAccountId())) != NULL)
        {
            // playerObject->setClient(NULL);	// To early for this, not as long as we have the playerObject active.
            gWorldManager->addDisconnectedPlayer(playerObject);
        }
    }
}

//=======================================================================================================================

void CharacterLoginHandler::_processClusterZoneTransferApprovedByTicket(Message* message, DispatchClient* client)
{
    PlayerObject* playerObject;
    uint64 ticketId = message->getUint64();

    // put it to the disconnected list
    if((playerObject = gWorldManager->getPlayerByAccId(message->getAccountId())) != NULL)
    {
        // remove the ticket from the DB so they can't use it again.
        TravelTicket* ticket	= dynamic_cast<TravelTicket*>(dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory))->getObjectById(ticketId));
        BString dstPointStr		= (int8*)((ticket->getAttribute<std::string>("travel_arrival_point")).c_str());
        uint16 dstPlanetId		= static_cast<uint16>(gWorldManager->getPlanetIdByName((int8*)((ticket->getAttribute<std::string>("travel_arrival_planet")).c_str())));

        TravelPoint* dstPoint = gTravelMapHandler->getTravelPoint(dstPlanetId,dstPointStr);

//		uint64 playerId = playerObject->getId();

        glm::vec3 destination;
        destination.x = dstPoint->spawnX + (gRandom->getRand()%5 - 2);
        destination.y = dstPoint->spawnY;
        destination.z = dstPoint->spawnZ + (gRandom->getRand()%5 - 2);

        // Reset to standing
        gStateManager.setCurrentPostureState(playerObject, CreaturePosture_Upright);


        // Delete the ticket then save the position then the player
        CharacterLoadingContainer* asyncContainer = new(CharacterLoadingContainer);

        asyncContainer->destination = destination;
        asyncContainer->planet		= dstPlanetId;
        asyncContainer->player		= playerObject;
        asyncContainer->callBack	= CLHCallBack_Transfer_Ticket;

        mDatabase->executeSqlAsync(this,asyncContainer,"DELETE FROM %s.items WHERE id = %" PRIu64 "", mDatabase->galaxy(),ticket->getId());


    }
}

//=======================================================================================================================

void CharacterLoginHandler::_processClusterZoneTransferApprovedByPosition(Message* message, DispatchClient* client)
{
    PlayerObject*	playerObject;
    uint32			planetId		= message->getUint32();
    float			x				= message->getFloat();
    float			z				= message->getFloat();

    if((playerObject = gWorldManager->getPlayerByAccId(message->getAccountId())) != NULL)
    {
        // reset to standing
        //playerObject->states.setPosture(CreaturePosture_Upright);

        // Save our player.
        gWorldManager->savePlayerSync(playerObject->getAccountId(),false);

        // Now update the DB with the new location/planetId
        mDatabase->destroyResult(mDatabase->executeSynchSql("UPDATE %s.characters SET parent_id=0,x='%f', y='0', z='%f', planet_id='%u' WHERE id='%" PRIu64 "';",mDatabase->galaxy(),x,z,planetId,playerObject->getId()));


        gMessageLib->sendClusterZoneTransferCharacter(playerObject,planetId);

        playerObject->setConnectionState(PlayerConnState_LinkDead);
        // playerObject->setClient(NULL);
        gWorldManager->destroyObject(playerObject);
    }
}

//=======================================================================================================================

void CharacterLoginHandler::_processClusterZoneTransferDenied(Message* message, DispatchClient* client)
{
    PlayerObject* playerObject;
    /*uint32 reason = */
    message->getUint32();

    // put it to the disconnected list
    if((playerObject = gWorldManager->getPlayerByAccId(message->getAccountId())) != NULL)
    {
        gMessageLib->SendSystemMessage(L"The Emperor has restricted travel to this planet at this time.", playerObject);
    }
}
