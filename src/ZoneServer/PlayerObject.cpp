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

#include "PlayerObject.h"
#include "Badge.h"
#include "Bank.h"
#include "Buff.h"
#include "BuildingObject.h"
#include "BuffManager.h"
#include "CellObject.h"
#include "CharSheetManager.h"
#include "CraftingSessionFactory.h"
#include "CraftingStation.h"
#include "Datapad.h"
#include "DraftSchematic.h"
#include "GroupManager.h"
#include "GroupObject.h"
#include "Inventory.h"
#include "QuadTree.h"

#include "SampleEvent.h"
#include "SchematicGroup.h"
#include "SchematicManager.h"
#include "SpawnPoint.h"
#include "StructureManager.h"
#include "ResourceCollectionManager.h"
#include "Tutorial.h"
#include "UIManager.h"
#include "UISkillSelectBox.h"
#include "UIOfferTeachBox.h"
#include "UIPlayerSelectBox.h"
#include "UICloneSelectListBox.h"
#include "VehicleController.h"
#include "WorldConfig.h"
#include "WorldManager.h"
#include "ZoneOpcodes.h"

#include "ZoneTree.h"
#include "MessageLib/MessageLib.h"
#include "ScriptEngine/ScriptEngine.h"
#include "LogManager/LogManager.h"
#include "DatabaseManager/Database.h"
#include "Common/atMacroString.h"
#include "Common/Message.h"
#include "Common/MessageFactory.h"
#include "Utils/clock.h"
#include "Utils/EventHandler.h"


//=============================================================================

PlayerObject::PlayerObject()
: CreatureObject()
, mBazaarPoint(NULL)
, mClient(NULL)
, mCraftingSession(NULL)
, mMount(NULL)
, mNearestCloningFacility(NULL)
, mTravelPoint(NULL)
, mTutorial(NULL)
, mCombatTargetId(0)
, mEntertainerPauseId(0)
, mEntertainerTaskId(0)
, mEntertainerWatchToId(0)
, mLastGroupMissionUpdateTime(0)
, mNearestCraftingStation(0)
, mPlacedInstrument(0)
, mPlayerObjId(0)
, mPreDesignatedCloningFacilityId(0)
, mSelectedInstrument(0)
, mTradePartner(NULL)
, mAccountId(0)
, mClientTickCount(0)
, mCraftingStage(0)
, mDConnTime(60)
, mExperimentationFlag(0)
, mExperimentationPoints(0)
, mFriendsListUpdateCounter(0)
, mHoloEmote(0)
, mPlayerCustomFlags(0)
, mIgnoresListUpdateCounter(0)
, mPlayerFlags(0)
, mMissionIdMask(0)
, mBindPlanet(-1)
, mHomePlanet(-1)
, mHoloCharge(0)
, mLots(10)
, mNewPlayerExemptions(0)
, mAutoAttack(false)
, mContactListUpdatePending(false)
, mMotdReceived(false)
, mMountCalled(false)
, mMounted(false)
, mNewPlayerMessage(false)
, mTrading(false)
, mHasCamp(false)
{
	mIsForaging			= false;
	mType				= ObjType_Player;
	mCreoGroup			= CreoGroup_Player;
	mStomach			= new Stomach(this);
	mMarriage			= L"";					// Unmarried
	mTrade				= new Trade(this);

	// register event functions
	registerEventFunction(this,&PlayerObject::onSurvey);
	registerEventFunction(this,&PlayerObject::onSample);
	registerEventFunction(this,&PlayerObject::onLogout);
	registerEventFunction(this,&PlayerObject::onBurstRun);
	registerEventFunction(this,&PlayerObject::onItemDeleteEvent);
	registerEventFunction(this,&PlayerObject::onInjuryTreatment);
	registerEventFunction(this,&PlayerObject::onWoundTreatment);
	registerEventFunction(this,&PlayerObject::onQuickHealInjuryTreatment);
	
	mLots = gWorldConfig->getConfiguration<uint32>("Player_Max_Lots",(uint32)10);

	mPermissionId = 0;

	mIDSession = IDSessionNONE;
	
	getSampleData()->mPassRadioactive	= false;
	getSampleData()->mPendingSample		= false;
	getSampleData()->mPendingSurvey		= false;
	getSampleData()->mSampleEventFlag	= false;
	getSampleData()->mSampleGambleFlag	= false;
	getSampleData()->mSampleNodeFlag	= false;
	getSampleData()->mSampleNodeRecovery= false;
	getSampleData()->mNextSampleTime	= 0;
}

//=============================================================================

PlayerObject::~PlayerObject()
{
	// store any eventually spawned vehicle
	Datapad* datapad			= getDataPad();

	if(mMount && datapad)
	{
		if(VehicleController* datapad_pet = dynamic_cast<VehicleController*>(datapad->getDataById(mMount->controller())))
		{
			datapad_pet->Store();
		}
	}

	// make sure we stop entertaining if we are an entertainer
	gEntertainerManager->stopEntertaining(this);

	mObjectController.setTaskId(0);
	mHam.setTaskId(0);
	mStomach->mFoodTaskId = 0;
	mStomach->mDrinkTaskId = 0;

	// delete currently placed instrument
	if(mPlacedInstrument)
	{
		if(Item* item = dynamic_cast<Item*>(gWorldManager->getObjectById(mPlacedInstrument)))
		{
			mObjectController.destroyObject(mPlacedInstrument);
		}
	}


	// make sure we are deleted out of entertainer Ticks when entertained
	if(mEntertainerWatchToId)
	{
		if(PlayerObject* entertainer = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(mEntertainerWatchToId)))
		{
			gEntertainerManager->removeAudience(entertainer,this);
		}
	}

	if(mEntertainerListenToId)
	{
		if(PlayerObject* entertainer = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(mEntertainerListenToId)))
		{
			gEntertainerManager->removeAudience(entertainer,this);
		}
	}

	// make sure we don't leave a crafting session open
	gCraftingSessionFactory->destroySession(mCraftingSession);
	this->setCraftingSession(NULL);
	this->toggleStateOff(CreatureState_Crafting);
	this->setCraftingStage(0);
	this->setExperimentationFlag(0);

	// can't zone or logout while in combat
	this->toggleStateOff(CreatureState_Combat);
	this->toggleStateOff(CreatureState_Dizzy);
	this->toggleStateOff(CreatureState_Stunned);
	this->toggleStateOff(CreatureState_Blinded);
	this->toggleStateOff(CreatureState_Intimidated);

	// update duel lists
	PlayerList::iterator duelIt = mDuelList.begin();

	while(duelIt != mDuelList.end())
	{
		if((*duelIt)->checkDuelList(this))
		{
			PlayerObject* duelPlayer = (*duelIt);

			duelPlayer->removeFromDuelList(this);

			gMessageLib->sendUpdatePvpStatus(this,duelPlayer);
			gMessageLib->sendUpdatePvpStatus(duelPlayer,this);
		}

		++duelIt;
	}		 
	mDuelList.clear();

	

	// update defender lists
	ObjectIDList::iterator defenderIt = mDefenders.begin();

	while (defenderIt != mDefenders.end())
	{
		if (CreatureObject* defenderCreature = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById((*defenderIt))))
		{
			defenderCreature->removeDefenderAndUpdateList(mId);

			if(PlayerObject* defenderPlayer = dynamic_cast<PlayerObject*>(defenderCreature))
			{
				gMessageLib->sendUpdatePvpStatus(this,defenderPlayer);
			}

			// if no more defenders, clear combat state
			if(!defenderCreature->getDefenders()->size())
			{
				defenderCreature->toggleStateOff(CreatureState_Combat);

				gMessageLib->sendStateUpdate(defenderCreature);
			}
		}

		++defenderIt;
	}

	// destroy known objects
	destroyKnownObjects();

	clearAllUIWindows();

	stopTutorial();

	/*// mission bag
	Object* missionBag = mEquipManager.getEquippedObject(CreatureEquipSlot_MissionBag);
	mEquipManager.removeEquippedObject(CreatureEquipSlot_MissionBag);
	delete(missionBag);

	// datapad
	mEquipManager.removeEquippedObject(CreatureEquipSlot_Datapad);
	delete(datapad);

	// bank
	Object* bank = mEquipManager.getEquippedObject(CreatureEquipSlot_Bank);
	mEquipManager.removeEquippedObject(CreatureEquipSlot_Bank);
	delete(bank);*/

	delete(mStomach);
	delete(mTrade);
	
	delete getClient();
	setClient(NULL);
}

//=============================================================================

void PlayerObject::stopTutorial()
{
	if (gWorldConfig->isTutorial() && mTutorial)
	{
		delete(mTutorial);
		mTutorial = NULL;
	}
}

//=============================================================================

void PlayerObject::startTutorial()
{
	if (gWorldConfig->isTutorial() && !mTutorial)
	{
		mTutorial = new Tutorial(this);
	}
}

//=============================================================================

void PlayerObject::resetProperties()
{
	mInMoveCount						= 0;
	mClientTickCount					= 0;
	mSkillCmdUpdateCounter				= mSkillCommands.size();
	mSkillModUpdateCounter				= mSkillMods.size();
	mXpUpdateCounter					= mXpList.size();
	mPosture							= CreaturePosture_Upright;

	// the client resets the bufftimers on local travel ... :(
	gBuffManager->InitBuffs(this);

	getSampleData()->mPendingSurvey		= false;
	getSampleData()->mPendingSample		= false;
	getSampleData()->mSampleNodeRecovery= false;

	this->togglePlayerCustomFlagOff(PlayerCustomFlag_LogOut);	
	this->togglePlayerCustomFlagOff(PlayerCustomFlag_BurstRun);	

	mDefenderUpdateCounter				= 0;
	mReady								= false;

	Datapad* datapad			= getDataPad();
	if(datapad)
	{
		datapad->mWaypointUpdateCounter = datapad->getWaypoints()->size();
	}

	updateMovementProperties();

	// We should not need to mess with HAM here, when doing intra planet (local on planet) travel.
	// mHam.resetCounters();
	mHam.updateRegenRates();

	// We should not be allowed to travel when in combat or in duel, if we are.. I want to see the bug happening so we can fix the cause of it.
	/*
	// clear duel lists
	PlayerList::iterator duelIt = mDuelList.begin();

	while(duelIt != mDuelList.end())
	{
		(*duelIt)->removeFromDuelList(this);

		++duelIt;
	}

	mDuelList.clear();

	// clear defender lists
	ObjectIDList::iterator defenderIt = mDefenders.begin();

	while(defenderIt != mDefenders.end())
	{
		if(CreatureObject* defenderCreature = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById((*defenderIt))))
		{
			defenderCreature->removeDefenderAndUpdateList(this->getId());
			// defenderCreature->removeDefender(this);

			if(PlayerObject* defenderPlayer = dynamic_cast<PlayerObject*>(defenderCreature))
			{
				gMessageLib->sendUpdatePvpStatus(this,defenderPlayer);
			}

			// gMessageLib->sendNewDefenderList(defenderCreature);
		}

		++defenderIt;
	}
	mDefenders.clear();
	*/
}

//=============================================================================

void PlayerObject::prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount)
{
	PlayerObject*	playerObject	= dynamic_cast<PlayerObject*>(creatureObject);

	if(!playerObject)
	{
		return;
	}

	RadialMenu*		radial			= new RadialMenu();

	uint8 radId = 0;

	//if we have a prefab Menu (we will have one as a player)  iterate through it and add it to our response
	//this way we will have our menu item numbering done right
	MenuItemList* menuItemList = 		getMenuList();
	if(menuItemList)
	{
		MenuItemList::iterator it	=	menuItemList->begin();

		while(it != menuItemList->end())
		{
			radId++;
			
			radial->addItem((*it)->sItem,(*it)->sSubMenu,(RadialIdentifier)(*it)->sIdentifier,(*it)->sOption,"");
			it++;
		}
	}


	// entertainer - dance
	if(mPendingPerform == PlayerPerformance_Dance)
	{
		// stop watching
		if(playerObject->getEntertainerWatchToId()== mId)
		{
			radial->addItem(radId++,0,radId_serverPerformanceWatchStop,radAction_ObjCallback,"Stop Watching");

		}
		// start watching
        else if(glm::distance(playerObject->mPosition, mPosition) < 20)
		{
			radial->addItem(radId++,0,radId_serverPerformanceWatch,radAction_ObjCallback,"Watch");
		}
	}
	// entertainer - music
	else if(mPendingPerform == PlayerPerformance_Music)
	{
		// stop listening
		if(playerObject->getEntertainerListenToId()== mId)
		{
			radial->addItem(radId++,0,radId_serverPerformanceListenStop,radAction_ObjCallback,"Stop Listening");
		}
		// start listening
        else if(glm::distance(playerObject->mPosition, mPosition) < 20)
		{
			radial->addItem(radId++,0,radId_serverPerformanceListen,radAction_ObjCallback,"Listen");

		}
	}

	// teach
	if(playerObject->getGroupId() && (mGroupId == playerObject->getGroupId()))
	{
		radial->addItem(radId++,0,radId_serverTeach,radAction_ObjCallback,"Teach");
		
	}

	RadialMenuPtr radialPtr(radial);
	mRadialMenu = radialPtr;
}

//=============================================================================

void PlayerObject::clearAllUIWindows()
{
	UIWindowList::iterator it = mUIWindowList.begin();

	while(it != mUIWindowList.end())
	{
		gUIManager->destroyUIWindow(*it);
		mUIWindowList.erase(it);
		it = mUIWindowList.begin();
	}
}

//=============================================================================

void PlayerObject::resetUICallbacks(Object* object)
{
	UIWindowList::iterator it = mUIWindowList.begin();

	while(it != mUIWindowList.end())
	{
		UIWindow* window = gUIManager->getUIWindow(*it);

		if(window && window->getCallback() == object)
		{
			window->setCallback(NULL);
		}

		++it;
	}
}

//=============================================================================

bool PlayerObject::UpdateXp(uint32 xpType,int32 value)
{
	XPList::iterator it = mXpList.begin();

	while(it != mXpList.end())
	{
		if(it->first == xpType)
		{
			it->second += value;
			return(true);
		}

		++it;
	}

	return(false);
}

//=============================================================================

bool PlayerObject::UpdateXpCap(uint32 xpType,int32 value)
{
	XPList::iterator it = mXpCapList.begin();

	while (it != mXpCapList.end())
	{
		if (it->first == xpType)
		{
			it->second = value;
			return (true);
		}

		++it;
	}

	return (false);
}

//=============================================================================

bool PlayerObject::UpdateIdAttributes(BString attribute,float value)
{
	AttributesList::iterator it = mIDAttributesList.begin();

	while(it != mIDAttributesList.end())
	{
		if (it->first.getCrc() == attribute.getCrc())
		{
			it->second = value;
			return(true);
		}

		++it;
	}

	mIDAttributesList.push_back(std::make_pair(attribute,value));

	return(true);
}

//=============================================================================

bool PlayerObject::UpdateIdColors(BString attribute,uint16 value)
{
	ColorList::iterator it = mIDColorList.begin();

	while(it != mIDColorList.end())
	{
		if (it->first.getCrc() == attribute.getCrc())
		{
			it->second = value;
			return(true);
		}

		++it;
	}

	mIDColorList.push_back(std::make_pair(attribute,value));

	return(true);
}

//=============================================================================

bool PlayerObject::checkDeductCredits(int32 amount)
{
	Bank*		bank		= dynamic_cast<Bank*>(mEquipManager.getEquippedObject(CreatureEquipSlot_Bank));
	Inventory*	inventory	= dynamic_cast<Inventory*>(mEquipManager.getEquippedObject(CreatureEquipSlot_Inventory));

	if(bank && inventory)
	{
		return(amount <= bank->getCredits() + inventory->getCredits());
	}
	else
	{
		return(false);
	}
}

//=============================================================================

bool PlayerObject::testBank(int32 amount)
{
	if(Bank* bank = dynamic_cast<Bank*>(mEquipManager.getEquippedObject(CreatureEquipSlot_Bank)))
	{
		return(amount <= bank->getCredits());
	}

	return(false);
}

//=============================================================================

bool PlayerObject::testCash(int32 amount)
{
	if(Inventory* inventory = dynamic_cast<Inventory*>(mEquipManager.getEquippedObject(CreatureEquipSlot_Inventory)))
	{
		return(amount <= inventory->getCredits());
	}

	return(false);
}

//=============================================================================

bool PlayerObject::deductCredits(int32 amount)
{
	if(Bank* bank = dynamic_cast<Bank*>(mEquipManager.getEquippedObject(CreatureEquipSlot_Bank)))
	{
		if(Inventory* inventory = dynamic_cast<Inventory*>(mEquipManager.getEquippedObject(CreatureEquipSlot_Inventory)))
		{
			if(amount <= bank->getCredits() + inventory->getCredits())
			{
				// bank first
				if(amount > bank->getCredits())
				{
					// first empty bank, then inv.
					amount -= bank->getCredits();
					bank->setCredits(0);
					inventory->setCredits(inventory->getCredits() - amount);
				}
				else
				{
					bank->setCredits(bank->getCredits() - amount);
				}

				gMessageLib->sendBankCreditsUpdate(this);
				gMessageLib->sendInventoryCreditsUpdate(this);

				return(true);
			}
			else
			{
				return(false);
			}
		}
	}

	return(false);
}

//=============================================================================

bool PlayerObject::checkXpType(uint32 xpType)
{
	XPList::iterator it = mXpList.begin();

	while(it != mXpList.end())
	{
		if(it->first == xpType)
			return(true);

		++it;
	}

	return(false);
}

//=============================================================================
//
// Here is where we shall add restrictions for JTL, Jedi, Pre-Pub 14
// or other restrictions for type of XP to allow.
//
bool PlayerObject::restrictedXpType(uint32 xpType)
{
	bool restricted = false;

	if ((xpType == 0) || (xpType >= 49)) // Out of bonds.
	{
		restricted = true;
	}
	else if (xpType == 1) // Apprenticeship
	{
		restricted = true;
	}
	// FS and Jedi
	else if ((xpType == 7) || (xpType == 8) || (xpType == 9) || // Lightsabers
			(xpType == 10) || (xpType == 31) ||				// Jedi and (unknown right now).
			(xpType == 42) ||									// Force Rank
			(xpType == 43) || (xpType == 44) || (xpType == 45) || (xpType == 46))	// Force sensitive
	{
		restricted = true;
	}
	else if ((xpType == 47) || (xpType == 48)) // Shipwright and Starship Combat
	{
		restricted = true;
	}

	return restricted;
}

//=============================================================================

int32 PlayerObject::getXpAmount(uint32 xpType)
{
	XPList::iterator it = mXpList.begin();

	while(it != mXpList.end())
	{
		if(it->first == xpType)
			return((*it).second);

		++it;
	}

	return(0);
}

//=============================================================================

bool PlayerObject::removeXpType(uint32 xpType)
{
	XPList::iterator it = mXpList.begin();

	while(it != mXpList.end())
	{
		if(it->first == xpType)
		{
			mXpList.erase(it);
			return(true);
		}

		++it;
	}

	return(false);
}

//=============================================================================

void PlayerObject::addXpType(uint32 xpType,int32 value)
{
	if(!(checkXpType(xpType)))
	{
		mXpList.push_back(std::make_pair(xpType,value));
	}
}

//=============================================================================

bool PlayerObject::checkXpCapType(uint32 xpType)
{
	XPList::iterator it = mXpCapList.begin();

	while(it != mXpCapList.end())
	{
		if(it->first == xpType)
			return(true);

		++it;
	}

	return(false);
}

//=============================================================================

int32 PlayerObject::getXpCapAmount(uint32 xpType)
{
	XPList::iterator it = mXpCapList.begin();

	while(it != mXpCapList.end())
	{
		if(it->first == xpType)
			return((*it).second);

		++it;
	}

	return(0);
}

//=============================================================================

bool PlayerObject::removeXpCapType(uint32 xpType)
{
	XPList::iterator it = mXpCapList.begin();

	while(it != mXpCapList.end())
	{
		if(it->first == xpType)
		{
			mXpList.erase(it);
			return(true);
		}

		++it;
	}

	return(false);
}

//=============================================================================

void PlayerObject::addXpCapType(uint32 xpType,int32 value)
{
	if(!(checkXpCapType(xpType)))
	{
		mXpCapList.push_back(std::make_pair(xpType,value));
	}
}

//=============================================================================

bool PlayerObject::checkSchematicId(uint64 id)
{
	SchematicsIdList::iterator it = mSchematicIdList.begin();

	while(it != mSchematicIdList.end())
	{
		if((*it) == id)
		{
			return(true);
		}

		++it;
	}

	return(false);
}

//=============================================================================

void PlayerObject::addSchematicIds(Skill* skill)
{
	SchematicGroupsList::iterator groupIt = skill->mSchematics.begin();

	mSchematicAddList.clear();

	while(groupIt != skill->mSchematics.end())
	{
		SchematicGroup*				sGroup		= gSchematicManager->getSchematicGroupById(*groupIt);
		SchematicsList::iterator	schematicIt = sGroup->mSchematics.begin();

		while(schematicIt != sGroup->mSchematics.end())
		{
			if(!checkSchematicId((*schematicIt)->getId()))
			{
				mSchematicAddList.push_back((*schematicIt)->getId());
			}

			++schematicIt;
		}

		++groupIt;
	}
}
//=============================================================================

void PlayerObject::prepareSchematicIds()
{
	mSchematicIdList.clear();

	SkillList::iterator skillIt = mSkills.begin();

	while(skillIt != mSkills.end())
	{
		SchematicGroupsList::iterator groupIt = (*skillIt)->mSchematics.begin();

		while(groupIt != (*skillIt)->mSchematics.end())
		{
			SchematicGroup*				sGroup		= gSchematicManager->getSchematicGroupById(*groupIt);
			SchematicsList::iterator	schematicIt = sGroup->mSchematics.begin();

			while(schematicIt != sGroup->mSchematics.end())
			{
				if(!checkSchematicId((*schematicIt)->getId()))
				{
					mSchematicIdList.push_back((*schematicIt)->getId());
				}

				++schematicIt;
			}

			++groupIt;
		}

		++skillIt;
	}
}

//=============================================================================

bool PlayerObject::checkBadges(uint32 badgeId)
{
	BadgesList::iterator it = mBadgeList.begin();

	while(it != mBadgeList.end())
	{
		if((*it) == badgeId)
		{
			return(true);
		}

		++it;
	}

	return(false);
}

//=============================================================================

void PlayerObject::addBadge(uint32 badgeId)
{
	// Since we use recursive calls to addBadge(), I would like to have an extra check for duplicates here.
	if(!checkBadges(badgeId))
	{
		mBadgeList.push_back(badgeId);
		Badge* badge = gCharSheetManager->getBadgeById(badgeId);

		gMessageLib->sendPlayMusicMessage(badge->getSoundId(),this);
    gMessageLib->sendSystemMessage(this,L"","badge_n","prose_grant","badge_n",badge->getName().getAnsi(),L"");

		(gWorldManager->getDatabase())->ExecuteSqlAsync(0,0,"INSERT INTO character_badges VALUES (%"PRIu64",%u)",mId,badgeId);

		gLogger->log(LogManager::DEBUG,"Badge %u granted to %"PRIu64"",badgeId,mId);

		_verifyBadges();

		if(badge->getCategory() >= 3 && badge->getCategory() <= 5)
		{
			_verifyExplorationBadges();
		}
	}
	else
	{
		// This is an unexpected condition.
		gLogger->log(LogManager::DEBUG,"Badge %u already exists for player with id %"PRIu64"",badgeId,mId);
	}
}

//=============================================================================

void PlayerObject::_verifyExplorationBadges()
{
	uint16					count	= 0;
	BadgesList::iterator	it		= mBadgeList.begin();

	while(it != mBadgeList.end())
	{
		Badge* badge = gCharSheetManager->getBadgeById((*it));

		if(badge->getCategory() >= 3 && badge->getCategory() <= 5)
		{
			++count;
		}

		++it;
	}

	switch(count)
	{
		case 10: addBadge(100); break;
		case 20: addBadge(101); break;
		case 30: addBadge(102); break;
		case 40: addBadge(103); break;
		case 45: addBadge(104); break;

		default: break;
	}

	_verifyBadges();
}

//=============================================================================

void PlayerObject::_verifyBadges()
{
	uint32 count = mBadgeList.size();

	switch(count)
	{
		case 5:		addBadge(0); break;
		case 10:	addBadge(1); break;
		case 25:	addBadge(2); break;
		case 50:	addBadge(3); break;
		case 75:	addBadge(4); break;
		case 100:	addBadge(5); break;
		case 125:	addBadge(6); break;

		default:break;
	}
}

//=============================================================================

bool PlayerObject::removeUIWindow(uint32 id)
{
	UIWindowList::iterator it = mUIWindowList.begin();

	while(it != mUIWindowList.end())
	{
		if((*it) == id)
		{
			mUIWindowList.erase(it);
			return(true);
		}

		++it;
	}

	return(false);
}

//=============================================================================

bool PlayerObject::checkUIWindow(string text)
{
	UIWindowList::iterator it = mUIWindowList.begin();

	while(it != mUIWindowList.end())
	{
		UIWindow* window = gUIManager->getUIWindow((*it));

		if(window->getEventStr().getCrc() == text.getCrc())
		{
			return(true);
		}

		++it;
	}

	return(false);
}

//=============================================================================

bool PlayerObject::removeFriend(uint32 nameCrc)
{
	ContactMap::iterator it = mFriendsList.find(nameCrc);

	if(it != mFriendsList.end())
	{
		mFriendsList.erase(it);
		return(true);
	}

	return(false);
}

//=============================================================================

bool PlayerObject::removeIgnore(uint32 nameCrc)
{
	ContactMap::iterator it = mIgnoreList.find(nameCrc);

	if(it != mIgnoreList.end())
	{
		mIgnoreList.erase(it);
		return(true);
	}

	return(false);
}

//=============================================================================

bool PlayerObject::checkFriendList(uint32 nameCrc)
{
	ContactMap::iterator it = mFriendsList.find(nameCrc);

	if(it != mFriendsList.end())
	{
		return(true);
	}

	return(false);
}

//=============================================================================

bool PlayerObject::checkIgnoreList(uint32 nameCrc) const
{
	ContactMap::const_iterator it = mIgnoreList.find(nameCrc);

	if(it != mIgnoreList.end())
	{
		return(true);
	}

	return(false);
}

//=============================================================================

PlayerList PlayerObject::getInRangeGroupMembers(bool self) const
{
	PlayerObjectSet::const_iterator	it			= mKnownPlayers.begin();
	PlayerList						members;

	if(self)
	{
		members.push_back((PlayerObject*)this);
	}

	if(mGroupId == 0)
	{
		return members;
	}

	while(it != mKnownPlayers.end())
	{
		if((*it)->getGroupId() == mGroupId)
		{
			members.push_back(*it);
		}

		++it;
	}

	return members;
}

//=============================================================================

void PlayerObject::handleObjectMenuSelect(uint8 messageType,Object* srcObject)
{
	PlayerObject* callingObject = dynamic_cast<PlayerObject*>(srcObject);

	if(!callingObject || !callingObject->isConnected())
	{
		return;
	}

	switch(messageType)
	{
		case radId_serverTeach:
		{
			// todo : In theory - if someone breaks our encryption - we might get a botter
			// in which case our checks need to be better! At this time we can *assume* that the client provided
			// us with a teacher and pupil!

			// check if our pupil already gets taught
			if (!mTrade->getTeacher())
			{
				mTrade->setTeacher(callingObject);
				gSkillManager->teach(this,callingObject,"");
			}
			else
			{
				gMessageLib->sendSystemMessage(callingObject,L"","teaching","student_has_offer_to_learn","","",L"",0,"","",L"",mId);
			}
		}
		break;

		case radId_tradeStart:
		break;

		case radId_serverPerformanceWatch:
		{
			gEntertainerManager->startWatching(callingObject,this);
		}
		break;

		case radId_serverPerformanceWatchStop:
		{
			gEntertainerManager->stopWatching(callingObject);
		}
		break;

		case radId_serverPerformanceListen:
		{
			gEntertainerManager->startListening(callingObject,this);
		}
		break;

		case radId_serverPerformanceListenStop:
		{
			gEntertainerManager->stopListening(callingObject);
		}
		break;

		default:
		{
			gLogger->log(LogManager::DEBUG,"PlayerObject: Unhandled MenuSelect: %u",messageType);
		}
		break;
	}
}

//=============================================================================

void PlayerObject::giveBankCredits(uint32 amount)
{
	if(Bank* bank = dynamic_cast<Bank*>(mEquipManager.getEquippedObject(CreatureEquipSlot_Bank)))
	{
		bank->setCredits(bank->getCredits() + amount);

		gMessageLib->sendBankCreditsUpdate(this);
	}
}

//=============================================================================

void PlayerObject::giveInventoryCredits(uint32 amount)
{
	if(Inventory* inventory = dynamic_cast<Inventory*>(mEquipManager.getEquippedObject(CreatureEquipSlot_Inventory)))
	{
		inventory->setCredits(inventory->getCredits() + amount);

		gMessageLib->sendInventoryCreditsUpdate(this);
	}
}

//=============================================================================
//
// handles any UIWindow callbacks for this player
//
void PlayerObject::handleUIEvent(uint32 action,int32 element,string inputStr,UIWindow* window)
{
	switch(window->getWindowType())
	{
		// clone activation list box
		case SUI_Window_CloneSelect_ListBox:
		{
			// gLogger->log(LogManager::DEBUG,"PlayerObject::handleUIEvent element = %d",element);

			// Handle non-selected response as if closest cloning facility was selected,
			// until we learn how to restart the dialog when nothing selected.
			if (element < 0 || element > 1)
			{
				element = 0;
				// No selection made, re-open the dialog.
				// window->sendCreate();
				// break;
			}

			UICloneSelectListBox* lb = dynamic_cast<UICloneSelectListBox*>(window);

			if ((lb) && (lb->getBuildingList()->size() > 0))
			{
				if (BuildingObject* building = lb->getBuildingList()->at(element))
				{
					if (SpawnPoint* sp = building->getRandomSpawnPoint())
					{
						if (element == 1)
						{
							// Clone at the pre-designated facility...
							mObjectController.cloneAtPreDesignatedFacility(this, sp);
						}
						else //  if (element == 0)  // Handle non-selected response as if closest cloning facility was selected,
							// until we learn how to restart the dialog when nothing selected.
						{
							// We selected the closest facility, but that CAN be the same as the pre-designated.
							if (lb->getBuildingList()->size() > 1)
							{
								// We do have more than one selection
								if (BuildingObject* preDesignatedBuilding = lb->getBuildingList()->at(1))
								{
									if (preDesignatedBuilding == building)
									{
										// Clone at the pre-designated facility...
										mObjectController.cloneAtPreDesignatedFacility(this, sp);
										break;
									}
								}
							}
							if (this->mNewPlayerExemptions == 0)
							{
								// Add wounds...
								this->getHam()->updatePrimaryWounds(100);

								// .. and some BF
								this->getHam()->updateBattleFatigue(100, true);
							}

							// Clone
							clone(sp->mCellId,sp->mDirection,sp->mPosition);
						}
					}
				}
			}
		}
		break;

		// generic message box
		case SUI_Window_MessageBox:
		{
			// identify by event string, none needed yet
			if(strcmp(window->getEventStr().getAnsi(),"example") == 0)
			{

			}
		}
		break;

		// generic list box
		case SUI_Window_ListBox:
		{
			// identify by event string, none needed yet
			if(strcmp(window->getEventStr().getAnsi(),"example") == 0)
			{

			}
		}
		break;

		// generix input box
		case SUI_Window_InputBox:
		{
			// identify by event string, none needed yet
			if(strcmp(window->getEventStr().getAnsi(),"example") == 0)
			{

			}
		}
		break;

		// teaching, skill select box
		case SUI_Window_Teach_SelectSkill_ListBox:
		{
			UISkillSelectBox*	skillSelectBox	= dynamic_cast<UISkillSelectBox*>(window);
			PlayerObject*		pupilObject		= skillSelectBox->getPupil();

			if(action == 1)
			{
				pupilObject->getTrade()->setTeacher(NULL);
				return;
			}

			// WE are the teacher at this point deciding what to teach
			BStringVector*		dataItems	= skillSelectBox->getDataItems();
			BStringVector		splitSkill;
			BStringVector		splitProf;
			string				skillString = dataItems->at(element);

			if(!skillString.getLength() || skillString.split(splitSkill,':') < 2)
			{
				return;
			}

			Skill* skill = gSkillManager->getSkillByName(splitSkill[1]);

			if(!skill)
			{
				gLogger->log(LogManager::DEBUG,"PlayerObject: teach skill : skill list surprisingly empty\n");
				return;
			}

			// test whether its a profession
			// in that case redo the box with the professions skills shown and any other profession reduced
			int8 caption[128],text[128];

			if(splitSkill[1].split(splitProf,'_') == 2)
			{
				// its a profession. Build the list new with the professions skills shown
				gSkillManager->teach(pupilObject,this,splitSkill[1]);

				return;
			}

			// its a skill - offer to teach it
			sprintf(text,"@skl_n:%s",skill->mName.getAnsi());
			sprintf(caption,"%s offers to teach you : %s",mFirstName.getAnsi(),text);

			gUIManager->createNewSkillTeachMessageBox(this,"askteach",caption,text,pupilObject,SUI_MB_OKCANCEL,skillSelectBox->getPupil(),skill);
		}
		break;

		// teaching, accept / decline box
		case SUI_Window_Teach_OfferSkill_MsgBox:
		{
			UIOfferTeachBox* teachBox = dynamic_cast<UIOfferTeachBox*>(window);

			if(action == 1)
			{
				teachBox->getPupil()->getTrade()->setTeacher(NULL);
				gUIManager->createNewMessageBox(NULL,"askteach","","@teaching:offer_refused",this);
				return;
			}

			teachBox->getPupil()->getTrade()->setTeacher(NULL);

			//send mission accomplished to teacher
			string convName = teachBox->getPupil()->getFirstName().getAnsi();
			convName.convert(BSTRType_Unicode16);

      gMessageLib->sendSystemMessage(this,L"","teaching","teacher_skill_learned","skl_n",teachBox->getSkill()->mName.getAnsi(),L"",0,"","",convName.getUnicode16());

			//add skill to our pupils repertoir and send mission accomplished to our pupil
			gSkillManager->learnSkill(teachBox->getSkill()->mId,teachBox->getPupil(),true);
		}
		break;

		// dance select list box
		case SUI_Window_SelectDance_Listbox:
		{
			if(action == 1)
				return;

			UIListBox*			danceSelectBox	= dynamic_cast<UIListBox*>(window);
			BStringVector*		dataItems		= danceSelectBox->getDataItems();
			BStringVector		splitDance;

			if(element > -1)
			{
				string	danceString = dataItems->at(element);

				if(!danceString.getLength() || danceString.split(splitDance,'+') < 2)
				{
					return;
				}

				string mDance = splitDance[1];

				// if we are already dancing, only change the dance, otherwise start entertaining
				if(mPendingPerform == PlayerPerformance_None)
				{
					mPendingPerform = PlayerPerformance_Dance;
					gEntertainerManager->startDancePerformance(this,mDance);
				}
				else
				{
					gEntertainerManager->changeDance(this,mDance);
				}
			}
		}
		break;

		// outcast select listbox
		case SUI_Window_SelectOutcast_Listbox:
		{
			UIListBox*		outcastSelectBox	= dynamic_cast<UIListBox*>(window);
			BStringVector*	dataItems			= outcastSelectBox->getDataItems();

			if(element > -1)
			{
				string outcastString = dataItems->at(element);

				if(!outcastString.getLength())
				{
					return;
				}

				gEntertainerManager->verifyOutcastName(this,outcastString);
			}
		}
		break;

		// music select list box
		case SUI_Window_SelectMusic_Listbox:
		{
			if(action == 1)
				return;

			UIListBox*		musicSelectBox	= dynamic_cast<UIListBox*>(window);
			BStringVector*	dataItems		= musicSelectBox->getDataItems();
			BStringVector	splitDance;

			if(element > -1)
			{
				string	danceString = dataItems->at(element);

				if(!danceString.getLength() || danceString.split(splitDance,'+') < 2)
				{
					return;
				}

				string mDance = splitDance[1];

				// if we are already making music, only change the piece, otherwise start entertaining
				if(mPendingPerform == PlayerPerformance_None)
				{
					mPendingPerform = PlayerPerformance_Music;
					gEntertainerManager->startMusicPerformance(this,mDance);
				}
				else
				{
					gEntertainerManager->changeMusic(this,mDance);
				}
			}
		}
		break;

		// group loot mode listbox
		case SUI_Window_SelectGroupLootMode_Listbox:
		{
			// only 4 loot modes allowed
			if (element < 0 || element > 3) { break; }

			// advise the chat server
			gMessageLib->sendGroupLootModeResponse(this,element);
		}
		break;

		// group loot master listbox
		case SUI_Window_SelectGroupLootMaster_Listbox:
		{
			if (element < 0) { break; }

			UIPlayerSelectBox* selectionBox = dynamic_cast<UIPlayerSelectBox*>(window);
			//PlayerObject* selectedPlayer = selectionBox->getPlayers()[];

			//I realise this is not the FASTEST way to do this, but PlayerList is used as a list EVERYWHERE else.
			PlayerList::iterator it = selectionBox->getPlayers().begin();
			std::advance(it, element-1);

			PlayerObject* selectedPlayer = *it;

			if(selectedPlayer == NULL)
			{
				gLogger->log(LogManager::DEBUG,"SUI_Window_SelectGroupLootMaster_Listbox: Invalid player selection");
				break;
			}

			gMessageLib->sendGroupLootMasterResponse(selectedPlayer,this);
		}
		break;

		default:
		{
			gLogger->log(LogManager::DEBUG,"handleUIEvent:Default: %u, %u, %s,", action, element, inputStr.getAnsi());
		}
		break;
	}
}

//=============================================================================
//
// checks if a player is in the duellist
//
bool PlayerObject::checkDuelList(PlayerObject* player)
{
	PlayerList::iterator it = mDuelList.begin();

	while(it != mDuelList.end())
	{
		if((*it) == player)
		{
			return(true);
		}

		++it;
	}

	return(false);
}

//=============================================================================
//
// remove a player from the duellist
//
void PlayerObject::removeFromDuelList(PlayerObject* player)
{
	PlayerList::iterator it = mDuelList.begin();

	while(it != mDuelList.end())
	{
		if((*it) == player)
		{
			mDuelList.erase(it);
			return;
		}

		++it;
	}
}

//=============================================================================
//
// remove a player from the duellist
//
//PlayerList::iterator PlayerObject::removeFromDuelList(PlayerList::iterator& it)
//{
//	return mDuelList.erase(it);		
//}

void PlayerObject::addToDuelList(PlayerObject* player)
{ 
	if(this->getId()!= player->getId())
		mDuelList.push_back(player); 
	else
		gLogger->log(LogManager::DEBUG,"PlayerObject::addToDuelList: %I64u wanted to add himself to his/her duel list", player->getId());
}
//=============================================================================
//
// sets and returns the nearest crafting station
//
CraftingStation* PlayerObject::getCraftingStation(ObjectSet*	inRangeObjects, ItemType toolType)
{
	ObjectSet::iterator it = inRangeObjects->begin();

	mNearestCraftingStation = 0;

	while(it != inRangeObjects->end())
	{
		if(CraftingStation*	station = dynamic_cast<CraftingStation*>(*it))
		{
			uint32 stationType = station->getItemType();

			// check whether the station fits to our tool
			switch(toolType)
			{
				case ItemType_ClothingTool:
				{
					if(stationType == ItemType_ClothingStation || stationType == ItemType_ClothingStationPublic)
					{
						if (glm::distance(this->getWorldPosition(), station->getWorldPosition()) <= 25)
						{
							mNearestCraftingStation = station->getId();
							return(station);
						}
					}
				}
				break;

				case ItemType_WeaponTool:
				{
					if(stationType == ItemType_WeaponStation || stationType == ItemType_WeaponStationPublic)
					{
						if (glm::distance(this->getWorldPosition(), station->getWorldPosition()) <= 25)
						{
							mNearestCraftingStation = station->getId();
							return(station);
						}
					}
				}
				break;

				case ItemType_FoodTool:
				{
					if(stationType == ItemType_FoodStation || stationType == ItemType_FoodStationPublic)
					{
						if (glm::distance(this->getWorldPosition(), station->getWorldPosition()) <= 25)
						{
							mNearestCraftingStation = station->getId();
							return(station);
						}
					}
				}
				break;

				case ItemType_StructureTool:
				{
					if(stationType == ItemType_StructureStation || stationType == ItemType_StructureStationPublic)
					{
						if (glm::distance(this->getWorldPosition(), station->getWorldPosition()) <= 25)
						{
							mNearestCraftingStation = station->getId();
							return(station);
						}
					}
				}
				break;

				case ItemType_SpaceTool:
				{
					if(stationType == ItemType_SpaceStation || stationType == ItemType_SpaceStationPublic)
					{
						if (glm::distance(this->getWorldPosition(), station->getWorldPosition()) <= 25)
						{
							mNearestCraftingStation = station->getId();
							return(station);
						}
					}
				}
				break;

				case ItemType_GenericTool:
				case ItemType_JediTool:
				default:
				{
					return(NULL);
				}
				break;
			}
		}

		++it;
	}

	return(NULL);
}

//=============================================================================

void PlayerObject::clone(uint64 parentId, const glm::quat& dir, const glm::vec3& pos, bool preDesignatedFacility)
{
	// Remove revive timer, if any.
	gWorldManager->removePlayerObjectForTimedCloning(mId);

	

	//reset buffs
	BuffList::iterator it = this->GetBuffList()->begin();
	while(it != this->GetBuffList()->end())
	{
		RemoveBuff((*it));
		it++;
	}
	this->GetBuffList()->clear();
	this->CleanUpBuffs();
	this->getHam()->resetModifiers();
	//TODO reset skillmodifiers
	gMessageLib->sendCurrentHitpointDeltasCreo6_Full(this);

	// Handle free deaths for newbies.
	if (mNewPlayerExemptions > 0)
	{
		// No insurance loss, no decay.
		--mNewPlayerExemptions;
		mNewPlayerMessage = true;		//We will send a message when we re-spawn.
	}
	else
	{
		if(!preDesignatedFacility)
		{
			//this seems ot be handled already somewhere
			//uddate wounds
			//getHam()->updateAllWounds(100);
			//getHam()->updateBattleFatigue(100,true);
		}

		// Update / remove insurance of items in inventory (or equppied).
		if (Inventory* inventory = dynamic_cast<Inventory*>(mEquipManager.getEquippedObject(CreatureEquipSlot_Inventory)))
		{
			SortedInventoryItemList insuranceList;
			inventory->getInsuredItems(&insuranceList);

			SortedInventoryItemList::iterator it;
			it = insuranceList.begin();

			while (it != insuranceList.end())
			{
				if (Object* object = gWorldManager->getObjectById((*it).second))
				{
					if (TangibleObject* tangibleObject = dynamic_cast<TangibleObject*>(object))
					{
						// Remove insurance.
						tangibleObject->setInternalAttribute("insured","0");
						gWorldManager->getDatabase()->ExecuteSqlAsync(NULL,NULL,"UPDATE item_attributes SET value=0 WHERE item_id=%"PRIu64" AND attribute_id=%u",tangibleObject->getId(), 1270);

						tangibleObject->setTypeOptions(tangibleObject->getTypeOptions() & ~((uint32)4));

						// Update insurance status.
						(void)gMessageLib->sendUpdateTypeOption(tangibleObject, this);
					}
				}

				++it;
			}
		}
	}

	// Update defenders, if any,  NOW when I'm gone...
	/*
	ObjectIDList::iterator defenderIt = mDefenders.begin();
	while (defenderIt != mDefenders.end())
	{
		if (CreatureObject* defenderCreature = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById((*defenderIt))))
		{
			defenderCreature->removeDefenderAndUpdateList(this->getId());

			if (PlayerObject* defenderPlayer = dynamic_cast<PlayerObject*>(defenderCreature))
			{
				gMessageLib->sendUpdatePvpStatus(this,defenderPlayer);
				gMessageLib->sendDefenderUpdate(defenderPlayer,0,0,this->getId());
			}

			// if no more defenders, clear combat state
			if (!defenderCreature->getDefenders()->size())
			{
				defenderCreature->toggleStateOff((CreatureState)(CreatureState_Combat + CreatureState_CombatAttitudeNormal));
				gMessageLib->sendStateUpdate(defenderCreature);
			}
		}
		// If we remove self from all defenders, then we should remove all defenders from self. Remember, we are dead.
		defenderIt = mDefenders.erase(defenderIt);
	}
	*/
	gWorldManager->warpPlanet(this,pos,parentId,dir);
}

//=============================================================================

void PlayerObject::setPreDesignatedCloningFacilityId(uint64 cloningId)
{
	mPreDesignatedCloningFacilityId = cloningId;
}

//=============================================================================

uint64 PlayerObject::getPreDesignatedCloningFacilityId(void)
{
	return mPreDesignatedCloningFacilityId;
}

//=============================================================================

uint8 PlayerObject::getNewPlayerExemptions(void)
{
	return this->mNewPlayerExemptions;
}

//=============================================================================

void PlayerObject::setNewPlayerExemptions(uint8 npe, bool displayMessage)
{
	this->mNewPlayerExemptions = npe;
	this->mNewPlayerMessage = displayMessage;
}

//=============================================================================

void PlayerObject::newPlayerMessage(void)
{
	if (mNewPlayerMessage)
	{
		if (mNewPlayerExemptions >= 2)
		{
			// New Player Auto-Insure Activated. You have %DI deaths before item insurance is required.
			gMessageLib->sendSystemMessage(this, L"", "base_player", "prose_newbie_insured", "", "", L"", mNewPlayerExemptions);
		}
		else if (mNewPlayerExemptions == 1)
		{
			// New Player Auto-Insure Activated. You have one death before item insurance is required.
			gMessageLib->sendSystemMessage(this, L"", "base_player", "last_newbie_insure");
		}
		else if (mNewPlayerExemptions == 0)
		{
			// New player exemption status has expired. You will now be required to insure your items if you wish to retain them after cloning.
			gMessageLib->sendSystemMessage(this, L"", "base_player", "newbie_expired");
		}

		mNewPlayerMessage = false;
	}
}
//=============================================================================

void PlayerObject::saveNearestCloningFacility(BuildingObject* nearestCloningFacility)
{
	mNearestCloningFacility = nearestCloningFacility;
}

//=============================================================================

void PlayerObject::cloneAtNearestCloningFacility(void)
{
	if (mNearestCloningFacility)
	{
		if (SpawnPoint* sp = mNearestCloningFacility->getRandomSpawnPoint())
		{
			if (mNewPlayerExemptions == 0)
			{
				// Add wounds...
				mHam.updatePrimaryWounds(100);

				// .. and some BF
				mHam.updateBattleFatigue(100, true);
			}

			// Clone
			clone(sp->mCellId,sp->mDirection,sp->mPosition);
		}
	}
}

//=============================================================================

EMLocationType PlayerObject::getPlayerLocation()
{
	// make sure we are eiter in a camp or in a playerstructure
	if(CellObject* cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(this->getParentId())))
	{
		if(Object* object = dynamic_cast<Object*>(gWorldManager->getObjectById(cell->getParentId())))
		{
			if(strstr(object->getModelString().getAnsi(),"cantina"))
			{
				return EMLocation_Cantina;
			}

			// not sure yet how to find playerstructures - leave this for when we have them implemented
		}
	}
	else
	{
		// we are not in a cell but maybe in a camp
		if(!gStructureManager->checkinCamp(this))
		{
			return EMLocation_Camp;
		}
	}

	return EMLocation_NULL;
}
Object* PlayerObject::getHealingTarget(PlayerObject* Player) const
{
	PlayerObject* PlayerTarget = dynamic_cast<PlayerObject*>(Player->getTarget());

	if (PlayerTarget && PlayerTarget->getId() != Player->getId())
	{
		//check duel
		if (Player->checkDuelList(PlayerTarget))
			return Player;
		//check pvp status
		if(Player->getPvPStatus() != PlayerTarget->getPvPStatus())
		{
			//send pvp_no_help
			gLogger->log(LogManager::DEBUG,"PVP Flag not right");
			gMessageLib->sendSystemMessage(Player,L"","healing","pvp_no_help");
			//return Player as the healing target
			return Player;
		}
		return PlayerTarget;
	}
	return Player;
}
//=============================================================================

void PlayerObject::setCombatTargetId(uint64 targetId)
{
	mCombatTargetId = targetId;
}

//=============================================================================

uint64 PlayerObject::getCombatTargetId(void)
{
	return mCombatTargetId;
}

//=============================================================================

void PlayerObject::enableAutoAttack(void)
{
	mAutoAttack = true;
}

//=============================================================================

void PlayerObject::disableAutoAttack(void)
{
	mAutoAttack = false;
}

//=============================================================================

bool PlayerObject::autoAttackEnabled(void)
{
	return mAutoAttack;
}

//=============================================================================
//
// we check whether we have enough Lots and update them if so including the db
//
bool PlayerObject::useLots(uint8 usedLots)
{
	int32 lots = mLots - usedLots;

	if(lots < 0)
	{
		return false;
	}

	mLots -= usedLots;

	return true;
}

//=============================================================================
//
// we check whether the amount of lots regained is plausibel
// and update the db
//
bool PlayerObject::regainLots(uint8 lots)
{
	uint32 maxLots = gWorldConfig->getConfiguration<uint32>("Player_Max_Lots",(uint32)10);

	if((mLots + lots) > maxLots)
	{
		return false;
	}

	mLots += lots;

	return true;
}

//=============================================================================
//
// assign the item a new parent id
//
void PlayerObject::setParentIdIncDB(uint64 parentId)
{ 
	mParentId = parentId; 

	gWorldManager->getDatabase()->ExecuteSqlAsync(0,0,"UPDATE characters SET parent_id=%"PRIu64" WHERE id=%"PRIu64"",mParentId,this->getId());
}

//=============================================================================
//
// Posture Commands
//

void PlayerObject::setSitting(Message* message)
{
	//uint8			currentPosture	= this->getPosture();
	string			data;
    glm::vec3       chair_position;
	uint64			chairCell		= 0;
	uint32			elementCount	= 0;

	
	if(this->checkPlayerCustomFlag(PlayerCustomFlag_LogOut))
	{
		this->togglePlayerFlagOff(PlayerCustomFlag_LogOut);	
		gMessageLib->sendSystemMessage(this,L"","logout","aborted");
	}

	if(this->isConnected())
		gMessageLib->sendHeartBeat(this->getClient());

	// see if we need to get out of sampling mode
	if(this->getSamplingState())
	{
		gMessageLib->sendSystemMessage(this,L"","survey","sample_cancel");
		this->setSamplingState(false);
	}

	message->getStringUnicode16(data); //Should be okay even if data is null! (I hope)

	this->setPosture(CreaturePosture_Sitting);
	this->getHam()->updateRegenRates();

	// sitting on chair
	if(data.getLength())
	{
		elementCount = swscanf(data.getUnicode16(), L"%f,%f,%f,%"WidePRIu64, &chair_position.x, &chair_position.y, &chair_position.z, &chairCell);

		if(elementCount == 4)
		{
			// outside
			if(!chairCell)
			{
				if(QTRegion* newRegion = gWorldManager->getSI()->getQTRegion(chair_position.x, chair_position.z))
				{
					// we didnt change so update the old one
					if((uint32)newRegion->getId() == this->getSubZoneId())
					{
						// this also updates the players position
						newRegion->mTree->updateObject(this, chair_position);
					}
					else
					{
						// remove from old
						if(QTRegion* oldRegion = gWorldManager->getQTRegion(this->getSubZoneId()))
						{
							oldRegion->mTree->removeObject(this);
						}

						// update players position
						this->mPosition = chair_position;

						// put into new
						this->setSubZoneId((uint32)newRegion->getId());
						newRegion->mTree->addObject(this);
					}
				}
				else
				{
					// we should never get here !
					gLogger->log(LogManager::DEBUG,"SitOnObject: could not find zone region in map");

					// hammertime !
					exit(-1);
				}
			}
			// we are in a cell
			else
			{
				// switch cells, if needed
				if(this->getParentId() != chairCell)
				{
					CellObject* cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(this->getParentId()));

					if(cell)
						cell->removeObject(this);
					else
						gLogger->log(LogManager::DEBUG,"Error removing %"PRIu64" from cell %"PRIu64"",this->getId(),this->getParentId());

					this->setParentId(chairCell);

					cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(chairCell));

					if(cell)
						cell->addObjectSecure(this);
					else
						gLogger->log(LogManager::DEBUG,"Error adding %"PRIu64" to cell %"PRIu64"",this->getId(),chairCell);
				}

				this->mPosition = chair_position;
			}

			//this->mDirection = Anh_Math::Quaternion();
			this->toggleStateOn(CreatureState_SittingOnChair);

			this->updateMovementProperties();

			// TODO: check if we need to send transforms to others
			if(chairCell)
			{
				gMessageLib->sendDataTransformWithParent053(this);
			}
			else
			{
				gMessageLib->sendDataTransform053(this);
			}

			gMessageLib->sendUpdateMovementProperties(this);
			gMessageLib->sendPostureAndStateUpdate(this);

			gMessageLib->sendSitOnObject(this);
		}
	}
	// sitting on ground
	else
	{
		gMessageLib->sendPostureUpdate(this);
		gMessageLib->sendSelfPostureUpdate(this);
	}

	//hack-fix clientside bug by manually sending client message
	gMessageLib->sendSystemMessage(this,L"","shared","player_sit");
}

void PlayerObject::setUpright()
{
	if(this->isConnected())
		gMessageLib->sendHeartBeat(this->getClient());

	// see if we need to get out of sampling mode
	if(this->getSamplingState())
	{
		gMessageLib->sendSystemMessage(this,L"","survey","sample_cancel");
		this->setSamplingState(false);
	}

	if(this->checkPlayerCustomFlag(PlayerCustomFlag_LogOut))
	{
		this->togglePlayerCustomFlagOff(PlayerCustomFlag_LogOut);	
		gMessageLib->sendSystemMessage(this,L"","logout","aborted");
	}

	this->toggleStateOff(CreatureState_SittingOnChair);

	this->setPosture(CreaturePosture_Upright);
	this->getHam()->updateRegenRates();
	this->updateMovementProperties();

	gMessageLib->sendUpdateMovementProperties(this);
	gMessageLib->sendPostureAndStateUpdate(this);
	gMessageLib->sendSelfPostureUpdate(this);

	//if player is seated on an a chair, hack-fix clientside bug by manually sending client message
	bool IsSeatedOnChair = this->checkState(CreatureState_SittingOnChair);
	if(IsSeatedOnChair)
	{
		gMessageLib->sendSystemMessage(this,L"","shared","player_stand");
	}
}

void PlayerObject::setProne()
{
	if(this->isConnected())
		gMessageLib->sendHeartBeat(this->getClient());

	// see if we need to get out of sampling mode
	if(this->getSamplingState())
	{
		gMessageLib->sendSystemMessage(this,L"","survey","sample_cancel");
		this->setSamplingState(false);
	}

	if(this->checkPlayerCustomFlag(PlayerCustomFlag_LogOut))
	{
		this->togglePlayerCustomFlagOff(PlayerCustomFlag_LogOut);	
		gMessageLib->sendSystemMessage(this,L"","logout","aborted");
	}

	this->toggleStateOff(CreatureState_SittingOnChair);
	

	// Can not compare bitwise data with equality... the test below will only work if ALL other states = 0.
	

	this->setPosture(CreaturePosture_Prone);
	this->getHam()->updateRegenRates();
	this->updateMovementProperties();

	gMessageLib->sendUpdateMovementProperties(this);
	gMessageLib->sendPostureAndStateUpdate(this);
	gMessageLib->sendSelfPostureUpdate(this);

	//if player is seated on an a chair, hack-fix clientside bug by manually sending client message
	bool IsSeatedOnChair = this->checkState(CreatureState_SittingOnChair);
	if(IsSeatedOnChair)
	{
		gMessageLib->sendSystemMessage(this,L"","shared","player_prone");
	}
}

void PlayerObject::setCrouched()
{
	if(this->isConnected())
		gMessageLib->sendHeartBeat(this->getClient());

	//Get whether player is seated on a chair before we toggle it
	// Can not compare bitwise data with equality... the test below will only work if ALL other states = 0.
	// bool IsSeatedOnChair = (playerObject->getState() == CreatureState_SittingOnChair);
	bool IsSeatedOnChair = this->checkState(CreatureState_SittingOnChair);

	//make sure we end states
	//the logoff states is an invention of mine btw 
	
	this->toggleStateOff(CreatureState_SittingOnChair);

	this->setPosture(CreaturePosture_Crouched);
	this->getHam()->updateRegenRates();
	this->updateMovementProperties();

	gMessageLib->sendUpdateMovementProperties(this);
	gMessageLib->sendPostureAndStateUpdate(this);
	gMessageLib->sendSelfPostureUpdate(this);

	//if player is seated on an a chair, hack-fix clientside bug by manually sending client message
	if(IsSeatedOnChair)
	{
		gMessageLib->sendSystemMessage(this,L"","shared","player_kneel");
	}
}
