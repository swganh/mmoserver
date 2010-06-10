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


#include "Tutorial.h"
#include "AttackableCreature.h"
#include "Buff.h"
#include "BuildingObject.h"
#include "CharSheetManager.h"
#include "Container.h"
#include "FillerNPC.h"
#include "Inventory.h"
#include "NonPersistentNpcFactory.h"
#include "PlayerObject.h"
#include "ObjectFactory.h"
#include "SampleEvent.h"
#include "SchematicManager.h"
#include "UICloneSelectListBox.h"
#include "UIManager.h"
#include "UISkillSelectBox.h"
#include "UIOfferTeachBox.h"
#include "UIPlayerSelectBox.h"
#include "WorldConfig.h"
#include "WorldManager.h"

#include "Utils/EventHandler.h"
#include "MessageLib/MessageLib.h"
#include "ScriptEngine/ScriptEngine.h"
#include "LogManager/LogManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include "Common/atMacroString.h"


#include "utils/rand.h"


class TutorialQueryContainer
{
	public:
		uint32 mQueryType;
		uint64 mId;
};

class TutorialStartingLocation
{
	public:
		uint32 destinationPlanet;
		float  destX;
		float  destY;
		float  destZ;
};

#if defined(_MSC_VER)
const uint64 StartingRoom = 2203318222960;
#else
const uint64 StartingRoom = 2203318222960LLU;
#endif

// This is a type of CDEF-Pistol
const uint32 DefaultQuestWeaponFamily = 10;
// const uint32 DefaultQuestWeaponType = 2755;
const uint32 DefaultQuestWeaponType = 2326; // A even more useless gun for the tutorial :)


// const uint64 ItemOfficerRoom = (StartingRoom + 1);
// const uint64 BankAndBazzzarOfficerRoom = (ItemOfficerRoom + 1);
// const uint64 CloningAndInsuranceRoom = (BankAndBazzzarOfficerRoom + 1);

//=============================================================================
Tutorial::Tutorial(PlayerObject* player) :
mPlayerObject(player),
mState(0),
mSubState(1),
mZoomCamera(false),
mChatBox(false),
mFoodSelected(false),
mFoodUsed(false),
mCloneDataSaved(false),
mItemsInsured(false),
mCloseHolocron(false),
mChangeLookAtTarget(false),
mOpenInventory(false),
mCloseInventory(false),
mCellId(0),
mNpcConversationId(0),
mContainerEventId(0),
mQuestWeaponFamily(DefaultQuestWeaponFamily),
mQuestWeaponType(DefaultQuestWeaponType)
// mSpawnedNpc(NULL)
{
	// State shall be stored/fetched to/from DB.
	
	// query tutorial
	TutorialQueryContainer* asContainer = new TutorialQueryContainer();
	// asContainer->mObject = playerObject;
	asContainer->mQueryType = TutorialQuery_MainData;
	asContainer->mId = player->getId(); 

	(gWorldManager->getDatabase())->ExecuteSqlAsync(this,asContainer,"SELECT character_state, character_substate, starting_profession FROM character_tutorial WHERE character_id = %I64u",player->getId());	
}

Tutorial::~Tutorial()
{

	// Save-update the state.
	gWorldManager->getDatabase()->ExecuteSqlAsync(0,0,"UPDATE character_tutorial SET character_state=%u,character_substate=%u WHERE character_id=%"PRIu64"",mState, mSubState, mPlayerObject->getId());

	// clear scripts
	ScriptList::iterator scriptIt = mPlayerScripts.begin();

	while(scriptIt != mPlayerScripts.end())
	{
		gScriptEngine->removeScript(*scriptIt);
		scriptIt = mPlayerScripts.erase(scriptIt);
	}
}

void Tutorial::warpToStartingLocation(string startingLocation)
{
	gLogger->log(LogManager::DEBUG,"Tutorial::warpToStartingLocation: Starting city = %s", startingLocation.getAnsi());

	TutorialQueryContainer* asContainer = new TutorialQueryContainer();
	asContainer->mQueryType = TutorialQuery_PlanetLocation;
	asContainer->mId = mPlayerObject->getId(); 

	(gWorldManager->getDatabase())->ExecuteSqlAsync(this,asContainer,"SELECT planet_id, x, y, z FROM starting_location WHERE location LIKE '%s'", startingLocation.getAnsi());	

}

void Tutorial::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
	TutorialQueryContainer* asyncContainer = reinterpret_cast<TutorialQueryContainer*>(ref);
	
	switch(asyncContainer->mQueryType)
	{
		case TutorialQuery_MainData:
		{
			DataBinding* binding = gWorldManager->getDatabase()->CreateDataBinding(3);
			binding->addField(DFT_uint32,offsetof(Tutorial,mState),4,0);
			binding->addField(DFT_int32,offsetof(Tutorial,mSubState),4,1);
			binding->addField(DFT_bstring,offsetof(Tutorial,mStartingProfession),64,2);

			uint64 count = result->getRowCount();

			if (count == 1)
			{
				result->GetNextRow(binding,this);
				gLogger->log(LogManager::DEBUG,"Tutorial::handleDatabaseJobComplete: Starting profession = %s", mStartingProfession.getAnsi());
			}
			else if (count == 0)
			{
				// First time, no tutorial data saved.
				mSubState = 1;
				mState = 1;

				// Save the state.
				(gWorldManager->getDatabase())->ExecuteSqlAsync(0,0,"INSERT INTO character_tutorial VALUES (%"PRIu64",%u,%u)",asyncContainer->mId,mState, mSubState);
			}
			gWorldManager->getDatabase()->DestroyDataBinding(binding);
		
			// Here we go...
			this->startScript();
		}
		break;

		case TutorialQuery_PlanetLocation:
		{
			PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(asyncContainer->mId));
			if (player)
			{
				DataBinding* binding = gWorldManager->getDatabase()->CreateDataBinding(4);
				TutorialStartingLocation startingLocation;
				
				binding->addField(DFT_uint32, offsetof(TutorialStartingLocation, destinationPlanet), 4, 0);
				binding->addField(DFT_float, offsetof(TutorialStartingLocation, destX), 4, 1);
				binding->addField(DFT_float, offsetof(TutorialStartingLocation, destY), 4, 2);
				binding->addField(DFT_float, offsetof(TutorialStartingLocation, destZ), 4, 3);

				result->GetNextRow(binding, &startingLocation);

				startingLocation.destX += (gRandom->getRand()%5 - 2);
				startingLocation.destZ += (gRandom->getRand()%5 - 2);

				gLogger->log(LogManager::DEBUG,"Tutorial::handleDatabaseJobComplete: New destination planet = %u", startingLocation.destinationPlanet);

				gMessageLib->sendClusterZoneTransferRequestByPosition(player, 
                    glm::vec3(startingLocation.destX, startingLocation.destY, startingLocation.destZ), 
																	  startingLocation.destinationPlanet);
			}
			else
			{
				gLogger->log(LogManager::DEBUG,"Tutorial::handleDatabaseJobComplete: Player gone!");
			}
		}
		break;

		default:
		{
			gLogger->log(LogManager::DEBUG,"Tutorial::handleDatabaseJobComplete: Unknown query = %u\n",  asyncContainer->mQueryType);
		}
		break;
	}
	delete asyncContainer;

}	

//======================================================================================================================

// script callback, lets scripts register themselves for an event


void Tutorial::ScriptRegisterEvent(void* script,std::string eventFunction)
{
	mCmdScriptListener.registerScript(reinterpret_cast<Script*>(script),(int8*)eventFunction.c_str());
}

//======================================================================================================================
//
// available for scripts
// 
void Tutorial::scriptPlayMusic(uint32 soundId)
{
	gMessageLib->sendPlayMusicMessage(soundId, mPlayerObject);
}

//======================================================================================================================
//
// send a system message
// available for scripts
//

void Tutorial::scriptSystemMessage(std::string message)
{
	if (mPlayerObject && mPlayerObject->isConnected())
	{
    std::wstring msg(message.begin(), message.end());
		gMessageLib->sendSystemMessage(mPlayerObject, msg);
	}
}

//======================================================================================================================
//
// update tutorial 
//

void Tutorial::updateTutorial(std::string customMessage)
{
	// gLogger->log(LogManager::DEBUG,"Tutorial::updateTutorial sending %s",(int8*)(customMessage.c_str()));
	if (strcmp((int8*)(customMessage.c_str()), "chatActive") == 0)
	{
		// I don't know how to get chat-info via client newbie-function, so here is a local fix.
		mChatBox = false;		// Clear previous chat indications.
		return;
	}
	else if (strcmp((int8*)(customMessage.c_str()), "foodSelected") == 0)
	{
		// I don't know how to get info of selected item from the Inventory, so here is a local fix.
		mFoodSelected = false;
		return;
	}
	else if (strcmp((int8*)(customMessage.c_str()), "foodUsed") == 0)
	{
		// I don't know how to get info of radial actions, so here is a local fix.
		mFoodUsed = false;
		return;
	}
	else if (strcmp((int8*)(customMessage.c_str()), "cloneDataSaved") == 0)
	{
		// I don't know how to get info of radial actions, so here is a local fix.
		mCloneDataSaved = false;
		return;
	}
	else if (strcmp((int8*)(customMessage.c_str()), "insureItemsDone") == 0)
	{
		// I don't know how to get info of radial actions, so here is a local fix.
		mItemsInsured = false;
		return;
	}
	else
	{
		tutorialResponseReset((int8*)(customMessage.c_str()));
	}
	gMessageLib->sendUpdateTutorialRequest(mPlayerObject, BString((int8*)(customMessage.c_str())));
}


void Tutorial::openHolocron()
{
	// gLogger->log(LogManager::DEBUG,"Tutorial::sendOpenHolocron");
	gMessageLib->sendOpenHolocron(mPlayerObject);
}

//======================================================================================================================
//
// enable HUD element
// available for scripts
//

void Tutorial::enableHudElement(std::string customMessage)
{
	// gLogger->log(LogManager::DEBUG,"Tutorial::enableHudElement sending %s",(int8*)(customMessage.c_str()));
	gMessageLib->sendEnableHudElement(mPlayerObject, BString((int8*)(customMessage.c_str())));
}

//======================================================================================================================
//
// disable HUD element
// available for scripts
//

void Tutorial::disableHudElement(std::string customMessage)
{
	// gLogger->log(LogManager::DEBUG,"Tutorial::disableHudElement sending %s",(int8*)(customMessage.c_str()));
	gMessageLib->sendDisableHudElement(mPlayerObject, BString((int8*)(customMessage.c_str())));
}

//======================================================================================================================
//
//

void Tutorial::tutorialResponse(string tutorialEventString)
{
	if (strcmp(tutorialEventString.getAnsi(), "zoomCamera") == 0)
	{
		mZoomCamera = true;
	}
	else if (strcmp(tutorialEventString.getAnsi(), "chatActive") == 0)
	{
		mChatBox = true;
	}
	else if (strcmp(tutorialEventString.getAnsi(), "foodSelected") == 0)
	{
		mFoodSelected = true;
	}
	else if (strcmp(tutorialEventString.getAnsi(), "foodUsed") == 0)
	{
		mFoodUsed = true;
	}
	else if (strcmp(tutorialEventString.getAnsi(), "cloneDataSaved") == 0)
	{
		mCloneDataSaved = true;
	}
	else if (strcmp(tutorialEventString.getAnsi(), "insureItemsDone") == 0)
	{
		mItemsInsured = true;
	}
	else if (strcmp(tutorialEventString.getAnsi(), "closeHolocron") == 0)
	{
		mCloseHolocron = true;
	}
	else if (strcmp(tutorialEventString.getAnsi(), "changeLookAtTarget") == 0)
	{
		mChangeLookAtTarget = true;
	}
	else if (strcmp(tutorialEventString.getAnsi(), "openInventory") == 0)
	{
		mOpenInventory = true;
	}
	else if (strcmp(tutorialEventString.getAnsi(), "closeInventory") == 0)
	{
		mCloseInventory = true;
	}
}

//======================================================================================================================
//
//

void Tutorial::tutorialResponseReset(string tutorialEventString)
{
	if (strcmp(tutorialEventString.getAnsi(), "zoomCamera") == 0)
	{
		mZoomCamera = false;
	}
	else if (strcmp(tutorialEventString.getAnsi(), "chatActive") == 0)
	{
		mChatBox = false;
	}
	else if (strcmp(tutorialEventString.getAnsi(), "closeHolocron") == 0)
	{
		mCloseHolocron = false;
	}
	else if (strcmp(tutorialEventString.getAnsi(), "changeLookAtTarget") == 0)
	{
		mChangeLookAtTarget = false;
	}
	else if (strcmp(tutorialEventString.getAnsi(), "openInventory") == 0)
	{
		mOpenInventory = false;
	}
	else if (strcmp(tutorialEventString.getAnsi(), "closeInventory") == 0)
	{
		mCloseInventory = false;
	}

}

//======================================================================================================================
//
// available for scripts
//
bool Tutorial::isZoomCamera()
{
	return mZoomCamera;
}

//======================================================================================================================
//
// available for scripts
//
bool Tutorial::isChatBox()
{
	return mChatBox;
}

//======================================================================================================================
//
// available for scripts
//
bool Tutorial::isFoodSelected()
{
	return mFoodSelected;
}


//======================================================================================================================
//
// available for scripts
//
bool Tutorial::isFoodUsed()
{
	return mFoodUsed;
}

//======================================================================================================================
//
// available for scripts
//
bool Tutorial::isCloneDataSaved()
{
	return mCloneDataSaved;
}

//======================================================================================================================
//
// available for scripts
//
bool Tutorial::isItemsInsured()
{
	return mItemsInsured;
}

//======================================================================================================================
//
// available for scripts
//
bool Tutorial::isCloseHolocron()
{
	return mCloseHolocron;
}

//======================================================================================================================
//
// available for scripts
//
bool Tutorial::isChangeLookAtTarget()
{
	return mChangeLookAtTarget;
}

//======================================================================================================================
//
// available for scripts
//
bool Tutorial::isOpenInventory()
{
	return mOpenInventory;
}

//======================================================================================================================
//
// available for scripts
//
bool Tutorial::isCloseInventory()
{
	return mCloseInventory;
}

//======================================================================================================================
//
// available for scripts
//
uint32 Tutorial::getState()
{
	return mState;
}

//======================================================================================================================
//
// available for scripts
//
void Tutorial::setState(uint32 state)
{
	mState = state;

	// Save-update the state.
	gWorldManager->getDatabase()->ExecuteSqlAsync(0,0,"UPDATE character_tutorial SET character_state=%u,character_substate=%u WHERE character_id=%"PRIu64"",mState, mSubState, mPlayerObject->getId());

}

//======================================================================================================================
//
// Start the script.
//
void Tutorial::startScript(void)
{
	Script* script = ScriptEngine::Init()->createScript();
	script->setPriority(0);
	
	script->setFileName("script/TutorialPart_1_9.lua");
	script->setTutorial(this);	// Dirty... I know..(eru)

	mPlayerScripts.push_back(script);
	script->run();
}


//======================================================================================================================
//
// available for scripts
//
uint32 Tutorial::getSubState()
{
	return mSubState;
}

//======================================================================================================================
//
// available for scripts
//
void Tutorial::setSubState(uint32 subState)
{
	mSubState = subState;

	// Save-update the state.
	gWorldManager->getDatabase()->ExecuteSqlAsync(0,0,"UPDATE character_tutorial SET character_state=%u,character_substate=%u WHERE character_id=%"PRIu64"",mState, mSubState, mPlayerObject->getId());

}

//======================================================================================================================
//
// available for scripts
//
bool Tutorial::getReady()
{
	bool status = false;
	if (mPlayerObject && mPlayerObject->isConnected())
	{
		status = mPlayerObject->getReady();
	}
	return status;
}

void Tutorial::setCellId(uint64 cellId)
{
	mCellId = cellId;
}

uint32 Tutorial::getRoom()
{
	if (mCellId == 0)
	{
		// We need to manually update the cell id since we have not passed any cell borders yet.
		if (mPlayerObject && mPlayerObject->isConnected())
		{
			mCellId = mPlayerObject->getParentId();
		}
	}
	return static_cast<uint32>(mCellId + 1 - StartingRoom); // First room is 1.
}

void Tutorial::spatialChat(uint64 targetId, std::string chatMsg)
{
	NPCObject* npc = dynamic_cast<NPCObject*>(gWorldManager->getObjectById(targetId));
	if (mPlayerObject && mPlayerObject->isConnected() && npc)
	{
		string msg = (int8*)chatMsg.c_str();
		msg.convert(BSTRType_Unicode16);
		char quack[5][32];
		memset(quack, 0, sizeof(quack));
		gMessageLib->sendSpatialChat(npc, msg, quack, mPlayerObject);
	}
}

void Tutorial::spatialChatShout(uint64 targetId, std::string chatMsg)
{
	NPCObject* npc = dynamic_cast<NPCObject*>(gWorldManager->getObjectById(targetId));
	if (mPlayerObject && mPlayerObject->isConnected() && npc)
	{
		string msg = (int8*)chatMsg.c_str();
		msg.convert(BSTRType_Unicode16);
		char quack[5][32];
		memset(quack, 0, sizeof(quack));
		quack[1][0] = '8';
		quack[1][1] = '0';
		gMessageLib->sendSpatialChat(npc, msg, quack, mPlayerObject);
	}
}


void Tutorial::enableTutorial()
{
	gWorldConfig->enableTutorial();
}

void Tutorial::disableTutorial()
{
	gWorldConfig->disableTutorial();
}

float Tutorial::getPlayerPosX()
{
	float posX = -9000.0f;	// Out of range.
	if (mPlayerObject && mPlayerObject->isConnected())
	{
		posX = mPlayerObject->mPosition.x;
	}
	return posX;
}

float Tutorial::getPlayerPosY()
{
	float posY = -9000.0f;	// Out of range.
	if (mPlayerObject && mPlayerObject->isConnected())
	{
		posY = mPlayerObject->mPosition.y;
	}
	return posY;
}

float Tutorial::getPlayerPosZ()
{
	float posZ = -9000.0f;	// Out of range.
	if (mPlayerObject && mPlayerObject->isConnected())
	{
		posZ = mPlayerObject->mPosition.z;
	}
	return posZ;
}

float Tutorial::getPlayerPosToObject(uint64 objectId)
{
	float distance = -9000.0f;

	Object* object = dynamic_cast<Object*>(gWorldManager->getObjectById(objectId));
	if (object && mPlayerObject && mPlayerObject->isConnected())
	{
        distance = glm::distance(mPlayerObject->mPosition, object->mPosition);
	}
	return distance;
}

void Tutorial::enableNpcConversationEvent(uint64 objectId)
{
	mNpcConversationStarted = false;
	mNpcConversationEnded = false;
	mNpcConversationId = objectId;
}

void Tutorial::npcConversationHasStarted(uint64 npcId)
{
	if (npcId == mNpcConversationId)
	{
		mNpcConversationStarted = true;
	}
}

void Tutorial::npcConversationHasEnded(uint64 npcId)
{
	// There are situations where we can not quarantee that we will setup the npcId for a conversion before it has already started.
	// AT least let us know when it's finished.

	// if ((npcId == mNpcConversationId) && (mNpcConversationStarted == true))
	if (npcId == mNpcConversationId)
	{
		mNpcConversationEnded = true;
		mNpcConversationStarted = true;	// If we set ended, we have to set started also, else objects may be "hanging".
	}
}

bool Tutorial::isNpcConversationStarted(uint64 npcId)
{
	return ((npcId == mNpcConversationId)  && (mNpcConversationStarted == true));
}

bool Tutorial::isNpcConversationEnded(uint64 npcId)
{
	return ((npcId == mNpcConversationId)  && (mNpcConversationEnded == true));
}

void Tutorial::enableItemContainerEvent(uint64 objectId)
{
	mContainerIsOpen = false;
	mContainerIsClosed = false;
	mContainerTransferredItemCount = 0;
	mHasTransfered = false;
	mContainerEventId = objectId;
}

bool Tutorial::isContainerOpen(uint64 containerId)
{
	return ((containerId == mContainerEventId)  && (mContainerIsOpen == true));
}

bool Tutorial::isContainerClosed(uint64 containerId)
{
	return ((containerId == mContainerEventId)  && (mContainerIsClosed == true));
}

void Tutorial::containerOpen(uint64 containerId)
{
	if (containerId == mContainerEventId)
	{
		mContainerIsOpen = true;
		mContainerIsClosed = false; 
	}
}

void Tutorial::containerClose(uint64 containerId)
{
	if (containerId == mContainerEventId)
	{
		mContainerIsClosed = true;
		mContainerIsOpen = false;
	}
}


bool Tutorial::isContainerEmpty(uint64 containerId)
{
	bool empty = false;
	if (containerId == mContainerEventId)
	{
		Container* container = dynamic_cast<Container*>(gWorldManager->getObjectById(containerId));
		if (container)
		{
			uint32 objectCount = 0;
			ObjectList* objList = container->getObjects();
			ObjectList::iterator it = objList->begin();

			ObjectList::iterator cEnd = objList->end();

			while(it != cEnd)
			{
				if((*it)->getPrivateOwner() == this->getPlayer())
					++objectCount;

				++it;
			}

			empty = (objectCount == 0);
		}
	}
	return empty;
}

void Tutorial::transferedItemFromContainer(uint64 itemId, uint64 containerId)
{
	if (containerId == mContainerEventId)
	{
		mContainerTransferredItemCount++;
		mHasTransfered = true;
	}
}

bool Tutorial::isItemTransferedFromContainer(uint64 containerId)
{
	bool hasTransfered = mHasTransfered;
	mHasTransfered = false;
	return hasTransfered;
}



uint64 Tutorial::getPlayer(void)
{
	uint64 playerId = 0;
	if (mPlayerObject)
	{
		playerId = mPlayerObject->getId();
	}
	return playerId;
}

void Tutorial::addQuestWeapon(uint32 familyId, uint32 typeId)
{
	// gLogger->log(LogManager::DEBUG,"Tutorial::addItem Invoked",MSG_NORMAL);
	if (mPlayerObject && mPlayerObject->isConnected())
	{
		Inventory* inventory = dynamic_cast<Inventory*>(mPlayerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
		if (inventory)
		{
			if ((familyId != 0) && (typeId != 0))
			{
				// Set new default weapon
				mQuestWeaponFamily = familyId;
				mQuestWeaponType = typeId;
			}
            gObjectFactory->requestNewDefaultItem(inventory,mQuestWeaponFamily,mQuestWeaponType,inventory->getId(),99, glm::vec3(), "");
		}
	}
}

uint32 Tutorial::getQuestWeaponFamily(void)
{
	return mQuestWeaponFamily;
}

uint32 Tutorial::getQuestWeaponType(void)
{
	return mQuestWeaponType;
}


void Tutorial::npcStopFight(uint64 npcId)
{
	AttackableCreature* npc = dynamic_cast<AttackableCreature*>(gWorldManager->getObjectById(npcId));
	if (npc)
	{
		// NpcManager::Instance()->removeNpc(npcId);
		// npc->makePeaceWithDefender(mPlayerObject->getId());
		// npc->haltAllActions();
		// npc->unequipWeapon();
	}
}

bool Tutorial::isLowHam(uint64 npcId, int32 hamLevel)
{
	NPCObject* npc = dynamic_cast<NPCObject*>(gWorldManager->getObjectById(npcId));
	if (npc)
	{
		if ((npc->getHam()->getPropertyValue(HamBar_Health,HamProperty_CurrentHitpoints) < hamLevel) || 
			(npc->getHam()->getPropertyValue(HamBar_Action,HamProperty_CurrentHitpoints) < hamLevel) ||
			(npc->getHam()->getPropertyValue(HamBar_Mind,HamProperty_CurrentHitpoints) < hamLevel) ||
			(mPlayerObject->getHam()->getPropertyValue(HamBar_Health,HamProperty_CurrentHitpoints) < hamLevel) || 
			(mPlayerObject->getHam()->getPropertyValue(HamBar_Action,HamProperty_CurrentHitpoints) < hamLevel) ||
			(mPlayerObject->getHam()->getPropertyValue(HamBar_Mind,HamProperty_CurrentHitpoints) < hamLevel))
		{
			return true;
		}
	}
	return false;
}

uint64 Tutorial::getSkillTrainerTypeId(void)
{
	uint64 typeId = getSkillTrainerTypeId(this->mStartingProfession);
	if (typeId == 0)
	{
		gLogger->log(LogManager::DEBUG,"Tutorial::getSkillTrainerTypeId WARNING: Player have no starting profession set.");
	}
	return typeId;
}


static const uint64 MedicTrainerId[3] = {10, 11, 12};
static const uint64 ArtisanTrainerId[3] = {13, 14, 15};
static const uint64 BrawlerTrainerId[3] = {16, 17, 18};
static const uint64 EntertainerTrainerId[3] = {19, 20, 21};
static const uint64 MarksmanTrainerId[3] = {22, 23, 24};
static const uint64 ScoutTrainerId[3] = {25, 26, 27};

/*
static const uint64 MedicTrainerId[3] = {47244640305, 47244640307, 47244640397};
static const uint64 ArtisanTrainerId[3] = {47244640497, 47244640261, 47244640351};
static const uint64 BrawlerTrainerId[3] = {47244640429, 47244640571, 47244640601};
static const uint64 EntertainerTrainerId[3] = {47244640277, 47244640333, 47244640299};
static const uint64 MarksmanTrainerId[3] = {47244640339, 47244640363, 47244640451};
static const uint64 ScoutTrainerId[3] = {47244640311, 47244640461, 47244640525};
*/

uint64 Tutorial::getSkillTrainerTypeId(string startingProfession)
{
	uint64 typeId = 0;
	uint8 index = (uint8)(gRandom->getRand() % 3);

	if (strcmp(startingProfession.getAnsi(), "crafting_artisan") == 0)
	{
		// typeId = 47244640805;
		typeId = ArtisanTrainerId[index];
	}
	else if (strcmp(startingProfession.getAnsi(), "combat_brawler") == 0)
	{
		// typeId = 47244640807;
		typeId = BrawlerTrainerId[index];
	}
	else if (strcmp(startingProfession.getAnsi(), "social_entertainer") == 0)
	{
		// typeId = 47244640879;
		typeId = EntertainerTrainerId[index];
	}
	else if (strcmp(startingProfession.getAnsi(), "combat_marksman") == 0)
	{
		// typeId = 47244640813;
		typeId = MarksmanTrainerId[index];
	}
	else if (strcmp(startingProfession.getAnsi(), "science_medic") == 0)
	{
		// typeId = 47244640799;
		typeId = MedicTrainerId[index];
	}
	else if (strcmp(startingProfession.getAnsi(), "outdoors_scout") == 0)
	{
		// typeId = 47244640819;
		typeId = ScoutTrainerId[index];
	}
	return typeId;
}

bool Tutorial::isPlayerTrained(void)
{
	bool isTrained = false;

	if (mPlayerObject && mPlayerObject->isConnected())
	{
		isTrained = (mPlayerObject->getSkillPointsLeft() < 250);
	}
	return isTrained;
}

void Tutorial::setTutorialRefugeeTaunts(uint64 npcId)
{
	// This IS a stretch. We have to make this a more general solution.
	FillerNPC* npc = dynamic_cast<FillerNPC*>(gWorldManager->getObjectById(npcId));
	if (npc)
	{
		npc->setupTutorialTaunts(mPlayerObject->getId(),
								 120000,
								 "They're going to ship us off to the spice mines of Kessel.",
								 "The Emperor has disbanded the Senate, now the Empire can do whatever it wants.",
								 "But we're innocent!  We weren't carrying any contraband.  They attacked us for no reason.",
								 "I never should have left Alderaan... Oh, wait...",
								 "I told you we couldn't outrun an Imperial Star Destroyer.");
	}
}

void Tutorial::setTutorialCelebTaunts(uint64 npcId)
{
	// This IS a stretch. We have to make this a more general solution.
	FillerNPC* npc = dynamic_cast<FillerNPC*>(gWorldManager->getObjectById(npcId));
	if (npc)
	{
		npc->setupTutorialTaunts(mPlayerObject->getId(),
								 90000,
								 "I can't believe that crazy pirate is dead.",
								 "That must have been a tough battle.",
								 "I'm sure glad that's over with.",
								 "Good thing someone came along to kill that pirate.  We'd have been trapped in here forever.",
								 "I hate pirates.");
	}
}

void Tutorial::makeCreatureAttackable(uint64 npcId)
{
	CreatureObject* creature = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById(npcId));
	if (creature)
	{
		creature->togglePvPStateOn(CreaturePvPStatus_Attackable);
		gMessageLib->sendUpdatePvpStatus(creature,mPlayerObject);
		// gLogger->log(LogManager::DEBUG,"Tutorial::makeCreatureAttackable DONE OK" ,MSG_NORMAL);
	}
	else
	{
		gLogger->log(LogManager::DEBUG,"Tutorial::makeCreatureAttackable FAILED\n");
	}
}


void Tutorial::npcSendAnimation(uint64 npcId, uint32 animId, uint64 targetId)
{
	CreatureObject* creature = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById(npcId));
	if (creature)
	{
		if (targetId != 0)
		{
			if (creature->getTargetId() != targetId)
			{
				gLogger->log(LogManager::DEBUG,"Tutorial::npcSendAnimation Setting new target for npc\n");
				creature->setTarget(targetId);
				gMessageLib->sendTargetUpdateDeltasCreo6(creature);
			}
		}
		gMessageLib->sendCreatureAnimation(creature,gWorldManager->getNpcConverseAnimation(animId), mPlayerObject);
	}
}

void Tutorial::sendStartingLocationList(void)
{

	(void)gMessageLib->sendTutorialServerStatusRequest(mPlayerObject->getClient(), mPlayerObject->getId(), mPlayerObject->getAccountId());

	// "Select your destination by clicking on one of the planets on the screen.  When you have selected the planet, select which city you wish to travel to by clicking on the picture to the right of the screen.  When you are ready to travel to the city, click on the arrow in the lower right-hand corner of the screen."
	this->scriptSystemMessage("@newbie_tutorial/system_messages:select_dest");

}
