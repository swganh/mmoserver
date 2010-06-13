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

#ifndef ANH_ZONESERVER_OBJECT_CONTROLLER_H
#define ANH_ZONESERVER_OBJECT_CONTROLLER_H

#include <vector>
#include <set>
#include <algorithm>
#include <deque>
#include "Utils/PriorityVector.h"
#include "DatabaseManager/DatabaseCallback.h"
#include "ObjectFactoryCallback.h"
#include "HeightMapCallback.h"
#include "ObjControllerEvent.h"
#include <boost/pool/pool.hpp>

// maximum commands allowed to be queued
#define COMMAND_QUEUE_MAX_SIZE 10

// typedef void (ObjectController::*adminFuncPointer)(string message);
//=======================================================================

class Message;
class Object;
class ObjectControllerCmdProperties;
class ObjectControllerCommandMap;
class ObjControllerCommandMessage;
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
class ArtisanHeightmapAsyncContainer;
class StructureHeightmapAsyncContainer;

typedef std::set<Object*>				ObjectSet;

typedef std::vector<EnqueueValidator*>	EnqueueValidators;
typedef std::vector<ProcessValidator*>	ProcessValidators;

// typedef Anh_Utils::priority_vector<ObjControllerCommandMessage*,CompareCommandMsg >	CommandQueue;
typedef std::deque<ObjControllerCommandMessage*>	CommandQueue;
typedef Anh_Utils::priority_vector<ObjControllerEvent*,CompareEvent >				EventQueue;

//=======================================================================

enum OCQueryType
{
	OCQuery_Null			= 0,
	OCQuery_AddFriend		= 1,
	OCQuery_RemoveFriend	= 2,
	OCQuery_AddIgnore		= 3,
	OCQuery_RemoveIgnore	= 4,
	OCQuery_Nope			= 5,
	OCQuery_StatRead		= 6,
	OCQuery_FindFriend		= 7,
	OCQuery_CloneAtPreDes	= 8
};

//=======================================================================

enum OCCmdGroup
{
	ObjControllerCmdGroup_Common		= 0,
	ObjControllerCmdGroup_Attack		= 1,
	ObjControllerCmdGroup_Area_Attack	= 2
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

class ObjectController : public DatabaseCallback, public ObjectFactoryCallback, public HeightMapCallBack
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
		virtual void			heightMapCallback(HeightmapAsyncContainer *ref);

		void HeightmapArtisanHandler(HeightmapAsyncContainer* ref);
		void HeightmapStructureHandler(HeightmapAsyncContainer* ref);

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
		void					handleSetTarget(Message* message);

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

		//structures
		void					handleResourceEmptyHopper(Message* message);

		//permissionsystem
		bool					checkContainingContainer(uint64 containingContainer, uint64 playerId);
								// Utility
		bool					objectsInRange(uint64 obj1Id, uint64 obj2Id, float range);

		// Auto attack
		void					enqueueAutoAttack(uint64 targetId);

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
		void	_BurstRun(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);


		// postures
		void	_handleSitServer(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleStand(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleKneel(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleProne(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleClientLogout(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);

		// destroy handlers
		void	_handleServerDestroyObject(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleDestroyCraftingTool(CraftingTool* tool);
		void	_handleDestroyInstrument(Item* item);

		//travelling
		void	_handlePurchaseTicket(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);

		// spatial
		void	_handleSpatialChatInternal(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleSocialInternal(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);

		//datapad
		void	_handleRequestWaypointAtPosition(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleSetWaypointActiveStatus(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleWaypoint(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleSetWaypointName(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);

		void	_handleBoardTransport(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleSetMoodInternal(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleOpenContainer(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleCloseContainer(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleTransferItem(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleTransferItemMisc(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleTransferItemMisc2(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		bool	checkTargetContainer(uint64 targetId, Object* object);

		bool	removeFromContainer(uint64 targetContainerId, uint64 targetId);

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

		//modify Objects
		void	_handleResourceContainerTransfer(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleResourceContainerSplit(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_ExtractObject(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleFactoryCrateSplit(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);

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
		void	_handleThrowTrap(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);

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

		//structures
		void	_handleStructurePlacement(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleModifyPermissionList(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleTransferStructure(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleNameStructure(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleHarvesterGetResourceData(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleHarvesterSelectResource(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleHarvesterActivate(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleHarvesterDeActivate(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleDiscardHopper(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
    
    /// This command is used to move an item forward .1 of a meter.
    /**
     * This command is invoked by the client to move items forward by .1 of a 
     * meter while inside a structure.
     *
     * The client enters the message in the following format:
     *   /itemMoveForward
     *
     * @param target_id The targeted item is the one being moved.
     * @param message The message from the client requesting this command.
     * @param cmd_properties Contextual information for use during processing this command.
     */
		void HandleItemMoveForward_(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
    
    /// This command is used to move an item back .1 of a meter.
    /**
     * This command is invoked by the client to move items backwards by .1 of a 
     * meter while inside a structure.
     *
     * The client enters the message in the following format:
     *   /itemMoveBack
     *
     * @param target_id The targeted item is the one being moved.
     * @param message The message from the client requesting this command.
     * @param cmd_properties Contextual information for use during processing this command.
     */
		void HandleItemMoveBack_(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
    
    /// This command is used to move an item up .1 of a meter.
    /**
     * This command is invoked by the client to move items up by .1 of a meter
     * while inside a structure.
     *
     * The client enters the message in the following format:
     *   /itemMoveUp
     *
     * @param target_id The targeted item is the one being moved.
     * @param message The message from the client requesting this command.
     * @param cmd_properties Contextual information for use during processing this command.
     */
		void HandleItemMoveUp_(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
    
    /// This command is used to move an item down .1 of a meter.
    /**
     * This command is invoked by the client to move items down by .1 of a meter
     * while inside a structure.
     *
     * The client enters the message in the following format:
     *   /itemMoveDown
     *
     * @param target_id The targeted item is the one being moved.
     * @param message The message from the client requesting this command.
     * @param cmd_properties Contextual information for use during processing this command.
     */
		void HandleItemMoveDown_(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
    
    /// This command is used to rotate an item left by 90 degrees.
    /**
     * This command is invoked by the client to rotate items by 90 degrees
     * while inside a structure.
     *
     * The client enters the message in the following format:
     *   /itemRotateLeft
     *
     * @param target_id The targeted item is the one being rotated.
     * @param message The message from the client requesting this command.
     * @param cmd_properties Contextual information for use during processing this command.
     */
		void HandleItemRotateLeft_(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
    
    /// This command is used to rotate an item right by 90 degrees.
    /**
     * This command is invoked by the client to rotate items by 90 degrees
     * while inside a structure.
     *
     * The client enters the message in the following format:
     *   /itemRotateRight
     *
     * @param target_id The targeted item is the one being rotated.
     * @param message The message from the client requesting this command.
     * @param cmd_properties Contextual information for use during processing this command.
     */
		void HandleItemRotateRight_(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
				
    /// This command is used to rotate items in a structure.
    /**
     * This command is invoked by the client to rotate items around in a structure.
     *
     * Format: /rotateFurniture <LEFT/RIGHT> <degrees>
     * 
     * @param target_id The targeted item is the one being rotated.
     * @param message The message from the client requesting this command.
     * @param cmd_properties Contextual information for use during processing this command.
     */
    void HandleRotateFurniture_(uint64 target_id, Message* message, ObjectControllerCmdProperties* cmd_properties);

		//pets
		void	_handleMount(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleDismount(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		// admin
		void	_handleAdminSysMsg(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleAdminWarpSelf(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties);
		void	_handleBroadcast(uint64 targetId, Message* message, ObjectControllerCmdProperties* cmdProperties);
		void	_handleBroadcastPlanet(uint64 targetId, Message* message, ObjectControllerCmdProperties* cmdProperties);
		void	_handleBroadcastGalaxy(uint64 targetId, Message* message, ObjectControllerCmdProperties* cmdProperties);
		void	_handleShutdownGalaxy(uint64 targetId, Message* message, ObjectControllerCmdProperties* cmdProperties);
		void	_handleCancelShutdownGalaxy(uint64 targetId, Message* message, ObjectControllerCmdProperties* cmdProperties);


		string	handleBroadcast(string message) const;
		string	handleBroadcastPlanet(string message) const;
		string	handleBroadcastGalaxy(string message) const;
		string	handleShutdownGalaxy(string message) const;
		string handleCancelShutdownGalaxy(string message) const;

		// Admin
		int32	getAdminCommandFunction(string command) const;
		int32	indexOfFirstField(const string message) const;
		int32	indexOfNextField(const string message) const;
		void	broadcastGalaxyMessage(string theBroadcast, int32 planetId) const;
		void	scheduleShutdown(int32 scheduledTime, string shutdownReason) const;
		void	cancelScheduledShutdown(string cancelShutdownReason) const;
		void	sendAdminFeedback(string reply) const;
		string	removeWhiteSpace(string str) const;
		string	skipToNextField(string str) const;

		// spatial object updates
		float	_GetMessageHeapLoadViewingRange();
		void	_findInRangeObjectsOutside(bool updateAll);
		bool	_updateInRangeObjectsOutside();
		void	_findInRangeObjectsInside(bool updateAll);
		bool	_updateInRangeObjectsInside();
		bool	_destroyOutOfRangeObjects(ObjectSet* inRangeObjects);


		// ham
		bool	_consumeHam(ObjectControllerCmdProperties* cmdProperties);


		boost::pool<boost::default_user_allocator_malloc_free>		mCmdMsgPool;
		boost::pool<boost::default_user_allocator_malloc_free>		mDBAsyncContainerPool;
		boost::pool<boost::default_user_allocator_malloc_free>		mEventPool;

		CommandQueue				mCommandQueue;
		EventQueue					mEventQueue;
		ObjectSet						mInRangeObjects;
		ObjectSet::iterator mObjectSetIt;

		EnqueueValidators	mEnqueueValidators;
		ProcessValidators	mProcessValidators;

		Database*			mDatabase;
		ZoneTree*			mSI;
		Object*				mObject;

		uint64				mCommandQueueProcessTimeLimit;
		uint64				mEventQueueProcessTimeLimit;
		uint64				mNextCommandExecution;
		uint64				mTaskId;
		uint64				mUnderrunTime;			// time "missed" due to late arrival of command queue.
		int32				mMovementInactivityTrigger;
		uint32				mFullUpdateTrigger;

		bool				mDestroyOutOfRangeObjects;
		bool				mInUseCommandQueue;
		bool				mRemoveCommandQueue;
		bool				mUpdatingObjects;


};

//=======================================================================

#endif


