/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_TUTORIAL_H
#define ANH_ZONESERVER_TUTORIAL_H

#include "ScriptEngine/ScriptEventListener.h"
#include "DatabaseManager/DatabaseCallback.h"
#include <map>

class PlayerObject;

//=============================================================================

enum TutorialQueryType
{
	TutorialQuery_MainData = 1,
	TutorialQuery_PlanetLocation = 2
};

//=============================================================================

// class Tutorial : public ObjectFactoryCallback
class Tutorial : public DatabaseCallback
{
	public:

		Tutorial(PlayerObject* playerObject);
		~Tutorial();

		void				handleDatabaseJobComplete(void* ref,DatabaseResult* result);

		ScriptEventListener* getScriptInterface() { return &mCmdScriptListener; }
		void				npcConversationHasStarted(uint64 npcId);
		void				npcConversationHasEnded(uint64 npcId);

		void				containerOpen(uint64 containerId);
		void				containerClose(uint64 containerId);
		void				transferedItemFromContainer(uint64 itemId, uint64 containerId);
		void				startScript(void);

		// scripting
		void				ScriptRegisterEvent(void* script,std::string eventFunction);
		void				scriptPlayMusic(uint32 soundId);
		void				scriptSystemMessage(std::string message);
		void				tutorialResponse(string tutorialEventString);
		void				updateTutorial(std::string customMessage);
		void				openHolocron();
		void				spatialChat(uint64 targetId, std::string chatMsg);
		void				spatialChatShout(uint64 targetId, std::string chatMsg);

		bool				isZoomCamera();
		bool				isChatBox();
		bool				isFoodSelected();
		bool				isFoodUsed();
		bool				isCloneDataSaved();
		bool				isItemsInsured();
		bool				isCloseHolocron();
		bool				isChangeLookAtTarget();
		bool				isOpenInventory();
		bool				isCloseInventory();

		void				enableHudElement(std::string customMessage);
		void				disableHudElement(std::string customMessage);
		uint32				getState();
		void				setState(uint32 state);
		uint32				getSubState();
		void				setSubState(uint32 subState);
		bool				getReady();
		void				setCellId(uint64 cellId);
		uint32				getRoom();
		void				enableTutorial();
		void				disableTutorial();
		float				getPlayerPosX();
		float				getPlayerPosY();
		float				getPlayerPosZ();

		float				getPlayerPosToObject(uint64 objectId);
		void				enableNpcConversationEvent(uint64 objectId);
		bool				isNpcConversationStarted(uint64 npcId);
		bool				isNpcConversationEnded(uint64 npcId);

		void				enableItemContainerEvent(uint64 objectId);
		bool				isContainerOpen(uint64 containerId);
		bool				isContainerClosed(uint64 containerId);

		bool				isContainerEmpty(uint64 containerId);
		bool				isItemTransferedFromContainer(uint64 containerId);

		uint64				getPlayer(void);

		// void				addItem(uint32 familyId, uint32 typeId);
		void				addQuestWeapon(uint32 familyId, uint32 typeId);
		void				npcStopFight(uint64 npcId);
		bool				isLowHam(uint64 npcId, int32 hamLevel);
		uint64				getSkillTrainerTypeId(void);
		bool				isPlayerTrained(void);
		void				setTutorialRefugeeTaunts(uint64 npcId);
		void				setTutorialCelebTaunts(uint64 npcId);
		void				makeCreatureAttackable(uint64 npcId);
		void				npcSendAnimation(uint64 npcId, uint32 animId, uint64 targetId);

		// Not scripted
		uint32				getQuestWeaponFamily(void);
		uint32				getQuestWeaponType(void);
		void				sendStartingLocationList(void);
		void				warpToStartingLocation(string startingLocation);
		
	private:
		void				tutorialResponseReset(string tutorialEventString);
		uint64				getSkillTrainerTypeId(string startingProfession);

		PlayerObject*		mPlayerObject;

		// scripting
		ScriptList			mPlayerScripts;
		ScriptEventListener	mCmdScriptListener;

		// state
		uint32				mState;
		uint32				mSubState;
		string				mStartingProfession;

		bool				mZoomCamera;
		bool				mChatBox;
		bool				mFoodSelected;
		bool				mFoodUsed;
		bool				mCloneDataSaved;
		bool				mItemsInsured;
		bool				mCloseHolocron;
		bool				mChangeLookAtTarget;
		bool				mOpenInventory;
		bool				mCloseInventory;

		uint64				mCellId;
		// NPCObject*			mSpawnedNpc;

		bool				mNpcConversationStarted;
		bool				mNpcConversationEnded;
		uint64				mNpcConversationId;

		bool				mContainerIsOpen;
		bool				mContainerIsClosed;

		uint32				mContainerHasTransferedItem;
		uint64				mContainerEventId;
		uint32				mQuestWeaponFamily;
		uint32				mQuestWeaponType;


};
//=============================================================================

#endif
