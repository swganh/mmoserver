/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_MESSAGELIB_H
#define ANH_ZONESERVER_MESSAGELIB_H

//#include "Utils/typedefs.h"
#include "ZoneServer/ObjectFactory.h"
#include "common/bytebuffer.h"

#define	 gMessageLib	MessageLib::getSingletonPtr()

class DispatchClient;
class DraftSchematic;
class CurrentResource;
class ConversationOption;
class CraftingTool;
class ActiveConversation;

typedef struct tagResourceLocation ResourceLocation;

typedef std::vector<PlayerObject*>				PlayerList; 

enum ObjectUpdate
{
	ObjectUpdateRewriteAll	= 0,
	ObjectUpdateDelete		= 1,
	ObjectUpdateAdd			= 2,
	ObjectUpdateChange		= 3,
};

//======================================================================================================================

class MessageLib
{
public:

	static MessageLib*	getSingletonPtr() { return mSingleton; }
	static MessageLib*	Init();

	// multiple messages, messagelib.cpp
	void				sendCreateObject(Object* object,PlayerObject* player,bool sendSelftoTarget = true);
	bool				sendCreateManufacturingSchematic(ManufacturingSchematic* manSchem,PlayerObject* playerObject,bool attributes = true);
	bool				sendCreateResourceContainer(ResourceContainer* resourceContainer,PlayerObject* targetObject);
	bool				sendCreateBuilding(BuildingObject* buildingObject,PlayerObject* playerObject);
	bool				sendCreateCamp(TangibleObject* camp,PlayerObject* player);
	bool				sendCreateCreature(CreatureObject* creatureObject,PlayerObject* targetObject);
	bool				sendCreatePlayer(PlayerObject* playerObject,PlayerObject* targetObject);
	bool				sendCreateTangible(TangibleObject* tangibleObject,const PlayerObject* const targetObject) const;
	void				sendInventory(PlayerObject* playerObject);
	bool				sendEquippedItems(PlayerObject* srcObject,PlayerObject* targetObject);

	// common messages, commonmessages.cpp
	bool				sendCreateObjectByCRC(Object* object,const PlayerObject* const targetObject,bool player) const;
	bool				sendContainmentMessage(uint64 objectId,uint64 parentId,uint32 linkType,const PlayerObject* const targetObject) const;
	bool				sendContainmentMessage_InRange(uint64 objectId,uint64 parentId,uint32 linkType,PlayerObject* targetObject);
	bool				sendContainmentMessage_InRange(uint64 objectId,uint64 parentId,uint32 linkType,CreatureObject* targetObject);
	bool				broadcastContainmentMessage(uint64 objectId,uint64 parentId,uint32 linkType,PlayerObject* targetObject);
	bool				broadcastContainmentMessage(uint64 objectId,uint64 parentId,uint32 linkType,Object* targetObject);	// Used by Creatures.
	bool				sendOpenedContainer(uint64 objectId, PlayerObject* targetObject);
	bool				sendPostureMessage(CreatureObject* creatureObject,PlayerObject* targetObject);
	bool				sendEndBaselines(uint64 objectId,const PlayerObject* const targetObject) const;
	bool				sendDestroyObject(uint64 objectId, PlayerObject* targetObject);
	bool				sendDestroyObject(uint64 objectId, CreatureObject* owner);
	bool				sendDestroyObject_InRange(uint64 objectId,PlayerObject* const owner, bool self);
	bool				sendDestroyObject_InRangeofObject(Object* object);

	bool				sendHeartBeat(DispatchClient* client);
	bool				sendChatServerStatus(uint8 unk1,uint8 unk2,DispatchClient* client);
	bool				sendParameters(uint32 parameters,DispatchClient* client);
	bool				sendStartScene(uint64 zoneId,PlayerObject* player);
	bool				sendSceneReady(DispatchClient* client);
	void				sendSceneReadyToChat(DispatchClient* client);
	bool				sendServerTime(uint64 time,DispatchClient* client);
	void				sendWeatherUpdate(Anh_Math::Vector3 cloudVec,uint32 weatherType,PlayerObject* player = NULL);
	bool				sendSystemMessage(PlayerObject* playerObject,string customMessage = L"",string mainFile = "",
		string mainVar = "",string toFile = "",string toVar = "",string toCustom = L"",int32 di = 0,
		string ttFile = "",string ttVar = "",string ttCustom = L"",uint64 ttId = 0,uint64 toId = 0,uint64 tuId = 0,
		string tuFile = "",string tuVar = "",string tuCustom = L"");

	bool				sendMacroSystemMessage(PlayerObject* playerObject,string message,string macro);
	ResourceLocation	sendSurveyMessage(uint16 range,uint16 points,CurrentResource* resource,PlayerObject* targetObject);
	bool				sendPlayMusicMessage(uint32 soundId,PlayerObject* targetObject);
	bool				sendPlayMusicMessage(uint32 soundId,Object* creatureObject);	// To be used by non-player objects.
	bool				sendErrorMessage(PlayerObject* playerObject,string errType,string errMsg,uint8 fatal);
	bool				sendEnterTicketPurchaseModeMessage(TravelTerminal* terminal,PlayerObject* targetObject);

	// Tutorials
	void				sendUpdateTutorialRequest(PlayerObject* playerObject, string request);
	void				sendOpenHolocron(PlayerObject* playerObject);
	void				sendEnableHudElement(PlayerObject* playerObject, string hudElement);
	void				sendDisableHudElement(PlayerObject* playerObject, string hudElement);

	// client effects
	bool				sendPlayClientEffectObjectMessage(string effect,string location,PlayerObject* targetObject);
	bool				sendPlayClientEffectObjectMessage(string effect,string location,PlayerObject* originObject,CreatureObject* targetObject);
	bool				sendPlayClientEffectObjectMessage(string effect,string location, CreatureObject* targetObject);
	bool				sendPlayClientEffectLocMessage(string effect,Anh_Math::Vector3 pos,PlayerObject* targetObject);

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
	// void				sendFlyText(CreatureObject* srcCreature,string stfFile,string stfVar,uint8 red = 255,uint8 green = 255,uint8 blue = 255,uint8 display = 5);
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
	bool				sendStartingLocationList(PlayerObject* player);

	// position updates
	void				sendDataTransform(Object* object);
	void				sendDataTransformWithParent(Object* object);
	void				sendSitOnObject(CreatureObject* creatureObject);


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
	void				sendListenToId(CreatureObject* creatureObject);
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
	void				sendOwnerUpdateCreo3(CreatureObject* creatureObject);

	// group
	void				sendGroupIdUpdateDeltasCreo6(uint64 groupId, const PlayerObject* const player, const PlayerObject* const target) const;
	void				sendInviteSenderUpdateDeltasCreo6(uint64 id, PlayerObject* targetPlayer);


	//entertainer
	bool				sendEntertainerCreo6Part3(PlayerObject* playerObject);

	void				sendAnimationString(CreatureObject* creatureObject);
	void				sendWatchEntertainer(PlayerObject* playerObject);
	void				sendperformFlourish(PlayerObject* playerObject, uint32 flourish);

	//image design
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
	// static object,	staticomessages.cpp
	bool				sendBaselinesSTAO_3(const StaticObject* const staticObject, const PlayerObject* const targetObject) const;
	bool				sendBaselinesSTAO_6(const StaticObject* const staticObject, const PlayerObject* const targetObject) const;
	// tangible object, tangiblemessages.cpp
	bool				sendBaselinesTANO_3(const TangibleObject* const tangibleObject,const PlayerObject* const targetObject) const;
	bool				sendBaselinesTANO_6(const TangibleObject* const tangibleObject,const PlayerObject* const targetObject) const;
	// bool				sendBaselinesTANO_7(const TangibleObject* const tangibleObject,const PlayerObject* const targetObject) const;
	bool				sendBaselinesTANO_8(const TangibleObject* const tangibleObject,const PlayerObject* const targetObject) const;
	bool				sendBaselinesTANO_9(const TangibleObject* const tangibleObject,const PlayerObject* const targetObject) const;

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
	
	
	bool				sendAttributeDeltasMSCO_7(ManufacturingSchematic* manSchem,PlayerObject* playerObject);
	
	bool				sendAttributeDeltasMSCO_7_New(ManufacturingSchematic* manSchem,PlayerObject* playerObject);


	bool				sendUpdateFilledManufactureSlots(ManufacturingSchematic* manSchem,PlayerObject* playerObject);
	bool				sendManufactureSlotUpdate(ManufacturingSchematic* manSchem,uint8 slotId,PlayerObject* playerObject);
	bool				sendManufactureSlotUpdateSmall(ManufacturingSchematic* manSchem,uint8 slotId,PlayerObject* playerObject);

	bool				sendGenericIntResponse(uint32 value,uint8 counter,PlayerObject* playerObject);

	// building / cell object, buildingmessages.cpp
	bool				sendBaselinesBUIO_3(BuildingObject* buildingObject,PlayerObject* playerObject);
	bool				sendBaselinesBUIO_6(BuildingObject* buildingObject,PlayerObject* playerObject);
	bool				sendBaselinesSCLT_3(CellObject* cellObject,uint64 cellNr,PlayerObject* playerObject);
	bool				sendBaselinesSCLT_6(CellObject* cellObject,PlayerObject* playerObject);


	bool				sendBaselinesBUIO_3(TangibleObject* structure,PlayerObject* player);
	bool				sendBaselinesBUIO_6(TangibleObject* structure,PlayerObject* player);

	//Harvester Messages
	bool				sendBaselinesHINO_3(HarvesterObject* harvester,PlayerObject* player);
	bool				sendBaselinesHINO_6(HarvesterObject* harvester,PlayerObject* player);

	// deltas
	bool				sendUpdateCellPermissionMessage(CellObject* cellObject,uint8 permission,PlayerObject* playerObject);

	//placement
	bool				sendEnterStructurePlacement(Object* deed, string objectString, PlayerObject* playerObject);

	// resource container, resourcecontainermessages.cpp
	bool				sendBaselinesRCNO_3(ResourceContainer* resourceContainer,PlayerObject* targetObject);
	bool				sendBaselinesRCNO_6(ResourceContainer* resourceContainer,PlayerObject* targetObject);
	bool				sendBaselinesRCNO_8(ResourceContainer* resourceContainer,PlayerObject* targetObject);
	bool				sendBaselinesRCNO_9(ResourceContainer* resourceContainer,PlayerObject* targetObject);

	// deltas
	bool				sendResourceContainerUpdateAmount(ResourceContainer* resourceContainer,PlayerObject* playerObject);

	// internal, internalmessages.cpp
	bool				sendClusterZoneTransferRequestByTicket(PlayerObject* playerObject,uint64 ticketId,uint32 destinationPlanet);
	bool				sendClusterZoneTransferRequestByPosition(PlayerObject* playerObject,Anh_Math::Vector3 position,uint32 destinationPlanet);
	bool				sendClusterZoneTransferCharacter(PlayerObject* playerObject,uint32 destinationPlanet);
	bool				sendGroupLootModeResponse(PlayerObject* playerObject,uint32 selection);

	//friendlist
	void				sendFindFriendLocation(PlayerObject* friendPlayer, uint64 friendId,uint64 player, float X, float Z);

	// groups

	void				sendIsmGroupBaselineRequest(PlayerObject* targetPlayer);
	void				sendIsmGroupPositionNotification(PlayerObject* targetPlayer);
	void				sendIsmGroupLeave(PlayerObject* player);

	// trading / bazaar
	bool				sendAbortTradeMessage(PlayerObject* playerObject);
	bool				sendBidAuctionResponseMessage(PlayerObject* playerObject, uint64 AuctionId, uint32 error);
	bool				sendAcceptTradeMessage(PlayerObject* playerObject);
	bool				sendUnacceptTradeMessage(PlayerObject* playerObject);
	bool				sendDeleteItemMessage(PlayerObject* playerObject,uint64 ItemId);
	bool				sendSecureTrade(PlayerObject* targetPlayer,PlayerObject* srcObject,uint32 error);
	bool				sendBeginTradeMessage(PlayerObject* targetObject,PlayerObject* srcObject);
	bool				sendVerifyTradeMessage(PlayerObject* targetObject);
	bool				sendAddItemMessage(PlayerObject* targetPlayer,TangibleObject* Object);
	bool				sendTangibleTrade(TangibleObject* tangibleObject,PlayerObject* targetObject);
	bool				sendBeginVerificationMessage(PlayerObject* playerObject);
	bool				sendVerificationMessage(PlayerObject* playerObject);
	bool				sendTradeCompleteMessage(PlayerObject* playerObject);
	bool				sendGiveMoneyMessage(PlayerObject* playerObject,uint32 Money);
	void				sendCreateAuctionItemResponseMessage(PlayerObject* targetPlayer,uint64 AuctionId,uint32 error);
	void				sendSoldInstantMail(uint64 oldOwner, PlayerObject* newOwner, string ItemName, uint32 Credits, string planet, string region);
	void				sendBoughtInstantMail(PlayerObject* newOwner, string ItemName, string SellerName, uint32 Credits, string planet, string region, int32 mX, int32 mY);
	void				sendBanktipMail(PlayerObject* playerObject, PlayerObject* targetObject, uint32 amount);
	void				sendBankTipDustOff(PlayerObject* playerObject, uint64 tipRecipient, uint32 amount, string recipientName);

	//Buffs
	void				sendPlayerAddBuff(PlayerObject* target, int32 CRC, float Duration);
	void				sendPlayerRemoveBuff(PlayerObject* playerObject, int32 CRC);


	~MessageLib();

private:

	MessageLib();

	bool				_checkPlayer(const PlayerObject* const player) const;
	bool				_checkPlayer(uint64 playerId) const;

	void				_sendToInRangeUnreliable(Message* message, Object* const object,uint16 priority,bool toSelf = true);
	void				_sendToInRange(Message* message, Object* const object,uint16 priority,bool toSelf = true);

	void				_sendToInstancedPlayersUnreliable(Message* message, uint16 priority, const PlayerObject* const player) const ;
	void				_sendToInstancedPlayers(Message* message, uint16 priority, const PlayerObject* const player) const ;
	void				_sendToAll(Message* message,uint16 priority,bool unreliable = false) const;

	static MessageLib*	mSingleton;
	static bool			mInsFlag;

};

//======================================================================================================================

#endif
