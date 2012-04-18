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

#include <cstdint>
#include <vector>
#include <list>
#include <glm/glm.hpp>

#include "Utils/typedefs.h"

#include "Common/OutOfBand.h"

#include "ZoneServer/MoodTypes.h"
#include "ZoneServer/ObjectController.h"
#include "ZoneServer/Skill.h"   //for skillmodslist
#include "ZoneServer/SocialChatTypes.h"

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
typedef std::list<Object*>				ObjectList;

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

    void				setGrid(zmap*	grid){mGrid = grid;}

    // multiple messages, messagelib.cpp
    bool				sendCreateManufacturingSchematic(ManufacturingSchematic* manSchem,PlayerObject* playerObject,bool attributes = true);


    /** Sends the baselines for a resource container to a target player.
    *
    * \param resource_container The resource container to send the baselines for.
    * \param target The target to receive the resource container baselines.
    */
    bool sendCreateResourceContainer(ResourceContainer* resource_container, PlayerObject* target);


    /** Sends the baselines for a tangible object to a target player.
    *
    * \param tangible The tangible object to send the baselines for.
    * \param target The target to receive the tangible object baselines.
    */
    bool sendCreateTano(TangibleObject* tangible, PlayerObject* target);

    
    /** Sends the baselines for a static object to a target player.
    *
    * \param tangible The static object to send the baselines for.
    * \param target The target to receive the static object baselines.
    */
    bool sendCreateStaticObject(TangibleObject* tangible, PlayerObject* target);

	bool				sendCreateInTangible(IntangibleObject* intangibleObject, uint64 containmentId, PlayerObject* targetObject);

	// structures
	bool				sendCreateInstallation(PlayerStructure* structure,PlayerObject* player);


    /** Sends the baselines for a structure to a target player.
    *
    * \param structure The structure to send the baselines for.
    * \param target The target to receive the structure baselines.
    */
	bool				sendCreateStructure(PlayerStructure* structure,PlayerObject* target);
	bool				sendCreateHarvester(HarvesterObject* harvester,PlayerObject* player);


    /** Sends the baselines for a factory to a target player.
    *
    * \param factory The factory to send the baselines for.
    * \param target The target to receive the factory baselines.
    */
	bool sendCreateFactory(FactoryObject* factory, PlayerObject* target);

	bool				sendCreateBuilding(BuildingObject* buildingObject,PlayerObject* playerObject);


    /** Sends the baselines for a camp to a target player.
    *
    * \param camp The camp to send the baselines for.
    * \param target The target to receive the camp baselines.
    */
	bool sendCreateCamp(TangibleObject* camp, PlayerObject* target);
	

    /** Sends the baselines for a creature object to a target player.
    *
    * \param creature The creature object to send the baselines for.
    * \param target The target to receive the creature object baselines.
    */
    bool sendCreateCreature(CreatureObject* creature, PlayerObject* target);


    /** Sends the baselines for a player object to a target player.
    *
    * \param player The player object to send the baselines for.
    * \param target The target to receive the player object baselines.
    */
    bool sendCreatePlayer(PlayerObject* player, PlayerObject* target);
    

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
    bool				sendDestroyObject(uint64 objectId, PlayerObject* const targetObject) const;
    bool				sendDestroyObject(uint64 objectId, CreatureObject* const owner) const;
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

    /**
     * Sends a custom text string as a system message.
     *
     * @param custom_message A custom text string to be sent.
     * @param player The recepient of the system message. If no player is passed the message is sent to everyone.
     * @param chatbox_only Determines whether the message is displayed on screen or in the chatbox
     *                     only. By default this is false meaning messages are by default displayed on screen and the chatbox.
     * @param send_to_inrange If true the message is sent to all in-range players of the target recipient.
     */
    bool SendSystemMessage(const std::wstring& custom_message, const PlayerObject* const player = NULL, bool chatbox_only = false, bool send_to_inrange = false);

    /**
     * Sends a STF package as a system message.
     *
     * @param prose A custom STF string package.
     * @param player The recepient of the system message. If no player is passed the message is sent to everyone.
     * @param chatbox_only Determines whether the message is displayed on screen or in the chatbox
     *                     only. By default this is false meaning messages are by default displayed on screen and the chatbox.
     * @param send_to_inrange If true the message is sent to all in-range players of the target recipient.
     */
    bool SendSystemMessage(const common::OutOfBand& prose, const PlayerObject* const player = NULL, bool chatbox_only = false, bool send_to_inrange = false);

    ResourceLocation	sendSurveyMessage(uint16 range,uint16 points,CurrentResource* resource,PlayerObject* targetObject);
    bool				sendPlayMusicMessage(uint32 soundId,PlayerObject* targetObject);
    bool				sendPlayMusicMessage(uint32 soundId,Object* creatureObject);	// To be used by non-player objects.
    bool				sendErrorMessage(PlayerObject* playerObject,BString errType,BString errMsg,uint8 fatal);
    bool				sendEnterTicketPurchaseModeMessage(TravelTerminal* terminal,PlayerObject* targetObject);

    // Tutorials
    bool				sendUpdateTutorialRequest(PlayerObject* playerObject, BString request);
    bool				sendOpenHolocron(PlayerObject* playerObject);
    bool				sendEnableHudElement(PlayerObject* playerObject, BString hudElement);
    bool				sendDisableHudElement(PlayerObject* playerObject, BString hudElement);

    // client effects
    bool				sendPlayClientEffectObjectMessage(std::string effect,BString location,Object* effectObject,PlayerObject* playerObject = NULL);
    bool				sendPlayClientEffectLocMessage(std::string effect, const glm::vec3& pos, PlayerObject* targetObject);

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
    
    /** Sends the results of a character match to a target player.
    *
    * \param matched_players The players that fit the character match requirements.
    * \param target The target to receive the character match results.
    */
    bool				sendCharacterMatchResults(const PlayerList* const matchedPlayers, const PlayerObject* const target) const;

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
    void				sendCombatSpam(Object* attacker,Object* defender,int32 damage,BString stfFile,BString stfVar,Item* item = NULL,uint8 colorFlag = 0,BString customText = L"");

    
    /** Sends a fly text message for all those in range of the creature.
    *
    * \param source The source creature for the fly-text.
    * \param stf_file The file containing the text for the fly-text method.
    * \param stf_var The variable name for the text used.
    * \param red The redness of the current level.
    * \param green The greenness of the current level.
    * \param blue The blueness of the current level.
    * \param display When active the text is displayed in the players chat history.
    */
    void sendFlyText(Object* source, const std::string& stf_file, const std::string& stf_var, unsigned char red = 255, unsigned char green = 255, unsigned char blue = 255, unsigned char display = 5);
    
    
    // Used by Tutorial

    /** A version of fly texts for usage by the tutorial
    *
    * @TODO Remove this method once better handling of instances is in.
    *
    * \param source The source creature for the fly-text.
    * \param player The player to base the instance off of.
    * \param stf_file The file containing the text for the fly-text method.
    * \param stf_var The variable name for the text used.
    * \param red The redness of the current level.
    * \param green The greenness of the current level.
    * \param blue The blueness of the current level.
    * \param display When active the text is displayed in the players chat history.
    */
    void sendFlyText(Object* source, PlayerObject* player, const std::string& stf_file, const std::string& stf_var, unsigned char red, unsigned char green, unsigned char blue, unsigned char display);


    void				sendSelfPostureUpdate(PlayerObject* playerObject);
    void				sendSetWaypointActiveStatus(WaypointObject* waypointObject, bool active, PlayerObject* targetObject);

    // spatial

    /**
     * Sends a custom text message via spatial chat, spoken by the specified object.
     *
     * @param speaking_object The object that is currently speaking this message.
     * @param custom_message The text message to be sent via spatial chat.
     * @param player_object This parameter allows the messages from npc's to be sent to the right instance players.
     * @param target_id The object id of the target the speaker is talking to.
     * @param text_size The size of the text for use in the spatial chat bubble.
     *                  Options: 0, 2, 3, 4, 6, 8, 10
     * @param chat_type_id An ID representing the type of chat. @todo: Add an ID table here.
     * @param mood_id An ID representing the mood of the speaking object. @todo: Add an ID table here.
     * @param whisper_target_animate If set to 1 the speaker will turn to the target and whisper.
     */
    void SendSpatialChat(CreatureObject* const speaking_object, const std::wstring& custom_message, PlayerObject* const player_object = NULL, uint64_t target_id = 0, uint16_t text_size = 0x32, SocialChatType chat_type_id = kSocialChatNone, MoodType mood_id = kMoodNone, uint8_t whisper_target_animate = 0);

    /**
     * Sends a message via spatial chat using a ProsePackage (STF string), spoken by the specified object.
     *
     * @param speaking_object The object that is currently speaking this message.
     * @param prose_message The OutOfBand to be sent via spatial chat.
     * @param player_object This parameter allows the messages from npc's to be sent to the right instance players.
     * @param target_id The object id of the target the speaker is talking to.
     * @param text_size The size of the text for use in the spatial chat bubble.
     *                  Options: 0, 2, 3, 4, 6, 8, 10
     * @param chat_type_id An ID representing the type of chat. @todo: Add an ID table here.
     * @param mood_id An ID representing the mood of the speaking object. @todo: Add an ID table here.
     * @param whisper_target_animate If set to 1 the speaker will turn to the target and whisper.
     */
    void SendSpatialChat(CreatureObject* const speaking_object, const common::OutOfBand& prose_message, PlayerObject* const player_object = NULL, uint64_t target_id = 0, uint16_t text_size = 0x32, SocialChatType chat_type_id = kSocialChatNone, MoodType mood_id = kMoodNone, uint8_t whisper_target_animate = 0);

    /**
     * Sends a spatial emote (such as a player or NPC cheering).
     *
     * @param source This is the CreatureObject (an NPC or player) that is performing the spatial emote.
     * @param emote_id This is the id of the emote the source CreatureObject is performing.
     * @param target_id This is optional value specifies a target for the source CreatureObject to direct the emote towards.
     * @param emote_flags This determines how the source CreatureObject performs the emote:
     *                    Options:
     *                      - 1 - Source performs an animation
     *                      - 2 - Source sends a text message
     *                      - 3 - Source performs an animation and sends a text message.
     */
    void SendSpatialEmote(CreatureObject* source, uint32_t emote_id, uint64_t target_id = 0, uint8_t emote_flags = 1);
    void                sendCreatureAnimation(CreatureObject* srcObject, const std::string& animation);
	void                sendCreatureAnimation(CreatureObject* srcObject, BString animation);
    // spatial for tutorial
    void				sendCreatureAnimation(CreatureObject* srcObject, const std::string &animation, PlayerObject* player);

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

    /** Sends an update of the equipped items on a creature object to a target player.
    *
    * \param creature The creature object to send the equipment list update about.
    * \param target The target player to receive the list update.
    */
	bool				sendEquippedListUpdate(CreatureObject* creatureObject, CreatureObject* targetObject);
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
	void				sendMoodString(CreatureObject* creatureObject,BString animation);
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
    void				sendIDChangeMessage(PlayerObject* targetObject,PlayerObject* srcObject,PlayerObject* otherObject, BString hair, uint32 iDsession,uint32 moneyOffered, uint32 moneyDemanded, uint32 customerAccept, uint8 designerCommit, uint8 flag3,uint32 smTimer, uint8 flag1, uint64 parentId,BString holoEmote);
    void				sendIDEndMessage(PlayerObject* targetObject,PlayerObject* srcObject,PlayerObject* otherObject, BString hair, uint32 iDsession,uint32 moneyOffered, uint32 moneyDemanded,uint32 unknown2, uint8 flag2, uint8 flag3,uint32 counter1);
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

	//bool				sendItemChildren(TangibleObject* srcObject,PlayerObject* targetObject);

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
	bool				sendEnterStructurePlacement(Object* deed, BString objectString, PlayerObject* playerObject);
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
    void				sendNewbieMail(PlayerObject* playerObject, BString subject,BString bodyDir, BString bodyStr);
	void				sendSoldInstantMail(uint64 oldOwner, PlayerObject* newOwner, BString ItemName, uint32 Credits, BString planet, BString region);
	void				sendBoughtInstantMail(PlayerObject* newOwner, BString ItemName, BString SellerName, uint32 Credits, BString planet, BString region, int32 mX, int32 mY);
	void				sendBanktipMail(PlayerObject* playerObject, PlayerObject* targetObject, uint32 amount);
	bool				sendBankTipDustOff(PlayerObject* playerObject, uint64 tipRecipient, uint32 amount, BString recipientName);

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

	void				_sendToInRangeUnreliable(Message* message, Object* const object, unsigned char priority, bool to_self = true);
	void				_sendToInRange(Message* message, Object* const object, unsigned char priority, bool to_self = true) const;

	void				_sendToInRangeUnreliableChat(Message* message, const CreatureObject* object, unsigned char priority, uint32_t crc);
	void				_sendToInRangeUnreliableChatGroup(Message* message, const CreatureObject* object, unsigned char priority, uint32_t crc);
	
	void				_sendToInstancedPlayersUnreliable(Message* message, unsigned char priority, const PlayerObject* const player) const;
	void				_sendToInstancedPlayers(Message* message, unsigned char priority, PlayerObject* const player) const;
	void				_sendToAll(Message* message, unsigned char priority, bool unreliable = false) const;
   
    /**
     * Sends a spatial message to in-range players.
     *
     * This helper method for spatial methods is used to send a message to in-range
     * players while respecting their ignore lists.
     *
     * @param message The spatial message to be sent out to players.
     * @param object This is the object from whom the message originates (ie., the speaker)
     * @param player_object This is used to send out spatial messages in a player instance.
     */
    void SendSpatialToInRangeUnreliable_(Message* message, Object* const object, PlayerObject* const player_object = NULL);
    /**
     * Sends out a system message.
     *
     * This internal method is invoked by the two SendSystemMessage overloads to send out a system message.
     *
     * @param custom_message A custom text string to be sent.
     * @param prose A custom STF string package.
     * @param player The recepient of the system message. If no player is passed the message is sent to everyone.
     * @param chatbox_only Determines whether the message is displayed on screen or in the chatbox
     *                     only. By default this is false meaning messages are by default displayed on screen and the chatbox.
     * @param send_to_inrange If true the message is sent to all in-range players of the target recipient.
     */
    bool SendSystemMessage_(const std::wstring& custom_message, const common::OutOfBand& prose, PlayerObject* player, bool chatbox_only, bool send_to_inrange);

    /**
     * Sends a message via spatial chat using a ProsePackage (STF string), spoken by the specified object.
     *
     * @param speaking_object The object that is currently speaking this message.
     * @param custom_message The text message to be sent via spatial chat.
     * @param prose_message The OutOfBand attachment to be sent via spatial chat.
     * @param player_object This parameter allows the messages from npc's to be sent to the right instance players.
     * @param target_id The object id of the target the speaker is talking to.
     * @param text_size The size of the text for use in the spatial chat bubble.
     *                  Options: 0, 2, 3, 4, 6, 8, 10
     * @param chat_type_id An ID representing the type of chat. @todo: Add an ID table here.
     * @param mood_id An ID representing the mood of the speaking object. @todo: Add an ID table here.
     * @param whisper_target_animate If set to 1 the speaker will turn to the target and whisper.
     */
    void SendSpatialChat_(CreatureObject* const speaking_object, const std::wstring& custom_message, const common::OutOfBand& prose_message, PlayerObject* const player_object, uint64_t target_id, uint16_t text_size, SocialChatType chat_type_id, MoodType mood_id, uint8_t whisper_target_animate);

	static MessageLib*	mSingleton;
	static bool			mInsFlag;

	zmap*				mGrid;

    MessageFactory*		mMessageFactory;
};

//======================================================================================================================

#endif
