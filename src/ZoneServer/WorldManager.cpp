/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

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
#include "GroupManager.h"
#include "GroupObject.h"
#include "Heightmap.h"
#include "MissionManager.h"
#include "NpcManager.h"
#include "NPCObject.h"
#include "PlayerStructure.h"
#include "ResourceCollectionManager.h"
#include "ResourceManager.h"
#include "SchematicManager.h"
#include "TreasuryManager.h"
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
#include "ForageManager.h"
#include "TicketCollector.h"
#include "ConfigManager/ConfigManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/DatabaseResult.h"
#include "MessageLib/MessageLib.h"
#include "ScriptEngine/ScriptEngine.h"
#include "ScriptEngine/ScriptSupport.h"
#include "Utils/Scheduler.h"
#include "Utils/VariableTimeScheduler.h"
#include "Utils/utils.h"

#include <cassert>

//======================================================================================================================

bool			WorldManager::mInsFlag    = false;
WorldManager*	WorldManager::mSingleton  = NULL;
//======================================================================================================================

WorldManager::WorldManager(uint32 zoneId,ZoneServer* zoneServer,Database* database)
: mWM_DB_AsyncPool(sizeof(WMAsyncContainer))
, mDatabase(database)
, mZoneServer(zoneServer)
, mState(WMState_StartUp)
, mServerTime(0)
, mTotalObjectCount(0)
, mZoneId(zoneId)
{
	gLogger->logMsg("WorldManager::StartUp");

	// set up spatial index
	mSpatialIndex = new ZoneTree();
	mSpatialIndex->Init(gConfig->read<float>("FillFactor"),
						gConfig->read<int>("IndexCap"),
						gConfig->read<int>("LeafCap"),
						2,
						gConfig->read<float>("Horizon"));

	try
	{
		mDebug = gConfig->read<bool>("LoadReduceDebug");
	}
	catch (...)
	{
		mDebug = false;
	}


	// create schedulers
	mSubsystemScheduler		= new Anh_Utils::Scheduler();
	mObjControllerScheduler = new Anh_Utils::Scheduler();
	mHamRegenScheduler		= new Anh_Utils::Scheduler();
	mPlayerScheduler		= new Anh_Utils::Scheduler();
	mEntertainerScheduler	= new Anh_Utils::Scheduler();
	mBuffScheduler			= new Anh_Utils::VariableTimeScheduler(100, 100);
	mMissionScheduler		= new Anh_Utils::Scheduler();
	mNpcManagerScheduler	= new Anh_Utils::Scheduler();
	mAdminScheduler			= new Anh_Utils::Scheduler();

	LoadCurrentGlobalTick();


	// preallocate
	mvClientEffects.reserve(1000);
	mvMoods.reserve(200);
	mvSounds.reserve(5000);
	mShuttleList.reserve(50);

	// load up subsystems

	SkillManager::Init(database);
	SchematicManager::Init(database);
	ResourceManager::Init(database,mZoneId);
	ResourceCollectionManager::Init(database);
	TreasuryManager::Init(database);
	ConversationManager::Init(database);
	CraftingSessionFactory::Init(database);
    MissionManager::Init(database,mZoneId);

	// register world script hooks
	_registerScriptHooks();

	// initiate loading of objects
	if(mDebug)
	{
		gLogger->logMsg("WorldManager::DebugStartUp with culled items, npcs, resources and stuff");
		mDatabase->ExecuteSqlAsync(this,new(mWM_DB_AsyncPool.ordered_malloc()) WMAsyncContainer(WMQuery_ObjectCount),"SELECT sf_getZoneObjectCountDebug(%i);",mZoneId);
	}
	else
		mDatabase->ExecuteSqlAsync(this,new(mWM_DB_AsyncPool.ordered_malloc()) WMAsyncContainer(WMQuery_ObjectCount),"SELECT sf_getZoneObjectCount(%i);",mZoneId);

#if defined(_MSC_VER)
	mNonPersistantId =   422212465065984;
#else
	mNonPersistantId =   422212465065984LLU;
#endif
}

//======================================================================================================================

WorldManager*	WorldManager::Init(uint32 zoneId,ZoneServer* zoneServer,Database* database)
{
	if(!mInsFlag)
	{
		mSingleton = new WorldManager(zoneId,zoneServer,database);
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
	delete(mHamRegenScheduler);
	delete(mMissionScheduler);
	delete(mPlayerScheduler);
	delete(mEntertainerScheduler);
	delete(mBuffScheduler);

	

	// we need to destroy that after the (player) objects!
	// as the playerobjects try to remove the Objectcontroller scheduler and crash us if the scheduler isnt existent anymore
	delete(mSubsystemScheduler);

	mPlayersToRemove.clear();
	mRegionMap.clear();

	// Npc conversation timers.
	mNpcConversionTimers.clear();

	// Player movement update timers.
	mPlayerMovementUpdateMap.clear();

	mCreatureObjectDeletionMap.clear();
	mPlayerObjectReviveMap.clear();

	mNpcDormantHandlers.clear();
	mNpcReadyHandlers.clear();
	mNpcActiveHandlers.clear();
	mAdminRequestHandlers.clear();

	// Handle creature spawn regions. These objects are not registred in the normal object map.
	CreatureSpawnRegionMap::iterator it = mCreatureSpawnRegionMap.begin();
	while (it != mCreatureSpawnRegionMap.end())
	{
		delete (*it).second;
		mCreatureSpawnRegionMap.erase(it++);
	}
	mCreatureSpawnRegionMap.clear();

	NpcManager::deleteManager();

	Heightmap::deleter();

	// Let's get REAL dirty here, since we have no solutions to the deletion-race of containers content.
	// Done by Eruptor. I got tired of the unhandled problem.
	// as we cannot keep the content out of the worldmanagers mainobjectlist - we might just store references in the container object ?
	// the point is that we then have to delete all containers first - so register them seperately?
	//
#if defined(_MSC_VER)
	if (getObjectById((uint64)(2533274790395904)))
#else
	if (getObjectById((uint64)(2533274790395904LLU)))
#endif
	{
#if defined(_MSC_VER)
		Container* container = dynamic_cast<Container*>(getObjectById((uint64)(2533274790395904)));
#else
		Container* container = dynamic_cast<Container*>(getObjectById((uint64)(2533274790395904LLU)));
#endif
		if (container)
		{
			gLogger->logMsg("WorldManager::Shutdown(): Deleting the Tutorial container");
			this->destroyObject(container);
			gLogger->logMsg("WorldManager::Shutdown(): Delete done!");
		}
	}

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
	mSpatialIndex->ShutDown();
	delete(mSpatialIndex);

	// finally delete them
	mQTRegionMap.clear();
	mObjectMap.clear();
	
	
	
}

//======================================================================================================================

WorldManager::~WorldManager()
{
	mInsFlag = false;
	delete(mSingleton);
}


//======================================================================================================================

void WorldManager::_loadBuildings()
{
	WMAsyncContainer* asynContainer = new(mWM_DB_AsyncPool.ordered_malloc()) WMAsyncContainer(WMQuery_All_Buildings);

	mDatabase->ExecuteSqlAsync(this,asynContainer,"SELECT id FROM buildings WHERE planet_id = %u;",mZoneId);
}


//======================================================================================================================

void WorldManager::handleObjectReady(Object* object,DispatchClient* client)
{
	if(QTRegion* region = dynamic_cast<QTRegion*>(object))
	{
		uint32 key = (uint32)region->getId();

		mQTRegionMap.insert(key,region);

		mSpatialIndex->insertQTRegion(key,region->mPosition.mX,region->mPosition.mZ,region->getWidth(),region->getHeight());
	}
	else
	{
		addObject(object);
	}

	// check if we done loading
	if ((mState == WMState_StartUp) && (mObjectMap.size() + mQTRegionMap.size() + mCreatureSpawnRegionMap.size() >= mTotalObjectCount))
	{
		_handleLoadComplete();
	}
}

//======================================================================================================================

RegionObject* WorldManager::getRegionById(uint64 regionId)
{
	RegionMap::iterator it = mRegionMap.find(regionId);

	if(it != mRegionMap.end())
		return((*it).second);
	else
		gLogger->logMsgF("Worldmanager::getRegionById: Could not find region %"PRIu64"",MSG_NORMAL,regionId);

	return(NULL);
}


//======================================================================================================================
//get the current tick
//

uint64 WorldManager::GetCurrentGlobalTick()
{
	return mTick;
}

//======================================================================================================================
//
//	Remove player from accountmap
//

Anh_Math::Vector3		WorldManager::getPlayerWorldPositionFromBuilding(PlayerObject* player)
{
	Object* cell = gWorldManager->getObjectById(player->getParentId());
	if(!cell)
	{
		Anh_Math::Vector3 position;
		return position;
	}
	Object* structure = gWorldManager->getObjectById(cell->getParentId());
	if(!structure)
	{
		Anh_Math::Vector3 position;
		return position;
	}

	Anh_Math::Vector3 position = player->mPosition;
	position.mX += structure->mPosition.mX;
	position.mZ += structure->mPosition.mZ;

	return position;

}

//======================================================================================================================
//still synch issues to adress with other servers
//

void WorldManager::LoadCurrentGlobalTick()
{
	uint64 Tick;
	DatabaseResult* temp = mDatabase->ExecuteSynchSql("SELECT Global_Tick_Count FROM galaxy WHERE galaxy_id = '2'");

	DataBinding*	tickbinding = mDatabase->CreateDataBinding(1);
	tickbinding->addField(DFT_uint64,0,8,0);

	temp->GetNextRow(tickbinding,&Tick);
	mDatabase->DestroyDataBinding(tickbinding);
	mDatabase->DestroyResult(temp);

	char strtemp[100];
	sprintf(strtemp, "Current Global Tick Count = %"PRIu64"\n",Tick);
	gLogger->logMsg(strtemp, FOREGROUND_GREEN);
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
	mSubsystemScheduler->process();
	mObjControllerScheduler->process();
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

			// Stop update timers.
			removePlayerMovementUpdateTime(playerObject);

			//remove the player out of his group - if any
			GroupObject* group = gGroupManager->getGroupObject(playerObject->getGroupId());
			if(group)
			{
				if(playerObject->getIDPartner() != 0)
				{
					if(PlayerObject* idPartner = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(playerObject->getIDPartner())))
					{
						idPartner->SetImageDesignSession(IDSessionNONE);
						idPartner->setIDPartner(0);
						playerObject->SetImageDesignSession(IDSessionNONE);
						playerObject->setIDPartner(0);
					}

				}
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
					shuttle->setPosture(0);
					shuttle->setAwayTime(0);
					shuttle->setShuttleState(ShuttleState_AboutBoarding);

					gMessageLib->sendPostureUpdate(shuttle);
					gMessageLib->sendCombatAction(shuttle,NULL,opChange_Posture);
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
					shuttle->setInPortTime(0);
					shuttle->setShuttleState(ShuttleState_Away);
					shuttle->setPosture(2);

					gMessageLib->sendPostureUpdate(shuttle);
					gMessageLib->sendCombatAction(shuttle,NULL,opChange_Posture);
				}
				else
				{
					shuttle->setInPortTime(inPortTime);
				}
			}
			break;

			default:break;
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
			gLogger->logMsgF("WorldManager::_handleCraftToolTimers missing crafting tool :(",MSG_NORMAL);
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

					gMessageLib->sendCreateTangible(item,player);

					tool->setCurrentItem(NULL);
				}
				//in case of logout/in interplanetary travel it will be in the inventory already

				gMessageLib->sendUpdateTimer(tool,player);

				it = mBusyCraftTools.erase(it);
				tool->setAttribute("craft_tool_status","@crafting:tool_status_ready");
				mDatabase->ExecuteSqlAsync(0,0,"UPDATE item_attributes SET value='@crafting:tool_status_ready' WHERE item_id=%"PRIu64" AND attribute_id=18",tool->getId());

				tool->setAttribute("craft_tool_time",boost::lexical_cast<std::string>(tool->getTimer()));
				gWorldManager->getDatabase()->ExecuteSqlAsync(0,0,"UPDATE item_attributes SET value='%i' WHERE item_id=%"PRIu64" AND attribute_id=%u",tool->getId(),tool->getTimer(),AttrType_CraftToolTime);

				continue;
			}
			// update the time display
			gMessageLib->sendUpdateTimer(tool,player);

			tool->setAttribute("craft_tool_time",boost::lexical_cast<std::string>(tool->getTimer()));
			//gLogger->logMsgF("timer : %i",MSG_HIGH,tool->getTimer());
			gWorldManager->getDatabase()->ExecuteSqlAsync(0,0,"UPDATE item_attributes SET value='%i' WHERE item_id=%"PRIu64" AND attribute_id=%u",tool->getId(),tool->getTimer(),AttrType_CraftToolTime);
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

	// gLogger->logMsgF("WorldManager::addCreatureObjectForTimedDeletion Adding new at %"PRIu64"",MSG_NORMAL, expireTime + when);

	CreatureObjectDeletionMap::iterator it = mCreatureObjectDeletionMap.find(creatureId);
	if (it != mCreatureObjectDeletionMap.end())
	{
		// Only remove object if new expire time is earlier than old. (else people can use "lootall" to add 10 new seconds to a corpse forever).
		if (expireTime + when < (*it).second)
		{
			// gLogger->logMsgF("Removing object with id %"PRIu64"",MSG_NORMAL, creatureId);
			mCreatureObjectDeletionMap.erase(it);
		}
		else
		{
			return;
		}

	}
	// gLogger->logMsgF("Adding new object with id %"PRIu64"",MSG_NORMAL, creatureId);
	mCreatureObjectDeletionMap.insert(std::make_pair(creatureId, expireTime + when));
}


bool WorldManager::_handleScoutForagingUpdate(uint64 callTime, void* ref)
{
	gForageManager->forageUpdate();
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
	mCurrentWeather.mClouds.mX = cloudX;
	mCurrentWeather.mClouds.mY = cloudY;
	mCurrentWeather.mClouds.mZ = cloudZ;

	gMessageLib->sendWeatherUpdate(mCurrentWeather.mClouds,mCurrentWeather.mWeather);
}

//======================================================================================================================
//
//	Add an admin request.
//

void WorldManager::addAdminRequest(uint64 requestId, uint64 when)
{
	gLogger->logMsgF("Adding admin request %d for schedule in %"PRIu64" minutes(s) and %"PRIu64" second(s)", MSG_NORMAL, requestId, when/60000, when % 60000);

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
				gLogger->logMsgF("Removed expired handler for admin request %d", MSG_NORMAL, (*it).first);

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
	gResourceManager->releaseAllPoolsMemory();
	gSchematicManager->releaseAllPoolsMemory();
	gSkillManager->releaseAllPoolsMemory();

	if (!Heightmap::Instance())
	{
		assert(false && "WorldManager::_handleLoadComplete Missing heightmap, download at http://www.swganh.com/!!planets!!/PLANET_NAME.rar");
	}

	// create a height-map cashe.
	int16 resolution = 0;
	if (gConfig->keyExists("heightMapResolution"))
	{
		resolution = gConfig->read<int>("heightMapResolution");
	}
	gLogger->logMsgF("WorldManager::_handleLoadComplete heightMapResolution = %d", MSG_NORMAL, resolution);

	if (Heightmap::Instance()->setupCache(resolution))
	{
		gLogger->logMsgF("WorldManager::_handleLoadComplete heigthmap cache setup successfully with resolution %d", MSG_NORMAL, resolution);
	}
	else
	{
		gLogger->logMsgF("WorldManager::_handleLoadComplete heigthmap cache setup FAILED", MSG_NORMAL);
	}

	// register script hooks
	_startWorldScripts();

	gLogger->logMsg("WorldManager::Load complete");

	// switch into running state
	mState = WMState_Running;

	// notify zoneserver
	mZoneServer->handleWMReady();
	mTotalObjectCount = 0;

	// initialize timers
	mSubsystemScheduler->addTask(fastdelegate::MakeDelegate(this,&WorldManager::_handleShuttleUpdate),7,1000,NULL);
	mSubsystemScheduler->addTask(fastdelegate::MakeDelegate(this,&WorldManager::_handleServerTimeUpdate),9,gWorldConfig->getServerTimeInterval()*1000,NULL);
	mSubsystemScheduler->addTask(fastdelegate::MakeDelegate(this,&WorldManager::_handleDisconnectUpdate),1,1000,NULL);
	mSubsystemScheduler->addTask(fastdelegate::MakeDelegate(this,&WorldManager::_handleRegionUpdate),2,2000,NULL);
	mSubsystemScheduler->addTask(fastdelegate::MakeDelegate(this,&WorldManager::_handleCraftToolTimers),3,1000,NULL);
	mSubsystemScheduler->addTask(fastdelegate::MakeDelegate(this,&WorldManager::_handleNpcConversionTimers),8,1000,NULL);
	
	//is this really necessary ?
	//whenever someone creates something near us were updated on it anyway ... ?
	mSubsystemScheduler->addTask(fastdelegate::MakeDelegate(this,&WorldManager::_handlePlayerMovementUpdateTimers),4,5000,NULL);
	
	mSubsystemScheduler->addTask(fastdelegate::MakeDelegate(this,&WorldManager::_handleGeneralObjectTimers),5,2000,NULL);
	mSubsystemScheduler->addTask(fastdelegate::MakeDelegate(this,&WorldManager::_handleGroupObjectTimers),5,gWorldConfig->getGroupMissionUpdateTime(),NULL);
	mSubsystemScheduler->addTask(fastdelegate::MakeDelegate(this,&WorldManager::_handleScoutForagingUpdate),7,2000, NULL);

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

void WorldManager::removeActiveRegion(RegionObject* regionObject)
{
	ActiveRegions::iterator it = mActiveRegions.begin();

	while(it != mActiveRegions.end())
	{
		if((*it) == regionObject)
		{
			mActiveRegions.erase(it);
			break;
		}

		++it;
	}
}

//======================================================================================================================

bool WorldManager::_handleRegionUpdate(uint64 callTime,void* ref)
{
	ActiveRegions::iterator it = mActiveRegions.begin();

	while(it != mActiveRegions.end())
	{
		(*it)->update();
		++it;
	}

	//now delete any camp regions that are due
	RegionDeleteList::iterator itR = mRegionDeleteList.begin();
	while(itR != mRegionDeleteList.end())
	{
		removeActiveRegion((*itR));
		//now remove region entries

		destroyObject(*itR);
		//delete(*itR);
		itR++;
	}

	mRegionDeleteList.clear();
	return(true);
}

//======================================================================================================================

int32 WorldManager::getPlanetIdByName(string name)
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

int32 WorldManager::getPlanetIdByNameLike(string name)
{
	uint8	id = 0;
	name.toLower();

	BStringVector::iterator it = mvPlanetNames.begin();

	while(it != mvPlanetNames.end())
	{
		// gLogger->logMsgF("Comparing: %s", MSG_NORMAL, name.getAnsi());
		// gLogger->logMsgF("with     : %s", MSG_NORMAL, (*it).getAnsi());
		if(Anh_Utils::cmpnistr((*it).getAnsi(),name.getAnsi(), 3) == 0)
		{
			// gLogger->logMsgF("Matched with planet id = %d", MSG_NORMAL, id);
			return (id);
		}
		++it;
		id++;
	}
	// gLogger->logMsgF("No match, compared %d planet names", MSG_NORMAL, id);
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
// returns a qtregion
//

QTRegion* WorldManager::getQTRegion(uint32 id)
{
	QTRegionMap::iterator it = mQTRegionMap.find(id);

	if(it != mQTRegionMap.end())
	{
		return((*it).second);
	}

	return(NULL);
}


//======================================================================================================================
//
// get an attribute string value from the global attribute map
//

string WorldManager::getAttributeKey(uint32 keyId)
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
	gLogger->logMsg("Loading world scripts...");

	ScriptList::iterator scriptIt = mWorldScripts.begin();

	while(scriptIt != mWorldScripts.end())
	{
		(*scriptIt)->run();

		++scriptIt;
	}
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
	string msg = (int8*)message.c_str();

	msg.convert(BSTRType_Unicode16);

	PlayerAccMap::iterator it = mPlayerAccMap.begin();

	while(it != mPlayerAccMap.end())
	{
		const PlayerObject* const player = (*it).second;

		if(player->isConnected())
		{
			gMessageLib->sendSystemMessage((PlayerObject*)player,msg);
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
	Buff* DestructibleBuff = buff;

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
		if (obj1->mPosition.distance2D(obj2->mPosition) > range)
		{
			inRange = false;
		}
	}
	else if ((obj1->getParentId() == 0) || (obj2->getParentId() == 0))
	{
		// One of us are outside.
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
			if (obj1->mPosition.distance2D(obj2->mPosition) > range)
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

bool WorldManager::objectsInRange(Anh_Math::Vector3 obj1Position,  uint64 obj1ParentId, uint64 obj2Id, float range)
{
	bool inRange = true;

	// Object* obj1 = dynamic_cast<Object*>(this->getObjectById(obj1Id));
	Object* obj2 = dynamic_cast<Object*>(this->getObjectById(obj2Id));

	// We have to be in the same building or outside.
	if (obj1ParentId == obj2->getParentId())
	{
		// In the same cell (or both outside is rarley the case here)
		if (obj1Position.distance2D(obj2->mPosition) > range)
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
			if (obj1Position.distance2D(obj2->mPosition) > range)
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
		const CreatureSpawnRegion *creatureSpawnRegion = (*it).second;
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
			gLogger->logMsgF("Player left: %"PRIu64", Total Players on zone : %i",MSG_NORMAL,player->getId(),(getPlayerAccMap())->size() -1);
			mPlayerAccMap.erase(playerAccIt);
		}
		else
		{
			gLogger->logErrorF("Worldmanager","WorldManager::destroyObject: error removing from playeraccmap : %u",MSG_HIGH,player->getAccountId());
		}
	}
	else
	{
		gLogger->logErrorF("Worldmanager","WorldManager::destroyObject: error removing from playeraccmap : %u",MSG_HIGH,player->getAccountId());
	}
}
