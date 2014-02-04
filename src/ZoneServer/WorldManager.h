/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2014 The SWG:ANH Team
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

#ifndef ANH_ZONESERVER_WORLDMANAGER_H
#define ANH_ZONESERVER_WORLDMANAGER_H

#include <list>
#include <map>
#include <unordered_map>
#include <vector>

#include <boost/ptr_container/ptr_unordered_map.hpp>

#include "Utils/TimerCallback.h"
#include "Utils/typedefs.h"

#include "MathLib/Rectangle.h"

#include "DatabaseManager/DatabaseCallback.h"

#include "ZoneServer/WorldManagerEnums.h"
#include "ZoneServer/GameSystemManagers/Spatial Index Manager/SpatialIndexManager.h"

//#include "ScriptEngine/ScriptEventListener.h"

#include "ZoneServer/Objects/ObjectFactoryCallback.h"
#include "ZoneServer/Objects/Tangible Object/TangibleEnums.h"
#include "ZoneServer/WorldManagerEnums.h"
#include "ZoneServer/Objects/RegionObject.h"

#include "ZoneServer/Weather.h"



//======================================================================================================================

#define	 gWorldManager	WorldManager::getSingletonPtr()

//======================================================================================================================

namespace	swganh	{
namespace	app	{
class	SwganhKernel;
}}

class DispatchClient;
class WMAsyncContainer;
class Script;
class NPCObject;
class CreatureSpawnRegion;
class Shuttle;
class NpcConversionTime;
class CharacterLoadingContainer;
class ZoneServer;
class Ham;
class Buff;
class MissionObject;
class Stomach;
class Weather;

//======================================================================================================================

namespace Anh_Utils
{
class Clock;
class Scheduler;
class VariableTimeScheduler;
}

// pwns all objects
typedef boost::ptr_unordered_map<uint64,Object>			ObjectMap;

typedef std::unordered_map<uint64, std::shared_ptr<Object>> SharedObjectMap;

typedef std::vector<std::string>						StringVector;

// Maps for objects in world
typedef std::map<uint32,const PlayerObject*>	        PlayerAccMap;
typedef std::map<uint64,std::shared_ptr<RegionObject>>	RegionMap;
typedef std::vector<std::shared_ptr<RegionObject>>		RegionDeleteList;

// Lists for objects in world
typedef std::list<PlayerObject*>				PlayerList;
typedef std::vector<Shuttle*>					ShuttleList;
typedef std::vector<std::shared_ptr<RegionObject>>  ActiveRegions;
typedef std::list<CreatureObject*>				CreatureQueue;
typedef std::vector<std::pair<uint64, NpcConversionTime*> >	NpcConversionTimers;
typedef std::map<uint64, uint64>				PlayerMovementUpdateMap;
typedef std::map<uint64, uint64>				CreatureObjectDeletionMap;
typedef std::map<uint64, uint64>				PlayerObjectReviveMap;

// a list of busy craft tools needing regular updates
typedef std::vector<uint64>						CraftTools;

// Creature spawn regions.
typedef std::map<uint64, const std::shared_ptr<CreatureSpawnRegion>>	CreatureSpawnRegionMap;

// Containers with handlers to Npc-objects handled by the NpcManager (or what we are going to call it its final version).
// The active container will be the most often checked, and the Dormant the less checked container.

// And yes. Handlers... handlers... no object refs that will be invalid all the time.
typedef std::map<uint64, uint64>				NpcDormantHandlers;
typedef std::map<uint64, uint64>				NpcReadyHandlers;
typedef std::map<uint64, uint64>				NpcActiveHandlers;
typedef std::map<uint64, uint64>				AdminRequestHandlers;

// AttributeKey map
typedef std::map<uint32,std::string>			AttributeKeyMap;
typedef std::map<uint32,uint32>					AttributeIDMap;

// non-persistent id set
typedef std::set<uint64>						NpIdSet;


//======================================================================================================================
//
// Container for asyncronous database queries
//
class WMAsyncContainer
{
public:

    WMAsyncContainer(WMQuery query) {
        mQuery = query;
        mObject = NULL;
        mClient = NULL;
        mBool = false;
    }

    WMLogOut					mLogout;
    WMQuery						mQuery;
    Object*						mObject;
    DispatchClient*				mClient;
    bool						mBool;
    CharacterLoadingContainer*	clContainer;
};

//======================================================================================================================
//
// container for simple queryresults
//
class WMQueryContainer
{
public:

    WMQueryContainer() {}

    uint64			mId;
    BString			mString;
};

//======================================================================================================================
//
// WorldManager
//
class WorldManager : public ObjectFactoryCallback, public swganh::database::DatabaseCallback, public TimerCallback
{
public:

    static WorldManager*	getSingletonPtr() {
        return mSingleton;
    }
    static WorldManager*	Init(uint32 zoneId, ZoneServer* zoneServer,swganh::app::SwganhKernel*	kernel, std::string trn, bool writeResourceMaps);
    void					Shutdown();

    void					Process();

	/*	@brief	gets the id of the current zone (planet) we are on
	*	please note that this is *not* the instance id and that until instancing is properly implemented
	*	the zoneId is used in some cases instead of the instance id
	*/
    uint32					getZoneId() {
        return mZoneId;
    }
    WMState					getState() {
        return mState;
    }
    uint64					getServerTime() {
        return mServerTime;
    }
    swganh::app::SwganhKernel*			getKernel() {
        return kernel_;
    }

    // swganh::database::DatabaseCallback
    virtual void			handleDatabaseJobComplete(void* ref,swganh::database::DatabaseResult* result);

    // ObjectFactoryCallback
    virtual void			handleObjectReady(Object* object,DispatchClient* client);

    virtual void            handleObjectReady(std::shared_ptr<Object>);

    // TimerCallback
    virtual void			handleTimer(uint32 id, void* container);

    // add / delete an object, make sure to cleanup any other references
    bool					existObject(Object* object);	// Returns true if object does exist.
    
	/*@brief	addObject adds an Object to the main Object Map
	*			AND if manual is set to true, to the Simulation
	*			Initialization happens based on Object type
	*			ToDo - this should (probably?) be separated
	*/
	bool				addObject(Object* object,bool manual = false);
    bool				addObject(std::shared_ptr<Object> object ,bool manual = false);
	bool				addSharedObject(std::shared_ptr<Object> &object, bool manual = false);

	void				initializeObject(std::shared_ptr <Object> &object);
	
	/*@brief	destroyObject removes an Object given by ObjId out of the world and destroys it
	*			If no Object with ObjId can be found in the WorldMap we LOG an error and
	*			bail out
	*/
	void					destroyObject(uint64 objId);

	/*@brief	destroyObject removes an Object given by its pointer out of the world and destroys it
	*			If the Objects ID cannot be found in the World Map we log an error and
	*			bail out
	*/
	void					destroyObject(Object* object);
	void					destroyObject(std::shared_ptr<Object> object);
		
	void					createObjectForKnownPlayers(PlayerObjectSet* knownPlayers, Object* object);
		
	
	Object*					getObjectById(uint64 objId);
	std::shared_ptr<Object>	getSharedObjectById(uint64 objId);

	void					eraseObject(uint64 key);

    // adds a creatures commandqueue to the main process queue
    uint64					addObjControllerToProcess(ObjectController* objController);
    void					removeObjControllerToProcess(uint64 taskId);

    // adds a creatures stomach which needs regeneration
    uint64					addCreatureDrinkToProccess(Stomach* stomach);
    uint64					addCreatureFoodToProccess(Stomach* stomach);
    void					removeCreatureStomachToProcess(uint64 taskId);
    bool					checkStomachTask(uint64 id);

    // adds a mission that needs checking
    uint64					addMissionToProcess(MissionObject* mission);
    void					removeMissionFromProcess(uint64 taskId);
    bool					checkForMissionProcess(uint64 taskId);

    // adds an performing entertainer which heals/gets exp
    uint64					addEntertainerToProccess(CreatureObject* entertainerObject,uint32 tick);
    void					removeEntertainerToProcess(uint64 taskId);

    uint64					addImageDesignerToProcess(CreatureObject* entertainerObject,uint32 tick);
    void					removeImagedesignerToProcess(uint64 taskId);

    // adds a Buff which Ticks
    uint64					addBuffToProcess(Buff* buff);
    void					removeBuffToProcess(uint64 taskId);

    // adds a save process
    uint64					getSaveTaskId() {
        return mSaveTaskId;
    }
    void					setSaveTaskId(uint64 taskId) {
        mSaveTaskId = taskId;
    }

    // saves a player asyncronously to the database
    void					savePlayer(uint32 accId,bool remove, WMLogOut mLogout, CharacterLoadingContainer* clContainer = NULL);

    // saves a player synched to the database
    void					savePlayerSync(uint32 accId,bool remove);

    // checks if the player save timer is up
    bool					checkSavePlayer(PlayerObject* playerObject);

    // find a player, returns NULL if not found
    PlayerObject*			getPlayerByAccId(uint32 accId);

    // adds a player to the timeout queue, will save and remove him, when timeout occurs
    void					addDisconnectedPlayer(PlayerObject* playerObject);

    // removes player from the timeout list and adds him to the world
    void					addReconnectedPlayer(PlayerObject* playerObject);

    // adds dead creature object to the pool of objects with delayed destruction.
    void					addCreatureObjectForTimedDeletion(uint64 creatureId, uint64 when);

    // adds dead object to the pool of objects to be send to nearest cloning facility.
    void					addPlayerObjectForTimedCloning(uint64 playerId, uint64 when);

    // remove dead player object from the pool of objects to be send to nearest cloning facility.
    void					removePlayerObjectForTimedCloning(uint64 playerId);

    // removes player from the timeout list
    void					removePlayerFromDisconnectedList(PlayerObject* playerObject);

    // adds a shuttle
    void					addShuttle(Shuttle* shuttle) {
        mShuttleList.push_back(shuttle);
    }

    // add / remove busy crafting tools
    void					addBusyCraftTool(CraftingTool* tool);
    void					removeBusyCraftTool(CraftingTool* tool);

    // add / remove expired npc conversations.
    void					addNpcConversation(uint64 interval, NPCObject* npc);

    // add player update of known objects and position in world.
    void					addPlayerMovementUpdateTime(PlayerObject* player, uint64 when);


    // check if objects are in range. Handles cell and buildings, but not distance between buildings or buildings and outside.
    bool					objectsInRange(uint64 obj1Id, uint64 obj2Id, float range);
    bool					objectsInRange(const glm::vec3& obj1Position, uint64 obj1ParentId, uint64 obj2Id, float range);

    // Add-remove npc from Npc-handler queue's.
    void					addDormantNpc(uint64 creature, uint64 when);
    void					removeDormantNpc(uint64 creature);
    void					forceHandlingOfDormantNpc(uint64 creature);

    void					addReadyNpc(uint64 creature, uint64 when);
    void					removeReadyNpc(uint64 creature);
    void					forceHandlingOfReadyNpc(uint64 creature);

    void					addActiveNpc(uint64 creature, uint64 when);
    void					removeActiveNpc(uint64 creature);

    void					addAdminRequest(uint64 requestId, uint64 when);
    void					cancelAdminRequest(int32 requestId);

    const					Anh_Math::Rectangle getSpawnArea(uint64 spawnRegionId);

    
    const PlayerAccMap*		getPlayerAccMap() {
        return &mPlayerAccMap;
    }
    ShuttleList*			getShuttleList() {
        return &mShuttleList;
    }
    ObjectIDList*			getStructureList(){
        return &mStructureList; 
    }
    // delete Player off of the accountmap
    void					removePlayerfromAccountMap(uint64 playerID);

    // retrieve spatial index for this zone

    // removes player from the current scene, and starts a new one after updating his position
    void					warpPlanet(PlayerObject* playerObject, const glm::vec3& destination,uint64 parentId, const glm::quat& direction = glm::quat());

    // get a client effect string by its id
    std::string				getClientEffect(uint32 effectId) {
        return mvClientEffects[effectId - 1];
    }

    // get sound string by its id
    std::string 			getSound(uint32 soundId) {
        return mvSounds[soundId - 1];
    }
    // get a mood string by its id
    std::string					getMood(uint32 moodId) {
        return mvMoods[moodId];
    }
    // get an attribute key
    //BString					getAttributeKey(uint32 keyId);

	std::string				getAttributeKey(uint32 keyId);
    // get an attribute ID
    uint32					getAttributeId(uint32 keyId);
    // get a npc animation
    std::string					getNpcConverseAnimation(uint32 animId) {
        return mvNpcConverseAnimations[animId - 1];
    }
    // get a random chat phrase
    std::pair<std::wstring,uint32>	getNpcChatter(uint32 id) {
        return mvNpcChatter[id];
    }
    std::pair<std::wstring,uint32>	getRandNpcChatter();

    // get planet, trn file name
    const int8* getPlanetNameThis() const {
        return mvPlanetNames[mZoneId].c_str();
    }
    const int8* getPlanetNameById(uint8 planetId) const {
		return mvPlanetNames[planetId].c_str();
    }
    int32					getPlanetIdByName(std::string name);
    int32					getPlanetIdByNameLike(std::string name);


    const int8* getTrnFileThis() const {
        return mvTrnFileNames[mZoneId].c_str();
    }
    const int8* getTrnFileById(uint8 trnId) const {
        return mvTrnFileNames[trnId].c_str();
    }

    WMState					getWMState() {
        return mState;
    }

    // get total count of planets
    uint32					getPlanetCount() {
        return mvPlanetNames.size();
    }
  
    Anh_Utils::Scheduler*	getPlayerScheduler() {
        return mPlayerScheduler;
    }

    Weather*				getCurrentWeather() {
        return &mCurrentWeather;
    }
    void					updateWeather(float cloudX,float cloudY,float cloudZ,uint32 weatherType);
    void					zoneSystemMessage(std::string message);

    void					ScriptRegisterEvent(void* script,std::string eventFunction);

	/*	@brief returns a pointer to the subsystem scheduler
	*
	*
	*/
	Anh_Utils::Scheduler*	getSubsystemScheduler(){ return mSubsystemScheduler;}


    // non-persistent ids in use
    uint64					getRandomNpId();
    bool					removeNpId(uint64 id);
    bool					checkdNpId(uint64 id);
    uint64					getRandomNpNpcIdSequence();

    //get the current tick
    uint64					GetCurrentGlobalTick();

    //load the tick from db
    void					LoadCurrentGlobalTick();

    bool					_handleTick(uint64 callTime,void* ref);
    

    void					removePlayerMovementUpdateTime(PlayerObject* player);

    //find objects in the world
    Object*					getNearestTerminal(PlayerObject* player, TangibleType terminalType, float searchrange = 32);
    
    ~WorldManager();

    AttributeKeyMap				mObjectAttributeKeyMap;
    AttributeIDMap				mObjectAttributeIDMap;
private:

    WorldManager(uint32 zoneId, ZoneServer* zoneServer,swganh::app::SwganhKernel*	kernel, std::string trn, bool writeResourceMaps);

    // load the global ObjectControllerCommandMap, maps command crcs to ObjController function pointers
    void	_loadObjControllerCommandMap();

    // initializations after completed object load
    void	_handleLoadComplete();

    bool					addNpId(uint64 id);

    // timed subsystems
    bool	_handleServerTimeUpdate(uint64 callTime,void* ref);
    bool	_handleShuttleUpdate(uint64 callTime,void* ref);
    bool	_handleDisconnectUpdate(uint64 callTime,void* ref);
    bool	_handleCraftToolTimers(uint64 callTime,void* ref);
    bool	_handleNpcConversionTimers(uint64 callTime,void* ref);
    bool	_handleFireworkLaunchTimers(uint64 callTime,void* ref);
    bool	_handleVariousUpdates(uint64 callTime, void* ref);

//		Save players, who haven't saved in x minutes
    bool	_handlePlayerSaveTimers(uint64 callTime, void* ref);

    bool	_handlePlayerMovementUpdateTimers(uint64 callTime, void* ref);

    bool	_handleGeneralObjectTimers(uint64 callTime, void* ref);
    bool	_handleGroupObjectTimers(uint64 callTime, void* ref);

    bool	_handleDormantNpcs(uint64 callTime, void* ref);
    bool	_handleReadyNpcs(uint64 callTime, void* ref);
    bool	_handleActiveNpcs(uint64 callTime, void* ref);

    bool	_handleAdminRequests(uint64 callTime, void* ref);

    void	_startWorldScripts();

    // process schedulers
    void	_processSchedulers();

    // New Method of Loading objects from DB.
    void    _loadWorldObjects();

    // load buildings and their contents
    void	_loadBuildings();

    // loads all child objects of the given parent
    void	_loadAllObjects(uint64 parentId);

    // planet names and neceessary terrain file names
    void    _loadPlanetNamesAndFiles();

    // load our script hooks
    void	_registerScriptHooks();


    /** Stores the characters position in the database asynchronously.
    *
    * \param player_object The Player object to save.
    * \param logout_type The type of logout. This is somewhat ambiguously named for the time being.
    * \param clContainer Another legacy data variable. This will go away in future commit.
    */
    void storeCharacterPosition_(PlayerObject* player_object, bool remove, WMLogOut logout_type, CharacterLoadingContainer* clContainer);
    
    /** Stores the characters attributes in the database asynchronously.
    *
    * Has a side effect that after a successful update if the player is logging
    * out their player object is deleted. This will go away when we switch to
    * using a logout event that services listen to and respond by doing the actions
    * currently handled within this member function.
    *
    * \param player_object The Player object to save.
    * \param remove Whether or not to remove the player.
    * \param logout_type The type of logout. This is somewhat ambiguously named for the time being.
    * \param clContainer Another legacy data variable. This will go away in future commit.
    */
    void storeCharacterAttributes_(PlayerObject* player_object, bool remove, WMLogOut logout_type, CharacterLoadingContainer* clContainer);


    static WorldManager*		mSingleton;
    static bool					mInsFlag;

    boost::pool<boost::default_user_allocator_malloc_free>	mWM_DB_AsyncPool;

    AdminRequestHandlers		mAdminRequestHandlers;
    CreatureObjectDeletionMap	mCreatureObjectDeletionMap;
    CreatureSpawnRegionMap		mCreatureSpawnRegionMap;
    NpcActiveHandlers			mNpcActiveHandlers;
    NpcDormantHandlers			mNpcDormantHandlers;
    NpcReadyHandlers			mNpcReadyHandlers;
    ObjectIDList			    mStructureList;
    

	SharedObjectMap				object_map_;
    
	PlayerAccMap				mPlayerAccMap;
    PlayerMovementUpdateMap		mPlayerMovementUpdateMap;
    PlayerObjectReviveMap		mPlayerObjectReviveMap;
    //RegionMap                   mRegionMap;
    NpIdSet						mUsedTmpIds;
    std::vector<std::string>	mvClientEffects;
    std::vector<std::string>    mvMoods;
    std::vector<std::string>	mvNpcConverseAnimations;
    std::vector<std::string>    mvSounds;

	StringVector				mvPlanetNames;
    StringVector				mvTrnFileNames;
    
	//todo make id lists
	ActiveRegions				mActiveRegions;
    CraftTools					mBusyCraftTools;
    
	std::vector<std::pair<std::wstring,uint32> >	mvNpcChatter;
    NpcConversionTimers			mNpcConversionTimers;
    PlayerList					mPlayersToRemove;
    RegionDeleteList			mRegionDeleteList;
    ShuttleList					mShuttleList;
    //ScriptList					mWorldScripts;
    CreatureQueue				mObjControllersToProcess;
    Weather						mCurrentWeather;
    //ScriptEventListener			mWorldScriptsListener;
    Anh_Utils::Scheduler*		mAdminScheduler;
    Anh_Utils::VariableTimeScheduler* mBuffScheduler;

    Anh_Utils::Scheduler*		mEntertainerScheduler;
    Anh_Utils::Scheduler*		mScoutScheduler;
    Anh_Utils::Scheduler*		mHamRegenScheduler;
    Anh_Utils::Scheduler*		mStomachFillingScheduler;
    Anh_Utils::Scheduler*		mMissionScheduler;
    Anh_Utils::Scheduler*		mNpcManagerScheduler;
    Anh_Utils::Scheduler*		mObjControllerScheduler;
    Anh_Utils::Scheduler*		mPlayerScheduler;
    Anh_Utils::Scheduler*		mSubsystemScheduler;
    ZoneServer*					mZoneServer;
    WMState						mState;
    uint64						mNonPersistantId;
    uint64						mObjControllersProcessTimeLimit;
    uint64						mServerTime;
    uint64						mTick;
    uint32						mTotalObjectCount;


    uint64						mSaveTaskId;

	/*	@brief the kernel stores the baseservices that are made accessible to the entire simulation
	*	this includes the db, the messageservice, the terrain service and all other services that will be added in the future
	*	like for example the resource service or structure service
	*/	
	swganh::app::SwganhKernel*	kernel_;

	//make this a vectored struct at some point whe muli instances are in
	uint32						mZoneId;
	std::string					mTrn;
	//swganh::database::Database*	mDatabase;
};



//======================================================================================================================


#endif // ANH_ZONESERVER_WORLDMANAGER_H



