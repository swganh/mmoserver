/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "MathLib/Quaternion.h"
#include "PlayerObject.h"
#include "SchematicManager.h"
#include "MessageLib/MessageLib.h"
#include "LogManager/LogManager.h"
#include "DatabaseManager/Database.h"
#include "WorldManager.h"
#include "CharSheetManager.h"
#include "UIManager.h"
#include "UISkillSelectBox.h"
#include "UIOfferTeachBox.h"
#include "UIPlayerSelectBox.h"
#include "UICloneSelectListBox.h"
#include "Common/atMacroString.h"
#include "SampleEvent.h"
#include "BuildingObject.h"
#include "Utils/EventHandler.h"
#include "WorldConfig.h"
#include "Buff.h"
#include "ScriptEngine/ScriptEngine.h"
#include "Tutorial.h"
#include "Structuremanager.h"


//=============================================================================

PlayerObject::PlayerObject() : CreatureObject(),
mAccountId(0),
mClient(NULL),
mTravelPoint(NULL),
mBazaarPoint(NULL),
mPlayerObjId(0),
mClientTickCount(0),
mDConnTime(60),
mMotdReceived(false),
mPendingSample(false),
mPendingSurvey(false),
mPlayerFlags(0),
mBindPlanet(-1),
mHomePlanet(-1),
mLots(10),
mTrading(false),
mTradePartner(NULL),
mFriendsListUpdateCounter(0),
mIgnoresListUpdateCounter(0),
mContactListUpdatePending(false),
mCraftingStage(0),
mExperimentationFlag(0),
mCraftingSession(NULL),
mExperimentationPoints(0),
mNearestCraftingStation(0),
mEntertainerWatchToId(0),
mEntertainerTaskId(0),
mEntertainerPauseId(0),
mNextSampleTime(0),
mPlacedInstrument(0),
mSelectedInstrument(0),
mHoloEmote(0),
mHoloCharge(0),
mSampleEventFlag(false),//rav
mPassRadioactive(false),//rav
mSampleGambleFlag(false),//rav
mSampleNodeFlag(false),//rav
mMissionIdMask(0),
mTutorial(NULL),
mNewPlayerExemptions(0),
mNewPlayerMessage(false),
mNearestCloningFacility(NULL),
mPreDesignatedCloningFacilityId(0),
mMounted(false),
mMountCalled(false),
mMount(NULL),
mLastGroupMissionUpdateTime(0)
{
	mDuelList.reserve(10);

	mType				= ObjType_Player;
	mCreoGroup			= CreoGroup_Player;
	mStomach			= new Stomach();
	// mMarriage			= L"Your Spouce";	// When testing
	mMarriage			= L"";					// Unmarried
	mTrade				= new Trade(this);

	// register event functions
	registerEventFunction(this,&PlayerObject::onSurvey);
	registerEventFunction(this,&PlayerObject::onSample);
}

//=============================================================================

PlayerObject::~PlayerObject()
{
	clearAllUIWindows();

	stopTutorial();
	// delete mTutorial;	// It's safe to delete a NULL-object.

	// delete missionBag
	Object* missionBag = mEquipManager.getEquippedObject(CreatureEquipSlot_MissionBag);
	mEquipManager.removeEquippedObject(CreatureEquipSlot_MissionBag);
	delete(missionBag);

	// delete datapad
	Object* datapad = mEquipManager.getEquippedObject(CreatureEquipSlot_Datapad);
	mEquipManager.removeEquippedObject(CreatureEquipSlot_Datapad);
	delete(datapad);

	// delete bank
	Object* bank = mEquipManager.getEquippedObject(CreatureEquipSlot_Bank);
	mEquipManager.removeEquippedObject(CreatureEquipSlot_Bank);
	delete(bank);

	delete(mStomach);
	delete(mTrade);
}

//=============================================================================

void PlayerObject::stopTutorial()
{
	if (gWorldConfig->isTutorial())
	{
		if (mTutorial)
		{
			// Save-update the state.
			// (gWorldManager->getDatabase())->ExecuteSqlAsync(0,0,"UPDATE character_tutorial SET character_state=%u,character_substate=%u WHERE character_id=%lld",mTutorial->getState(), mTutorial->getSubState(),mId);

			delete mTutorial;
			mTutorial = NULL;
		}
	}
}

//=============================================================================

void PlayerObject::startTutorial()
{
	if (gWorldConfig->isTutorial())
	{
		if (!mTutorial)
		{
			// Create a Tutorial
			mTutorial = new Tutorial(this);
		}
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
	mPendingSurvey						= false;
	mPendingSample						= false;
	mDefenderUpdateCounter				= 0;
	mReady								= false;
	mIncapCount							= 0;
	mFirstIncapTime						= 0;
	mCurrentIncapTime					= 0;
	mState								= 0;

	if(Datapad* datapad = dynamic_cast<Datapad*>(mEquipManager.getEquippedObject(CreatureEquipSlot_Datapad)))
	{
		datapad->mWaypointUpdateCounter = datapad->getWaypoints()->size();
	}

	updateMovementProperties();

	mHam.resetCounters();
	mHam.updateRegenRates();

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
}

//=============================================================================

void PlayerObject::prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount)
{
	PlayerObject* playerObject = dynamic_cast<PlayerObject*>(creatureObject);
	// gLogger->logMsgF("PlayerObject::prepareCustomRadialMenu",MSG_NORMAL);
	RadialMenu* radial	= new RadialMenu();

	uint8 radId = itemCount;

	// #1 entertainer - are we performing???
	if(getPerformingState() == PlayerPerformance_Dance)
	{
		// check that we are not currently watching
		if(playerObject->getEntertainerWatchToId()== this->getId())
		{
			radial->addItem(radId,0,radId_serverPerformanceWatchStop,radAction_ObjCallback,"Stop Watching");

			++radId;
		}
		else
		{
			// are we in range????
			if(playerObject->mPosition.inRange2D(this->mPosition,20))
			{
				radial->addItem(radId,0,radId_serverPerformanceWatch,radAction_ObjCallback,"Watch");

				++radId;
			}
		}
	}
	else if(getPerformingState() == PlayerPerformance_Music)
	{
		// check that we are not currently watching
		if(playerObject->getEntertainerListenToId()== this->getId())
		{
			radial->addItem(radId,0,radId_serverPerformanceListenStop,radAction_ObjCallback,"Stop Listening");

			++radId;
		}
		else
		{
			// are we in range????
			if(playerObject->mPosition.inRange2D(this->mPosition,20))
			{
				radial->addItem(radId,0,radId_serverPerformanceListen,radAction_ObjCallback,"Listen");
				++radId;
			}
		}
	}

	// teach
	if((playerObject->getGroupId())&&(this->getGroupId() == playerObject->getGroupId()))
	{
		radial->addItem(radId,0,radId_serverTeach,radAction_ObjCallback,"Teach");

		++radId;
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
	return(amount <= dynamic_cast<Bank*>(mEquipManager.getEquippedObject(CreatureEquipSlot_Bank))->getCredits() + dynamic_cast<Inventory*>(mEquipManager.getEquippedObject(CreatureEquipSlot_Inventory))->getCredits());
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
				// ok always do bank first
				if(amount > bank->getCredits())
				{
					//ok more than on the bank first empty bank, then inv.
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

// Here is where we shall add restrictions for JTL, Jedi, Pre-Pub 14
// or other restrictions for type of XP to allow.

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
	BadgesList::iterator it = mBadges.begin();

	while(it != mBadges.end())
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
		mBadges.push_back(badgeId);
		Badge* badge = gCharSheetManager->getBadgeById(badgeId);

		gMessageLib->sendPlayMusicMessage(badge->getSoundId(),this);
		gMessageLib->sendSystemMessage(this,L"","badge_n","prose_grant","badge_n",badge->getName(),L"");

		(gWorldManager->getDatabase())->ExecuteSqlAsync(0,0,"INSERT INTO character_badges VALUES (%lld,%u)",mId,badgeId);

		gLogger->logMsgF("Badge %u granted to %lld\n",MSG_NORMAL,badgeId,mId);

		_verifyBadges();

		if(badge->getCategory() >= 3 && badge->getCategory() <= 5)
		{
			_verifyExplorationBadges();
		}
	}
	else
	{
		// This is an unexpected condition.
		gLogger->logMsgF("Badge %u already exists for player with id %lld\n",MSG_NORMAL,badgeId,mId);
	}
}

//=============================================================================

void PlayerObject::_verifyExplorationBadges()
{
	uint16					count	= 0;
	BadgesList::iterator	it		= mBadges.begin();

	while(it != mBadges.end())
	{
		Badge* badge = gCharSheetManager->getBadgeById((*it));

		if(badge->getCategory() >= 3 && badge->getCategory() <= 5)
		{
			count++;
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
	uint32 count = mBadges.size();

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
	PlayerList					members;

	if(self)
	{
		members.push_back((PlayerObject *)this);
	}

	if(mGroupId == 0)
	{
		return members;
	}

	while(it != mKnownPlayers.end())
	{
		if((*it)->getType() == ObjType_Player)
		{
			PlayerObject* player = dynamic_cast<PlayerObject*>(*it);

			if(player->getGroupId() == mGroupId)
			{
				members.push_back(player);
			}
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

			//check if our pupil already gets taught
			if (!this->getTrade()->getTeacher())
			{
				this->getTrade()->setTeacher(callingObject);
				gSkillManager->teach(this,callingObject,"");
			}
			else
			{
				gMessageLib->sendSystemMessage(callingObject,L"","teaching","student_has_offer_to_learn","","",L"",0,"","",L"",this->getId());
			}
		}
		break;

		case radId_tradeStart: // deposit all
		break;

		case radId_serverPerformanceWatch:
		{
			// start watching
			gEntertainerManager->startWatching(callingObject,this);
		}
		break;

		case radId_serverPerformanceWatchStop: 
		{
			// stop watching
			gEntertainerManager->stopWatching(callingObject);
		}
		break;

		case radId_serverPerformanceListen:
		{
			// start listening
			gEntertainerManager->startListening(callingObject,this);
		}
		break;

		case radId_serverPerformanceListenStop:
		{
			// stop listening
			gEntertainerManager->stopListening(callingObject);
		}
		break;

	default:
		{
			gLogger->logMsgF("PlayerObject: Unhandled MenuSelect: %u\n",MSG_HIGH,messageType);
		}
		break;
	}
}

//=============================================================================

void PlayerObject::giveBankCredits(uint32 amount)
{
	Bank* bank = dynamic_cast<Bank*>(mEquipManager.getEquippedObject(CreatureEquipSlot_Bank));

	bank->setCredits(bank->getCredits() + amount);

	gMessageLib->sendBankCreditsUpdate(this);
}

//=============================================================================

void PlayerObject::giveInventoryCredits(uint32 amount)
{
	Inventory* inventory = dynamic_cast<Inventory*>(mEquipManager.getEquippedObject(CreatureEquipSlot_Inventory));

	inventory->setCredits(inventory->getCredits() + amount);

	gMessageLib->sendInventoryCreditsUpdate(this);
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
			// gLogger->logMsgF("PlayerObject::handleUIEvent element = %d",MSG_NORMAL,element);

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
							this->getController()->cloneAtPreDesignatedFacility(this, sp);
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
										this->getController()->cloneAtPreDesignatedFacility(this, sp);
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

			if(!skillString.getLength())
				return;

			if(skillString.split(splitSkill,':') < 2)
				return;

			Skill* skill = gSkillManager->getSkillByName(splitSkill[1]);

			if(!skill)
			{
				gLogger->logMsg("PlayerObject: teach skill : skill list surprisingly empty\n");
				return;
			}

			//test whether its a profession
			//in that case redo the box with the professions skills shown and any other profession reduced
			int8 caption[128],text[128];

			if(splitSkill[1].split(splitProf,'_') == 2)
			{
				//its a profession. Build the list new with the professions skills shown;
				gSkillManager->teach(pupilObject,this,splitSkill[1]);

				return;
			}
			//its a skill - offer to teach it

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

			gMessageLib->sendSystemMessage(this,L"","teaching","teacher_skill_learned","skl_n",teachBox->getSkill()->mName,L"",0,"","",convName);

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

				if(!danceString.getLength())
					return;

				if(danceString.split(splitDance,'+') < 2)
					return;

				string mDance = splitDance[1];

				//if we are already dancing only change the dance, otherwise start entertaining
				if(this->getPerformingState() == PlayerPerformance_None)
				{
					this->setPerformingState(PlayerPerformance_Dance);
					gEntertainerManager->startDancePerformance(this,mDance);
				}
				else
				{
					gEntertainerManager->changeDance(this,mDance);
				}
			}
		}
		break;

		case SUI_Window_SelectOutcast_Listbox:
		{
			UIListBox*		outcastSelectBox	= dynamic_cast<UIListBox*>(window);
			BStringVector*	dataItems			= outcastSelectBox->getDataItems();

			if(element > -1)
			{
				string	outcastString = dataItems->at(element);

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

				if(!danceString.getLength())
					return;

				if(danceString.split(splitDance,'+') < 2)
					return;

				string mDance = splitDance[1];

				//if we are already making music only change the piece, otherwise start entertaining
				if(this->getPerformingState() == PlayerPerformance_None)
				{
					this->setPerformingState(PlayerPerformance_Music);
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

		case SUI_Window_SelectGroupLootMaster_Listbox:
		{
			if (element < 0) { break; }

			UIPlayerSelectBox* selectionBox = dynamic_cast<UIPlayerSelectBox*>(window);
			PlayerObject* selectedPlayer = selectionBox->getPlayers()[element];
			if(selectedPlayer == NULL)
			{
				gLogger->logMsg("SUI_Window_SelectGroupLootMaster_Listbox: Invalid player selection");
				break;
			}

			// advise the chat server
			// put in messagelib?
			gMessageFactory->StartMessage();
			Message* newMessage;
			gMessageFactory->addUint32(opIsmGroupLootMasterResponse);
			gMessageFactory->addUint32(selectedPlayer->getAccountId());
			newMessage = gMessageFactory->EndMessage();
			this->getClient()->SendChannelA(newMessage,this->getAccountId(),CR_Chat,2);	
		}
		break;

		// Sampling Radioactive Msg Box
		case SUI_Window_SmplRadioactive_MsgBox:
		{
			if(action == 1)
			{				
				gLogger->logMsg("sampling radioactive box: No\n");
				mPassRadioactive = false;
				mPendingSample = false;
				mPosture	   = CreaturePosture_Upright;
				gMessageLib->sendUpdateMovementProperties(this);
				gMessageLib->sendPostureAndStateUpdate(this);
				gMessageLib->sendSelfPostureUpdate(this);
				return;
			}else{		
				gLogger->logMsg("sampling radioactive box: Yes\n");
				gLogger->logMsg("Please hit 'get sample' to continue.\n");
				mPassRadioactive = true;
				mPendingSample = true;
				//TODO:invoke sample action
				if(mHam.checkMainPools(0,150,0))
				{
					//SurveyTool*			tool		= event->getTool();
					//CurrentResource*	resource	= event->getResource();
					mNextSampleTime = Anh_Utils::Clock::getSingleton()->getLocalTime() + 30000;
					//mObjectController.addEvent(new SampleEvent(tool,resource),10000);
				}
			}
		}
		break;

		case SUI_Window_SmplGamble_ListBox:
		{
			if(action == 1)
			{				
				gLogger->logMsg("sampling gamble box action=1 (continue?)\n");
				mPendingSample = true;
				mSampleGambleFlag = false;

				//TODO:invoke sample action
				if(mHam.checkMainPools(0,150,0))
				{
					//SurveyTool*			tool		= event->getTool();
					//CurrentResource*	resource	= event->getResource();
					mNextSampleTime = Anh_Utils::Clock::getSingleton()->getLocalTime() + 30000;
					//mObjectController.addEvent(new SampleEvent(tool,resource),10000);
				}
			}
			else
			{		
				gLogger->logMsg("sampling gamble box action != 1 (chance?)\n");
				//action costs				
				mHam.updatePropertyValue(HamBar_Action,HamProperty_CurrentHitpoints,300);
				mPendingSample = true;

				//determine whether gamble is good or not
				int32 gambleRoll = int(gRandom->getRand()%2) + 1;

				if(gambleRoll == 1)
				{
					mSampleEventFlag = true;
					mSampleGambleFlag = true;
				}
				else
				{
					mSampleGambleFlag = false;
				}

				//TODO:invoke sample action
				if(mHam.checkMainPools(0,150,0))
				{
					//SurveyTool*			tool		= event->getTool();
					//CurrentResource*	resource	= event->getResource();
					mNextSampleTime = Anh_Utils::Clock::getSingleton()->getLocalTime() + 30000;
					//mObjectController.addEvent(new SampleEvent(tool,resource),10000);
				}
			}
		}
		break;

		case SUI_Window_SmplWaypNode_ListBox:
		{
			if(action == 1)
			{				
				gLogger->logMsg("sampling wayp node box action=1 (continue?)\n");
				mPendingSample = true;
				//TODO:need to create wayp obj
				mSampleNodeFlag = true;
				return;
			}
			else
			{		
				gLogger->logMsg("sampling wayp node box action != 1 (stay here?)\n");
				mPendingSample = true;
				mSampleNodeFlag = false;
				//TODO:need to invoke sample action
				if(mHam.checkMainPools(0,150,0))
				{
					//SurveyTool*			tool		= event->getTool();
					//CurrentResource*	resource	= event->getResource();
					mNextSampleTime = Anh_Utils::Clock::getSingleton()->getLocalTime() + 30000;
					//mObjectController.addEvent(new SampleEvent(tool,resource),10000);
				}
			}
		}
		break;

	default:
		{
			gLogger->logMsgF("handleUIEvent:Default: %u, %u, %s,\n",MSG_NORMAL, action, element, inputStr.getAnsi());
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

CraftingStation* PlayerObject::getCraftingStation(ObjectSet	inRangeObjects)
{
	// iterate through the results
	ObjectSet::iterator it = inRangeObjects.begin();

	while(it != inRangeObjects.end())
	{
		if(CraftingStation*	station = dynamic_cast<CraftingStation*>(*it))
		{
			setNearestCraftingStation(station->getId());

			return(station);
		}

		++it;
	}

	setNearestCraftingStation(0);

	return(NULL);
}

//=============================================================================

void PlayerObject::clone(uint64 parentId,Anh_Math::Quaternion dir,Anh_Math::Vector3 pos)
{
	// Remove revive timer, if any.
	gWorldManager->removePlayerObjectForTimedCloning(this->getId());

	// TODO: decay, wounds

	// Handle free deaths for newbies.
	if (this->mNewPlayerExemptions > 0)
	{
		// No insurance loss, no decay.
		this->mNewPlayerExemptions--;
		mNewPlayerMessage = true;		//We will send a message when we re-spawn.
	}
	else
	{
		// Update / remove insurance of items in inventory (or equppied).
		if (Inventory* inventory = dynamic_cast<Inventory*>(mEquipManager.getEquippedObject(CreatureEquipSlot_Inventory)))
		{
			SortedInventoryItemList insuranceList;
			inventory->getInsuredItems(&insuranceList);

			SortedInventoryItemList::iterator it;
			it = insuranceList.begin();
			while (it != insuranceList.end())
			{
				Object* object = gWorldManager->getObjectById((*it).second);
				if (object)
				{
					TangibleObject* tangibleObject = dynamic_cast<TangibleObject*>(object);
					if (tangibleObject)
					{
						// Remove insurance.
						tangibleObject->setInternalAttribute("insured","0");
						gWorldManager->getDatabase()->ExecuteSqlAsync(NULL,NULL,"UPDATE item_attributes SET value=0 WHERE item_id=%llu AND attribute_id=%u",tangibleObject->getId(), 1270);

						tangibleObject->setTypeOptions(tangibleObject->getTypeOptions() & ~((uint32)4));

						// Update insurance status.
						(void)gMessageLib->sendUpdateTypeOption(tangibleObject, this);
					}
				}
				it++;
			}
		}
	}
	gWorldManager->warpPlanet(this,pos,parentId,dir);


}

//=============================================================================

void PlayerObject::setPreDesignatedCloningFacilityId(uint64 cloningId)
{
	this->mPreDesignatedCloningFacilityId = cloningId;
}


//=============================================================================
uint64 PlayerObject::getPreDesignatedCloningFacilityId(void)
{
	return this->mPreDesignatedCloningFacilityId;
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
	if (this->mNewPlayerMessage)
	{
		if (this->mNewPlayerExemptions >= 2)
		{
			// New Player Auto-Insure Activated. You have %DI deaths before item insurance is required.
			gMessageLib->sendSystemMessage(this, L"", "base_player", "prose_newbie_insured", "", "", L"", mNewPlayerExemptions);
		}
		else if (this->mNewPlayerExemptions == 1)
		{
			// New Player Auto-Insure Activated. You have one death before item insurance is required.
			gMessageLib->sendSystemMessage(this, L"", "base_player", "last_newbie_insure");
		}
		else if (this->mNewPlayerExemptions == 0)
		{
			// New player exemption status has expired. You will now be required to insure your items if you wish to retain them after cloning. 
			gMessageLib->sendSystemMessage(this, L"", "base_player", "newbie_expired");
		}
		this->mNewPlayerMessage = false;
	}
}
//=============================================================================
//
//
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



//=============================================================================
EMLocationType PlayerObject::getPlayerLocation()
{
	// make sure we are eiter in a camp or in a playerstructure
	CellObject* cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(this->getParentId()));
	if(cell)
	{
		Object* object = dynamic_cast<Object*>(gWorldManager->getObjectById(cell->getParentId()));
		if(object)
		{
			// string modelname = object->getModelString();
			if(strstr(object->getModelString().getAnsi(),"cantina"))
			{
				return EMLocation_Cantina;
			}

			//not sure yet how to find playerstructures - leave this for when we have them implemented
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
