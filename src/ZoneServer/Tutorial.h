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

#ifndef ANH_ZONESERVER_TUTORIAL_H
#define ANH_ZONESERVER_TUTORIAL_H

#include "ScriptEngine/ScriptEventListener.h"
#include "DatabaseManager/DatabaseCallback.h"
#include <map>

class PlayerObject;
class Container;

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

    ScriptEventListener* getScriptInterface() {
        return &mCmdScriptListener;
    }
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
    void				tutorialResponse(BString tutorialEventString);
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
    void				warpToStartingLocation(BString startingLocation);

    void				sendStartingMails(void);

private:
    void				tutorialResponseReset(BString tutorialEventString);
    uint64				getSkillTrainerTypeId(BString startingProfession);

    PlayerObject*		mPlayerObject;

    // scripting
    ScriptList			mPlayerScripts;
    ScriptEventListener	mCmdScriptListener;

    // state
    uint32				mState;
    uint32				mSubState;
    BString				mStartingProfession;

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
    bool				mHasTransfered;

    uint32				mContainerTransferredItemCount;
    uint64				mContainerEventId;
    uint32				mQuestWeaponFamily;
    uint32				mQuestWeaponType;

};
//=============================================================================

#endif
