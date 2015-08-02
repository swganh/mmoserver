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

#include "CreatureObject.h"
#include "AttackableCreature.h"
#include "AttackableStaticNpc.h"
#include "Buff.h"
#include "BuildingObject.h"
#include "EntertainerManager.h"
#include "IncapRecoveryEvent.h"
#include "PlayerObject.h"
#include "StateManager.h"
#include "SpawnPoint.h"
#include "UIManager.h"
#include "VehicleController.h"
#include "WorldManager.h"
#include "SpatialIndexManager.h"
#include "WorldConfig.h"
#include "ZoneServer/Tutorial.h"
#include "MessageLib/MessageLib.h"
// events
#include "PostureEvent.h"
#include "ActionStateEvent.h"
#include "LocomotionStateEvent.h"
#include "Common/EventDispatcher.h"

#include "Utils/clock.h"

#include "Utils/rand.h"
#include <cfloat>

using ::common::IEventPtr;
using ::common::EventDispatcher;
using ::common::EventType;
using ::common::EventListener;
using ::common::EventListenerType;

//=============================================================================

CreatureObject::CreatureObject()
: MovingObject()
,	mTargetId(0)
, mDefenderUpdateCounter(0)
, mSkillModUpdateCounter(0)
, mCurrentAnimation("")
, mCustomizationStr("")
, mFaction("")
, mSpecies("")
, mSpeciesGroup("")
, mHair(NULL)
, mPerformance(NULL)
, mPvPStatus(CreaturePvPStatus_None)
, mPendingPerform(PlayerPerformance_None)
, mCurrentIncapTime(0)
, mEntertainerListenToId(0)
, mFirstIncapTime(0)
, mGroupId(0)
, mLastEntertainerXP(0)
, mScale(1.0)
, mLanguage(1)
, mLastMoveTick(0)
, mPerformanceCounter(0)
, mPerformanceId(0)
, mRaceGenderMask(0)
, mSkillUpdateCounter(0)
, mCL(1)
, mFactionRank(0)
, mIncapCount(0)
, mMoodId(0)
, mReady(false)
{
    mType = ObjType_Creature;

    mSkillMods.reserve(50);
    mSkillCommands.reserve(50);
    mFactionList.reserve(50);
    // mDefenders.reserve(10); // It's a std::list now, for faster insertion at front.

    mHam.setParent(this);
    mEquipManager.setParent(this);

    for(uint16 i = 1;i<256;i++)
        mCustomization[i]=0;
    
    // initialize state struct
    states.action          = 0;
    states.posture         = 0;
    states.locomotion      = 0;
    
    states.blockAction     = false;
    states.blockLocomotion = false;
    states.blockPosture    = false;

    // register event functions
    registerEventFunction(this,&CreatureObject::onIncapRecovery);

    // register new event listeners
}

//=============================================================================

CreatureObject::~CreatureObject()
{
    mBuffList.clear();
}

//=============================================================================

void CreatureObject::prepareSkillMods()
{
    mSkillMods.clear();

    SkillList::iterator skillIt = mSkills.begin();

    while(skillIt != mSkills.end())
    {
        SkillModsList::iterator smIt = (*skillIt)->mSkillMods.begin();
        SkillModsList::iterator localSmIt;

        while(smIt != (*skillIt)->mSkillMods.end())
        {
            localSmIt = findSkillMod((*smIt).first);

            if(localSmIt != mSkillMods.end())
                (*localSmIt).second += (*smIt).second;
            else
                mSkillMods.push_back((*smIt));

            ++smIt;
        }
        ++skillIt;
    }
}

//=============================================================================

void CreatureObject::prepareSkillCommands()
{
    mSkillCommands.clear();
    mSkillCommandMap.clear();

    SkillList::iterator skillIt = mSkills.begin();

    while(skillIt != mSkills.end())
    {
        // we dont want race specific skills here
        if((*skillIt)->mSpeciesRequired.size())
        {
            ++skillIt;
            continue;
        }

        SkillCommandList::iterator scIt = (*skillIt)->mCommands.begin();
        SkillCommandList::iterator localScIt;

        while(scIt != (*skillIt)->mCommands.end())
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

bool CreatureObject::verifyAbility(uint32 abilityCRC)
{
    SkillCommandMap::iterator it = mSkillCommandMap.find(abilityCRC);

    if(it != mSkillCommandMap.end())
        return(true);

    return(false);
}

//=============================================================================

SkillModsList::iterator CreatureObject::findSkillMod(uint32 modId)
{
    SkillModsList::iterator it = mSkillMods.begin();

    while(it != mSkillMods.end())
    {
        if(modId == (*it).first)
            return(it);

        ++it;
    }
    return(mSkillMods.end());
}

//=============================================================================

int32 CreatureObject::getSkillModValue(uint32 modId)
{
    SkillModsList::iterator it = mSkillMods.begin();

    while(it != mSkillMods.end())
    {
        if(modId == (*it).first)
            return((*it).second);

        ++it;
    }

    return(-1000);
}

//=============================================================================

bool CreatureObject::setSkillModValue(uint32 modId,int32 value)
{
    SkillModsList::iterator it = mSkillMods.begin();

    while(it != mSkillMods.end())
    {
        if(modId == (*it).first)
        {
            (*it).second = value;
            return(true);
        }

        ++it;
    }

    return(false);
}
bool CreatureObject::modifySkillModValue(uint32 modId,int32 modifier)
{
    SkillModsList::iterator it = mSkillMods.begin();

    while(it != mSkillMods.end())
    {
        if(modId == (*it).first)
        {
            (*it).second += modifier;

            //If we have a player, send mod updates
            PlayerObject* temp = dynamic_cast<PlayerObject*>(this);
            if(temp)
            {
                SkillModsList modlist;
                modlist.push_back(*it);
                //TODO: Skillmods should use a Delta, not a Baseline
                gMessageLib->sendBaselinesCREO_4(temp);
                //gMessageLib->sendSkillModUpdateCreo4(temp);
                //gMessageLib->sendSkillModDeltasCREO_4(modlist, 0, temp, temp);
            }
            return(true);
        }

        ++it;
    }



    return(false);
}

//=============================================================================

int32 CreatureObject::getFactionPointsByFactionId(uint32 id)
{
    FactionList::iterator it = mFactionList.begin();

    while(it != mFactionList.end())
    {
        if((*it).first == id)
            return((*it).second);

        ++it;
    }

    return(-10000);
}

//=============================================================================

bool CreatureObject::updateFactionPoints(uint32 factionId,int32 value)
{
    FactionList::iterator it = mFactionList.begin();

    while(it != mFactionList.end())
    {
        if((*it).first == factionId)
        {
            (*it).second += value;
            return(true);
        }

        ++it;
    }

    return(false);
}

//=============================================================================

bool CreatureObject::checkSkill(uint32 skillId)
{
    SkillList::iterator skillIt = mSkills.begin();

    while(skillIt != mSkills.end())
    {
        if((*skillIt)->mId == skillId)
            return(true);
        ++skillIt;
    }

    return(false);
}

//=============================================================================

bool CreatureObject::removeSkill(Skill* skill)
{
    SkillList::iterator skillIt = mSkills.begin();

    while(skillIt != mSkills.end())
    {
        if((*skillIt) == skill)
        {
            mSkills.erase(skillIt);
            return(true);
        }
        ++skillIt;
    }

    return(false);
}

//=============================================================================

uint32 CreatureObject::getSkillPointsLeft()
{
    SkillList::iterator skillIt		= mSkills.begin();
    uint32 skillPointsLeft			= 250;

    while(skillIt != mSkills.end())
    {
        skillPointsLeft -= (*skillIt)->mSkillPointsRequired;
        ++skillIt;
    }

    return(skillPointsLeft);
}

//=============================================================================

bool CreatureObject::handlePerformanceTick(uint64 time,void* ref)
{
    return(gEntertainerManager->handlePerformanceTick(this));
}

bool CreatureObject::handleImagedesignerTimeOut(uint64 time,void* ref)
{
    return(gEntertainerManager->handleImagedesignTimeOut(this));
}

//=============================================================================
//
// update current movement properties
//

void CreatureObject::updateMovementProperties()
{
    switch(states.posture)
    {
        case CreaturePosture_KnockedDown:
        case CreaturePosture_Incapacitated:
        case CreaturePosture_Dead:
        case CreaturePosture_Sitting:
        {
            mCurrentRunSpeedLimit		= 0.0f;
            mCurrentAcceleration		= 0.0f;
            mCurrentTurnRate			= 0.0f;
            mCurrentTerrainNegotiation	= 0.0f;
        }
        break;

        case CreaturePosture_Upright:
        {
            mCurrentRunSpeedLimit		= mBaseRunSpeedLimit;
            mCurrentAcceleration		= mBaseAcceleration;
            mCurrentTurnRate			= mBaseTurnRate;
            mCurrentTerrainNegotiation	= mBaseTerrainNegotiation;
        }
        break;

        case CreaturePosture_Prone:
        {
            mCurrentRunSpeedLimit		= 1.0f;
            mCurrentAcceleration		= 0.25f;
            mCurrentTurnRate			= mBaseTurnRate;
            mCurrentTerrainNegotiation	= mBaseTerrainNegotiation;
        }
        break;

        case CreaturePosture_Crouched:
        {
            mCurrentRunSpeedLimit		= 0.0f;
            mCurrentAcceleration		= 0.0f;
            mCurrentTurnRate			= mBaseTurnRate;
            mCurrentTerrainNegotiation	= mBaseTerrainNegotiation;
        }
        break;

        default:
        {
            mCurrentRunSpeedLimit		= mBaseRunSpeedLimit;
            mCurrentAcceleration		= mBaseAcceleration;
            mCurrentTurnRate			= mBaseTurnRate;
            mCurrentTerrainNegotiation	= mBaseTerrainNegotiation;
        }
        break;
    }
}

//=============================================================================
Buff* CreatureObject::GetBuff(uint32 BuffIcon)
{
    //Cycle through all buffs for the creature
    BuffList::iterator it = mBuffList.begin();
    //Check if the Icon CRCs Match (ie Duplication)
    while(it != mBuffList.end())
    {
        Buff* temp = *it;
        if(temp->GetIcon() == BuffIcon)
        {
            return temp;
        }
        it++;
    }
    return 0;
}
bool CreatureObject::GetBuffExists(uint32 BuffIcon)
{
    //Cycle through all buffs for the creature
    BuffList::iterator it = mBuffList.begin();
    while(it != mBuffList.end())
    {
        //Check if the Icon CRCs Match (ie Duplication)
        Buff* temp = *it;
        if(temp->GetIcon() == BuffIcon)
        {
            //Check if the duplicate isn't marked for deletion
            if(!temp->GetIsMarkedForDeletion())
            {
                return true;
            }
        }
        it++;
    }
    return false;
}
//=============================================================================
void CreatureObject::AddBuff(Buff* buff,  bool stackable, bool overwrite)
{
    if(!buff) return;

    if(buff->GetRemainingTime(gWorldManager->GetCurrentGlobalTick()) <= 0)
    {
        SAFE_DELETE(buff);
        return;
    }

    //Use this opportunity to clean up any dead buffs in BuffList
    this->CleanUpBuffs();

    //Check we don't have a null ptr
    if(!buff)
        return;

    buff->SetInit(true);
    //If we already have a buff with this CRC Icon   we want to decide whether we can stack this buff
    //stacking is for example important for filling and food buffs

    //the next thing to be aware of, is that a type of foodbuff can have several effects (plus burstrun minus ham)
    //but has only one icon
    //in this case every effect we send will have the icon associated with it
    if(GetBuffExists(buff->GetIcon()) && !stackable )
    {
        if(overwrite)
        {
            //find old buff

            //apply changes

            //exit

            //for now do nothing
        }
        PlayerObject* player = dynamic_cast<PlayerObject*>(this);
        if(player != 0)
        {
            //gMessageLib->sendSystemMessage(player, "You appear to have attempted to stack Buffs. The server has prevented this");
            DLOG(INFO) << "Attempt to duplicate buffs prevented.";
        }
        SAFE_DELETE(buff);
        return;
        //TODO Currently, no overwrites - need to make it so certain buffs replace older ones
        //which will be important for doctor buffs
    }

    //Set the target to this creature, in case it isn't already
    buff->mTarget = this;

    //Add to Buff List
    mBuffList.push_back(buff);

    //Perform Inital Buff Changes
    buff->InitialChanges();

    //Add Buff to Scheduler
    gWorldManager->addBuffToProcess(buff);
}


int CreatureObject::GetNoOfBuffs()
{
    /*int No =0;
    BuffList::iterator it = mBuffList.begin();
    while(*it != *mBuffList.end())
    {
    	No++;it++;
    }
    return No;*/
    return mBuffList.size();
    /*if(*mBuffList.end())
    {
    	return 1 + ((*mBuffList.end()) - (*mBuffList.begin()));
    } else {
    	return 0;
    }*/
}
void CreatureObject::RemoveBuff(Buff* buff)
{
    if(!buff)
        return;

    //Cancel the buff so the next Event doesn't do anything
    buff->mCancelled = true;

    //Remove from the Process Queue
    gWorldManager->removeBuffToProcess(buff->GetID());
    buff->SetID(0);

    //Perform any Final changes
    buff->FinalChanges();
    buff->MarkForDeletion();
}

//================================================
//
void CreatureObject::ClearAllBuffs()
{
    BuffList::iterator it = mBuffList.begin();
    while(it != mBuffList.end())
    {
        (*it)->FinalChanges();
        gWorldManager->removeBuffToProcess((*it)->GetID());
        (*it)->MarkForDeletion();
        ++it;
    }
    CleanUpBuffs();
}
void CreatureObject::CleanUpBuffs()
{
    BuffList::iterator it = mBuffList.begin();
    while(it != mBuffList.end())
    {
        if((*it)->GetIsMarkedForDeletion())
        {
            SAFE_DELETE(*it);
            mBuffList.erase(it++);
        }
        else
        {
            it++;
        }
    }
}

//=============================================================================

void CreatureObject::updateRaceGenderMask(bool female)
{
    // set race flag
    switch(mRaceId)
    {
    case 0:
        mRaceGenderMask |= 0x4;
        break;
    case 1:
        mRaceGenderMask |= 0x8;
        break;
    case 2:
        mRaceGenderMask |= 0x10;
        break;
    case 3:
        mRaceGenderMask |= 0x20;
        break;
    case 4:
        mRaceGenderMask |= 0x40;
        break;
    case 5:
        mRaceGenderMask |= 0x80;
        break;
    case 6:
        mRaceGenderMask |= 0x100;
        break;
    case 7:
        mRaceGenderMask |= 0x200;
        break;
    case 33:
        mRaceGenderMask |= 0x400;
        break;
    case 49:
        mRaceGenderMask |= 0x800;
        break;

    default:
        break;
    }

    // set gender flag
    if(female)
    {
        mRaceGenderMask |= 0x1;
    }
    else
    {
        mRaceGenderMask |= 0x2;
    }

    // set jedi flag
    if(PlayerObject* player = dynamic_cast<PlayerObject*>(this))
    {
        if(player->getJediState())
        {
            mRaceGenderMask |= 0x1000;
        }
    }
}
//gets called by the active Object of the statemanager
void CreatureObject::creatureActionStateUpdate()
{
	//gMessageLib->sendPostureAndStateUpdate(this);   
}
void CreatureObject::creaturePostureUpdate()
{
    this->getHam()->updateRegenRates();
    this->updateMovementProperties();
}

void CreatureObject::creatureLocomotionUpdate()
{
    this->getHam()->updateRegenRates();
    this->updateMovementProperties();
}

//=============================================================================
//
// incap
//
void CreatureObject::incap()
{
    // sanity check
    if (isIncapacitated() || isDead())
    {
        return;
    }

    if (this->getType() == ObjType_Player)
    {
        // first incap, update the initial time
        uint64 localTime = Anh_Utils::Clock::getSingleton()->getLocalTime();
        if(!mIncapCount)
        {
            mFirstIncapTime = localTime;
        }
        // reset the counter if the reset time has passed
        else if(mIncapCount != 0 && (localTime - mFirstIncapTime) >= gWorldConfig->getIncapResetTime() * 1000)
        {
            mIncapCount = 0;
            mFirstIncapTime = localTime;
        }
        /*
        if (mIncapCount != 0)
        {
        }
        */

        PlayerObject* player = dynamic_cast<PlayerObject*>(this);
        if (player)
        {
            player->disableAutoAttack();
        }
        //See if our player is mounted -- if so dismount him
        if(player->checkIfMounted())
        {
            //Get the player's mount
            if(VehicleController* vehicle = dynamic_cast<VehicleController*>(gWorldManager->getObjectById(player->getMount()->controller())))
            {
                //Now dismount
                vehicle->DismountPlayer();
            }

        }

        // advance incaps counter
        uint32 configIncapCount = gWorldConfig->getConfiguration<uint32>("Player_Incapacitation",3);
        if(++mIncapCount < (uint8)configIncapCount)
        {
            // update the posture and locomotion
            gStateManager.setCurrentPostureState(this, CreaturePosture_Incapacitated);
            gStateManager.setCurrentActionState(this, CreatureState_ClearState);

            // send timer updates
            mCurrentIncapTime = gWorldConfig->getBaseIncapTime() * 1000;
            gMessageLib->sendIncapTimerUpdate(this);

            // schedule recovery event
            mObjectController.addEvent(new IncapRecoveryEvent(),mCurrentIncapTime);

            // reset states
            mState = 0;

            // reset ham regeneration
            mHam.updateRegenRates();
            gWorldManager->removeCreatureHamToProcess(mHam.getTaskId());
            mHam.setTaskId(0);

            //updateMovementProperties();

            //gMessageLib->sendPostureAndStateUpdate(this);

            //if(PlayerObject* player = dynamic_cast<PlayerObject*>(this))
            //{
            //    gMessageLib->sendUpdateMovementProperties(player);
            //    gMessageLib->sendSelfPostureUpdate(player);
            //}
        }
        // we hit the max -> death
        else
        {
            die();
        }
    }
    else if (this->getType() == ObjType_Creature)	// A Creature.
    {
        die();
    }
    else
    {
        LOG(INFO) << "CreatureObject::incap Incapped unsupported type " <<  this->getType();
    }

}

//=============================================================================

void CreatureObject::die()
{
    mIncapCount			= 0;
    mCurrentIncapTime	= 0;
    mFirstIncapTime		= 0;

    gMessageLib->sendIncapTimerUpdate(this);

    if(PlayerObject* player = dynamic_cast<PlayerObject*>(this))
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("base_player", "victim_dead"), player);
        player->disableAutoAttack();
    }

    // reset ham regeneration
    mHam.updateRegenRates();
    gWorldManager->removeCreatureHamToProcess(mHam.getTaskId());
    mHam.setTaskId(0);

    updateMovementProperties();

    gStateManager.setCurrentPostureState(this, CreaturePosture_Dead);
    // clear states
    gStateManager.setCurrentActionState(this, CreatureState_ClearState);

    //gMessageLib->sendPostureAndStateUpdate(this);

    if(PlayerObject* player = dynamic_cast<PlayerObject*>(this))
    {
        /*gMessageLib->sendUpdateMovementProperties(player);
        gMessageLib->sendSelfPostureUpdate(player);*/

        // update duel lists
        player->clearDuelList();

        // update defender lists
        ObjectIDList::iterator defenderIt = mDefenders.begin();
        while (defenderIt != mDefenders.end())
        {
            if (CreatureObject* defenderCreature = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById((*defenderIt))))
            {
                defenderCreature->removeDefenderAndUpdateList(this->getId());

                if (PlayerObject* defenderPlayer = dynamic_cast<PlayerObject*>(defenderCreature))
                {
                    gMessageLib->sendUpdatePvpStatus(this,defenderPlayer);
                    // gMessageLib->sendDefenderUpdate(defenderPlayer,0,0,this->getId());
                }

                // Defender not hostile to me any more.
                gMessageLib->sendUpdatePvpStatus(defenderCreature, player);

                // if no more defenders, clear combat state
                if (!defenderCreature->getDefenders()->size())
                {
                    gStateManager.setCurrentActionState(defenderCreature, CreatureState_Peace);
                }
            }
            // If we remove self from all defenders, then we should remove all defenders from self. Remember, we are dead.
            defenderIt = mDefenders.erase(defenderIt);
        }

        // bring up the clone selection window
        ObjectSet inRangeBuildings;
        BStringVector buildingNames;
        std::vector<BuildingObject*> buildings;
        BuildingObject*	nearestBuilding = NULL;
        BuildingObject* preDesignatedBuilding = NULL;

        // Search for cloning facilities.
        gSpatialIndexManager->getObjectsInRange(this,&inRangeBuildings,ObjType_Building,8192, false);

        ObjectSet::iterator buildingIt = inRangeBuildings.begin();

        while (buildingIt != inRangeBuildings.end())
        {
            BuildingObject* building = dynamic_cast<BuildingObject*>(*buildingIt);
            if (building)
            {
                // Do we have any personal clone location?
                if (building->getId() == player->getPreDesignatedCloningFacilityId())
                {
                    preDesignatedBuilding = building;
                }

                if (building->getBuildingFamily() == BuildingFamily_Cloning_Facility)
                {
                    // TODO: This code is not working as intended if player dies inside, since buildings use world coordinates and players inside have cell coordinates.
                    // Tranformation is needed before the correct distance can be calculated.
                    if(!nearestBuilding	||
                        (nearestBuilding != building && (glm::distance(mPosition, building->mPosition) < glm::distance(mPosition, nearestBuilding->mPosition))))
                    {
                        nearestBuilding = building;
                    }
                }
            }
            ++buildingIt;
        }

        if (nearestBuilding)
        {
            if (nearestBuilding->getSpawnPoints()->size())
            {
                // buildingNames.push_back(nearestBuilding->getRandomSpawnPoint()->mName.getAnsi());
                // buildingNames.push_back("Closest Cloning Facility");
                buildingNames.push_back("@base_player:revive_closest");
                buildings.push_back(nearestBuilding);

                // Save nearest building in case we need to clone when reviev times out.
                player->saveNearestCloningFacility(nearestBuilding);
            }
        }

        if (preDesignatedBuilding)
        {
            if (preDesignatedBuilding->getSpawnPoints()->size())
            {
                // buildingNames.push_back("Pre-Designated Facility");
                buildingNames.push_back("@base_player:revive_bind");
                buildings.push_back(preDesignatedBuilding);
            }
        }

        if (buildings.size())
        {
            // Set up revive timer in case of user don't clone.
            gWorldManager->addPlayerObjectForTimedCloning(player->getId(), 10 * 60 * 1000);	// TODO: Don't use hardcoded value.
            gUIManager->createNewCloneSelectListBox(player,"cloneSelect","Select Clone Destination","Select clone destination",buildingNames,buildings,player);
        }
        else
        {
            DLOG(INFO) << "No cloning facility available ";
        }
    }
    else // if(CreatureObject* creature = dynamic_cast<CreatureObject*>(this))
    {
        // gMessageLib->sendUpdateMovementProperties(player);
        // gMessageLib->sendSelfPostureUpdate(player);

        // Who killed me
        // this->prepareCustomRadialMenu(this,0);

        //
        // update defender lists
        ObjectIDList::iterator defenderIt = mDefenders.begin();

        while(defenderIt != mDefenders.end())
        {
            // This may be optimized, but rigth now THIS code does the job.
            this->makePeaceWithDefender(*defenderIt);
            defenderIt = mDefenders.begin();
        }

        // I'm dead. De-spawn after "loot-timer" has expired, if any.
        // 3-5 min.
        uint64 timeBeforeDeletion = (180 + (gRandom->getRand() % (int32)120)) * 1000;
        if (this->getCreoGroup() == CreoGroup_AttackableObject)
        {
            // timeBeforeDeletion = 2 * 1000;
            // Almost NOW!
            timeBeforeDeletion = 500;

            // Play the death-animation, if any.
            if (AttackableStaticNpc* staticNpc = dynamic_cast<AttackableStaticNpc*>(this))
            {
                staticNpc->playDeathAnimation();
            }
        }
        else if (this->getCreoGroup() == CreoGroup_Creature)
        {
            if (AttackableCreature* creature = dynamic_cast<AttackableCreature*>(this))
            {
                creature->unequipWeapon();

                // Give XP to the one(s) attacking me.
                // creature->updateAttackersXp();
            }
        }

        if (NPCObject* npc = dynamic_cast<NPCObject*>(this))
        {
            // Give XP to the one(s) attacking me.
            npc->updateAttackersXp();
        }

        // Put this creature in the pool of delayed destruction.
        gWorldManager->addCreatureObjectForTimedDeletion(this->getId(), timeBeforeDeletion);
        // this->killEvent();
    }
}

//=============================================================================

void CreatureObject::addDefender(uint64 defenderId)
{
    ObjectIDList::iterator it = mDefenders.begin();

    while(it != mDefenders.end())
    {
        if((*it) == defenderId)
        {
            return;
        }

        ++it;
    }

    mDefenders.push_back(defenderId);
}


//=============================================================================

void CreatureObject::clearDefenders()
{
    if (mDefenders.size())
    {
        mDefenders.clear();
    }
}

//=============================================================================

void CreatureObject::removeAllDefender(void)
{
    ObjectIDList::iterator it = mDefenders.begin();
    uint16 index = 0;
    while (it != mDefenders.end())
    {
        gMessageLib->sendDefenderUpdate(this,0,index,(*it));
        it = mDefenders.erase(it);
        index++;
    }
}

//=============================================================================

void CreatureObject::removeDefenderAndUpdateList(uint64 defenderId)
{
    // Eruptor

    ObjectIDList::iterator it = mDefenders.begin();
    uint16 index = 0;
    while(it != mDefenders.end())
    {
        if ((*it) == defenderId)
        {
            gMessageLib->sendDefenderUpdate(this,0,index,defenderId);
            (void)mDefenders.erase(it);
            break;
        }
        index++;
        ++it;
    }
}

//=============================================================================
bool CreatureObject::setAsActiveDefenderAndUpdateList(uint64 defenderId)
{
    ObjectIDList::iterator it = mDefenders.begin();
    uint16 index = 0;
    bool valid = false;

    while(it != mDefenders.end())
    {
        if ((*it) == defenderId)
        {
            valid = true;
            break;
        }
        index++;
        ++it;
    }

    if (valid)
    {
        if (index != 0)
        {
            // Move the defender to top of list.
            (void)mDefenders.erase(it);
            mDefenders.push_front(defenderId);
            // gMessageLib->sendDefenderUpdate(this,2,0,defenderId);

            // gMessageLib->sendNewDefenderList(this);

            // THIS code should be in player, not creature, for obvious reasons.
            PlayerObject* player = dynamic_cast<PlayerObject*>(this);
            assert(player && "CreatureObject::setAsActiveDefenderAndUpdateList This should always be a player object");

            gMessageLib->sendBaselinesCREO_6(player,player);
            gMessageLib->sendEndBaselines(player->getPlayerObjId(),player);

            // gMessageLib->sendDefenderUpdate(this,0,0,0);
            // gMessageLib->sendDefenderUpdate(this,1,0,defenderId);		// Overwrite whatever we have there
        }
    }
    else
    {
        
    }
    return valid;
}


//=============================================================================

bool CreatureObject::checkDefenderList(uint64 defenderId)
{
    ObjectIDList::iterator it = mDefenders.begin();

    while(it != mDefenders.end())
    {
        if((*it) == defenderId)
        {
            return(true);
        }

        ++it;
    }

    return(false);
}

//=============================================================================

uint64 CreatureObject::getNearestDefender(void)
{
    uint64 defenederId = 0;
    float minLenght = FLT_MAX;

    ObjectIDList::iterator it = mDefenders.begin();

    while(it != mDefenders.end())
    {
        if((*it) != 0)
        {
            CreatureObject* defender = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById((*it)));
            if (defender && !defender->isDead() && !defender->isIncapacitated())
            {
                float len = glm::distance(this->mPosition, defender->mPosition);
                if (len < minLenght)
                {
                    minLenght = len;
                    defenederId = (*it);
                }
            }
        }
        ++it;
    }

    return defenederId;
}

//=============================================================================
//
//	Get nearest defender, but sorting out the kind of objects that can't attack you, like lairs and swoops.
//
//

uint64 CreatureObject::getNearestAttackingDefender(void)
{
    uint64 defenederId = 0;
    float minLenght = FLT_MAX;

    ObjectIDList::iterator it = mDefenders.begin();

    while(it != mDefenders.end())
    {
        if((*it) != 0)
        {
            CreatureObject* defender = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById((*it)));
            if (defender && !defender->isDead() && !defender->isIncapacitated())
            {
                if ((defender->getCreoGroup() == CreoGroup_Player) || (defender->getCreoGroup() == CreoGroup_Creature))
                {
                    float len = glm::distance(this->mPosition, defender->mPosition);
                    if (len < minLenght)
                    {
                        minLenght = len;
                        defenederId = (*it);
                    }
                }
            }
        }
        ++it;
    }

    return defenederId;
}


//=============================================================================

void CreatureObject::buildCustomization(uint16 customization[])
{
    uint8 len = 0x73;

    uint8* playerCustomization = new uint8[512];

    uint16 byteCount = 4; // 2 byte header + footer
    uint8 elementCount = 0;

    // get absolute bytecount(1 byte index + value)
    for(uint8 i = 1; i < len; i++)
    {
        if((customization[i] != 0))
        {
            if(customization[i] < 255)
                byteCount += 2;
            else
                byteCount += 3;

            elementCount++;
        }
    }
    //get additional bytes for female chest
    bool			female = false;
    PlayerObject*	player = dynamic_cast<PlayerObject*>(this);

    if(player)
        female = player->getGender();

    if(female)
    {
        //please note its 1 index with 2 attribute values!!!!
        byteCount += 1;
        elementCount++;

        for(uint8 i = 171; i < 173; i++)
        {
            if(customization[i] == 0)
                customization[i] = 511;

            if(customization[i] == 255)
                customization[i] = 767;

            if((customization[i] < 255)&&(customization[i] > 0))
                byteCount += 1;
            else
                byteCount += 2;
        }
    }

    // elements count
    playerCustomization[0] = 0x01;
    playerCustomization[1] = elementCount;

    uint16 j = 2;
    //get additional bytes for female chest
    if(female)
    {
        playerCustomization[j] = 171;
        j += 1;

        for(uint8 i = 171; i < 173; i++)
        {
            if((customization[i] < 255)&(customization[i] > 0))
            {
                playerCustomization[j] = (uint8)(customization[i] & 0xff);
                j += 1;
            }
            else
            {
                playerCustomization[j] = (uint8)(customization[i] & 0xff);
                playerCustomization[j+1] = (uint8)((customization[i] >> 8) & 0xff);
                j += 2;
            }

        }
    }



    // fill our string
    for(uint8 i = 1; i < len; i++)
    {
        if((customization[i] != 0))
        {
            playerCustomization[j] = i;

            if(customization[i] < 255)
            {
                playerCustomization[j+1] = (uint8)(customization[i] & 0xff);
                j += 2;
            }
            else
            {
                playerCustomization[j+1] = (uint8)(customization[i] & 0xff);
                playerCustomization[j+2] = (uint8)((customization[i] >> 8) & 0xff);
                j += 3;
            }
        }
    }

    // footer
    playerCustomization[j] = 0xff;
    playerCustomization[j+1] = 0x03;
    playerCustomization[j+2] = '\0';

    setCustomizationStr((int8*)playerCustomization);
}


//=============================================================================
//
//	Make peace with a creature or player.
//
//	This is the prefered method for creatures to handle peace.
//	Remove both themself and defender from each others defender list and eventually clear combat state and pvp status.
//

void CreatureObject::makePeaceWithDefender(uint64 defenderId)
{
    // Atempting a forced peace is no good.

    PlayerObject* attackerPlayer = dynamic_cast<PlayerObject*>(this);

    CreatureObject* defenderCreature = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById(defenderId));
    // assert(defenderCreature)
    PlayerObject* defenderPlayer = NULL;
    if (defenderCreature)
    {
        defenderPlayer = dynamic_cast<PlayerObject*>(defenderCreature);
    }

    if (defenderPlayer)
    {
        if (attackerPlayer)
        {
            // Players do not make peace with other players.
            return;
        }
    }
    else
    {
        return;
    }

    // Remove defender from my list.
    if (defenderCreature)
    {
        this->removeDefenderAndUpdateList(defenderCreature->getId());
    }

    if (defenderPlayer)
    {
        // Update defender about attacker pvp status.
        gMessageLib->sendUpdatePvpStatus(this, defenderPlayer);
    }

    if (this->getDefenders()->size() == 0)
    {
        // I have no more defenders.
        if (attackerPlayer)
        {
            // Update player (my self) with the new status.
            gMessageLib->sendUpdatePvpStatus(this,attackerPlayer);
        }
        else
        {
            // We are a npc, and we have no more defenders (for whatever reason).
            // Inform npc about this event.
            this->inPeace();
        }
        gStateManager.setCurrentActionState(this, CreatureState_Peace);
    }
    else
    {
        // gMessageLib->sendNewDefenderList(this);
    }
    // gMessageLib->sendNewDefenderList(this);


    if (defenderCreature)
    {
        // Remove us from defenders list.
        defenderCreature->removeDefenderAndUpdateList(this->getId());

        if (attackerPlayer)
        {
            // Update attacker about defender pvp status.
            gMessageLib->sendUpdatePvpStatus(defenderCreature, attackerPlayer);
        }

        if (defenderCreature->getDefenders()->size() == 0)
        {
            // He have no more defenders.

            if (defenderPlayer)
            {
                gMessageLib->sendUpdatePvpStatus(defenderCreature,defenderPlayer);
            }
            else
            {
                // We are a npc, and we have no more defenders (for whatever reason).
                // Inform npc about this event.
                defenderCreature->inPeace();
            }
            gStateManager.setCurrentActionState(defenderCreature, CreatureState_Peace);
        }
        else
        {
            //gMessageLib->sendNewDefenderList(defenderCreature);
        }
        // gMessageLib->sendNewDefenderList(defenderCreature);
    }
}




uint32	CreatureObject::UpdatePerformanceCounter()
{
    return (uint32) gWorldManager->GetCurrentGlobalTick();
}

Object* CreatureObject::getTarget() const
{
    return gWorldManager->getObjectById(mTargetId);
}


//=============================================================================
//handles building custom radials
void CreatureObject::prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount)
{

    return;
}

//=============================================================================
//handles the radial selection

void CreatureObject::handleObjectMenuSelect(uint8 messageType,Object* srcObject)
{

    if(PlayerObject* player = dynamic_cast<PlayerObject*>(srcObject))
    {
            
    }
}
//=============================================================================
// maps the incoming posture
//void CreatureObject::setLocomotionByPosture(uint32 posture)
//{
//    switch (posture)
//    {
//        case CreaturePosture_Upright: mLocomotion = CreatureLocomotion_Standing; break;
//        // not even sure if this is used.
//        case CreaturePosture_Crouched:
//        {
//            if(this->getCurrentSpeed() < this->getBaseAcceleration())
//                mLocomotion = CreatureLocomotion_CrouchWalking;
//            else
//                mLocomotion = CreatureLocomotion_CrouchSneaking;
//            break;
//        }
//        case CreaturePosture_Prone: mLocomotion = CreatureLocomotion_Prone; break;
//        case CreaturePosture_Sneaking: mLocomotion = CreatureLocomotion_Sneaking; break;
//        // is this used?
//        case CreaturePosture_Blocking: mLocomotion = CreatureLocomotion_Blocking; break;
//        // is this used?
//        case CreaturePosture_Climbing:
//        {
//                if(this->getCurrentSpeed() >0)
//                    mLocomotion = CreatureLocomotion_Climbing;
//                else
//                    mLocomotion = CreatureLocomotion_ClimbingStationary;
//                break;
//        }
//        case CreaturePosture_Flying: mLocomotion = CreatureLocomotion_Flying; break;
//        case CreaturePosture_LyingDown:	mLocomotion = CreatureLocomotion_LyingDown; break;
//        case CreaturePosture_Sitting: mLocomotion = CreatureLocomotion_Sitting; break;
//        case CreaturePosture_SkillAnimating: mLocomotion = CreatureLocomotion_SkillAnimating; break;
//        case CreaturePosture_DrivingVehicle: mLocomotion = CreatureLocomotion_DrivingVehicle; break;
//        case CreaturePosture_RidingCreature: mLocomotion = CreatureLocomotion_RidingCreature; break;
//        case CreaturePosture_KnockedDown: mLocomotion = CreatureLocomotion_KnockedDown; break;
//        case CreaturePosture_Incapacitated: mLocomotion = CreatureLocomotion_Incapacitated; break;
//        case CreaturePosture_Dead: mLocomotion = CreatureLocomotion_Dead; break;
//    }
//}
