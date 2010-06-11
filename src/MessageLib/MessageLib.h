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

#ifndef ANH_ZONESERVER_MESSAGELIB_H
#define ANH_ZONESERVER_MESSAGELIB_H

#include "Utils/typedefs.h"
//#include "Utils/typedefs.h"
//#include "ZoneServer/ObjectFactory.h"
#include "ZoneServer/ObjectController.h"
#include "ZoneServer/Skill.h"   //for skillmodslist

#include "Common/bytebuffer.h"
#include <vector>
#include <list>
#include <glm/glm.hpp>

#define	 gMessageLib	MessageLib::getSingletonPtr()

class MessageFactory;
class Item;
class IntangibleObject;
class BuildingObject;
class TravelTerminal;
class WaypointObject;
class NPCObject;
class CreatureObject;
class Object;
class Skill;
class ResourceContainer;
class TangibleObject;
class HarvesterObject;
class HouseObject;
class FactoryObject;
class ManufacturingSchematic;
class PlayerObject;
class MovingObject;
class MountObject;
class MissionObject;
class CellObject;
class StaticObject;
class PlayerStructure;
class FactoryCrate;

class DispatchClient;
class DraftSchematic;
class CurrentResource;
class ConversationOption;
class CraftingTool;
class ActiveConversation;

typedef struct tagResourceLocation ResourceLocation;

typedef std::set<PlayerObject*>			PlayerObjectSetML;
typedef std::list<PlayerObject*>		PlayerList;

enum ObjectUpdate
{
	ObjectUpdateRewriteAll	= 0,
	ObjectUpdateDelete		= 1,
	ObjectUpdateAdd			= 2,
	ObjectUpdateChange		= 3
};

//======================================================================================================================

class MessageLib
{
public:

	static MessageLib*	getSingletonPtr() { return mSingleton; }
	static MessageLib*	Init();

	// multiple messages, messagelib.cpp
	bool				sendCreateObject(Object* object,PlayerObject* player,bool sendSelftoTarget = true);
	bool				sendCreateManufacturingSchematic(ManufacturingSchematic* manSchem,PlayerObject* playerObject,bool attributes = true);
	bool				sendCreateResourceContainer(ResourceContainer* resourceContainer,PlayerObject* targetObject);
	bool				sendCreateFactoryCrate(FactoryCrate* crate,PlayerObject* targetObject);
	bool				sendCreateTangible(TangibleObject* tangibleObject,PlayerObject* targetObject, bool sendchildren = true);
	void				sendCreateTangible(TangibleObject* tangibleObject, PlayerObjectSetML*	knownPlayers, bool sendchildren = true) ;
	bool				sendCreateStaticObject(TangibleObject* tangibleObject,PlayerObject* targetObject);
	bool				sendCreateInTangible(IntangibleObject* intangibleObject, uint64 containmentId, PlayerObject* targetObject);

	// structures
	bool				sendCreateInstallation(PlayerStructure* structure,PlayerObject* player);
	bool				sendCreateStructure(PlayerStructure* structure,PlayerObject* player);
	bool				sendCreateHarvester(HarvesterObject* harvester,PlayerObject* player);
	bool				sendCreateFactory(FactoryObject* factory,PlayerObject* player);
	bool				sendCreateBuilding(BuildingObject* buildingObject,PlayerObject* playerObject);
	bool				sendCreateCamp(TangibleObject* camp,PlayerObject* player);
	
	// creatures
	bool				sendCreateCreature(CreatureObject* creatureObject,PlayerObject* targetObject);
	bool				sendCreatePlayer(PlayerObject* playerObject,PlayerObject* targetObject);
	

	void				sendInventory(PlayerObject* playerObject);
	bool				sendEquippedItems(PlayerObject* srcObject,PlayerObject* targetObject);
	

	// common messages, commonmessages.cpp
	bool				sendCreateObjectByCRC(Object* object,const PlayerObject* const targetObject,bool player) const;
	bool				sendContainmentMessage(uint64 objectId,uint64 parentId,uint32 linkType,const PlayerObject* const targetObject) const;
	bool				sendContainmentMessage_InRange(uint64 objectId,uint64 parentId,uint32 linkType,PlayerObject* targetObject);
	bool				sendContainmentMessage_InRange(uint64 objectId,uint64 parentId,uint32 linkType,CreatureObject* targetObject);
	bool				broadcastContainmentMessage(uint64 objectId,uint64 parentId,uint32 linkType,PlayerObject* targetObject);
	bool				broadcastContainmentMessage(Object* targetObject,uint64 parentId,uint32 linkType);	// Used by Creatures.
	bool				sendOpenedContainer(uint64 objectId, PlayerObject* targetObject);
	bool				sendPostureMessage(CreatureObject* creatureObject,PlayerObject* targetObject);
	bool				sendEndBaselines(uint64 objectId,const PlayerObject* const targetObject) const;
	bool				sendDestroyObject(uint64 objectId, PlayerObject* targetObject);
	bool				sendDestroyObject(uint64 objectId, CreatureObject* owner);
	bool				sendDestroyObject_InRange(uint64 objectId,PlayerObject* const owner, bool self);
	bool				sendDestroyObject_InRangeofObject(Object* object);
	void				sendGroupLeaderRequest(PlayerObject* sender, uint64 requestId, uint32 operation, uint64 groupId);
	bool				sendLogout(PlayerObject* playerObject);

	bool				sendHeartBeat(DispatchClient* client);
	bool				sendChatServerStatus(uint8 unk1,uint8 unk2,DispatchClient* client);
	bool				sendParameters(uint32 parameters,DispatchClient* client);
	bool				sendStartScene(uint64 zoneId,PlayerObject* player);
	bool				sendSceneReady(DispatchClient* client);
	bool				sendSceneReadyToChat(DispatchClient* client);
	bool				sendServerTime(uint64 time,DispatchClient* client);
	void				sendWeatherUpdate(const glm::vec3& cloudVec, uint32 weatherType, PlayerObject* player = NULL);
	
	bool				sendSysMsg(PlayerObject* playerObject,string mainFile,string mainVar,Object* to= NULL, Object* tt = NULL, Object* tu = NULL, int32 di = 0);
	
	bool				sendSystemMessage(PlayerObject* playerObject, std::wstring customMessage = L"", std::string mainFile = "",
											std::string mainVar = "",std::string toFile = "",std::string toVar = "", std::wstring toCustom = L"",int32 di = 0,
											std::string ttFile = "",std::string ttVar = "",std::wstring ttCustom = L"",uint64 ttId = 0,uint64 toId = 0,uint64 tuId = 0,
											std::string tuFile = "",std::string tuVar = "",std::wstring tuCustom = L"");

	bool				sendSystemMessageInRange(PlayerObject* playerObject, bool toSelf, string customMessage = L"",string mainFile = "",
											string mainVar = "",string toFile = "",string toVar = "",string toCustom = L"",int32 di = 0,
											string ttFile = "",string ttVar = "",string ttCustom = L"",uint64 ttId = 0,uint64 toId = 0,uint64 tuId = 0,
											string tuFile = "",string tuVar = "",string tuCustom = L"");

	bool				sendMacroSystemMessage(PlayerObject* playerObject,string message,string macro);
	bool				sendSystemMessage(PlayerObject* playerObject, string message, bool chatOnly);

	ResourceLocation	sendSurveyMessage(uint16 range,uint16 points,CurrentResource* resource,PlayerObject* targetObject);
	bool				sendPlayMusicMessage(uint32 soundId,PlayerObject* targetObject);
	bool				sendPlayMusicMessage(uint32 soundId,Object* creatureObject);	// To be used by non-player objects.
	bool				sendErrorMessage(PlayerObject* playerObject,string errType,string errMsg,uint8 fatal);
	bool				sendEnterTicketPurchaseModeMessage(TravelTerminal* terminal,PlayerObject* targetObject);

	// Tutorials
	bool				sendUpdateTutorialRequest(PlayerObject* playerObject, string request);
	bool				sendOpenHolocron(PlayerObject* playerObject);
	bool				sendEnableHudElement(PlayerObject* playerObject, string hudElement);
	bool				sendDisableHudElement(PlayerObject* playerObject, string hudElement);

	// client effects
	bool				sendPlayClientEffectObjectMessage(string effect,string location,Object* effectObject,PlayerObject* playerObject = NULL);
	bool				sendPlayClientEffectLocMessage(string effect, const glm::vec3& pos, PlayerObject* targetObject);

	// position updates
	void				sendUpdateTransformMessage(MovingObject* object);
	void				sendUpdateTransformMessageWithParent(MovingObject* object);

	// position updates. used with Tutorial
	void				sendUpdateTransformMessage(MovingObject* object, PlayerObject* player);
	void				sendUpdateTransformMessageWithParent(MovingObject* object, PlayerObject* player);

	// character sheet
	bool				sendBadges(PlayerObject* srcObject,PlayerObject* targetObject);
	bool				sendBiography(PlayerObject* playerObject,PlayerObject* targetObject);
	bool				sendCharacterSheetResponse(PlayerObject* playerObject);
	bool				sendCharacterMatchResults(const PlayerList* const matchedPlayers, const PlayerObject* const targetObject) const;

	// objcontroller, objcontrollermessages.cpp
	void				sendCombatAction(CreatureObject* attacker,Object* defender,uint32 animation,uint8 trail1 = 0,uint8 trail2 = 0,uint8 hit = 0);
	bool				sendCommandQueueRemove(uint32 sequence,float tickCounter,uint32 reply1,uint32 reply2,PlayerObject* playerObject);
	bool				sendManufactureSlots(ManufacturingSchematic* manSchem,CraftingTool* tool,TangibleObject* item,PlayerObject* playerObject);
	bool				sendDraftslotsResponse(DraftSchematic* schematic,PlayerObject* playerObject);
	bool				sendDraftWeightsResponse(DraftSchematic* schematic,PlayerObject* playerObject);
	bool				sendDraftSchematicsList(CraftingTool* tool,PlayerObject* playerObject);
	bool				sendCraftAcknowledge(uint32 ackType,uint32 errorId,uint8 counter,PlayerObject* playerObject);
	bool				sendCraftExperimentResponse(uint32 ackType,uint32 resultId,uint8 counter,PlayerObject* playerObject);
	bool				sendSharedNetworkMessage(PlayerObject* playerObject,uint32 unknown1,uint32 unknown2);
	void				sendCombatSpam(Object* attacker,Object* defender,int32 damage,string stfFile,string stfVar,Item* item = NULL,uint8 colorFlag = 0,string customText = L"");
	void				sendFlyText(Object* srcCreature,string stfFile,string stfVar,uint8 red = 255,uint8 green = 255,uint8 blue = 255,uint8 display = 5);
	// Used by Tutorial.
	void				sendFlyText(Object* srcCreature, PlayerObject* player, string stfFile,string stfVar,uint8 red,uint8 green,uint8 blue,uint8 display);
	void				sendSelfPostureUpdate(PlayerObject* playerObject);
	void				sendSetWaypointActiveStatus(WaypointObject* waypointObject, bool active, PlayerObject* targetObject);

	// spatial
	void				sendSpatialChat(CreatureObject* const srcObject, string chatMsg, char chatElement[5][32]);
	bool				sendSpatialChat(const CreatureObject* const srcObject, const PlayerObject* const playerObject,string customMessage = L"",string mainFile = "",
										string mainVar = "",string toFile = "",string toVar = "",string toCustom = L"",int32 di = 0,
										string ttFile = "",string ttVar = "",string ttCustom = L"",uint64 ttId = 0,uint64 toId = 0,uint64 tuId = 0) const;
	void				sendSpatialEmote(CreatureObject* srcObject,uint16 emoteId,uint16 sendText,uint64 emoteTarget);
	void				sendCreatureAnimation(CreatureObject* srcObject,string animation);

	// spatial for tutorial
	void				sendSpatialChat(const CreatureObject* const srcObject,string chatMsg,char chatElement[5][32], const PlayerObject* const player) const ;
	void				sendCreatureAnimation(CreatureObject* srcObject,string animation, PlayerObject* player);

	// npc conversations
	bool				sendStartNPCConversation(NPCObject* srcObject,PlayerObject* targetObject);
	bool				sendStopNPCConversation(NPCObject* srcObject,PlayerObject* targetObject);
	bool				sendNPCDialogMessage(ActiveConversation* av,PlayerObject* targetObject);
	bool				sendNPCDialogOptions(std::vector<ConversationOption*>* options,PlayerObject* targetObject);

	// radial menu
	bool				sendObjectMenuResponse(Object* object,PlayerObject* targetObject,uint8 responseNr);
	bool				sendEmptyObjectMenuResponse(uint64 requestedId,PlayerObject* targetObject,uint8 responseNr, MenuItemList mMenuItemList);

	// starting location list
	bool				sendStartingLocationList(PlayerObject* player, uint8 tatooine, uint8 corellia, uint8 talus, uint8 rori, uint8 naboo);

	// position updates
	void				sendDataTransform053(Object* object);
	void				sendDataTransformWithParent053(Object* object);
	void				sendSitOnObject(CreatureObject* creatureObject);
	void				sendDataTransformWithParent0B(Object* object);
	void				sendDataTransform0B(Object* object);
	void				sendDataTransform023(Object* object);
	void				sendDataTransform071(Object* object);

	// position updates for tutorial
	void				sendDataTransform(Object* object, PlayerObject* player);
	void				sendDataTransformWithParent(Object* object, PlayerObject* player);

	// creature object, creaturemessages.cpp
	bool				sendBaselinesCREO_1(PlayerObject* player);
	bool				sendBaselinesCREO_3(CreatureObject* srcObject,PlayerObject* targetObject);
	bool				sendBaselinesCREO_4(PlayerObject* player);
	bool				sendBaselinesCREO_6(CreatureObject* creatureObject,PlayerObject* targetObject);

	// deltas
	void				sendDefenderUpdate(CreatureObject* creatureObject,uint8 updateType,uint16 index,uint64 defenderId);
	void				sendNewDefenderList(CreatureObject* creatureObject);
	bool				sendDeltasCREO_3(CreatureObject* creatureObject,PlayerObject* targetObject);
	bool				sendEquippedListUpdate_InRange(CreatureObject* creatureObject);
	bool				sendEquippedItemUpdate_InRange(CreatureObject* creatureObject, uint64 itemId);
	void				sendPostureUpdate(CreatureObject* creatureObject);
	void				sendMoodUpdate(CreatureObject* creatureObject);
	bool				sendBankCreditsUpdate(PlayerObject* playerObject);
	bool				sendInventoryCreditsUpdate(PlayerObject* playerObject);
	bool				sendUpdateMovementProperties(PlayerObject* playerObject);
	void				sendStateUpdate(CreatureObject* creatureObject);
	void				sendSingleBarUpdate(CreatureObject* creatureObject);
	void				sendPostureAndStateUpdate(CreatureObject* creatureObject);
	bool				sendSkillDeltasCreo1(Skill* skill,uint8 action,PlayerObject* targetObject);
	bool				sendSkillModDeltasCREO_4(SkillModsList smList,uint8 remove,CreatureObject* creatureObject,PlayerObject* playerObject);
	bool				sendUpdatePvpStatus(CreatureObject* creatureObject,PlayerObject* targetObject,uint32 statusMask = 0);
	void				sendTargetUpdateDeltasCreo6(CreatureObject* creatureObject);
	void				sendPerformanceId(CreatureObject* creatureObject);
	void				UpdateEntertainerPerfomanceCounter(CreatureObject* creatureObject);
	void				sendListenToId(PlayerObject* playerObject);
	void				sendTerrainNegotiation(CreatureObject* creatureObject);
	void				sendMoodString(CreatureObject* creatureObject,string animation);
	void				sendWeaponIdUpdate(CreatureObject* creatureObject);
	void				sendIncapTimerUpdate(CreatureObject* creatureObject);
	bool				sendSkillModUpdateCreo4(PlayerObject* playerObject);

	// ham
	void				sendMaxHitpointDeltasCreo6_Single(CreatureObject* creatureObject,uint8 barIndex);
	void				sendBaseHitpointDeltasCreo1_Single(CreatureObject* creatureObject,uint8 barIndex);

	void				sendCurrentHitpointDeltasCreo6_Single(CreatureObject* creatureObject,uint8 barIndex);
	void				sendCurrentHitpointDeltasCreo6_Full(CreatureObject* creatureObject);
	void				sendWoundUpdateCreo3(CreatureObject* creatureObject,uint8 barIndex);
	void				sendBFUpdateCreo3(CreatureObject* playerObject);

	// creature owner
	void				sendOwnerUpdateCreo3(MountObject* mount);

	// group
	void				sendGroupIdUpdateDeltasCreo6(uint64 groupId, const PlayerObject* const player, const PlayerObject* const target) const;
	void				sendInviteSenderUpdateDeltasCreo6(uint64 id, PlayerObject* targetPlayer);


	// entertainer
	bool				sendEntertainerCreo6Part3(PlayerObject* playerObject);

	void				sendAnimationString(CreatureObject* creatureObject);
	void				sendWatchEntertainer(PlayerObject* playerObject);
	void				sendperformFlourish(PlayerObject* playerObject, uint32 flourish);

	// image design
	void				sendImageDesignStartMessage(PlayerObject* srcObject,PlayerObject* targetObject);
	void				sendIDChangeMessage(PlayerObject* targetObject,PlayerObject* srcObject,PlayerObject* otherObject, string hair, uint32 iDsession,uint32 moneyOffered, uint32 moneyDemanded, uint32 customerAccept, uint8 designerCommit, uint8 flag3,uint32 smTimer, uint8 flag1, uint64 parentId,string holoEmote);
	void				sendIDEndMessage(PlayerObject* targetObject,PlayerObject* srcObject,PlayerObject* otherObject, string hair, uint32 iDsession,uint32 moneyOffered, uint32 moneyDemanded,uint32 unknown2, uint8 flag2, uint8 flag3,uint32 counter1);
	void				sendCustomizationUpdateCreo3(CreatureObject* creatureObject);
	void				sendScaleUpdateCreo3(CreatureObject* creatureObject);
	void				sendStatMigrationStartMessage(PlayerObject* targetObject);

	// player object, playermessages.cpp
	bool				sendBaselinesPLAY_3(PlayerObject* playerObject,PlayerObject* targetObject);
	bool				sendBaselinesPLAY_6(PlayerObject* playerObject,PlayerObject* targetObject);
	bool				sendBaselinesPLAY_8(PlayerObject* playerObject,PlayerObject* targetObject);
	bool				sendBaselinesPLAY_9(PlayerObject* playerObject,PlayerObject* targetObject);

	// deltas
	bool				sendUpdateCurrentForce(PlayerObject* playerObject);
	bool				sendUpdateMaxForce(PlayerObject* playerObject);
	bool				sendMatchPlay3(PlayerObject* playerObject);
	bool				sendWaypointsUpdate(PlayerObject* playerObject);
	void				sendTitleUpdate(PlayerObject* playerObject);
	void				sendUpdatePlayerFlags(PlayerObject* playerObject);
	bool				sendUpdateWaypoint(WaypointObject* waypoint,ObjectUpdate updateType,PlayerObject* playerObject);
	bool				sendSkillCmdDeltasPLAY_9(PlayerObject* playerObject);
	bool				sendSchematicDeltasPLAY_9(PlayerObject* playerObject);
	bool				sendUpdateXpTypes(SkillXpTypesList newXpTypes,uint8 remove,PlayerObject* playerObject);
	bool				sendXpUpdate(uint32 xpType,PlayerObject* playerObject);
	bool				sendFriendListPlay9(PlayerObject* playerObject);
	bool				sendIgnoreListPlay9(PlayerObject* playerObject);
	bool				sendUpdateCraftingStage(PlayerObject* playerObject);
	bool				sendUpdateExperimentationFlag(PlayerObject* playerObject);
	bool				sendUpdateExperimentationPoints(PlayerObject* playerObject);
	bool				sendUpdateNearestCraftingStation(PlayerObject* playerObject);
	void				sendLanguagePlay9(PlayerObject* playerObject);
	void				sendFoodUpdate(PlayerObject* playerObject);
	void				sendDrinkUpdate(PlayerObject* playerObject);
	void				sendStationaryFlagUpdate(PlayerObject* playerObject);
	
	// static object,	staticomessages.cpp
	bool				sendBaselinesSTAO_3(TangibleObject* staticObject, PlayerObject* targetObject);
	bool				sendBaselinesSTAO_6(TangibleObject* staticObject, PlayerObject* targetObject);
	
	//crates
	bool				sendBaselinesTYCF_3(FactoryCrate* crate,PlayerObject* targetObject);
	bool				sendBaselinesTYCF_6( FactoryCrate* crate,PlayerObject* targetObject);
	bool				sendBaselinesTYCF_8(FactoryCrate* crate,PlayerObject* targetObject);
	bool				sendBaselinesTYCF_9(FactoryCrate* crate,PlayerObject* targetObject);

	bool				sendUpdateCrateContent(FactoryCrate* crate,PlayerObject* playerObject);

	// tangible object, tangiblemessages.cpp
	bool				sendBaselinesTANO_3(const TangibleObject* const tangibleObject,const PlayerObject* const targetObject) const;
	bool				sendBaselinesTANO_6(const TangibleObject* const tangibleObject,const PlayerObject* const targetObject) const;
	// bool				sendBaselinesTANO_7(const TangibleObject* const tangibleObject,const PlayerObject* const targetObject) const;
	bool				sendBaselinesTANO_8(const TangibleObject* const tangibleObject,const PlayerObject* const targetObject) const;
	bool				sendBaselinesTANO_9(const TangibleObject* const tangibleObject,const PlayerObject* const targetObject) const;

	bool				sendItemChildren(TangibleObject* srcObject,PlayerObject* targetObject);

	// deltas
	bool				sendUpdateComplexity(TangibleObject* tangibleObject,PlayerObject* playerObject);
	bool				sendUpdateTimer(TangibleObject* tangibleObject,PlayerObject* playerObject);
	bool				sendUpdateTypeOption(TangibleObject* tangibleObject,PlayerObject* playerObject);
	bool				sendUpdateCustomization_InRange(TangibleObject* tangibleObject,PlayerObject* playerObject);
	bool				sendUpdateUses(TangibleObject* tangibleObject,PlayerObject* playerObject);

	// intangible object, intangiblemessages.cpp
	bool				sendBaselinesITNO_3(IntangibleObject* tangibleObject,PlayerObject* targetObject);
	bool				sendBaselinesITNO_6(IntangibleObject* tangibleObject,PlayerObject* targetObject);
	bool				sendBaselinesITNO_8(IntangibleObject* tangibleObject,PlayerObject* targetObject);
	bool				sendBaselinesITNO_9(IntangibleObject* tangibleObject,PlayerObject* targetObject);

	// mission object, missionmessages.cpp
	bool				sendBaselinesMISO_3(MissionObject* missionObject,PlayerObject* targetObject);
	bool				sendBaselinesMISO_6(MissionObject* missionObject,PlayerObject* targetObject);
	bool				sendBaselinesMISO_8(MissionObject* missionObject,PlayerObject* targetObject);
	bool				sendBaselinesMISO_9(MissionObject* missionObject,PlayerObject* targetObject);
	bool				sendMISO_Delta(MissionObject* missionObject,PlayerObject* targetObject);
	bool				sendMissionGenericResponse(MissionObject* missionObject,PlayerObject* targetObject);
	bool				sendMissionAbort(MissionObject* missionObject,PlayerObject* targetObject);
	bool				sendMissionComplete(PlayerObject* targetObject);

	// manufacturing schematic object, manschematicmessages.cpp
	bool				sendBaselinesMSCO_3(ManufacturingSchematic* manSchem,PlayerObject* playerObject,bool sendAttributes = true);
	bool				sendBaselinesMSCO_6(ManufacturingSchematic* manSchem,PlayerObject* playerObject);
	bool				sendBaselinesMSCO_8(ManufacturingSchematic* manSchem,PlayerObject* playerObject);
	bool				sendBaselinesMSCO_9(ManufacturingSchematic* manSchem,PlayerObject* playerObject);
	bool				sendBaselinesMSCO_7(ManufacturingSchematic* manSchem,PlayerObject* playerObject);

	// deltas
	bool				sendDeltasMSCO_3(ManufacturingSchematic* manSchem,PlayerObject* playerObject);
	bool				sendDeltasMSCO_7(ManufacturingSchematic* manSchem,PlayerObject* playerObject);
	bool				sendMSCO_3_ComplexityUpdate(ManufacturingSchematic* manSchem,PlayerObject* playerObject);


	bool				sendAttributeDeltasMSCO_7(ManufacturingSchematic* manSchem,PlayerObject* playerObject);

	bool				sendAttributeDeltasMSCO_7_New(ManufacturingSchematic* manSchem,PlayerObject* playerObject);


	bool				sendUpdateFilledManufactureSlots(ManufacturingSchematic* manSchem,PlayerObject* playerObject);
	bool				sendManufactureSlotUpdate(ManufacturingSchematic* manSchem,uint8 slotId,PlayerObject* playerObject);
	bool				sendManufactureSlotUpdateSmall(ManufacturingSchematic* manSchem,uint8 slotId,PlayerObject* playerObject);

	bool				sendGenericIntResponse(uint32 value,uint8 counter,PlayerObject* playerObject);

	// building / cell object, buildingmessages.cpp
	bool				sendBaselinesBUIO_3(BuildingObject* buildingObject,PlayerObject* playerObject);
	bool				sendBaselinesBUIO_6(BuildingObject* buildingObject,PlayerObject* playerObject);
	bool				sendBaselinesBUIO_3(HouseObject* buildingObject,PlayerObject* playerObject);
	bool				sendBaselinesBUIO_6(HouseObject* buildingObject,PlayerObject* playerObject);
	bool				sendBaselinesSCLT_3(CellObject* cellObject,uint64 cellNr,PlayerObject* playerObject);
	bool				sendBaselinesSCLT_6(CellObject* cellObject,PlayerObject* playerObject);


	bool				sendBaselinesBUIO_3(TangibleObject* structure,PlayerObject* player);
	bool				sendBaselinesBUIO_6(TangibleObject* structure,PlayerObject* player);

	// Harvester Messages
	bool				sendBaselinesHINO_3(HarvesterObject* harvester,PlayerObject* player);
	bool				sendBaselinesHINO_6(HarvesterObject* harvester,PlayerObject* player);
	bool				sendBaselinesHINO_7(HarvesterObject* harvester,PlayerObject* player);
	
	void				sendNewHarvesterName(PlayerStructure* harvester);
	void				sendOperateHarvester(PlayerStructure* harvester,PlayerObject* player);
	void				sendHarvesterResourceData(PlayerStructure* structure,PlayerObject* player);
	void				sendCurrentResourceUpdate(HarvesterObject* harvester, PlayerObject* player);
	void				sendCurrentExtractionRate(HarvesterObject* harvester, PlayerObject* player);
	void				sendHarvesterActive(HarvesterObject* harvester);
	void				SendHarvesterHopperUpdate(HarvesterObject* harvester, PlayerObject* player);
	void				sendResourceEmptyHopperResponse(PlayerStructure* structure,PlayerObject* player, uint32 amount, uint8 b1, uint8 b2);
	void				sendHarvesterCurrentConditionUpdate(PlayerStructure* structure);

	void				sendConstructionComplete(PlayerObject* playerObject, PlayerStructure* Structure);
	void				SendUpdateHarvesterWorkAnimation(HarvesterObject* harvester);

	// Installation Messages
	bool				sendBaselinesINSO_3(FactoryObject* factory,PlayerObject* player);
	bool				sendBaselinesINSO_6(FactoryObject* factory,PlayerObject* player);

	bool				sendBaselinesINSO_3(PlayerStructure* structure,PlayerObject* player);
	bool				sendBaselinesINSO_6(PlayerStructure* structure,PlayerObject* player);
	void				SendUpdateFactoryWorkAnimation(FactoryObject* factory);

	// deltas
	bool				sendUpdateCellPermissionMessage(CellObject* cellObject,uint8 permission,PlayerObject* playerObject);
	
	// Structures admin / placement
	bool				sendEnterStructurePlacement(Object* deed, string objectString, PlayerObject* playerObject);
	bool				sendAdminList(PlayerStructure* structure, PlayerObject* playerObject);
	bool				sendHopperList(PlayerStructure* structure, PlayerObject* playerObject);
	bool				sendBanList(PlayerStructure* structure, PlayerObject* playerObject);
	bool				sendEntryList(PlayerStructure* structure, PlayerObject* playerObject);

	// resource container, resourcecontainermessages.cpp
	bool				sendBaselinesRCNO_3(ResourceContainer* resourceContainer,PlayerObject* targetObject);
	bool				sendBaselinesRCNO_6(ResourceContainer* resourceContainer,PlayerObject* targetObject);
	bool				sendBaselinesRCNO_8(ResourceContainer* resourceContainer,PlayerObject* targetObject);
	bool				sendBaselinesRCNO_9(ResourceContainer* resourceContainer,PlayerObject* targetObject);

	// deltas
	bool				sendResourceContainerUpdateAmount(ResourceContainer* resourceContainer,PlayerObject* playerObject);

	// internal, internalmessages.cpp
	bool				sendClusterZoneTransferRequestByTicket(PlayerObject* playerObject,uint64 ticketId,uint32 destinationPlanet);
	bool				sendClusterZoneTransferRequestByPosition(PlayerObject* playerObject, const glm::vec3& position, uint32 destinationPlanet);
	bool				sendClusterZoneTransferCharacter(PlayerObject* playerObject,uint32 destinationPlanet);
	bool				sendGroupLootModeResponse(PlayerObject* playerObject,uint32 selection);
	bool				sendGroupLootMasterResponse(PlayerObject* masterLooter, PlayerObject* playerObject);

	// friendlist
	bool				sendFindFriendLocation(PlayerObject* friendPlayer, uint64 friendId,uint64 player, float X, float Z);

	// groups
	bool				sendIsmGroupBaselineRequest(PlayerObject* targetPlayer);
	bool				sendIsmGroupPositionNotification(PlayerObject* targetPlayer);
	bool				sendIsmGroupLeave(PlayerObject* player);
	bool				sendIsmGroupInviteInRangeResponse(PlayerObject* sender, PlayerObject* target, bool inRange );

	// trading / bazaar
	bool				sendAbortTradeMessage(PlayerObject* playerObject);
	bool				sendBidAuctionResponseMessage(PlayerObject* playerObject, uint64 AuctionId, uint32 error);
	bool				sendAcceptTradeMessage(PlayerObject* playerObject);
	bool				sendUnacceptTradeMessage(PlayerObject* playerObject);
	bool				sendDeleteItemMessage(PlayerObject* playerObject,uint64 ItemId);
	bool				sendSecureTrade(PlayerObject* targetPlayer,PlayerObject* srcObject,uint32 error);
	bool				sendBeginTradeMessage(PlayerObject* targetObject,PlayerObject* srcObject);
	bool				sendVerifyTradeMessage(PlayerObject* targetObject);
	bool				sendAddItemMessage(PlayerObject* targetPlayer,TangibleObject* object);
	bool				sendTangibleTrade(TangibleObject* tangibleObject,PlayerObject* targetObject);
	bool				sendBeginVerificationMessage(PlayerObject* playerObject);
	bool				sendVerificationMessage(PlayerObject* playerObject);
	bool				sendTradeCompleteMessage(PlayerObject* playerObject);
	bool				sendGiveMoneyMessage(PlayerObject* playerObject,uint32 Money);
	bool				sendCreateAuctionItemResponseMessage(PlayerObject* targetPlayer,uint64 AuctionId,uint32 error);
	void				sendSoldInstantMail(uint64 oldOwner, PlayerObject* newOwner, string ItemName, uint32 Credits, string planet, string region);
	void				sendBoughtInstantMail(PlayerObject* newOwner, string ItemName, string SellerName, uint32 Credits, string planet, string region, int32 mX, int32 mY);
	void				sendBanktipMail(PlayerObject* playerObject, PlayerObject* targetObject, uint32 amount);
	bool				sendBankTipDustOff(PlayerObject* playerObject, uint64 tipRecipient, uint32 amount, string recipientName);

	// Buffs
	void				sendPlayerAddBuff(PlayerObject* target, int32 CRC, float Duration);
	void				sendPlayerRemoveBuff(PlayerObject* playerObject, int32 CRC);

	void				sendTutorialServerStatusRequest(DispatchClient* client, uint64 playerId, uint32 accountID);

	~MessageLib();

private:

	MessageLib();
	
	bool				_checkDistance(const glm::vec3& mPosition1, Object* object, uint32 heapWarningLevel);

	bool				_checkPlayer(const PlayerObject* const player) const;
	bool				_checkPlayer(uint64 playerId) const;

	void				_sendToInRangeUnreliable(Message* message, Object* const object,uint16 priority,bool toSelf = true);
	void				_sendToInRange(Message* message, Object* const object,uint16 priority,bool toSelf = true);

	void				_sendToInstancedPlayersUnreliable(Message* message, uint16 priority, const PlayerObject* const player) const ;
	void				_sendToInstancedPlayers(Message* message, uint16 priority, const PlayerObject* const player) const ;
	void				_sendToAll(Message* message,uint16 priority,bool unreliable = false) const;

	static MessageLib*	mSingleton;
	static bool			mInsFlag;

	MessageFactory*		mMessageFactory;
};

//======================================================================================================================

#endif
