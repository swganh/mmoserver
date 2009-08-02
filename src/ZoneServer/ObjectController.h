/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_OBJECT_CONTROLLER_H
#define ANH_ZONESERVER_OBJECT_CONTROLLER_H

#include <vector>
#include <set>
#include <algorithm>
#include "Utils/PriorityVector.h"
#include "Utils/Clock.h"
#include "ObjControllerCommandMessage.h"
#include "ObjControllerEvent.h"
#include "DatabaseManager/DatabaseCallback.h"
#include "ObjectFactoryCallback.h"
#include <boost/pool/pool.hpp>

// maximum commands allowed to be queued
#define COMMAND_QUEUE_MAX_SIZE 10

//=======================================================================

class Message;
class Object;
class ObjectControllerCommandMap;
class ObjectFactory;
class Database;
class DatabaseResult;
class ZoneTree;
class ObjControllerAsyncContainer;
class UIWindow;
class EnqueueValidator;
class ProcessValidator;
class PlayerObject;
class CraftingTool;
class Item;
class SpawnPoint;

typedef std::set<Object*>				ObjectSet;

typedef std::vector<EnqueueValidator*>	EnqueueValidators;
typedef std::vector<ProcessValidator*>	ProcessValidators;

typedef Anh_Utils::priority_vector<ObjControllerCommandMessage*,CompareCommandMsg >	CommandQueue;
typedef Anh_Utils::priority_vector<ObjControllerEvent*,CompareEvent >				EventQueue;

//=======================================================================

enum OCQueryType
{
	OCQuery_AddFriend		= 1,
	OCQuery_RemoveFriend	= 2,
	OCQuery_AddIgnore		= 3,
	OCQuery_RemoveIgnore	= 4,
	OCQuery_Nope			= 5,
	OCQuery_StatRead		= 6,
	OCQuery_FindFriend		= 7,
	OCQuery_CloneAtPreDes	= 8,
};

//=======================================================================

enum OCCmdGroup
{
	ObjControllerCmdGroup_Common		= 0,
	ObjControllerCmdGroup_Attack		= 1,
	ObjControllerCmdGroup_Area_Attack	= 2,
};

//=======================================================================

struct StatTargets
{
	uint32 TargetHealth;
	uint32 TargetStrength;
	uint32 TargetConstitution;
	uint32 TargetAction;
	uint32 TargetQuickness;
	uint32 TargetStamina;
	uint32 TargetMind;
	uint32 TargetFocus;
	uint32 TargetWillpower;
};

struct MenuItem
{
	uint8	sItem;
	uint8	sSubMenu;
	uint8	sIdentifier;
	uint8	sOption;
	
};

typedef std::vector<MenuItem*>	MenuItemList;
//=======================================================================

class ObjControllerAsyncContainer
{
	public:

		ObjControllerAsyncContainer(OCQueryType qt){ mQueryType = qt;}
		~ObjControllerAsyncContainer(){}

		OCQueryType		mQueryType;
		string			mString;
		PlayerObject*	playerObject;
		void*			anyPtr;			// generall purpose pointer.
};

//=======================================================================

class ObjectController : public DatabaseCallback, public ObjectFactoryCallback
{
	friend class ObjectControllerCommandMap;

	public:

		ObjectController();
		ObjectController(Object* object);
		~ObjectController();

		// inherited callback
		virtual void	handleObjectReady(Object* object,DispatchClient* client);

		// process the command queues
		bool					process(uint64 callTime,void*);

		// inherited callbacks
		virtual void			handleDatabaseJobComplete(void* ref,DatabaseResult* result);

		// Anh_Utils::Clock*		getClock(){ return mClock; }

		// object
		void					setObject(Object* object){ mObject = object; }
		Object*					getObject(){ return mObject; }

		// scheduler task id
		uint64					getTaskId(){ return mTaskId; }
		void					setTaskId(uint64 id){ mTaskId = id; }

		// event queue
		EventQueue*				getEventQueue(){ return &mEventQueue; }
		void					addEvent(Anh_Utils::Event* event,uint64 timeDelta);

		// command queue
		void					initEnqueueValidators();
		void					initProcessValidators();
		void					enqueueCommandMessage(Message* message);
		void					removeCommandMessage(Message* message);
		void					removeMsgFromCommandQueueBySequence(uint32 sequemce);
		void					removeMsgFromCommandQueue(uint32 opcode);
		CommandQueue*			getCommandQueue(){ return &mCommandQueue; }
		bool					mHandlerCompleted;

		void					clearQueues();

		// target
		void					setTarget(Message* message);

		// radial request
		void					handleObjectMenuRequest(Message* message);

		// movement updates
		void					handleDataTransform(Message* message,bool inRangeUpdate);
		void					handleDataTransformWithParent(Message* message,bool inRangeUpdate);
		uint64					playerWorldUpdate(bool forcedUpdate);	// Is called from the two above AND from timer function. We need updates even when client are not moving the player.

		// trade
		void					handleSecureTradeInvitation(uint64 targetId,Message* message);

		// crafting
		void					handleCraftFillSlot(Message* message);
		void					handleCraftEmptySlot(Message* message);
		void					handleCraftExperiment(Message* message);
		void					handleCraftCustomization(Message* message);
		
		//Imagedesign
		void					handleImageDesignChangeMessage(Message* message,uint64 targetId);
		void					handleImageDesignStopMessage(Message* message,uint64 targetId);
		void					handleStatMigrationMessage(Message* message,uint64 targetId);
		
		// destroy an object
		void					destroyObject(uint64 objectId);

		// Cloning at pre-designated facility
		void					cloneAtPreDesignatedFacility(PlayerObject* player, SpawnPoint* spawnPoint);

		// Loot all.
		void					lootAll(uint64 targetId, PlayerObject* playerObject);

		// missions
        void                    handleMissionListRequest(Message* message);
        void                    handleMissionDetailsRequest(Message* message);
        void                    handleMissionCreateRequest(Message* message);
		void					handleGenericMissionRequest(Message* message);
		void					handleMissionAbort(Message* message);

								// Utility
		bool					objectsInRange(uint64 obj1Id, uint64 obj2Id, float range);



		ObjectSet*				getInRangeObjects(){return(&mInRangeObjects);}
		ObjectSet::iterator		getInRangeObjectsIterator(){return mObjectSetIt;}

	private:

		// validate command
		bool	_validateEnqueueCommand(uint32 &reply1,uint32 &reply2,uint64 targetId,uint32 opcode,ObjectControllerCmdProperties*& cmdProperties);
		bool	_validateProcessCommand(uint32 &reply1,uint32 &reply2,uint64 targetId,uint32 opcode,ObjectControllerCmdProperties*& cmdProperties);
		
		// process queues
		bool	_processCommandQueue();
		bool	_processEventQueue();

		// command methods
		void	_handleTeach(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleMatch(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleLoot(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);

		// postures
		void	_handleSitServer(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleStand(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleKneel(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleProne(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);

		// destroy handlers
		void	_handleServerDestroyObject(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleDestroyCraftingTool(CraftingTool* tool);
		void	_handleDestroyInstrument(Item* item);

		//travelling
		void	_handlePurchaseTicket(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);

		// spatial
		void	_handleSpatialChatInternal(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleSocialInternal(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);

		void	_handleBoardTransport(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleSetMoodInternal(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleOpenContainer(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleCloseContainer(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleTransferItem(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleTransferItemMisc(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleRequestQuestTimersAndCounters(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleNPCConversationStart(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleNPCConversationStop(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleNPCConversationSelect(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		
		void	_handleGetAttributesBatch(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleTarget(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleRequestDraftslotsBatch(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleRequestResourceWeightsBatch(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleSynchronizedUIListen(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleSetCurrentSkillTitle(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleRequestBadges(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleSetSpokenLanguage(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleLfg(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleNewbieHelper(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleRolePlay(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleToggleAFK(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleToggleDisplayFactionRank(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleAnon(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleResourceContainerTransfer(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleResourceContainerSplit(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleRequestWaypointAtPosition(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleSetWaypointActiveStatus(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleWaypoint(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleSetWaypointName(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleRequestCharacterSheetInfo(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleRequestBiography(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleSetBiography(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleEditBiography(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleSurrenderSkill(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleClientQualifiedForSkill(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleRequestCharacterMatch(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleTip(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleAddFriend(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleRemoveFriend(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleAddIgnore(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleRemoveIgnore(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleNewbieSelectStartingLocation(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);

		// combat
		void	_handleDuel(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleEndDuel(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handlePeace(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleDeathBlow(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		//attackhandlers are NOT part of the commandMap theyll get handled separately!!!

		// brawler
		void	_handleBerserk1(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleCenterOfBeing(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleIntimidate1(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleTaunt(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleWarcry1(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleBerserk2(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleIntimidate2(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleWarcry2(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);

		// marksman
		void	_handleTumbleToKneeling(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleTumbleToProne(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleTumbleToStanding(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleTakeCover(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleAim(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);

		// custom methods
		void	_endBurstRun(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);

		// friend / ignore replies
		void	_handleAddFriendDBReply(uint32 retCode,string friendName);
		void	_handleFindFriendDBReply(uint64 retCode,string friendName);
		void	_handleRemoveFriendDBReply(uint32 retCode,string friendName);
		void	_handleAddIgnoreDBReply(uint32 retCode,string ignoreName);
		void	_handleRemoveIgnoreDBReply(uint32 retCode,string ignoreName);
		void	_handlefindfriend(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);

		// groups
		void	_handleInvite(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleUninvite(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleJoin(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleDecline(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleDisband(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleLeaveGroup(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleMakeLeader(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleDismissGroupMember(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleGroupChat(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleGroupLootMode(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleMakeMasterLooter(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);

		//entertainer
		void	_handlestartdance(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handlestopdance(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handlestartmusic(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handlestopmusic(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleflourish(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handlestopwatching(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handlestoplistening(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		
		void	_handlewatch(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handlelisten(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handlePauseDance(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handlePauseMusic(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleChangeDance(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleChangeMusic(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleDenyService(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleStartBand(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleStopBand(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleBandFlourish(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleDazzle(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleFireJet(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleDistract(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleColorLights(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleSmokeBomb(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleSpotLight(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleVentriloquism(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		
		// ImageDesigner
		void	_handleImageDesign(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleStatMigration(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleRequestStatMigrationData(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handlePlayHoloEmote(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
	
		//artisan
		void	_handleRequestCraftingSession(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleCancelCraftingSession(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleSelectDraftSchematic(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleNextCraftingStage(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleCreatePrototype(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleCreateManufactureSchematic(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleRequestSurvey(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleSurvey(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleRequestCoreSample(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleSample(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);

		// scout
		void	_handleHarvestCorpse(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleMaskScent(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleForage(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);

		// medic
		void	_handleDiagnose(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleHealDamage(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleHealWound(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleMedicalForage(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleTendDamage(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleTendWound(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleFirstAid(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleQuickHeal(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleDragIncapacitatedPlayer(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);

		// bio - engineer
		void	_handleSampleDNA(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);

		// combat medic
		void	_handleApplyPoison(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleApplyDisease(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleHealMind(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);

		// doctor
		void	_handleHealState(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleCurePoison(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleCureDisease(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleExtinguishFire(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleHealEnhance(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleRevivePlayer(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);

		// ranger
		void	_handleAreaTrack(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleConceal(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleRescue(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);

		// smuggler
		void	_handleFeignDeath(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		
		// squadleader
		void	_handleSysGroup(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleSteadyAim(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleVolleyFire(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleFormup(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleBoostMorale(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleRally(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleRetreat(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);

		// teras kasi
		void	_handleMeditate(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handlePowerBoost(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleForceOfWill(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		
		// force defense
		void	_handleAvoidIncapacitation(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);

		// force enhancement
		void	_handleForceAbsorb1(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleForceAbsorb2(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleForceSpeed1(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleForceSpeed2(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleForceRun1(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleForceRun2(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleForceRun3(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleForceFeedback1(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleForceFeedback2(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleForceArmor1(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleForceArmor2(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleForceResistBleeding(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleForceResistDisease(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleForceResistPoison(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleForceResistStates(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleTransferForce(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleChannelForce(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleDrainForce(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleForceShield1(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleForceShield2(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleForceMeditate(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleRegainConsciousness(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);

		// force healing
		void	_handleHealAllSelf1(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleHealAllSelf2(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleHealHealthSelf1(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleHealHealthSelf2(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleHealActionSelf1(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleHealActionSelf2(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleHealMindSelf1(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleHealMindSelf2(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleHealHealthWoundSelf1(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleHealHealthWoundSelf2(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleHealActionWoundSelf1(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleHealActionWoundSelf2(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleHealMindWoundSelf1(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleHealMindWoundSelf2(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleHealBattleFatigueSelf1(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleHealBattleFatigueSelf2(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleHealHealthWoundOther1(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleHealHealthWoundOther2(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleHealActionWoundOther1(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleHealActionWoundOther2(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleHealMindWoundOther1(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleHealMindWoundOther2(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleHealAllOther1(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleHealAllOther2(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleHealStatesSelf(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleHealStatesOther(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleStopBleeding(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleForceCureDisease(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleForceCurePoison(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleTotalHealSelf(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleTotalHealOther(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);

		// force powers
		void	_handleAnimalScare(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleForceLightningSingle1(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleForceLightningSingle2(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleForceLightningCone1(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleForceLightningCone2(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleMindblast1(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleMindblast2(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleAnimalCalm(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleAnimalAttack(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleForceWeaken1(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleForceWeaken2(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleForceIntimidate1(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleForceIntimidate2(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleForceThrow1(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleForceThrow2(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleForceKnockdown1(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleForceKnockdown2(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleForceKnockdown3(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleForceChoke(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleJediMindTrick(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);

		void	_handleStructurePlacement(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);

		//pets
		void	_handleMount(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleDismount(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		// admin
		void	_handleAdminSysMsg(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleAdminWarpSelf(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);

		// spatial object updates
		void	_findInRangeObjectsOutside(bool updateAll);
		bool	_updateInRangeObjectsOutside();
		void	_findInRangeObjectsInside(bool updateAll);
		bool	_updateInRangeObjectsInside();
		bool	_destroyOutOfRangeObjects(ObjectSet* inRangeObjects);
		
		
		// ham
		bool	_consumeHam(ObjectControllerCmdProperties* cmdProperties);



		uint64				mTaskId;
		Database*			mDatabase;
		ZoneTree*			mSI;
		Object*				mObject;

		CommandQueue		mCommandQueue;
		EventQueue			mEventQueue;

		EnqueueValidators	mEnqueueValidators;
		ProcessValidators	mProcessValidators;

		uint64				mCommandQueueProcessTimeLimit;
		uint64				mEventQueueProcessTimeLimit;
		uint64				mNextCommandExecution;
		uint64				mNextCombatCmdExecution;
		
		bool				mUpdatingObjects;
		bool				mDestroyOutOfRangeObjects;
		
		ObjectSet			mInRangeObjects;
		ObjectSet::iterator mObjectSetIt;
		uint32				mFullUpdateTrigger;
		int32				mMovementInactivityTrigger;

		boost::pool<boost::default_user_allocator_malloc_free>		mCmdMsgPool;
		boost::pool<boost::default_user_allocator_malloc_free>		mEventPool;
		boost::pool<boost::default_user_allocator_malloc_free>		mDBAsyncContainerPool;
};

//=======================================================================

#endif


