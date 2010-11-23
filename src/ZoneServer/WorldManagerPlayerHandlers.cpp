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

#include "MountObject.h"
#include "PlayerObject.h"
#include "WorldManager.h"
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
#include "Heightmap.h"
#include "MissionManager.h"
#include "NpcManager.h"
#include "NPCObject.h"
#include "PlayerStructure.h"
#include "ResourceManager.h"
#include "SchematicManager.h"
#include "StateManager.h"
#include "TreasuryManager.h"
#include "VehicleController.h"
#include "WorldConfig.h"
#include "ZoneOpcodes.h"
#include "ZoneServer.h"
#include "ZoneTree.h"
#include "HarvesterFactory.h"
#include "HarvesterObject.h"
#include "FactoryFactory.h"
#include "FactoryObject.h"
#include "Inventory.h"
#include "MissionObject.h"
#include "ObjectFactory.h"
#include "QuadTree.h"
#include "Shuttle.h"
#include "TicketCollector.h"
#include "Common/ConfigManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/DatabaseResult.h"
#include "MessageLib/MessageLib.h"
#include "ScriptEngine/ScriptEngine.h"
#include "ScriptEngine/ScriptSupport.h"
#include "Utils/Scheduler.h"
#include "Utils/VariableTimeScheduler.h"
#include "Utils/utils.h"

//======================================================================================================================

void  WorldManager::initPlayersInRange(Object* object,PlayerObject* player)
{
    // we still query for players here, cause they are found through the buildings and arent kept in a qtree
    ObjectSet inRangeObjects;
    mSpatialIndex->getObjectsInRange(object,&inRangeObjects,(ObjType_Player),gWorldConfig->getPlayerViewingRange());

    // query the according qtree, if we are in one
    if(object->getSubZoneId())
    {
        if(std::shared_ptr<QTRegion>region = getQTRegion(object->getSubZoneId()))
        {
            float				viewingRange	= _GetMessageHeapLoadViewingRange();
            //float				viewingRange	= (float)gWorldConfig->getPlayerViewingRange();
            Anh_Math::Rectangle qRect;

            if(!object->getParentId())
            {
                qRect = Anh_Math::Rectangle(object->mPosition.x - viewingRange,object->mPosition.z - viewingRange,viewingRange * 2,viewingRange * 2);
            }
            else
            {
                CellObject*		cell		= dynamic_cast<CellObject*>(getObjectById(object->getParentId()));
                BuildingObject* building	= dynamic_cast<BuildingObject*>(getObjectById(cell->getParentId()));

                qRect = Anh_Math::Rectangle(building->mPosition.x - viewingRange,building->mPosition.z - viewingRange,viewingRange * 2,viewingRange * 2);
            }

            region->mTree->getObjectsInRange(object,&inRangeObjects,ObjType_Player,&qRect);
        }
    }

    // iterate through the results
    ObjectSet::iterator it = inRangeObjects.begin();

    while(it != inRangeObjects.end())
    {
        PlayerObject* pObject = dynamic_cast<PlayerObject*>(*it);

        if(pObject)
        {
            if(pObject != player)
            {
                gMessageLib->sendCreateObject(object,pObject);
                pObject->addKnownObjectSafe(object);
                object->addKnownObjectSafe(pObject);
            }

        }

        ++it;
    }

}


//======================================================================================================================

void WorldManager::savePlayer(uint32 accId,bool remove, WMLogOut mLogout, CharacterLoadingContainer* clContainer)
{
    PlayerObject* playerObject			= getPlayerByAccId(accId);
    if(!playerObject) {
        DLOG(INFO) << "WorldManager::savePlayer could not find player with AccId:" <<accId<<", save aborted.";
        return;
    }

    switch (mLogout)
    {
    case WMLogOut_LogOut:
    case WMLogOut_Char_Load:
        {
            // update PlayerAttributes
            _updatePlayerAttributesToDB(accId);
            _updatePlayerLogoutToDB(accId, clContainer);
        }
        break;

    case WMLogOut_No_LogOut:
    case WMLogOut_Zone_Transfer:
        //start by saving the buffs the buffmanager will deal with the buffspecific db callbacks and start the position safe at their end
        //which will return its callback to the worldmanager
        //if no buff was there to be saved we will continue directly
        gBuffManager->SaveBuffs(playerObject, GetCurrentGlobalTick());
        _updatePlayerAttributesToDB(accId);
        gObjectFactory->requestObject(ObjType_Player,0,0,clContainer->ofCallback,clContainer->mPlayerId,clContainer->mClient);
        //now destroy the clContainer
        SAFE_DELETE(clContainer);
        
        _updatePlayerPositionToDB(accId);
        break;
    }
}

//======================================================================================================================

void WorldManager::savePlayerSync(uint32 accId,bool remove)
{
    PlayerObject* playerObject = getPlayerByAccId(accId);
    Ham* ham = playerObject->getHam();

    mDatabase->destroyResult(mDatabase->executeSynchSql("UPDATE characters SET parent_id=%"PRIu64",oX=%f,oY=%f,oZ=%f,oW=%f,x=%f,y=%f,z=%f,planet_id=%u WHERE id=%"PRIu64"",playerObject->getParentId()
                             ,playerObject->mDirection.x,playerObject->mDirection.y,playerObject->mDirection.z,playerObject->mDirection.w
                             ,playerObject->mPosition.x,playerObject->mPosition.y,playerObject->mPosition.z
                             ,mZoneId,playerObject->getId()));
   

    mDatabase->destroyResult(mDatabase->executeSynchSql("UPDATE character_attributes SET health_current=%u,action_current=%u,mind_current=%u"
                             ",health_wounds=%u,strength_wounds=%u,constitution_wounds=%u,action_wounds=%u,quickness_wounds=%u"
                             ",stamina_wounds=%u,mind_wounds=%u,focus_wounds=%u,willpower_wounds=%u,battlefatigue=%u,posture=%u,moodId=%u,title=\'%s\'"
                             ",character_flags=%u,states=%"PRIu64",language=%u, group_id=%"PRIu64" WHERE character_id=%"PRIu64"",
                             ham->mHealth.getCurrentHitPoints() - ham->mHealth.getModifier(), //Llloydyboy Added the -Modifier so that when buffs are reinitialised, it doesn't screw up HAM
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
            // gLogger->log(LogManager::DEBUG,"WorldManager::addDisconnectedPlayer Deleted object with id  %"PRIu64"",privateOwnedObjectId);
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

    //add to the disconnect list
    addPlayerToDisconnectedList(playerObject);

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

void WorldManager::addPlayerToDisconnectedList(PlayerObject* playerObject)
{
    PlayerList::iterator it;
    it = std::find(mPlayersToRemove.begin(),mPlayersToRemove.end(),playerObject);
    if( it == mPlayersToRemove.end())
    {
        mPlayersToRemove.push_back(playerObject);
    }
    else
    {
        DLOG(INFO) << "WorldManager::addPlayerToDisconnectedList: Error adding Player : already on List: " << playerObject->getId();
    }
}

//======================================================================================================================
//
// wide range move on the same planet
//

void WorldManager::warpPlanet(PlayerObject* playerObject, const glm::vec3& destination, uint64 parentId, const glm::quat& direction)
{
    removePlayerMovementUpdateTime(playerObject);

    // remove player from objects in his range.
    playerObject->destroyKnownObjects();

    // remove from cell / SI
    if(playerObject->getParentId())
    {
        if(CellObject* cell = dynamic_cast<CellObject*>(getObjectById(playerObject->getParentId())))
        {
            cell->removeObject(playerObject);
        }
        else
        {
            DLOG(INFO) << "WorldManager::removePlayer: couldn't find cell " << playerObject->getParentId();
        }
    }
    else
    {
        if(playerObject->getSubZoneId())
        {
            if(std::shared_ptr<QTRegion> region = getQTRegion(playerObject->getSubZoneId()))
            {
                playerObject->setSubZoneId(0);
                region->mTree->removeObject(playerObject);
            }
        }
    }

    // remove any timers running
    //why remove that ?
    //removeObjControllerToProcess(playerObject->getController()->getTaskId());
    //playerObject->getController()->clearQueues();
    //playerObject->getController()->setTaskId(0);

    //why remove that ?
    removeCreatureHamToProcess(playerObject->getHam()->getTaskId());
    removeCreatureStomachToProcess(playerObject->getStomach()->mDrinkTaskId);
    removeCreatureStomachToProcess(playerObject->getStomach()->mFoodTaskId);
    //we've removed the taskId, now lets reset the Id
    playerObject->getHam()->setTaskId(0);

    // reset player properties
    playerObject->resetProperties();

    playerObject->setParentId(parentId);
    playerObject->mPosition		= destination;
    playerObject->mDirection	= direction;

    // start the new scene
    gMessageLib->sendStartScene(mZoneId,playerObject);
    gMessageLib->sendServerTime(gWorldManager->getServerTime(),playerObject->getClient());

    // add us to cell / SI
    if(parentId)
    {
        if(CellObject* cell = dynamic_cast<CellObject*>(getObjectById(parentId)))
        {
            cell->addObjectSecure(playerObject);
        }
        else
        {
            DLOG(INFO) << "WorldManager::warpPlanet: couldn't find cell " << parentId;
        }
    }
    else
    {
        if(std::shared_ptr<QTRegion> region = mSpatialIndex->getQTRegion(playerObject->mPosition.x,playerObject->mPosition.z))
        {
            playerObject->setSubZoneId((uint32)region->getId());
            region->mTree->addObject(playerObject);
        }
        else
        {
            // we should never get here !
            return;
        }
    }

    // initialize objects in range
    initObjectsInRange(playerObject);

    // initialize at new position
    gMessageLib->sendCreatePlayer(playerObject,playerObject);

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
    //setSaveTaskId(mSubsystemScheduler->addTask(fastdelegate::MakeDelegate(this,&WorldManager::_handlePlayerSaveTimers), 4, 60000, NULL));
    return true;
}
//======================================================================================================================
//
// Handle update of player movements. We need to have a consistent update of the world around us,
// even we we are not moving in world.
// this is especially important when we are limiting object creates to say 50 objects per second

bool WorldManager::_handlePlayerMovementUpdateTimers(uint64 callTime, void* ref)
{
    PlayerMovementUpdateMap::iterator it = mPlayerMovementUpdateMap.begin();

    while (it != mPlayerMovementUpdateMap.end())
    {
        PlayerObject* player = dynamic_cast<PlayerObject*>(getObjectById((*it).first));
        if (player)
        {
            if (player->isConnected())
            {
                //  The timer has expired?
                if (callTime >= ((*it).second))
                {
                    // Yes, handle it.

                    ObjectController* ObjCtl = player->getController();

                    uint64 next = ObjCtl->playerWorldUpdate(false);
                    mPlayerMovementUpdateMap.erase(it++);
                    if (next)
                    {
                        // Add next scheduled update.
                        addPlayerMovementUpdateTime(player, next);
                    }
                }
                else
                {
                    ++it;
                }
            }
            else
            {
                // Remove the disconnected...
                mPlayerMovementUpdateMap.erase(it++);
            }
        }
        else
        {
            // Remove the disconnected...
            mPlayerMovementUpdateMap.erase(it++);
        }
    }
    return (true);
}


//======================================================================================================================
//
//	Add a timer entry for updating of players known objects.
//

void WorldManager::addPlayerMovementUpdateTime(PlayerObject* player, uint64 when)
{
    uint64 expireTime = Anh_Utils::Clock::getSingleton()->getLocalTime();
    // gLogger->log(LogManager::DEBUG,"Adding new at %"PRIu64"", expireTime + when);
    mPlayerMovementUpdateMap.insert(std::make_pair(player->getId(), expireTime + when));
}

//======================================================================================================================
//
//	Remove timer entry for player.
//

void WorldManager::removePlayerMovementUpdateTime(PlayerObject* player)
{
    if (player)
    {
        PlayerMovementUpdateMap::iterator it = mPlayerMovementUpdateMap.find(player->getId());
        while (it != mPlayerMovementUpdateMap.end())
        {
            // Remove the disconnected...
            mPlayerMovementUpdateMap.erase(it);
            it = mPlayerMovementUpdateMap.find(player->getId());
        }
    }
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