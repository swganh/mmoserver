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

#include "CharacterLoginHandler.h"
#include "BuffManager.h"
#include "Inventory.h"
#include "ObjectFactory.h"
#include "PlayerObject.h"
#include "TravelMapHandler.h"
#include "TravelTicket.h"
#include "Tutorial.h"
#include "WorldConfig.h"
#include "WorldManager.h"
#include "ZoneOpcodes.h"
#include "MessageLib/MessageLib.h"
#include "LogManager/LogManager.h"
#include "DatabaseManager/Database.h"
#include "Common/DispatchClient.h"
#include "Common/Message.h"
#include "Common/MessageDispatch.h"
#include "Common/MessageFactory.h"
#include "Common/MessageOpcodes.h"
#include "ConfigManager/ConfigManager.h"
#include "utils/rand.h"


//======================================================================================================================

CharacterLoginHandler::CharacterLoginHandler(Database* database, MessageDispatch* dispatch)
{
	// Store our members
	mDatabase = database;
	mMessageDispatch = dispatch;

	// Register our opcodes
	mMessageDispatch->RegisterMessageCallback(opSelectCharacter, this);
	mMessageDispatch->RegisterMessageCallback(opCmdSceneReady, this);
	mMessageDispatch->RegisterMessageCallback(opClusterClientDisconnect, this);
	mMessageDispatch->RegisterMessageCallback(opClusterZoneTransferApprovedByTicket,this);
	mMessageDispatch->RegisterMessageCallback(opClusterZoneTransferApprovedByPosition,this);
	mMessageDispatch->RegisterMessageCallback(opClusterZoneTransferDenied, this);
	mMessageDispatch->RegisterMessageCallback(opNewbieTutorialResponse, this);
	mMessageDispatch->RegisterMessageCallback(opCmdSceneReady2, this);

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
	mMessageDispatch->UnregisterMessageCallback(opCmdSceneReady2);
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

void CharacterLoginHandler::handleDispatchMessage(uint32 opcode, Message* message, DispatchClient* client)
{
	//when we get a opClusterClientDisconnect the client is already destroyed !!!!!!!!!!!!!!111


  switch(opcode)
  {
	case opSelectCharacter:
	{
        boost::recursive_mutex::scoped_lock lk(mSessionMutex);

		PlayerObject*	playerObject;
		uint64			playerId = message->getUint64();

		// player already exists and is in logged state

		playerObject = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(playerId));

		if((playerObject) && playerObject->isLinkDead())
		{
			// Remove old client, if any.
			delete playerObject->getClient();

			playerObject->setClient(client);
			
			playerObject->destroyKnownObjects();

			gWorldManager->addReconnectedPlayer(playerObject);

			gMessageLib->sendChatServerStatus(0x01,0x36,client);
			gMessageLib->sendParameters(900,client);
			gMessageLib->sendStartScene(mZoneId,playerObject);
			gMessageLib->sendServerTime(gWorldManager->getServerTime(),client);

			Weather* weather = gWorldManager->getCurrentWeather();

			gMessageLib->sendWeatherUpdate(weather->mClouds,weather->mWeather,playerObject);

			//playerObject->toggleStateOff(CreatureState_Crafting);
			// resend our objects
			gWorldManager->initObjectsInRange(playerObject);
			gMessageLib->sendCreatePlayer(playerObject,playerObject);
			playerObject->togglePlayerCustomFlagOff(PlayerCustomFlag_LogOut);	
			gMessageLib->sendUpdatePlayerFlags(playerObject);

			playerObject->getHam()->checkForRegen();
			playerObject->getStomach()->checkForRegen();
		}
		else
		if(playerObject  && playerObject->isBeingDestroyed())
		{
			gLogger->log(LogManager::DEBUG,"Were being destroyed but want to log in again ");
			//dont quite understand this one - the player is about to be destroyed
			//so just ignore it ????

			//we might want to wait and then reload the character

			// Remove old client, if any.
			delete playerObject->getClient();

			//client->Disconnect(0); darn it this disconects the only zone session!!!!
		}
		// player logged in with another char, while still in ld
		else if((playerObject = gWorldManager->getPlayerByAccId(client->getAccountId())))
		{

			gLogger->log(LogManager::DEBUG,"same account : new player ");
			// remove old char immidiately
			gWorldManager->removePlayerFromDisconnectedList(playerObject);

			// need to make sure char is saved and removed, before requesting the new one
			// so doing it sync

			//start async save with command character request and relevant ID
			CharacterLoadingContainer* clContainer = new(CharacterLoadingContainer);

			clContainer->mClient		= client;
			clContainer->mPlayerId		= playerId;
			clContainer->ofCallback		= this;

			gWorldManager->savePlayer(playerObject->getAccountId(),true, WMLogOut_Char_Load, clContainer);
		}
		// request a load from db
		else
		{
			gLogger->log(LogManager::DEBUG,"all other cases");
			gObjectFactory->requestObject(ObjType_Player,0,0,this,playerId,client);
		}
	}
    break;

    case opClusterClientDisconnect:
	{
      _processClusterClientDisconnect(message, client);
	}
    break;

    case opClusterZoneTransferApprovedByTicket:
    {
      _processClusterZoneTransferApprovedByTicket(message, client);
    }
    break;

	case opClusterZoneTransferApprovedByPosition:
	{
		_processClusterZoneTransferApprovedByPosition(message, client);
	}
	break;

    case opClusterZoneTransferDenied:
    {
      _processClusterZoneTransferDenied(message, client);
    }
    break;

    case opCmdSceneReady:
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
			string moT = "welcome to swgAnh";
			moT	= (int8*)((gWorldConfig->getConfiguration<std::string>("motD",moT.getAnsi())).c_str());

			moT.convert(BSTRType_Unicode16);
			if(player && !(player->getMotdReceived()) && moT.getLength())
			{
				player->setMotdReceived(true);
        gMessageLib->sendSystemMessage(player,moT.getUnicode16());
			}

			// Send newbie info.
			player->newPlayerMessage();

			player->togglePlayerCustomFlagOff(PlayerCustomFlag_LogOut);	

			// Fix/workaround for addIgnore (Eruptor)
			// If we send this info to client as soon as we get connected, client will miss the info most of the time.
			// In this case client will end up with an empty "Ignore List" even if the Ignore-list should be populated.

			gLogger->log(LogManager::DEBUG,"CharacterLoginHandler::handleDispatchMessage: opCmdSceneReady");

			// Update: The same apply to frindsList.
			gMessageLib->sendFriendListPlay9(player);
			gMessageLib->sendIgnoreListPlay9(player);
			gMessageLib->sendSceneReadyToChat(client);	// will get any mails received when offline. The point is: Notidy about new mails AFTER the user have got the "logged in" message.

			// Init and start player world position updates.
			ObjectController* ObjCtl = player->getController();
			(void)ObjCtl->playerWorldUpdate(true);	// Force a world object update.

			// This timed event will handle updates of world objects when no external events arrives (movement events from client).
			gWorldManager->addPlayerMovementUpdateTime(player, 1000);

			//Initialise the buffs
			gBuffManager->InitBuffs(player);

			// Some info about the current build
			int8 rawData[128];
			// sprintf(rawData,"Running %s",ConfigManager::getBuildString());
			sprintf(rawData,"Running build %s created %s", ConfigManager::getBuildNumber().c_str(), ConfigManager::getBuildTime().c_str());

			string buildString(rawData);
			buildString.convert(BSTRType_Unicode16);
			gMessageLib->sendSystemMessage(player,buildString.getUnicode16());

			// Temp fix for testing instances at normal planets (Corellia).
			/*
			if (player->isConnected())
			{
				if (!gWorldConfig->isTutorial())
				{
					// Some special message when we are testing...
					if (gWorldConfig->isInstance())
					{
						gMessageLib->sendSystemMessage(player,L"Welcome to Corellia. This planet is temporarily used for testing of instancing.");
						gMessageLib->sendSystemMessage(player,L"You should not see any other players. But if you are in a group, the member of the group should see each other and also be able to interact.");
						gMessageLib->sendSystemMessage(player,L"Hard to group with a friend when you can't find him or her? Go back to another planet and group and then come back!");
					}
				}
			}
			*/
		}
		else
		{
			gLogger->log(LogManager::DEBUG,"CharacterLoginHandler::handleDispatchMessage: Invalid Player object");
		}
	}
    break;

	case opCmdSceneReady2:           // always sent with scene_ready, contains uint16
	break;

	case opNewbieTutorialResponse:   // always sent with scene_ready, contains string "clientReady"
	{								// Updated comment (Eru): string depends on tutorial, default is "clientReady" though.
		PlayerObject* player = gWorldManager->getPlayerByAccId(client->getAccountId());
		if (player->isConnected())
		{
			string tutorialEventString;
			message->getStringAnsi(tutorialEventString);
			gLogger->log(LogManager::DEBUG,"%s",tutorialEventString.getAnsi());
			if (gWorldConfig->isTutorial())
			{
				// Notify tutorial
				player->getTutorial()->tutorialResponse(tutorialEventString);
			}
		}
	}
	break;

	default:
		gLogger->log(LogManager::NOTICE,"CharacterLoginHandler::handleDispatchMessage: Unhandled opcode %u",opcode);
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

			gWorldManager->addObject(player);
		}
		break;

		default:
			gLogger->log(LogManager::NOTICE,"CharacterLoginHandler::ObjectFactoryCallback: Unhandled object: %i",object->getType());
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
		gLogger->log(LogManager::DEBUG,"Removed Player: Total Players on zone : %i",(gWorldManager->getPlayerAccMap())->size());
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
		string dstPointStr		= (int8*)((ticket->getAttribute<std::string>("travel_arrival_point")).c_str());
		uint16 dstPlanetId		= static_cast<uint16>(gWorldManager->getPlanetIdByName((int8*)((ticket->getAttribute<std::string>("travel_arrival_planet")).c_str())));

		TravelPoint* dstPoint = gTravelMapHandler->getTravelPoint(dstPlanetId,dstPointStr);

//		uint64 playerId = playerObject->getId();

        glm::vec3 destination;
		destination.x = dstPoint->spawnX + (gRandom->getRand()%5 - 2);
		destination.y = dstPoint->spawnY;
		destination.z = dstPoint->spawnZ + (gRandom->getRand()%5 - 2);

		gLogger->log(LogManager::DEBUG,"CharacterLoginHandler::_processClusterZoneTransferApprovedByTicket : (x)%f:(z)%f:(y)%f",destination.x,destination.y,destination.z);

		// reset to standing
		playerObject->setPosture(CreaturePosture_Upright);
		playerObject->updateMovementProperties();


		//delete the ticket then save the position then the player
		CharacterLoadingContainer* asyncContainer = new(CharacterLoadingContainer);

		asyncContainer->destination = destination;
		asyncContainer->planet		= dstPlanetId;
		asyncContainer->player		= playerObject;
		asyncContainer->callBack	= CLHCallBack_Transfer_Ticket;

		mDatabase->ExecuteSqlAsync(this,asyncContainer,"DELETE FROM items WHERE id = %"PRIu64"", ticket->getId());


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
		playerObject->setPosture(CreaturePosture_Upright);
		playerObject->updateMovementProperties();

		// Save our player.
		gWorldManager->savePlayerSync(playerObject->getAccountId(),false);

		// Now update the DB with the new location/planetId
		mDatabase->DestroyResult(mDatabase->ExecuteSynchSql("UPDATE characters SET parent_id=0,x='%f', y='0', z='%f', planet_id='%u' WHERE id='%I64u';",x,z,planetId,playerObject->getId()));

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
	/*uint32 reason = */message->getUint32();

	// put it to the disconnected list
	if((playerObject = gWorldManager->getPlayerByAccId(message->getAccountId())) != NULL)
	{
		gMessageLib->sendSystemMessage(playerObject,L"The Emperor has restricted travel to this planet at this time.");
	}
}
