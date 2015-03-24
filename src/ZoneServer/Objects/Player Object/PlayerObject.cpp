/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2014 The SWG:ANH Team
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

#include "ZoneServer/Objects/Player Object/PlayerObject.h"
#include "Zoneserver/Objects/Badge.h"
#include "Zoneserver/Objects/Bank.h"
#include "Zoneserver/GameSystemManagers/Buff Manager/Buff.h"
#include "ZoneServer/GameSystemManagers/Structure Manager/BuildingObject.h"
#include "ZoneServer/GameSystemManagers/Structure Manager/StructureManager.h"
#include "Zoneserver/GameSystemManagers/Buff Manager/BuffManager.h"
#include "ZoneServer/GameSystemManagers/Structure Manager/CellObject.h"
#include "ZoneServer/CharSheetManager.h"
#include "ZoneServer/GameSystemManagers/Crafting Manager/CraftingSessionFactory.h"
#include "ZoneServer/GameSystemManagers/Crafting Manager/DraftSchematic.h"
#include "ZoneServer/GameSystemManagers/Crafting Manager/SchematicGroup.h"
#include "ZoneServer/GameSystemManagers/Crafting Manager/SchematicManager.h"
#include "Zoneserver/Objects/CraftingStation.h"
#include "Zoneserver/Objects/Datapad.h"

//#include "ZoneServer/GameSystemManagers/Group Manager/GroupManager.h"
//#include "ZoneServer/GameSystemManagers/Group Manager/GroupObject.h"

#include "Zoneserver/GameSystemManagers/Event Manager/ActionStateEvent.h"
#include "Zoneserver/GameSystemManagers/Event Manager/LocomotionStateEvent.h"
#include "Zoneserver/GameSystemManagers/Event Manager/PostureEvent.h"
#include "ZoneServer/ProfessionManagers/Artisan Manager/SampleEvent.h"

#include "ZoneServer/GameSystemManagers/Spawn Manager/SpawnPoint.h"
#include "ZoneServer/GameSystemManagers/State Manager/StateManager.h"
#include "ZoneServer/Tutorial.h"
#include "ZoneServer/GameSystemManagers/UI Manager/UIManager.h"
#include "ZoneServer/GameSystemManagers/UI Manager/UISkillSelectBox.h"
#include "ZoneServer/GameSystemManagers/UI Manager/UIOfferTeachBox.h"
#include "ZoneServer/GameSystemManagers/UI Manager/UIPlayerSelectBox.h"
#include "ZoneServer/GameSystemManagers/UI Manager/UICloneSelectListBox.h"

//#include "ZoneServer/ProfessionManagers/Artisan Manager/SurveyEvent.h"
//#include "ZoneServer/ProfessionManagers/Artisan Manager/SampleEvent.h"
#include "ZoneServer/GameSystemManagers/Event Manager/LogOutEvent.h"
#include "ZoneServer/Objects/ItemDeleteEvent.h"
#include "ZoneServer/ProfessionManagers/Medic Manager/InjuryTreatmentEvent.h"
#include "ZoneServer/ProfessionManagers/Medic Manager/QuickHealInjuryTreatmentEvent.h"
#include "ZoneServer/ProfessionManagers/Medic Manager/WoundTreatmentEvent.h"

#include "ZoneServer/Objects/VehicleController.h"
#include "ZoneServer/WorldConfig.h"
#include "ZoneServer/WorldManager.h"
#include "ZoneServer/ZoneOpcodes.h"

#include "MessageLib/MessageLib.h"
#include "DatabaseManager/Database.h"
#include "Common/atMacroString.h"
#include "Common/EventDispatcher.h"
#include "NetworkManager/Message.h"
#include "NetworkManager/MessageFactory.h"
#include "anh/Utils/clock.h"
#include "Utils/EventHandler.h"

#include "ZoneServer\Services\equipment\equipment_service.h"
#include "ZoneServer\Services\ham\ham_service.h"
#include "anh\service\service_manager.h"

#include <anh\app\swganh_kernel.h>

using ::common::IEventPtr;
using ::common::EventDispatcher;
using ::common::EventType;
using ::common::EventListener;
using ::common::EventListenerType;


//=============================================================================

PlayerObject::PlayerObject()
    : IntangibleObject()
	, mLanguage(1)
    , mHasCamp(false)
    , mDataPad(nullptr)
    , mBazaarPoint(nullptr)
    , mClient(nullptr)
    , mCraftingSession(nullptr)
    , mMount(nullptr)
    , mNearestCloningFacility(nullptr)
    , mTravelPoint(nullptr)
    , mTutorial(nullptr)
    , mCombatTargetId(0)
    , mLastGroupMissionUpdateTime(0)
    , mNearestCraftingStation(0)
    , mPlacedInstrument(0)
    , mPreDesignatedCloningFacilityId(0)
    , mSelectedInstrument(0)
    , mTradePartner(0)
    , mAccountId(0)
    , mClientTickCount(0)
    , mCraftingStage(0)
    , mDConnTime(60)
    , mExperimentationFlag(0)
    , mExperimentationPoints(0)
    , mFriendsListUpdateCounter(0)
    , mHoloEmote(0)
    , mIgnoresListUpdateCounter(0)
    , mPlayerFlags(0)
    , mPlayerCustomFlags(0)
    , mLots(10)
    , mMissionIdMask(0)
    , mBindPlanet(-1)
    , mHomePlanet(-1)
    , mHoloCharge(0)
    , mNewPlayerExemptions(0)
    , mAutoAttack(false)
    , mContactListUpdatePending(false)
    , mMotdReceived(false)
    , mMountCalled(false)
    , mMounted(false)
    , mNewPlayerMessage(false)
    , mTrading(false)
{
    mIsForaging			= false;
    mType				= ObjType_Player;
	object_type_		= SWG_PLAYER;
    mStomach			= new Stomach(this);
    mMarriage			= L"";					// Unmarried
    mTrade				= new Trade(this);

    // register event functions
    registerEventFunction(this,&PlayerObject::onLogout);
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

    // register new event type functions

    gEventDispatcher.Connect(PostureUpdateEvent::type, EventListener(EventListenerType("PostureUpdate::handlePostureUpdate"), std::bind(&PlayerObject::handlePostureUpdate, this, std::placeholders::_1)));
    gEventDispatcher.Connect(LocomotionStateUpdateEvent::type, EventListener(EventListenerType("LocomotionStateUpdate::handleLocomotionUpdate"), std::bind(&PlayerObject::handleLocomotionUpdate, this, std::placeholders::_1)));
    gEventDispatcher.Connect(ActionStateUpdateEvent::type, EventListener(EventListenerType("ActionStateUpdate::handleActionStateUpdate"), std::bind(&PlayerObject::handleActionStateUpdate, this, std::placeholders::_1)));
}

//=============================================================================

PlayerObject::~PlayerObject()
{
	//at this point the client is already NULL so do NOT call any functions that might send messages
	LOG(error) << "player destructor started";
    
	

	//LOG(info) << "player destructor removing controllers";
    
	// remove any timers we got running
    gWorldManager->removeObjControllerToProcess(mObjectController.getTaskId());
        

    mObjectController.setTaskId(0);
    //mHam.setTaskId(0);
    
    // delete currently placed instrument
    if(mPlacedInstrument)
    {
        if(Item* item = dynamic_cast<Item*>(gWorldManager->getObjectById(mPlacedInstrument)))
        {
            mObjectController.destroyObject(mPlacedInstrument);
        }
    }

    // make sure we are deleted out of entertainer Ticks when entertained
    if(GetCreature()->getEntertainerWatchToId())
    {
        if(PlayerObject* entertainer = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(GetCreature()->getEntertainerWatchToId())))
        {
			gEntertainerManager->removeAudience(entertainer,this->getId());
        }
    }

    if(GetCreature()->getEntertainerListenToId())
    {
        if(PlayerObject* entertainer = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(GetCreature()->getEntertainerListenToId())))
        {
			gEntertainerManager->removeAudience(entertainer,this->getId());
        }
    }

    // make sure we don't leave a crafting session open
    gCraftingSessionFactory->destroySession(mCraftingSession);
    this->setCraftingSession(NULL);
    gStateManager.removeActionState(GetCreature(), CreatureState_Crafting);
    this->setCraftingStage(0);
    this->setExperimentationFlag(0);

	LOG(error) << "PlayerObject::~PlayerObject() : duel lists";

   // update duel lists
    PlayerList::iterator duelIt = mDuelList.begin();

    while(duelIt != mDuelList.end())
    {
        if((*duelIt)->checkDuelList(this))
        {
            PlayerObject* duelPlayer = (*duelIt);

            duelPlayer->removeFromDuelList(this);

            gMessageLib->sendUpdatePvpStatus(GetCreature(),duelPlayer);
            gMessageLib->sendUpdatePvpStatus(duelPlayer->GetCreature(),this);
        }

        ++duelIt;
    }
    mDuelList.clear();

<<<<<<< HEAD:src/ZoneServer/Objects/Player Object/PlayerObject.cpp
    
    clearAllUIWindows();    
=======


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
                // TODO: replace
                gStateManager.removeActionState(this, CreatureState_Combat);

                gMessageLib->sendStateUpdate(defenderCreature);
            }
        }

        ++defenderIt;
    }

    // remove us from cell / SI
    //please note that players are always in the si and get handled through it!
    //we dont need to update the cellcontainer to make them disappear
    if(mParentId)
    {
        if(CellObject* cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(mParentId)))
        {
            cell->removeObject(this);
        }
        else
        {
            DLOG(WARNING) << "PlayerObject::destructor: couldn't find cell " << mParentId;
        }
    }

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
>>>>>>> parent of 5bd772a... got rid of google log:src/ZoneServer/PlayerObject.cpp

    delete(mStomach);
    delete(mTrade);

	LOG(error) << "PlayerObject::~PlayerObject() end";
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
	//mSkillModUpdateCounter				= mSkillMods.size();
    mXpUpdateCounter					= mXpList.size();
    gStateManager.setCurrentPostureState(this->GetCreature(), CreaturePosture_Upright);

    // the client resets the bufftimers on local travel ... :(
    gBuffManager->InitBuffs(this);

    getSampleData()->mPendingSurvey		= false;
    getSampleData()->mPendingSample		= false;
    getSampleData()->mSampleNodeRecovery= false;

    this->togglePlayerCustomFlagOff(PlayerCustomFlag_LogOut);
    this->togglePlayerCustomFlagOff(PlayerCustomFlag_BurstRun);

    //mDefenderUpdateCounter				= 0;
    mReady								= false;

    Datapad* datapad			= getDataPad();
    if(datapad)
    {
		datapad->mWaypointUpdateCounter = datapad->GetWaypoints().size();
    }

    GetCreature()->updateMovementProperties();

    // We might have been invited to a duel
    clearDuelList();

    /*
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
    if(GetCreature()->getPerformingState() == PlayerPerformance_Dance)
    {
        // stop watching
        if(playerObject->GetCreature()->getEntertainerWatchToId()== mId)
        {
            radial->addItem(radId++,0,radId_serverPerformanceWatchStop,radAction_ObjCallback,"Stop Watching");

        }
        // start watching
        else if(glm::distance(playerObject->GetCreature()->mPosition, mPosition) < 20)
        {
            radial->addItem(radId++,0,radId_serverPerformanceWatch,radAction_ObjCallback,"Watch");
        }
    }
    // entertainer - music
    else if(GetCreature()->getPerformingState() == PlayerPerformance_Music)
    {
        // stop listening
        if(playerObject->GetCreature()->getEntertainerListenToId()== mId)
        {
            radial->addItem(radId++,0,radId_serverPerformanceListenStop,radAction_ObjCallback,"Stop Listening");
        }
        // start listening
        else if(glm::distance(playerObject->GetCreature()->mPosition, mPosition) < 20)
        {
            radial->addItem(radId++,0,radId_serverPerformanceListen,radAction_ObjCallback,"Listen");

        }
    }

    // teach
    if(playerObject->GetCreature()->getGroupId() && (GetCreature()->getGroupId() == playerObject->GetCreature()->getGroupId()))    {
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

bool PlayerObject::testCredits(uint32 amount)
{
	auto equipment_service = gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::equipment::EquipmentService>("EquipmentService");
	auto inventory = dynamic_cast<Inventory*>(equipment_service->GetEquippedObject(this->GetCreature(), "inventory"));
	auto bank = dynamic_cast<Bank*>(equipment_service->GetEquippedObject(this->GetCreature(), "bank"));

    if((!bank) || (!inventory))    {
		LOG(error) << "PlayerObject::updateCredits No Bank / Inventory for : " << this->getId();
		return false;
    }
	return(amount <= bank->getCredits() + inventory->getCredits());
    
}

//=============================================================================

bool PlayerObject::testBank(uint32 amount)
{
	auto equipment_service = gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::equipment::EquipmentService>("EquipmentService");
	auto bank = dynamic_cast<Bank*>(equipment_service->GetEquippedObject(this->GetCreature(), "bank"));

    if(bank)    {
        return(amount <= bank->getCredits());
    }

    return(false);
}

//=============================================================================

bool PlayerObject::testCash(uint32 amount)
{
	auto equipment_service = gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::equipment::EquipmentService>("EquipmentService");
	auto inventory = dynamic_cast<Inventory*>(equipment_service->GetEquippedObject(this->GetCreature(), "inventory"));

    if(inventory)    {
        return(amount <= inventory->getCredits());
    }

    return(false);
}

//=============================================================================

bool PlayerObject::updateCredits(int32 amount)
{
	auto equipment_service = gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::equipment::EquipmentService>("EquipmentService");
	auto inventory = dynamic_cast<Inventory*>(equipment_service->GetEquippedObject(this->GetCreature(), "inventory"));
	auto bank = dynamic_cast<Bank*>(equipment_service->GetEquippedObject(this->GetCreature(), "bank"));

    if(!bank)    {
		LOG(error) << "PlayerObject::updateCredits No Bank for : " << this->getId();
		return false;
	}
    
	if(!inventory)    {
		LOG(error) << "PlayerObject::updateCredits No Inventory for : " << this->getId();
		return false;
	}
    int64 cash = bank->getCredits() + inventory->getCredits();

	if((cash += amount) < 0)    {
		return(false);
	}

    // bank first
	int64 cash_64 =  bank->getCredits() + amount;
    bank->setCredits((cash_64 < 0) ? 0 : (uint32) cash_64);

	//int64 cash = ((cash_64 < 0) ? cash_64 : 0);
	inventory->updateCredits(((cash_64 < 0) ? cash_64 : 0));

	return(true);
    
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

	auto skills = this->GetCreature()->GetSkills();

    auto skillIt = skills.begin();

    while(skillIt != skills.end())
    {
		Skill* skill = gSkillManager->getSkillByName((*skillIt).c_str());
		SchematicGroupsList::iterator groupIt = skill->mSchematics.begin();

        while(groupIt != skill->mSchematics.end())
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
        gMessageLib->SendSystemMessage(::common::OutOfBand("badge_n", "prose_grant", "", "", "", "", "badge_n", badge->getName().getAnsi()), this);

        (gWorldManager->getKernel()->GetDatabase())->executeSqlAsync(0,0,"INSERT INTO %s.character_badges VALUES (%"PRIu64",%u)",gWorldManager->getKernel()->GetDatabase()->galaxy(),mId,badgeId);

        _verifyBadges();

        if(badge->getCategory() >= 3 && badge->getCategory() <= 5)
        {
            _verifyExplorationBadges();
        }
    }
    else
    {
        // This is an unexpected condition.
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
    case 10:
        addBadge(100);
        break;
    case 20:
        addBadge(101);
        break;
    case 30:
        addBadge(102);
        break;
    case 40:
        addBadge(103);
        break;
    case 45:
        addBadge(104);
        break;

    default:
        break;
    }

    _verifyBadges();
}

//=============================================================================

void PlayerObject::_verifyBadges()
{
    uint32 count = mBadgeList.size();

    switch(count)
    {
    case 5:
        addBadge(0);
        break;
    case 10:
        addBadge(1);
        break;
    case 25:
        addBadge(2);
        break;
    case 50:
        addBadge(3);
        break;
    case 75:
        addBadge(4);
        break;
    case 100:
        addBadge(5);
        break;
    case 125:
        addBadge(6);
        break;

    default:
        break;
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

bool PlayerObject::checkUIWindow(BString text)
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
            gMessageLib->SendSystemMessage(::common::OutOfBand("teaching", "student_has_offer_to_learn", 0, mId, 0), callingObject);
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
        DLOG(INFO) << "PlayerObject: Unhandled MenuSelect: " << messageType;
    }
    break;
    }
}

//=============================================================================

void PlayerObject::updateBankCredits(int32 amount)
{
	auto equip_service	= gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::equipment::EquipmentService>("EquipmentService");
	auto bank			= dynamic_cast<Bank*>(equip_service->GetEquippedObject(this->GetCreature(), "bank"));

    if(!bank)    {
		LOG (error) << "PlayerObject::giveBankCredits no bank for " << this->getId();
		return;
	}
    
	bank->updateCredits(amount);
}

//=============================================================================

void PlayerObject::updateInventoryCredits(int32 amount)
{
	auto equip_service	= gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::equipment::EquipmentService>("EquipmentService");
	auto inventory		= dynamic_cast<Inventory*>(equip_service->GetEquippedObject(this->GetCreature(), "inventory"));

    if(! inventory)    {
		LOG (error) << "PlayerObject::giveBankCredits no inventory for " << this->getId();
		return;
	}
    
	inventory->updateCredits(amount);
}

//=============================================================================
//
// handles any UIWindow callbacks for this player
// get these out of the playerobject asap
void PlayerObject::handleUIEvent(uint32 action,int32 element,std::u16string inputStr,UIWindow* window, std::shared_ptr<WindowAsyncContainerCommand> AsyncContainer)
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
                         // Invoke the actual cloning process.
						this->clone(sp->mCellId,sp->mDirection,sp->mPosition,true);
        
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
                                    this->clone(sp->mCellId,sp->mDirection,sp->mPosition,true);
                                    break;
                                }
                            }
                        }
                        if (this->mNewPlayerExemptions == 0)
                        {
							//auto ham = gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::ham::HamService>("HamService");
                            // Add wounds...
                            //this->getHam()->updatePrimaryWounds(100);

                            // .. and some BF
                            //this->getHam()->updateBattleFatigue(100, true);
                        }

                        // Clone
                        clone(sp->mCellId,sp->mDirection,sp->mPosition);
                    }
                }
            }
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
        StringVector*		dataItems	= skillSelectBox->getDataItems();
        BStringVector		splitSkill;
        BStringVector		splitProf;
        BString				skillString = dataItems->at(element).c_str();

        if(!skillString.getLength() || skillString.split(splitSkill,':') < 2)
        {
            return;
        }

        Skill* skill = gSkillManager->getSkillByName(splitSkill[1]);

        if(!skill)
        {
            DLOG(INFO) << "PlayerObject: teach skill : skill list surprisingly empty";
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
		sprintf(caption,"%s offers to teach you : %s",GetCreature()->getFirstName().c_str(),text);

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
		BString convName = teachBox->getPupil()->GetCreature()->getFirstName().c_str();
        convName.convert(BSTRType_Unicode16);

        ::common::ProsePackage prose("teaching", "teacher_skill_learned");
        prose.to_stf_file = "skl_n";
        prose.to_stf_label = teachBox->getSkill()->mName.getAnsi();
        prose.tt_custom_string = convName.getUnicode16();

        gMessageLib->SendSystemMessage(::common::OutOfBand(prose), this);

        //add skill to our pupils repertoir and send mission accomplished to our pupil
		gSkillManager->learnSkill(teachBox->getSkill()->mId,teachBox->getPupil()->GetCreature(),true);
    }
    break;

    // dance select list box
    case SUI_Window_SelectDance_Listbox:
    {
        if(action == 1)
            return;

        UIListBox*			danceSelectBox	= dynamic_cast<UIListBox*>(window);
        StringVector*		dataItems		= danceSelectBox->getDataItems();
        BStringVector		splitDance;

        if(element > -1)
        {
			BString	danceString = dataItems->at(element).c_str();

            if(!danceString.getLength() || danceString.split(splitDance,'+') < 2)
            {
                return;
            }

            BString mDance = splitDance[1];

            // if we are already dancing, only change the dance, otherwise start entertaining
			if(GetCreature()->getPerformingState() == PlayerPerformance_None)
            {
				GetCreature()->setPerformingState(PlayerPerformance_Dance);
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
        StringVector*	dataItems			= outcastSelectBox->getDataItems();

        if(element > -1)
        {
            std::string outcastString = dataItems->at(element);

            if(!outcastString.length())
            {
                return;
            }

            gEntertainerManager->verifyOutcastName(this,outcastString.c_str());
        }
    }
    break;

    // music select list box
    case SUI_Window_SelectMusic_Listbox:
    {
        if(action == 1)
            return;

        UIListBox*		musicSelectBox	= dynamic_cast<UIListBox*>(window);
        StringVector*	dataItems		= musicSelectBox->getDataItems();
        BStringVector	splitDance;

        if(element > -1)
        {
			BString	danceString = dataItems->at(element).c_str();

            if(!danceString.getLength() || danceString.split(splitDance,'+') < 2)
            {
                return;
            }

            BString mDance = splitDance[1];

            // if we are already making music, only change the piece, otherwise start entertaining
            if(GetCreature()->getPerformingState() == PlayerPerformance_None)            {
				GetCreature()->setPerformingState(PlayerPerformance_Music);
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
        if (element < 0 || element > 3) {
            break;
        }

        // advise the chat server
        gMessageLib->sendGroupLootModeResponse(this,element);
    }
    break;

    // group loot master listbox
    case SUI_Window_SelectGroupLootMaster_Listbox:
    {
        if (element < 0) {
            break;
        }

        UIPlayerSelectBox* selectionBox = dynamic_cast<UIPlayerSelectBox*>(window);
        //PlayerObject* selectedPlayer = selectionBox->getPlayers()[];

        //I realise this is not the FASTEST way to do this, but PlayerList is used as a list EVERYWHERE else.
        PlayerList::iterator it = selectionBox->getPlayers().begin();
        std::advance(it, element-1);

        PlayerObject* selectedPlayer = *it;

        if(selectedPlayer == NULL)
        {
            break;
        }

        gMessageLib->sendGroupLootMasterResponse(selectedPlayer,this);
    }
    break;

    default:
    {
<<<<<<< HEAD:src/ZoneServer/Objects/Player Object/PlayerObject.cpp
<<<<<<< HEAD:src/ZoneServer/Objects/Player Object/PlayerObject.cpp
		LOG(info) << "handleUIEvent:Default: " << action << "," << element << "," << inputStr.c_str();
=======
        DLOG(INFO) << "handleUIEvent:Default: " <<action<<","<<element<<","<<inputStr.getAnsi();
>>>>>>> parent of 5bd772a... got rid of google log:src/ZoneServer/PlayerObject.cpp
=======
        DLOG(INFO) << "handleUIEvent:Default: " <<action<<","<<element<<","<<inputStr.getAnsi();
>>>>>>> parent of 5bd772a... got rid of google log:src/ZoneServer/PlayerObject.cpp
    }
    break;
    }
}

//=============================================================================
//
// empty a players duellist and update the contaiuned players duellists
//
void PlayerObject::clearDuelList()
{
    PlayerList::iterator duelIt = mDuelList.begin();

    while(duelIt != mDuelList.end())
    {
        //please note that this player doesnt necessarily exist anymore
        //being challenged by us he might have logged out without him updating this list
        //please note, that a challenge means, that only the challengers list is updated with the challenged players name
        // the challenged players list remains empty
        PlayerObject* duelPlayer;
        try
        {
            duelPlayer = dynamic_cast<PlayerObject*>((*duelIt));
        }
        catch (...)
        {
            // The player might have logged out without accepting the duel
            duelPlayer = nullptr;
        }

        if(duelPlayer && duelPlayer->checkDuelList(this))
        {
            duelPlayer->removeFromDuelList(this);

			gMessageLib->sendUpdatePvpStatus(this->GetCreature(),duelPlayer);
            gMessageLib->sendUpdatePvpStatus(duelPlayer->GetCreature(),this);
        }

        ++duelIt;
    }
    mDuelList.clear();
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
        DLOG(INFO) << "PlayerObject::addToDuelList: "<<player->getId() << " wanted to add himself to his/her duel list";
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
bool PlayerObject::isNearestCraftingStationPrivate(uint64 station)
{
    CraftingStation* craftStation = dynamic_cast<CraftingStation*>(gWorldManager->getObjectById(station));
    if(craftStation)
    {
        switch(craftStation->getItemType())
        {
        case ItemType_ClothingStation:
        case ItemType_WeaponStation:
        case ItemType_FoodStation:
        case ItemType_StructureStation:
        case ItemType_SpaceStation:
            return true;
        default:
            return false;
        }
    }
    return false;
}
//=============================================================================

void PlayerObject::clone(uint64 parentId, const glm::quat& dir, const glm::vec3& pos, bool preDesignatedFacility)
{
    // Remove revive timer, if any.
    gWorldManager->removePlayerObjectForTimedCloning(mId);



    //reset buffs
    BuffList::iterator it = this->GetCreature()->GetBuffList()->begin();
    while(it != this->GetCreature()->GetBuffList()->end())
    {
        GetCreature()->RemoveBuff((*it));
        it++;
    }
    this->GetCreature()->GetBuffList()->clear();
    this->GetCreature()->CleanUpBuffs();
    
	//Im not 100% positive if this is necessary
	//this->getHam()->resetModifiers();

	//TODO reset skillmodifiers
    

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
		auto equip_service	= gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::equipment::EquipmentService>("EquipmentService");
		auto inventory		= dynamic_cast<Inventory*>(equip_service->GetEquippedObject(this->GetCreature(), "inventory"));

        if (inventory)        {
            SortedInventoryItemList insuranceList;
            inventory->getInsuredItems(&insuranceList);

            SortedInventoryItemList::iterator it;
            it = insuranceList.begin();

            while (it != insuranceList.end())	{
                if (Object* object = gWorldManager->getObjectById((*it).second))
                {
                    if (TangibleObject* tangibleObject = dynamic_cast<TangibleObject*>(object))
                    {
                        // Remove insurance.
                        tangibleObject->setInternalAttribute("insured","0");
                        gWorldManager->getKernel()->GetDatabase()->executeSqlAsync(NULL,NULL,"UPDATE %s.item_attributes SET value=0 WHERE item_id=%"PRIu64" AND attribute_id=%u",gWorldManager->getKernel()->GetDatabase()->galaxy(),tangibleObject->getId(), 1270);


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
    
	auto defenderList = GetCreature()->GetDefender();
    auto defenderIt = defenderList.begin();
    while (defenderIt != defenderList.end())
    {
        if (CreatureObject* defenderCreature = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById((*defenderIt))))
        {
            defenderCreature->RemoveDefender(this->getId());

            if (PlayerObject* defenderPlayer = dynamic_cast<PlayerObject*>(defenderCreature))
            {
                gMessageLib->sendUpdatePvpStatus(this->GetCreature(),defenderPlayer);
            }

            // if no more defenders, clear combat state
            if (!defenderCreature->GetDefender().size())
            {
                gStateManager.setCurrentActionState(defenderCreature, CreatureState_Peace);
            }
        }
        // If we remove self from all defenders, then we should remove all defenders from self. Remember, we are dead.
        GetCreature()->RemoveDefender(*defenderIt);//were using a copy
		defenderIt++;
    }
    
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
            gMessageLib->SendSystemMessage(::common::OutOfBand("base_player", "prose_newbie_insured", 0, 0, 0, mNewPlayerExemptions), this);
        }
        else if (mNewPlayerExemptions == 1)
        {
            // New Player Auto-Insure Activated. You have one death before item insurance is required.
            gMessageLib->SendSystemMessage(::common::OutOfBand("base_player", "last_newbie_insure"), this);
        }
        else if (mNewPlayerExemptions == 0)
        {
            // New player exemption status has expired. You will now be required to insure your items if you wish to retain them after cloning.
            gMessageLib->SendSystemMessage(::common::OutOfBand("base_player", "newbie_expired"), this);
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
//get this out here asap
void PlayerObject::cloneAtNearestCloningFacility(void)
{
	auto ham = gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::ham::HamService>("HamService");
    if (mNearestCloningFacility)
    {
        if (SpawnPoint* sp = mNearestCloningFacility->getRandomSpawnPoint())
        {
            if (mNewPlayerExemptions == 0)
            {
                // Add wounds...
				ham->UpdateWound(this->GetCreature(), HamBar_Health, 100);
				ham->UpdateWound(this->GetCreature(), HamBar_Action, 100);
				ham->UpdateWound(this->GetCreature(), HamBar_Mind, 100);
               
                // .. and some BF
				ham->UpdateBattleFatigue(this->GetCreature(), 100);
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
            if(strstr(object->GetTemplate().c_str(),"cantina"))
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
    PlayerObject* PlayerTarget = dynamic_cast<PlayerObject*>(Player->GetCreature()->getTarget());

    if (PlayerTarget && PlayerTarget->getId() != Player->getId())
    {
        //check duel
        if (Player->checkDuelList(PlayerTarget))
            return Player;
        //check pvp status
        if(Player->GetCreature()->getPvPStatus() != PlayerTarget->GetCreature()->getPvPStatus())
        {
            //send pvp_no_help
            DLOG(INFO) << "PVP Flag not right";
            gMessageLib->SendSystemMessage(::common::OutOfBand("healing", "pvp_no_help"), Player);
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

    gWorldManager->getKernel()->GetDatabase()->executeSqlAsync(0,0,"UPDATE %s.characters SET parent_id=%"PRIu64" WHERE id=%"PRIu64"",gWorldManager->getKernel()->GetDatabase()->galaxy(),mParentId,this->getId());

}

//=============================================================================
//
// Posture Commands
//
bool PlayerObject::handlePostureUpdate(IEventPtr triggered_event)
{
    // Cast the IEvent to the PostureUpdateEvent.
    auto pre_event = std::dynamic_pointer_cast<PostureUpdateEvent>(triggered_event);
    if (!pre_event) {
        gMessageLib->SendSystemMessage(L"Received an invalid event!", this);
        return false;
    }
    if (CreatureObject* creo = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById(pre_event->getCreatureObjectByID())))
    {
        creo->creaturePostureUpdate();
        // Lookup the creature and ensure it is a valid object.
        if (PlayerObject* player = dynamic_cast<PlayerObject*>(creo))
        {
            // process the appropriate command.
            switch (pre_event->getNewPostureState())
            {
            case CreaturePosture_Upright:
                player->setUpright();
                break;
            case CreaturePosture_Crouched:
                player->setCrouched();
                break;
            case CreaturePosture_Prone:
                player->setProne();
                break;
            case CreaturePosture_Sitting:
                player->setSitting();
                break;
            
            default:
                break;
            }

            gMessageLib->sendUpdateMovementProperties(player);

            
        }
        return true;
    }
    return false;
}

void PlayerObject::setSitting()
{
    //hack-fix clientside bug by manually sending client message
    gMessageLib->SendSystemMessage(::common::OutOfBand("shared", "player_sit"), this);
}

void PlayerObject::setUpright()
{
    // see if we need to get out of sampling mode
    if(this->getSamplingState())
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("survey", "sample_cancel"), this);
        this->setSamplingState(false);
    }

    if(this->checkPlayerCustomFlag(PlayerCustomFlag_LogOut))
    {
        this->togglePlayerCustomFlagOff(PlayerCustomFlag_LogOut);
        gMessageLib->SendSystemMessage(::common::OutOfBand("logout", "aborted"), this);
    }

    //if player is seated on an a chair, hack-fix clientside bug by manually sending client message
    bool IsSeatedOnChair = this->GetCreature()->states.checkState(CreatureState_SittingOnChair);
    if(IsSeatedOnChair)
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("shared", "player_stand"), this);
    }
}

void PlayerObject::setProne()
{

    // see if we need to get out of sampling mode
    if(this->getSamplingState())
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("survey", "sample_cancel"), this);
        this->setSamplingState(false);
    }

    if(this->checkPlayerCustomFlag(PlayerCustomFlag_LogOut))
    {
        this->togglePlayerCustomFlagOff(PlayerCustomFlag_LogOut);
        gMessageLib->SendSystemMessage(::common::OutOfBand("logout", "aborted"), this);
    }
    //if player is seated on an a chair, hack-fix clientside bug by manually sending client message
    bool IsSeatedOnChair = this->GetCreature()->states.checkState(CreatureState_SittingOnChair);
    if(IsSeatedOnChair)
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("shared", "player_prone"), this);
    }
}

void PlayerObject::setCrouched()
{

	   // see if we need to get out of sampling mode
    if(this->getSamplingState())
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("survey", "sample_cancel"), this);
        this->setSamplingState(false);
    }

    if(this->checkPlayerCustomFlag(PlayerCustomFlag_LogOut))
    {
        this->togglePlayerCustomFlagOff(PlayerCustomFlag_LogOut);
        gMessageLib->SendSystemMessage(::common::OutOfBand("logout", "aborted"), this);
    }

    //Get whether player is seated on a chair before we toggle it
    bool IsSeatedOnChair = this->GetCreature()->states.checkState(CreatureState_SittingOnChair);

    //if player is seated on an a chair, hack-fix clientside bug by manually sending client message
    if(IsSeatedOnChair)
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("shared", "player_kneel"), this);
    }
}

void PlayerObject::playFoodSound(bool food, bool drink)
{
    bool gender = getGender();
    switch (GetCreature()->getRaceId())
    {
        // wookiee
    case 4:
        if (gender)
        {
            // female
            if (food)
                gMessageLib->sendPlayClientEffectLocMessage(gWorldManager->getClientEffect(545),mPosition,this);
            if (drink)
                gMessageLib->sendPlayMusicMessage(WMSound_Drink_Wookiee_Female, this);
        }
        else
        {
            // male
            if (food)
                gMessageLib->sendPlayClientEffectLocMessage(gWorldManager->getClientEffect(552),mPosition,this);
            if (drink)
                gMessageLib->sendPlayMusicMessage(WMSound_Drink_Wookiee_Male, this);
        }
        break;
        // reptile aka Trandoshan
    case 6:
        if (gender)
        {
            // female
            if (food)
                gMessageLib->sendPlayClientEffectLocMessage(gWorldManager->getClientEffect(560),mPosition,this);
            if (drink)
                gMessageLib->sendPlayMusicMessage(WMSound_Drink_Reptile_Female, this);
        }
        else
        {
            // male
            if (food)
                gMessageLib->sendPlayClientEffectLocMessage(gWorldManager->getClientEffect(568),mPosition,this);
            if (drink)
                gMessageLib->sendPlayMusicMessage(WMSound_Drink_Reptile_Male, this);
        }
        break;
        // all else
    default:
        if (gender)
        {
            // female
            if (food)
                gMessageLib->sendPlayClientEffectLocMessage(gWorldManager->getClientEffect(560),mPosition,this);
            if (drink)
                gMessageLib->sendPlayMusicMessage(WMSound_Drink_Human_Female, this);
        }
        else
        {
            // male
            if (food)
                gMessageLib->sendPlayClientEffectLocMessage(gWorldManager->getClientEffect(568),mPosition,this);
            if (drink)
                gMessageLib->sendPlayMusicMessage(WMSound_Drink_Human_Male, this);
        }
        break;
    }
}
//
bool PlayerObject::handleActionStateUpdate(::common::IEventPtr triggered_event)
{
    // Cast the IEvent to the ActionStateUpdateEvent.
    auto pre_event = std::dynamic_pointer_cast<ActionStateUpdateEvent>(triggered_event);
    if (!pre_event) {
        gMessageLib->SendSystemMessage(L"Received an invalid event!", this);
        return false;
    }
    if (CreatureObject* creo = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById(pre_event->getCreatureObjectByID())))
    {
        return true;
    }
    return false;
}
bool PlayerObject::handleLocomotionUpdate(::common::IEventPtr triggered_event)
{
    // Cast the IEvent to the LocomotionUpdateEvent.
    auto pre_event = std::dynamic_pointer_cast<LocomotionStateUpdateEvent>(triggered_event);
    if (!pre_event) {
        gMessageLib->SendSystemMessage(L"Received an invalid event!", this);
        return false;
    }
    if (CreatureObject* creo = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById(pre_event->getCreatureObjectByID())))
    {
        creo->creatureLocomotionUpdate();
        return true;
    }
    return false;
}

int32_t PlayerObject::GetCurrentForcePower()
{
    auto lock = AcquireLock();
    return GetCurrentForcePower(lock);
}

int32_t PlayerObject::GetCurrentForcePower(boost::unique_lock<boost::mutex>& lock)
{
    return current_force_power_;
}

void PlayerObject::SetCurrentForcePower(int32_t force_power)
{
    auto lock = AcquireLock();
    SetCurrentForcePower(force_power, lock);
}

void PlayerObject::SetCurrentForcePower(int32_t force_power, boost::unique_lock<boost::mutex>& lock)
{
    current_force_power_ = force_power;
    //DISPATCH(Player, ForcePower);
}

void PlayerObject::IncrementForcePower(int32_t force_power)
{
    auto lock = AcquireLock();
    IncrementForcePower(force_power, lock);
}

void PlayerObject::IncrementForcePower(int32_t force_power, boost::unique_lock<boost::mutex>& lock)
{
    int32_t new_force_power = current_force_power_ + force_power;

    current_force_power_ = (new_force_power > GetMaxForcePower()) ? GetMaxForcePower() : new_force_power;
    //DISPATCH(Player, ForcePower);
}

int32_t PlayerObject::GetMaxForcePower()
{
    auto lock = AcquireLock();
    return GetMaxForcePower(lock);
}

int32_t PlayerObject::GetMaxForcePower(boost::unique_lock<boost::mutex>& lock)
{
    return max_force_power_;
}

void PlayerObject::SetMaxForcePower(int32_t force_power)
{
    auto lock = AcquireLock();
    SetMaxForcePower(force_power, lock);
}

void PlayerObject::SetMaxForcePower(int32_t force_power, boost::unique_lock<boost::mutex>& lock)
{
    max_force_power_ = force_power;
    //DISPATCH(Player, MaxForcePower);
}

void PlayerObject::CreateBaselines(PlayerObject* observer)
{
    if (auto dispatch = GetEventDispatcher())
    {
        dispatch->Dispatch(std::make_shared<swganh::event_dispatcher::ObserverEvent>("Player::Baselines", this, observer));
    }
}

CreatureObject*	PlayerObject::GetCreature()
{
	auto lock = AcquireLock();
	return GetCreature(lock);
}

CreatureObject*	PlayerObject::GetCreature(boost::unique_lock<boost::mutex>& lock)
{
	return body_;
}


void PlayerObject::prepareSkillCommands()
{
	auto player_skills = this->GetCreature()->GetSkills();

    mSkillCommands.clear();
    mSkillCommandMap.clear();

	auto skillIt		= player_skills.begin();

    while(skillIt != player_skills.end())
    {
		Skill* skill = gSkillManager->getSkillByName((*skillIt).c_str());
        // we dont want race specific skills here
        if(skill->mSpeciesRequired.size())
        {
            ++skillIt;
            continue;
        }

        SkillCommandList::iterator scIt = skill->mCommands.begin();
        SkillCommandList::iterator localScIt;

        while(scIt != skill->mCommands.end())
        {
            localScIt = std::find(mSkillCommands.begin(),mSkillCommands.end(),(*scIt));

            if(localScIt == mSkillCommands.end())
            {
                mSkillCommands.push_back((*scIt));
                mSkillCommandMap.insert(std::make_pair((gSkillManager->getSkillCmdById(*scIt)).getCrc(),(void*)0));
            }

            ++scIt;
        }
        ++skillIt;
    }
}

//=============================================================================

bool PlayerObject::verifyAbility(uint32 abilityCRC)
{
    SkillCommandMap::iterator it = mSkillCommandMap.find(abilityCRC);

    if(it != mSkillCommandMap.end())
        return(true);

    return(false);
}
