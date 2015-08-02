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

#include "WorldManager.h"

#include <sstream>

#include "Utils/Scheduler.h"
#include "Utils/typedefs.h"
#include "Utils/VariableTimeScheduler.h"
#include "Utils/utils.h"

#include "DatabaseManager/Database.h"
#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/DatabaseResult.h"

#include "MessageLib/MessageLib.h"

#include "ScriptEngine/ScriptEngine.h"
#include "ScriptEngine/ScriptSupport.h"

#include "AdminManager.h"
#include "Buff.h"
#include "BuffEvent.h"
#include "BuffManager.h"
#include "BuildingObject.h"
#include "CellObject.h"
#include "CharacterLoginHandler.h"
#include "Container.h"
#include "ConversationManager.h"
#include "CraftingSessionFactory.h"
#include "CraftingTool.h"
#include "CreatureSpawnRegion.h"
#include "Datapad.h"
#include "GroupManager.h"
#include "GroupObject.h"
#include "FactoryFactory.h"
#include "FactoryObject.h"
#include "HarvesterFactory.h"
#include "HarvesterObject.h"
#include "Heightmap.h"
#include "Inventory.h"
#include "MissionManager.h"
#include "MissionObject.h"
#include "MountObject.h"
#include "NpcManager.h"
#include "NPCObject.h"
#include "ObjectFactory.h"
#include "PlayerObject.h"
#include "PlayerStructure.h"
#include "ResourceManager.h"
#include "SchematicManager.h"
#include "Shuttle.h"
#include "SpatialIndexManager.h"
#include "StateManager.h"
#include "TicketCollector.h"
#include "TreasuryManager.h"
#include "VehicleController.h"
#include "WorldConfig.h"
#include "ZoneOpcodes.h"
#include "ZoneServer.h"

using std::stringstream;

//======================================================================================================================

void WorldManager::savePlayer(uint32 accId, bool remove, WMLogOut logout_type, CharacterLoadingContainer* clContainer) {
    // Lookup the requested player and abort if not found
    PlayerObject* player_object = getPlayerByAccId(accId);
    if(!player_object) {
        DLOG(WARNING) << "WorldManager::savePlayer could not find player with AccId:" << accId << ", save aborted.";
        return;
    }

    // @TODO These functions should all return future<bool> and at the end a
    // PlayerSavedEvent created with a conditional on the completion of all
    // the futures.
    storeCharacterPosition_(player_object, logout_type, clContainer);
    storeCharacterAttributes_(player_object, remove, logout_type, clContainer);
}

void WorldManager::storeCharacterPosition_(PlayerObject* player_object, WMLogOut logout_type, CharacterLoadingContainer* clContainer) {
    if(!player_object) {
        DLOG(WARNING) << "Trying to save character position with an invalid PlayerObject";
        return;
    }

    // Determine whether this is a save for a zone transfer, if so the location
    // we save will change.
    bool transfer = (logout_type == WMLogOut_Zone_Transfer);

    stringstream query_stream;

    query_stream << "UPDATE "<<mDatabase->galaxy()<<".characters SET parent_id=" << player_object->getParentId() << ", "
                 << "oX=" << player_object->mDirection.x << ", "
                 << "oY=" << player_object->mDirection.y << ", "
                 << "oZ=" << player_object->mDirection.z << ", "
                 << "oW=" << player_object->mDirection.w << ", "
                 << "x=" << (transfer ? clContainer->destination.x : player_object->mPosition.x) << ", "
                 << "y=" << (transfer ? clContainer->destination.y : player_object->mPosition.y) << ", "
                 << "z=" << (transfer ? clContainer->destination.z : player_object->mPosition.z) << ", "
                 << "planet_id=" << (transfer ? 0 : mZoneId) << ", "
                 << "jedistate=" << player_object->getJediState() << " "
                 << "WHERE id=" << player_object->getId();

    mDatabase->executeAsyncSql(query_stream.str());
}

void WorldManager::storeCharacterAttributes_(PlayerObject* player_object, bool remove, WMLogOut logout_type, CharacterLoadingContainer* clContainer) {
    if(!player_object) {
        DLOG(WARNING) << "Trying to save character position with an invalid PlayerObject";
        return;
    }

    Ham* ham = player_object->getHam();
    if(!ham) {
        DLOG(WARNING) << "Unable to retrieve Ham for player: [" << player_object->getId() << "]";
        return;
    }

    stringstream query_stream;

    query_stream << "UPDATE "<<mDatabase->galaxy()<<".character_attributes SET health_current=" << (ham->mHealth.getCurrentHitPoints() - ham->mHealth.getModifier()) << ", "
                 << "action_current=" << (ham->mAction.getCurrentHitPoints() - ham->mAction.getModifier()) << ", "
                 << "mind_current=" << (ham->mMind.getCurrentHitPoints() - ham->mMind.getModifier()) << ", "
                 << "health_wounds=" << ham->mHealth.getWounds() << ", "
                 << "strength_wounds=" << ham->mStrength.getWounds() << ", "
                 << "constitution_wounds=" << ham->mConstitution.getWounds() << ", "
                 << "action_wounds=" << ham->mAction.getWounds() << ", "
                 << "quickness_wounds=" << ham->mQuickness.getWounds() << ", "
                 << "stamina_wounds=" << ham->mStamina.getWounds() << ", "
                 << "mind_wounds=" << ham->mMind.getWounds() << ", "
                 << "focus_wounds=" << ham->mFocus.getWounds() << ", "
                 << "willpower_wounds=" << ham->mWillpower.getWounds() << ", "
                 << "battlefatigue=" << ham->getBattleFatigue() << ", "
                 << "posture=" << player_object->states.getPosture() << ", "
                 << "moodId=" << static_cast<uint16_t>(player_object->getMoodId()) << ", "
                 << "title='" << mDatabase->escapeString(player_object->getTitle().getAnsi()) << "', "
                 << "character_flags=" << player_object->getPlayerFlags() << ", "
                 << "states=" << player_object->states.getAction() << ", "
                 << "language=" << player_object->getLanguage() << ", "
                 << "new_player_exemptions=" <<  static_cast<uint16_t>(player_object->getNewPlayerExemptions()) << " "
                 << "WHERE character_id=" << player_object->getId();

    mDatabase->executeAsyncSql(query_stream.str(), [=, &clContainer] (DatabaseResult* result) {
        if(remove) {
            if(!player_object) {
                return;
            }

            GroupObject* group = gGroupManager->getGroupObject(player_object->getGroupId());
            if(group) {
                group->removePlayer(player_object->getId());
            }

            destroyObject(player_object);
        }

        if(logout_type == WMLogOut_Char_Load && !clContainer) {
            gObjectFactory->requestObject(ObjType_Player, 0, 0, clContainer->ofCallback, clContainer->mPlayerId, clContainer->mClient);
            SAFE_DELETE(clContainer);
        }
    });
}
//======================================================================================================================

void WorldManager::savePlayerSync(uint32 accId,bool remove)
{
    PlayerObject* playerObject = getPlayerByAccId(accId);
    Ham* ham = playerObject->getHam();

    mDatabase->destroyResult(mDatabase->executeSynchSql("UPDATE %s.characters SET parent_id=%" PRIu64 ",oX=%f,oY=%f,oZ=%f,oW=%f,x=%f,y=%f,z=%f,planet_id=%u WHERE id=%" PRIu64 "",
                              mDatabase->galaxy(),playerObject->getParentId()
                             ,playerObject->mDirection.x,playerObject->mDirection.y,playerObject->mDirection.z,playerObject->mDirection.w
                             ,playerObject->mPosition.x,playerObject->mPosition.y,playerObject->mPosition.z
                             ,mZoneId,playerObject->getId()));


    mDatabase->destroyResult(mDatabase->executeSynchSql("UPDATE %s.character_attributes SET health_current=%u,action_current=%u,mind_current=%u"
                             ",health_wounds=%u,strength_wounds=%u,constitution_wounds=%u,action_wounds=%u,quickness_wounds=%u"
                             ",stamina_wounds=%u,mind_wounds=%u,focus_wounds=%u,willpower_wounds=%u,battlefatigue=%u,posture=%u,moodId=%u,title=\'%s\'"
                             ",character_flags=%u,states=%" PRIu64 ",language=%u, group_id=%" PRIu64 " WHERE character_id=%" PRIu64 "",
                             mDatabase->galaxy(),ham->mHealth.getCurrentHitPoints() - ham->mHealth.getModifier(), //Llloydyboy Added the -Modifier so that when buffs are reinitialised, it doesn't screw up HAM
                             ham->mAction.getCurrentHitPoints() - ham->mAction.getModifier(), //Llloydyboy Added the -Modifier so that when buffs are reinitialised, it doesn't screw up HAM
                             ham->mMind.getCurrentHitPoints() - ham->mMind.getModifier(),	 //Llloydyboy Added the -Modifier so that when buffs are reinitialised, it doesn't screw up HAM
                             ham->mHealth.getWounds(),
                             ham->mStrength.getWounds(),
                             ham->mConstitution.getWounds(),
                             ham->mAction.getWounds(),
                             ham->mQuickness.getWounds(),
                             ham->mStamina.getWounds(),
                             ham->mMind.getWounds(),
                             ham->mFocus.getWounds(),
                             ham->mWillpower.getWounds(),
                             ham->getBattleFatigue(),
                             playerObject->states.getPosture(),
                             playerObject->getMoodId(),
                             playerObject->getTitle().getAnsi(),
                             playerObject->getPlayerFlags(),
                             playerObject->states.getAction(),
                             playerObject->getLanguage(),
                             playerObject->getGroupId(),
                             playerObject->getId()));

    gBuffManager->SaveBuffs(playerObject, GetCurrentGlobalTick());
    if(remove)
        destroyObject(playerObject);
}

//======================================================================================================================
// here is where we change how often a player automatically saves
// TODO: add in server config how often they can save
bool WorldManager::checkSavePlayer(PlayerObject* playerObject)
{
    return (playerObject->getSaveTimer() >= 12000);
}
//======================================================================================================================

PlayerObject*	WorldManager::getPlayerByAccId(uint32 accId)
{
    PlayerAccMap::iterator it = mPlayerAccMap.find(accId);

    if(it != mPlayerAccMap.end())
    {
        return(PlayerObject*)((*it).second);
    }

    return(NULL);
}

//======================================================================================================================

void WorldManager::addDisconnectedPlayer(PlayerObject* playerObject)
{
    uint32 timeOut = gWorldConfig->getConfiguration<uint32>("Zone_Player_Logout",300);

    // Halt the tutorial scripts, if running.
    playerObject->stopTutorial();


    Datapad* datapad			= playerObject->getDataPad();

    if(playerObject->getMount() && datapad)
    {
        if(VehicleController* datapad_pet = dynamic_cast<VehicleController*>(datapad->getDataById(playerObject->getMount()->controller())))
        {
            datapad_pet->Store();
        }
    }

    // Delete private owned spawned objects, like npc's in the Tutorial.
    uint64 privateOwnedObjectId = ScriptSupport::Instance()->getObjectOwnedBy(playerObject->getId());
    while (privateOwnedObjectId != 0)
    {
        // Delete the object ref from script support.
        ScriptSupport::Instance()->eraseObject(privateOwnedObjectId);

        // We did have a private npc. Let us delete him/her/that.
        if (Object* object = getObjectById(privateOwnedObjectId))
        {
            // But first, remove npc from our defender list.
            playerObject->removeDefenderAndUpdateList(object->getId());

            destroyObject(object);
            // gLogger->log(LogManager::DEBUG,"WorldManager::addDisconnectedPlayer Deleted object with id  %" PRIu64 "",privateOwnedObjectId);
        }

        privateOwnedObjectId = ScriptSupport::Instance()->getObjectOwnedBy(playerObject->getId());
    }

    removeObjControllerToProcess(playerObject->getController()->getTaskId());
    removeCreatureHamToProcess(playerObject->getHam()->getTaskId());
    removeCreatureStomachToProcess(playerObject->getStomach()->mDrinkTaskId);
    removeCreatureStomachToProcess(playerObject->getStomach()->mFoodTaskId);
    removeEntertainerToProcess(playerObject->getEntertainerTaskId());

    gCraftingSessionFactory->destroySession(playerObject->getCraftingSession());
    playerObject->setCraftingSession(NULL);
    gStateManager.removeActionState(playerObject, CreatureState_Crafting);

    //despawn camps ??? - every reference is over id though

    playerObject->getController()->setTaskId(0);
    playerObject->getHam()->setTaskId(0);
    playerObject->setSurveyState(false);
    playerObject->setSamplingState(false);
    playerObject->togglePlayerFlagOn(PlayerFlag_LinkDead);
    playerObject->setConnectionState(PlayerConnState_LinkDead);
    playerObject->setDisconnectTime(timeOut);

    mPlayersToRemove.push_back(playerObject);

    gMessageLib->sendUpdatePlayerFlags(playerObject);
}

//======================================================================================================================

void WorldManager::addReconnectedPlayer(PlayerObject* playerObject)
{
    uint32 timeOut = gWorldConfig->getConfiguration<uint32>("Zone_Player_Logout",300);

    playerObject->togglePlayerFlagOff(PlayerFlag_LinkDead);
    playerObject->setConnectionState(PlayerConnState_Connected);

    // Restart the tutorial.
    playerObject->startTutorial();

    playerObject->setDisconnectTime(timeOut);

    // resetting move, save and tickcounters
    playerObject->setInMoveCount(0);
    playerObject->setClientTickCount(0);
    playerObject->setSaveTimer(0);

    removePlayerFromDisconnectedList(playerObject);
}

//======================================================================================================================

void WorldManager::removePlayerFromDisconnectedList(PlayerObject* playerObject)
{
    PlayerList::iterator it;

    it = std::find(mPlayersToRemove.begin(),mPlayersToRemove.end(),playerObject);
    if(it == mPlayersToRemove.end())
    {
        DLOG(INFO) << "WorldManager::addReconnectedPlayer: Error removing Player from Disconnected List: " << playerObject->getId();
    }
    else
    {
        mPlayersToRemove.erase(it);
    }
}

//======================================================================================================================
//
// wide range move on the same planet
//

void WorldManager::warpPlanet(PlayerObject* playerObject, const glm::vec3& destination, uint64 parentId, const glm::quat& direction)
{
    // remove from cell if we are in one / SI
	gSpatialIndexManager->RemoveObjectFromWorld(playerObject);
	
	//we've removed the taskId, now lets reset the Id
	playerObject->getHam()->setTaskId(0);

    //
// Handle update of player movements. We need to have a consistent update of the world around us,
// even we we are not moving in world.
// this is especially important when we are limiting object creates to say 50 objects per second

	// reset player properties
	playerObject->resetProperties();

	playerObject->setParentId(parentId);
	playerObject->mPosition		= destination;
	playerObject->mDirection	= direction;

	// start the new scene
	gMessageLib->sendStartScene(mZoneId,playerObject);
	gMessageLib->sendServerTime(gWorldManager->getServerTime(),playerObject->getClient());

	// add us to the cell should we be in one
	gSpatialIndexManager->createInWorld(playerObject);

	// initialize at new position
	gSpatialIndexManager->sendCreatePlayer(playerObject,playerObject);

	// initialize ham regeneration
	playerObject->getHam()->checkForRegen();
	playerObject->getStomach()->checkForRegen();
}
//======================================================================================================================
//
// Handles the saving of all players on a fixed interval
// eventually we will put some logic to only save x players at a time
// in this once the server becomes more stable
//
bool	WorldManager::_handlePlayerSaveTimers(uint64 callTime, void* ref)
{
    //uint32 playerCount = mPlayerAccMap.size();
    //// don't save all players if > 100
    uint32 playerSaveCount = 0;
    PlayerAccMap::iterator playerIt = mPlayerAccMap.begin();
    while (playerIt != mPlayerAccMap.end())
    {
        const PlayerObject* const playerObject = (*playerIt).second;
        if (playerObject)
        {
            if (playerObject->isConnected())
            {
                // TODO: don't save all players if > x players (100ish)
                // set the timer to save rest of players again
                // TODO: check if player has saved recently
                // save player async
                gWorldManager->savePlayer(playerObject->getAccountId(), false, WMLogOut_No_LogOut);
                ++playerSaveCount;
            }

        }

        ++playerIt;
    }
    LOG(WARNING) << "Periodic Save of "<< playerSaveCount <<" Players";
    return true;
}
//======================================================================================================================
//
//	Add a timed entry for cloning of dead player objects.
//

void WorldManager::addPlayerObjectForTimedCloning(uint64 playerId, uint64 when)
{
    uint64 expireTime = Anh_Utils::Clock::getSingleton()->getLocalTime();

    mPlayerObjectReviveMap.insert(std::make_pair(playerId, expireTime + when));
}

//======================================================================================================================
//
//	Remove a timed entry for cloning of dead player objects.
//

void WorldManager::removePlayerObjectForTimedCloning(uint64 playerId)
{
    PlayerObjectReviveMap::iterator it = mPlayerObjectReviveMap.find(playerId);
    if (it != mPlayerObjectReviveMap.end())
    {
        // Remove player.
        mPlayerObjectReviveMap.erase(it);
    }
}