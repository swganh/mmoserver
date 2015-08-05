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

#include <cassert>

#include <cppconn/resultset.h>

#ifdef WIN32
#undef ERROR
#endif
#include "Utils/logger.h"

#include "Utils/Scheduler.h"
#include "Utils/VariableTimeScheduler.h"
#include "Utils/utils.h"

#include "Common/Crc.h"

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
#include "FactoryFactory.h"
#include "FactoryObject.h"
#include "FireworkManager.h"
#include "ForageManager.h"
#include "GroupManager.h"
#include "GroupObject.h"
#include "HarvesterFactory.h"
#include "HarvesterObject.h"
#include "Heightmap.h"
#include "Inventory.h"
#include "MissionManager.h"
#include "MissionObject.h"
#include "NpcManager.h"
#include "NPCObject.h"
#include "ObjectFactory.h"
#include "PlayerObject.h"
#include "PlayerStructure.h"
#include "ResourceManager.h"
#include "SchematicManager.h"
#include "Shuttle.h"
#include "SpatialIndexManager.h"
#include "TicketCollector.h"
#include "TreasuryManager.h"
#include "WorldConfig.h"
#include "ZoneOpcodes.h"
#include "ZoneServer.h"

using std::dynamic_pointer_cast;
using std::shared_ptr;

//======================================================================================================================

bool			WorldManager::mInsFlag    = false;
WorldManager*	WorldManager::mSingleton  = NULL;
//======================================================================================================================

WorldManager::WorldManager(uint32 zoneId,ZoneServer* zoneServer,Database* database, uint16 heightmapResolution, bool writeResourceMaps, std::string zoneName)
    : mWM_DB_AsyncPool(sizeof(WMAsyncContainer))
    , mDatabase(database)
    , mZoneServer(zoneServer)
    , mState(WMState_StartUp)
    , mServerTime(0)
    , mTotalObjectCount(0)
    , mZoneId(zoneId)
	, mHeightmapResolution(heightmapResolution)
{
    DLOG(INFO) << "WorldManager initialization";

	SpatialIndexManager::Init(mDatabase);


    // load planet names and terrain files so we can start heightmap loading
    _loadPlanetNamesAndFiles();

    // create schedulers
    mSubsystemScheduler		= new Anh_Utils::Scheduler();
    mObjControllerScheduler = new Anh_Utils::Scheduler();
    mHamRegenScheduler		= new Anh_Utils::Scheduler();
    mStomachFillingScheduler= new Anh_Utils::Scheduler();
    mPlayerScheduler		= new Anh_Utils::Scheduler();
    mEntertainerScheduler	= new Anh_Utils::Scheduler();
    //mImagedesignerScheduler	= new Anh_Utils::Scheduler();
    mBuffScheduler			= new Anh_Utils::VariableTimeScheduler(100, 100);
    mMissionScheduler		= new Anh_Utils::Scheduler();
    mNpcManagerScheduler	= new Anh_Utils::Scheduler();
    mAdminScheduler			= new Anh_Utils::Scheduler();

    LoadCurrentGlobalTick();

    // load up subsystems

    SkillManager::Init(database);
    SchematicManager::Init(database);

    //the resourcemanager gets accessed by lowlevel functions to check the IDs we get send by the client
    //it will have to be initialized in the tutorial, too
    if(zoneId != 41) {
		ResourceManager::Init(database,mZoneId, writeResourceMaps, zoneName);
    } else {
        //by not assigning a db we force the resourcemanager to not load db data
        ResourceManager::Init(NULL,mZoneId, writeResourceMaps, zoneName);
    }
    TreasuryManager::Init(database);
    ConversationManager::Init(database);
    CraftingSessionFactory::Init(database);
    /*if(zoneId != 41)
        MissionManager::Init(database,mZoneId);*/

    // register world script hooks
    _registerScriptHooks();

    // initiate loading of objects
    int8 sql[128];
    sprintf(sql, "SELECT %s.sf_getZoneObjectCount(%i);", mDatabase->galaxy(), mZoneId);
    mDatabase->executeAsyncSql(sql, [=] (DatabaseResult* result) {
        std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();
        if (!result_set->next())
        {
            return;
        }
        // we got the total objectCount we need to load
        mTotalObjectCount = result_set->getUInt(1);
        LOG(INFO) << "Loading " << mTotalObjectCount << " World Manager Objects... ";

        _loadWorldObjects();
    } ) ;

#if defined(_MSC_VER)
    mNonPersistantId =   422212465065984;
#else
    mNonPersistantId =   422212465065984LLU;
#endif
}

//======================================================================================================================

WorldManager*	WorldManager::Init(uint32 zoneId,ZoneServer* zoneServer,Database* database, uint16 heightmapResolution, bool writeResourceMaps, std::string zoneName)
{
    if(!mInsFlag)
    {
        mSingleton = new WorldManager(zoneId,zoneServer,database, heightmapResolution, writeResourceMaps, zoneName);
        mInsFlag = true;
        return mSingleton;
    }
    else
        return mSingleton;
}

//======================================================================================================================

void WorldManager::Shutdown()
{
    // clear scripts
    ScriptList::iterator scriptIt = mWorldScripts.begin();

    while(scriptIt != mWorldScripts.end())
    {
        gScriptEngine->removeScript(*scriptIt);
        scriptIt = mWorldScripts.erase(scriptIt);
    }

    // objects
    PlayerAccMap::iterator playerIt = mPlayerAccMap.begin();
    while(! mPlayerAccMap.empty())
    {
        const PlayerObject* player = (*playerIt).second;
        destroyObject((Object*)player);
        // destroying the referenced object seems to invalidate our iterator
        playerIt = mPlayerAccMap.begin();
    }

    // timers
    delete(mAdminScheduler);
    delete(mNpcManagerScheduler);
    delete(mObjControllerScheduler);
    delete(mStomachFillingScheduler);
    delete(mHamRegenScheduler);
    delete(mMissionScheduler);
    delete(mPlayerScheduler);
    //delete(mImagedesignerScheduler);
    delete(mEntertainerScheduler);
    delete(mBuffScheduler);



    // we need to destroy that after the (player) objects!
    // as the playerobjects try to remove the Objectcontroller scheduler and crash us if the scheduler isnt existent anymore
    delete(mSubsystemScheduler);

    mPlayersToRemove.clear();

    // Npc conversation timers.
    mNpcConversionTimers.clear();

    mCreatureObjectDeletionMap.clear();
    mPlayerObjectReviveMap.clear();

    mNpcDormantHandlers.clear();
    mNpcReadyHandlers.clear();
    mNpcActiveHandlers.clear();
    mAdminRequestHandlers.clear();

    // Handle creature spawn regions. These objects are not registred in the normal object map.
    CreatureSpawnRegionMap::iterator it = mCreatureSpawnRegionMap.begin();
    /*while (it != mCreatureSpawnRegionMap.end())
    {
        mCreatureSpawnRegionMap.erase(it++);
    }*/
    mCreatureSpawnRegionMap.clear();

    NpcManager::deleteManager();

    Heightmap::deleter();

	mCreatureObjectDeletionMap.clear();
	mPlayerObjectReviveMap.clear();

	// remove all cells and factories first so we dont get a racecondition with their content 
	// when clearing the mainObjectMap
	ObjectIDList::iterator itStruct = mStructureList.begin();
	while(itStruct != mStructureList.end())
	{
		ObjectMap::iterator objMapIt = mObjectMap.find(*itStruct);

		if(objMapIt != mObjectMap.end())
		{
			mObjectMap.erase(objMapIt);
		}
		itStruct++;
	}

	// shutdown SI
	gSpatialIndexManager->Shutdown();
	//delete(mSpatialIndex);
}

//======================================================================================================================

WorldManager::~WorldManager()
{
    mInsFlag = false;
    delete(mSingleton);
}
//======================================================================================================================

void WorldManager::handleObjectReady(Object* object,DispatchClient* client)
{
    addObject(object);

    // check if we done loading
    if ((mState == WMState_StartUp) && (mObjectMap.size() + mCreatureSpawnRegionMap.size() >= mTotalObjectCount))
    {
        _handleLoadComplete();
    }
}
void WorldManager::handleObjectReady(shared_ptr<Object> object)
{
    addObject(object);
}

//======================================================================================================================

//======================================================================================================================
//get the current tick
//

uint64 WorldManager::GetCurrentGlobalTick()
{
    return mTick;
}

//======================================================================================================================
//still synch issues to adress with other servers
//

void WorldManager::LoadCurrentGlobalTick()
{
    uint64 Tick;
    DatabaseResult* temp = mDatabase->executeSynchSql("SELECT Global_Tick_Count FROM %s.galaxy WHERE galaxy_id = '2'",mDatabase->galaxy());


    DataBinding*	tickbinding = mDatabase->createDataBinding(1);
    tickbinding->addField(DFT_uint64,0,8,0);

    temp->getNextRow(tickbinding,&Tick);
    mDatabase->destroyDataBinding(tickbinding);
    mDatabase->destroyResult(temp);


    LOG(INFO) << "Current global tick count [" << Tick << "]";
    mTick = Tick;
    mSubsystemScheduler->addTask(fastdelegate::MakeDelegate(this,&WorldManager::_handleTick),7,1000,NULL);
}

//======================================================================================================================
//
//

bool	WorldManager::_handleTick(uint64 callTime,void* ref)
{
    mTick += 1000;
    return true;
}

//======================================================================================================================
//
//

Object*	WorldManager::getObjectById(uint64 objId)
{
    ObjectMap::iterator it = mObjectMap.find(objId);

    if(it != mObjectMap.end())
    {
        return((*it).second);
    }

    return(NULL);
}



//======================================================================================================================

void WorldManager::Process()
{
    _processSchedulers();
}

//======================================================================================================================

void WorldManager::_processSchedulers()
{
    mHamRegenScheduler->process();
    mStomachFillingScheduler->process();
    mSubsystemScheduler->process();
    mObjControllerScheduler->process();
    //mImagedesignerScheduler->process();
    mPlayerScheduler->process();
    mEntertainerScheduler->process();
    mBuffScheduler->process();
    mMissionScheduler->process();
    mNpcManagerScheduler->process();
    mAdminScheduler->process();
}

//======================================================================================================================

bool WorldManager::_handleDisconnectUpdate(uint64 callTime,void* ref)
{
    PlayerList::iterator it = mPlayersToRemove.begin();

    while(it != mPlayersToRemove.end())
    {
        PlayerObject* playerObject = (*it);

        // we timed out, so save + remove it
        if(--*(playerObject->getDisconnectTime()) <= 0 && playerObject->isLinkDead())
        {
            // reset link dead state
            playerObject->togglePlayerFlagOff(PlayerFlag_LinkDead);
            playerObject->setConnectionState(PlayerConnState_Destroying);

			//remove the player out of his group - if any
			GroupObject* group = gGroupManager->getGroupObject(playerObject->getGroupId());
			if(group)
			{
				group->removePlayer(playerObject->getId());
			}

            //asynch save
            savePlayer(playerObject->getAccountId(),true,WMLogOut_LogOut);

            it = mPlayersToRemove.erase(it);
        }
        else
            ++it;

    }

    return(true);
}

//======================================================================================================================

bool WorldManager::_handleShuttleUpdate(uint64 callTime,void* ref)
{
    ShuttleList::iterator shuttleIt = mShuttleList.begin();
    while(shuttleIt != mShuttleList.end())
    {
        Shuttle* shuttle = (*shuttleIt);

        // The Ticket Collector need a valid shuttle-object.
        if (!shuttle->ticketCollectorEnabled())
        {
            TicketCollector* collector = dynamic_cast<TicketCollector*>(getObjectById(shuttle->getCollectorId()));
            if (collector)
            {
                if (!collector->getShuttle())
                {
                    // Enable the collector.
                    collector->setShuttle(shuttle);
                }
                shuttle->ticketCollectorEnable();
            }
        }

        switch(shuttle->getShuttleState())
        {
        case ShuttleState_Away:
        {
            uint32 awayTime = shuttle->getAwayTime() + 1000;
            if(awayTime >= shuttle->getAwayInterval())
            {
                uint32 awayTime = shuttle->getAwayTime() + 1000;
                if(awayTime >= shuttle->getAwayInterval())
                {
                    shuttle->states.setPosture(0);
                    shuttle->setAwayTime(0);
                    shuttle->setShuttleState(ShuttleState_AboutBoarding);
                    gMessageLib->sendPostureUpdate(shuttle);
                    gMessageLib->sendCombatAction(shuttle,NULL,opChange_Posture);
                }
            }
            else
                shuttle->setAwayTime(awayTime);
        }
        break;

        case ShuttleState_Landing:
        {
            uint32 landingTime = shuttle->getLandingTime() + 1000;

            if(landingTime >= SHUTTLE_LANDING_ANIMATION_TIME - 5000)
            {
                shuttle->setShuttleState(ShuttleState_AboutBoarding);
            }
            else
                shuttle->setLandingTime(landingTime);
        }
        break;

        case ShuttleState_AboutBoarding:
        {
            uint32 landingTime = shuttle->getLandingTime() + 1000;

            if(landingTime >= SHUTTLE_LANDING_ANIMATION_TIME)
            {
                shuttle->setLandingTime(0);

                shuttle->setShuttleState(ShuttleState_InPort);
            }
            else
                shuttle->setLandingTime(landingTime);
        }
        break;

        case ShuttleState_InPort:
        {
            uint32 inPortTime = shuttle->getInPortTime() + 1000;
            if(inPortTime >= shuttle->getInPortInterval())
            {
                uint32 inPortTime = shuttle->getInPortTime() + 1000;
                if(inPortTime >= shuttle->getInPortInterval())
                {
                    shuttle->setInPortTime(0);
                    shuttle->setShuttleState(ShuttleState_Away);
                    shuttle->states.setPosture(2);
                    gMessageLib->sendPostureUpdate(shuttle);
                    gMessageLib->sendCombatAction(shuttle,NULL,opChange_Posture);
                }
            }
            else
            {
                shuttle->setInPortTime(inPortTime);
            }
        }
        break;

        default:
            break;
        }

        ++shuttleIt;
    }

    return(true);
}

//======================================================================================================================
//
// update the current planet time
//
bool WorldManager::_handleServerTimeUpdate(uint64 callTime,void* ref)
{
    mServerTime += gWorldConfig->getServerTimeInterval() + gWorldConfig->getServerTimeSpeed();

    PlayerAccMap::iterator playerIt = mPlayerAccMap.begin();
    while(playerIt != mPlayerAccMap.end())
    {
        const PlayerObject* const playerObject = (*playerIt).second;

        if (playerObject->isConnected())
        {
            gMessageLib->sendServerTime(mServerTime,playerObject->getClient());
        }

        ++playerIt;
    }

    return(true);
}

//======================================================================================================================
//
// update busy crafting tools, called every 2 seconds
//
bool WorldManager::_handleCraftToolTimers(uint64 callTime,void* ref)
{
    CraftTools::iterator it = mBusyCraftTools.begin();

    while(it != mBusyCraftTools.end())
    {
        CraftingTool*	tool	=	dynamic_cast<CraftingTool*>(getObjectById((*it)));
        if(!tool)
        {
            LOG(ERR) << "Missing crafting tool";
            it = mBusyCraftTools.erase(it);
            continue;
        }

        PlayerObject*	player	=	dynamic_cast<PlayerObject*>(getObjectById(tool->getParentId() - 1));
        Item*			item	= tool->getCurrentItem();

        if(player)
        {
            // we done, create the item
            if(!tool->updateTimer(callTime))
            {
                // add it to the world, if it holds an item
                if(item)
                {
                    Inventory* temp =  dynamic_cast<Inventory*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
                    if(!temp) continue;

                    item->setParentId(temp->getId());
                    dynamic_cast<Inventory*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory))->addObject(item);
                    gWorldManager->addObject(item,true);

                    gMessageLib->sendCreateTano(item,player);

                    gMessageLib->SendSystemMessage(::common::OutOfBand("system_msg", "prototype_transferred"), player);

                    tool->setCurrentItem(NULL);
                }
                //in case of logout/in interplanetary travel it will be in the inventory already

                gMessageLib->sendUpdateTimer(tool,player);

                it = mBusyCraftTools.erase(it);
                tool->setAttribute("craft_tool_status","@crafting:tool_status_ready");
                mDatabase->executeSqlAsync(0,0,"UPDATE %s.item_attributes SET value='@crafting:tool_status_ready' WHERE item_id=%" PRIu64 " AND attribute_id=18",mDatabase->galaxy(),tool->getId());

                tool->setAttribute("craft_tool_time",boost::lexical_cast<std::string>(tool->getTimer()));
                gWorldManager->getDatabase()->executeSqlAsync(0,0,"UPDATE %s.item_attributes SET value='%i' WHERE item_id=%" PRIu64 " AND attribute_id=%u",mDatabase->galaxy(),tool->getId(),tool->getTimer(),AttrType_CraftToolTime);


                continue;
            }
            // update the time display
            gMessageLib->sendUpdateTimer(tool,player);

            tool->setAttribute("craft_tool_time",boost::lexical_cast<std::string>(tool->getTimer()));
            //gLogger->log(LogManager::DEBUG,"timer : %i",tool->getTimer());
            mDatabase->executeSqlAsync(0,0,"UPDATE %s.item_attributes SET value='%i' WHERE item_id=%" PRIu64 " AND attribute_id=%u",mDatabase->galaxy(),tool->getId(),tool->getTimer(),AttrType_CraftToolTime);

        }

        ++it;
    }

    return(true);
}

//======================================================================================================================

void WorldManager::addBusyCraftTool(CraftingTool* tool)
{
    mBusyCraftTools.push_back(tool->getId());
}

//======================================================================================================================

void WorldManager::removeBusyCraftTool(CraftingTool* tool)
{
    CraftTools::iterator it = mBusyCraftTools.begin();

    while(it != mBusyCraftTools.end())
    {
        if((*it) == tool->getId())
        {
            mBusyCraftTools.erase(it);
            break;
        }

        ++it;
    }
}

//======================================================================================================================
//
//	Add a timed entry for deletion of dead creature objects.
//
void WorldManager::addCreatureObjectForTimedDeletion(uint64 creatureId, uint64 when)
{
    uint64 expireTime = Anh_Utils::Clock::getSingleton()->getLocalTime();

    // gLogger->log(LogManager::DEBUG,"WorldManager::addCreatureObjectForTimedDeletion Adding new at %" PRIu64 "", expireTime + when);

    CreatureObjectDeletionMap::iterator it = mCreatureObjectDeletionMap.find(creatureId);
    if (it != mCreatureObjectDeletionMap.end())
    {
        // Only remove object if new expire time is earlier than old. (else people can use "lootall" to add 10 new seconds to a corpse forever).
        if (expireTime + when < (*it).second)
        {
            // gLogger->log(LogManager::DEBUG,"Removing object with id %" PRIu64 "", creatureId);
            mCreatureObjectDeletionMap.erase(it);
        }
        else
        {
            return;
        }

    }
    // gLogger->log(LogManager::DEBUG,"Adding new object with id %" PRIu64 "", creatureId);
    mCreatureObjectDeletionMap.insert(std::make_pair(creatureId, expireTime + when));
}


bool WorldManager::_handleVariousUpdates(uint64 callTime, void* ref)
{
    gForageManager->forageUpdate();
    gFireworkManager->Process();
    return true;
}

//======================================================================================================================
//
// Handle delayed deletion of dead creature objects and revive of dead player objects.
//

bool WorldManager::_handleGroupObjectTimers(uint64 callTime, void* ref)
{
    //iterate through all groups and update the missionwaypoints
    GroupList* groupList = gGroupManager->getGroupList();
    GroupList::iterator it = groupList->begin();

    while(it != groupList->end())
    {
        GroupObject* group = (*it);
        gGroupManager->sendGroupMissionUpdate(group);
        it++;
    }

    return (true);
}

//======================================================================================================================
//
// Handle delayed deletion of dead creature objects and revive of dead player objects.
//


//======================================================================================================================
//
// update the current planet weather
//
void WorldManager::updateWeather(float cloudX,float cloudY,float cloudZ,uint32 weatherType)
{
    mCurrentWeather.mWeather = weatherType;
    mCurrentWeather.mClouds.x = cloudX;
    mCurrentWeather.mClouds.y = cloudY;
    mCurrentWeather.mClouds.z = cloudZ;

    gMessageLib->sendWeatherUpdate(mCurrentWeather.mClouds,mCurrentWeather.mWeather);
}

//======================================================================================================================
//
//	Add an admin request.
//

void WorldManager::addAdminRequest(uint64 requestId, uint64 when)
{
    uint64 expireTime = Anh_Utils::Clock::getSingleton()->getLocalTime();
    mAdminRequestHandlers.insert(std::make_pair(requestId, expireTime + when));

}

//======================================================================================================================
//
//	Cancel an admin request.
//

void WorldManager::cancelAdminRequest(int32 requestId)
{
    AdminRequestHandlers::iterator it = mAdminRequestHandlers.find(requestId);

    if (it != mAdminRequestHandlers.end())
    {
        // Cancel shutdown.
        mAdminRequestHandlers.erase(it);
    }
}

//======================================================================================================================
//
// Handle the queue with admin requests.
//

bool WorldManager::_handleAdminRequests(uint64 callTime, void* ref)
{

    // callTime = callTime - (callTime % 1000)
    AdminRequestHandlers::iterator it = mAdminRequestHandlers.begin();
    while (it != mAdminRequestHandlers.end())
    {
        //  The timer has expired?
        if (callTime >= ((*it).second))
        {
            // Yes, handle it.
            uint64 waitTime = AdminManager::Instance()->handleAdminRequest(((*it).first), callTime - ((*it).second));

            if (waitTime)
            {
                // Set next execution time.
                (*it).second = callTime + waitTime;
            }
            else
            {
                // Requested to remove the handler.
                mAdminRequestHandlers.erase(it++);
                continue;
            }
        }
        ++it;
    }

    return true;
}


//======================================================================================================================

void WorldManager::handleTimer(uint32 id, void* container)
{

}

//======================================================================================================================
//
// called on startup, after all objects have been loaded
//
void WorldManager::_handleLoadComplete()
{
	// release memory
	mDatabase->releaseResultPoolMemory();
	mDatabase->releaseJobPoolMemory();
	mDatabase->releaseBindingPoolMemory();
	mWM_DB_AsyncPool.release_memory();
	gObjectFactory->releaseAllPoolsMemory();
	if(mZoneId != 41)
		gResourceManager->releaseAllPoolsMemory();
	gSchematicManager->releaseAllPoolsMemory();
	gSkillManager->releaseAllPoolsMemory();

	// register script hooks
	_startWorldScripts();

	LOG(INFO) << "World load complete";
			
	if(mZoneId != 41)
	{
		while(!gHeightmap->isReady())
			boost::this_thread::sleep(boost::posix_time::milliseconds(100));
	}

	// switch into running state
	mState = WMState_Running;

	// notify zoneserver
	mZoneServer->handleWMReady();
	mTotalObjectCount = 0;

	// initialize timers
	mSubsystemScheduler->addTask(fastdelegate::MakeDelegate(this,&WorldManager::_handleShuttleUpdate),7,1000,NULL);
	mSubsystemScheduler->addTask(fastdelegate::MakeDelegate(this,&WorldManager::_handleServerTimeUpdate),9,gWorldConfig->getServerTimeInterval()*1000,NULL);
	mSubsystemScheduler->addTask(fastdelegate::MakeDelegate(this,&WorldManager::_handleDisconnectUpdate),1,1000,NULL);
	mSubsystemScheduler->addTask(fastdelegate::MakeDelegate(this,&WorldManager::_handleCraftToolTimers),3,1000,NULL);
	mSubsystemScheduler->addTask(fastdelegate::MakeDelegate(this,&WorldManager::_handleNpcConversionTimers),8,1000,NULL);

	setSaveTaskId(mSubsystemScheduler->addTask(fastdelegate::MakeDelegate(this,&WorldManager::_handlePlayerSaveTimers), 4, 120000, NULL));
	
	mSubsystemScheduler->addTask(fastdelegate::MakeDelegate(this,&WorldManager::_handleGeneralObjectTimers),5,2000,NULL);
	mSubsystemScheduler->addTask(fastdelegate::MakeDelegate(this,&WorldManager::_handleGroupObjectTimers),5,gWorldConfig->getGroupMissionUpdateTime(),NULL);
	mSubsystemScheduler->addTask(fastdelegate::MakeDelegate(this,&WorldManager::_handleVariousUpdates),7,1000, NULL);

	// Init NPC Manager, will load lairs from the DB.
	(void)NpcManager::Instance();

	// Initialize the queues for NPC-Manager.
	mNpcManagerScheduler->addTask(fastdelegate::MakeDelegate(this,&WorldManager::_handleDormantNpcs),5,2500,NULL);
	mNpcManagerScheduler->addTask(fastdelegate::MakeDelegate(this,&WorldManager::_handleReadyNpcs),5,1000,NULL);
	mNpcManagerScheduler->addTask(fastdelegate::MakeDelegate(this,&WorldManager::_handleActiveNpcs),5,250,NULL);

	// Initialize static creature lairs.
	mAdminScheduler->addTask(fastdelegate::MakeDelegate(this,&WorldManager::_handleAdminRequests),5,5000,NULL);
}

//======================================================================================================================

//======================================================================================================================

int32 WorldManager::getPlanetIdByName(BString name)
{
    uint8	id = 0;
    name.toLower();

    BStringVector::iterator it = mvPlanetNames.begin();

    while(it != mvPlanetNames.end())
    {
        if(strcmp((*it).getAnsi(),name.getAnsi()) == 0)
            return(id);

        ++it;
        id++;
    }

    return(-1);
}

//======================================================================================================================

int32 WorldManager::getPlanetIdByNameLike(BString name)
{
    uint8	id = 0;
    name.toLower();

    BStringVector::iterator it = mvPlanetNames.begin();

    while(it != mvPlanetNames.end())
    {
        // gLogger->log(LogManager::DEBUG,"Comparing: %s",  name.getAnsi());
        // gLogger->log(LogManager::DEBUG,"with     : %s",  (*it).getAnsi());
        if(Anh_Utils::cmpnistr((*it).getAnsi(),name.getAnsi(), 3) == 0)
        {
            // gLogger->log(LogManager::DEBUG,"Matched with planet id = %d",  id);
            return (id);
        }
        ++it;
        id++;
    }
    return(-1);
}


//======================================================================================================================
//
// remove an entertainer from the entertaining scheduler
//

void WorldManager::removeEntertainerToProcess(uint64 taskId)
{
    mEntertainerScheduler->removeTask(taskId);
}

void WorldManager::removeImagedesignerToProcess(uint64 taskId)
{
    mEntertainerScheduler->removeTask(taskId);
}

//======================================================================================================================
//
// add a creature from the Stomach Filling scheduler
//
uint64 WorldManager::addCreatureDrinkToProccess(Stomach* stomach)
{
    return((mStomachFillingScheduler->addTask(fastdelegate::MakeDelegate(stomach,&Stomach::regenDrink),1,stomach->getDrinkInterval(),NULL)));
}
uint64 WorldManager::addCreatureFoodToProccess(Stomach* stomach)
{
    return((mStomachFillingScheduler->addTask(fastdelegate::MakeDelegate(stomach,&Stomach::regenFood),1,stomach->getFoodInterval(),NULL)));
}
void WorldManager::removeCreatureStomachToProcess(uint64 taskId)
{
    mStomachFillingScheduler->removeTask(taskId);
}
bool WorldManager::checkStomachTask(uint64 id)
{
    return mStomachFillingScheduler->checkTask(id);
}
//======================================================================================================================
//
// add a creature from the ham regeneration scheduler
//
uint64 WorldManager::addCreatureHamToProccess(Ham* ham)
{
    return((mHamRegenScheduler->addTask(fastdelegate::MakeDelegate(ham,&Ham::regenerate),1,1000,NULL)));
}


//======================================================================================================================
//
// remove a creature from the ham regeneration scheduler
//

void WorldManager::removeCreatureHamToProcess(uint64 taskId)
{
    mHamRegenScheduler->removeTask(taskId);
}


//======================================================================================================================

bool WorldManager::checkTask(uint64 id)
{
    return mHamRegenScheduler->checkTask(id);
}


//======================================================================================================================
//
// add an object from the object controller scheduler
//

uint64 WorldManager::addObjControllerToProcess(ObjectController* objController)
{
    //make sure the Objectcontroller wont be added to the processqueue after we removed it there when logging out

    //Q: can only players be added to the process queue ???
    //A: probably yes - so put this to the playerhandlers
    if(!objController)
        return 0;

    //we get added automatically when the client sends a command to process
    if(objController->getObject()->getType() == ObjType_Player)
    {
        PlayerObject* player = dynamic_cast<PlayerObject*>(objController->getObject());

        if ((player->getConnectionState() == PlayerConnState_LinkDead) || (player->getConnectionState() == PlayerConnState_Destroying))
            return 0;
    }
    return((mObjControllerScheduler->addTask(fastdelegate::MakeDelegate(objController,&ObjectController::process),1,125,NULL)));
}


//======================================================================================================================
//
// remove an object from the object controller scheduler
//

void WorldManager::removeObjControllerToProcess(uint64 taskId)
{
    mObjControllerScheduler->removeTask(taskId);
}


//======================================================================================================================

uint64 WorldManager::addMissionToProcess(MissionObject* mission)
{
    return mMissionScheduler->addTask(fastdelegate::MakeDelegate(mission,&MissionObject::check),1,10000,NULL);
}


//======================================================================================================================

void WorldManager::removeMissionFromProcess(uint64 taskId)
{
    mMissionScheduler->removeTask(taskId);
}


//======================================================================================================================

bool WorldManager::checkForMissionProcess(uint64 taskId)
{
    return mMissionScheduler->checkTask(taskId);
}


//======================================================================================================================

uint64 WorldManager::addEntertainerToProccess(CreatureObject* entertainerObject,uint32 tick)
{
    return((mEntertainerScheduler->addTask(fastdelegate::MakeDelegate(entertainerObject,&CreatureObject::handlePerformanceTick),1,tick,NULL)));
}

uint64 WorldManager::addImageDesignerToProcess(CreatureObject* entertainerObject,uint32 tick)
{
    return((mEntertainerScheduler->addTask(fastdelegate::MakeDelegate(entertainerObject,&CreatureObject::handleImagedesignerTimeOut),1,tick,NULL)));
}

//======================================================================================================================
//
// returns true if object exist in world manager object list.
//
bool WorldManager::existObject(Object* object)
{
    if (object)
    {
        return (mObjectMap.find(object->getId()) != mObjectMap.end());
    }
    else
    {
        return false;
    }
}
//======================================================================================================================
//
// get an attribute string value from the global attribute map
//

BString WorldManager::getAttributeKey(uint32 keyId)
{
    AttributeKeyMap::iterator it = mObjectAttributeKeyMap.find(keyId);

    if(it != mObjectAttributeKeyMap.end())
        return((*it).second);

    return BString();
}

//======================================================================================================================
//
// get an attribute id based on the name from the global attribute map
//
uint32 WorldManager::getAttributeId(uint32 keyId)
{
    AttributeIDMap::iterator it = mObjectAttributeIDMap.find(keyId);

    if(it != mObjectAttributeIDMap.end())
        return((*it).second);

    return 0;
}

//======================================================================================================================
//
// initialize / start scripts
//

void WorldManager::_startWorldScripts()
{
    ScriptList::iterator scriptIt = mWorldScripts.begin();

    while(scriptIt != mWorldScripts.end())
    {
        (*scriptIt)->run();

        ++scriptIt;
    }
    LOG(ERR) << "Loaded world scripts";
}

//======================================================================================================================
//
// script callback, lets scripts register themselves for an event
//

void WorldManager::ScriptRegisterEvent(void* script,std::string eventFunction)
{
    mWorldScriptsListener.registerScript(reinterpret_cast<Script*>(script),(int8*)eventFunction.c_str());
}

//======================================================================================================================
//
// send a system message to every player on the planet
// available for scripts
//

void WorldManager::	zoneSystemMessage(std::string message)
{
    PlayerAccMap::iterator it = mPlayerAccMap.begin();

    while(it != mPlayerAccMap.end())
    {
        const PlayerObject* const player = (*it).second;

        if(player->isConnected())
        {
            std::wstring msg(message.begin(), message.end());
            gMessageLib->SendSystemMessage(msg, player);
        }

        ++it;
    }
}

//======================================================================================================================
//
// register our script hooks with the listener
//

void WorldManager::_registerScriptHooks()
{
    mWorldScriptsListener.registerFunction("onPlayerEntered");
    mWorldScriptsListener.registerFunction("onPlayerLeft");
}

//======================================================================================================================

bool WorldManager::addNpId(uint64 id)
{
    if(mUsedTmpIds.find(id) == mUsedTmpIds.end())
    {
        mUsedTmpIds.insert(id);

        return(true);
    }

    return(false);
}

//======================================================================================================================




//======================================================================================================================
//
// remove a Buff from the Buff scheduler
//

void WorldManager::removeBuffToProcess(uint64 taskId)
{
    mBuffScheduler->removeTask(taskId);
}

uint64 WorldManager::addBuffToProcess(Buff* buff)
{
    //Create a copy of Buff* which can be Destructed when task completes (leaving the original ptr intact)
    //Buff** pBuff = &buff;
    Buff* DestructibleBuff = new Buff(*buff);

    //Create Event
    //BuffEvent* bEvent = new BuffEvent(buff);

    //Create Callback
    VariableTimeCallback callback = fastdelegate::MakeDelegate(DestructibleBuff,&Buff::Update);

    //Add Callback to Scheduler
    uint64 temp = mBuffScheduler->addTask(callback,1,buff->GetTickLength(),NULL);

    //Give Buff the ID from Scheduler
    buff->SetID(temp);
    return temp;
}


//=========================================================================================
//
//	return true if object are withing range.
//  Both objects need to be inside the same building or both object outside, to be concidered to be in range.
//

bool WorldManager::objectsInRange(uint64 obj1Id, uint64 obj2Id, float range)
{
    bool inRange = true;

    Object* obj1 = dynamic_cast<Object*>(this->getObjectById(obj1Id));
    Object* obj2 = dynamic_cast<Object*>(this->getObjectById(obj2Id));
    if (!obj1 || !obj2)
    {
        inRange = false;
    }
    // We have to be in the same building or outside.
    else if (obj1->getParentId() == obj2->getParentId())
    {
        // In the same cell (or both outside is rarley the case here)
        if (glm::distance(obj1->mPosition, obj2->mPosition) > range)
		{
			inRange = false;
		}
	}
	else if ((obj1->getParentId() == 0) || (obj2->getParentId() == 0))
	{
		// One of us is outside
		inRange = false;
	}
	else
	{
		// We may be in the same building.
		CellObject* obj1Cell = dynamic_cast<CellObject*>(this->getObjectById(obj1->getParentId()));
		CellObject* obj2Cell = dynamic_cast<CellObject*>(this->getObjectById(obj2->getParentId()));
		if (obj1Cell && obj2Cell && (obj1Cell->getParentId() == obj2Cell->getParentId()))
		{
			// In the same building
            if (glm::distance(obj1->mPosition, obj2->mPosition) > range)
            {
                // But out of range.
                inRange = false;
            }
        }
        else
        {
            // In different buildings.
            inRange = false;
        }
    }
    return inRange;
}

bool WorldManager::objectsInRange(const glm::vec3& obj1Position,  uint64 obj1ParentId, uint64 obj2Id, float range)
{
    bool inRange = true;

    // Object* obj1 = dynamic_cast<Object*>(this->getObjectById(obj1Id));
    Object* obj2 = dynamic_cast<Object*>(this->getObjectById(obj2Id));

    // We have to be in the same building or outside.
    if (obj1ParentId == obj2->getParentId())
    {
        // In the same cell (or both outside is rarley the case here)
        if (glm::distance(obj1Position, obj2->mPosition) > range)
        {
            inRange = false;
        }
    }
    else if ((obj1ParentId == 0) || (obj2->getParentId() == 0))
    {
        // One of us are outside.
        inRange = false;
    }
    else
    {
        // We may be in the same building.
        CellObject* obj1Cell = dynamic_cast<CellObject*>(this->getObjectById(obj1ParentId));
        CellObject* obj2Cell = dynamic_cast<CellObject*>(this->getObjectById(obj2->getParentId()));
        if (obj1Cell && obj2Cell && (obj1Cell->getParentId() == obj2Cell->getParentId()))
        {
            // In the same building
            if (glm::distance(obj1Position, obj2->mPosition) > range)
            {
                // But out of range.
                inRange = false;
            }
        }
        else
        {
            // In different buildings.
            inRange = false;
        }
    }
    return inRange;
}





//======================================================================================================================
//
//	Get the spawn area for a region.
//

const Anh_Math::Rectangle WorldManager::getSpawnArea(uint64 spawnRegionId)
{
    Anh_Math::Rectangle spawnArea(0,0,0,0);

    CreatureSpawnRegionMap::iterator it = mCreatureSpawnRegionMap.find(spawnRegionId);
    if (it != mCreatureSpawnRegionMap.end())
    {
        const auto creatureSpawnRegion = dynamic_pointer_cast<CreatureSpawnRegion> (it->second);
        Anh_Math::Rectangle sa(creatureSpawnRegion->mPosX, creatureSpawnRegion->mPosZ, creatureSpawnRegion->mWidth ,creatureSpawnRegion->mLength);
        spawnArea = sa;
    }
    return spawnArea;
}

//======================================================================================================================
//
//	Remove player from accountmap
//

void WorldManager::removePlayerfromAccountMap(uint64 playerID)
{
    if(PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(playerID)))
    {
        PlayerAccMap::iterator playerAccIt = mPlayerAccMap.find(player->getAccountId());

        if(playerAccIt != mPlayerAccMap.end())
        {
            LOG(INFO) << "Player left [" << player->getId() << "] Total players on zone [" << (getPlayerAccMap()->size() -1) << "]";
            mPlayerAccMap.erase(playerAccIt);
        }
        else
        {
            LOG(ERR) << "Error removing player from account map [" << player->getAccountId() << "]";
        }
    }
    else
    {
        LOG(ERR) << "Error removing player from account map [" << player->getAccountId() << "]";
    }
}
