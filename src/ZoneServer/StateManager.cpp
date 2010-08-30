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
#include "StateManager.h"
#include "PostureEvent.h"
#include "ActionStateEvent.h"
#include "LocomotionStateEvent.h"
#include "MessageLib/MessageLib.h"
#include "Common/EventDispatcher.h"

using ::common::SimpleEvent;
using ::common::EventType;

StateManager::StateManager()
{
    loadActionStateMap();
    loadPostureStateMap();
    loadLocomotionStateMap();
}
StateManager::~StateManager()
{
    mActionStateMap.empty();
    mPostureStateMap.empty();
}

void StateManager::loadActionStateMap()
{
    mActionStateMap.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_Cover, std::unique_ptr<ActionState>(new StateCover())));
    mActionStateMap.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_Combat, std::unique_ptr<ActionState>(new StateCombat())));
    mActionStateMap.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_Peace, std::unique_ptr<ActionState>(new StatePeace())));
    mActionStateMap.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_Aiming, std::unique_ptr<ActionState>(new StateAiming())));
    mActionStateMap.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_Alert, std::unique_ptr<ActionState>(new StateAlert())));
    mActionStateMap.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_Berserk, std::unique_ptr<ActionState>(new StateBerserk())));
    mActionStateMap.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_FeignDeath, std::unique_ptr<ActionState>(new StateFeign())));
    mActionStateMap.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_CombatAttitudeEvasive, std::unique_ptr<ActionState>(new StateCombatEvasive())));
    mActionStateMap.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_CombatAttitudeNormal, std::unique_ptr<ActionState>(new StateCombatNormal())));
    mActionStateMap.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_CombatAttitudeAggressive, std::unique_ptr<ActionState>(new StateCombatAggressive())));
    mActionStateMap.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_Tumbling, std::unique_ptr<ActionState>(new StateTumbling())));
    mActionStateMap.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_Rallied, std::unique_ptr<ActionState>(new StateRallied())));
    mActionStateMap.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_Stunned, std::unique_ptr<ActionState>(new StateStunned())));
    mActionStateMap.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_Blinded, std::unique_ptr<ActionState>(new StateBlinded())));
    mActionStateMap.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_Dizzy, std::unique_ptr<ActionState>(new StateDizzy())));
    mActionStateMap.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_Intimidated, std::unique_ptr<ActionState>(new StateIntimidated())));
    mActionStateMap.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_Immobilized, std::unique_ptr<ActionState>(new StateImmobolized())));
    mActionStateMap.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_Frozen, std::unique_ptr<ActionState>(new StateFrozen())));
    mActionStateMap.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_Swimming, std::unique_ptr<ActionState>(new StateSwimming())));
    mActionStateMap.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_SittingOnChair, std::unique_ptr<ActionState>(new StateSittingOnChair())));
    mActionStateMap.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_Crafting, std::unique_ptr<ActionState>(new StateCrafting())));
    mActionStateMap.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_GlowingJedi, std::unique_ptr<ActionState>(new StateGlowingJedi())));
    mActionStateMap.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_MaskScent, std::unique_ptr<ActionState>(new StateMaskScent())));
    mActionStateMap.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_Poisoned, std::unique_ptr<ActionState>(new StatePoisoined())));
    mActionStateMap.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_Bleeding, std::unique_ptr<ActionState>(new StateBleeding())));
    mActionStateMap.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_Diseased, std::unique_ptr<ActionState>(new StateDiseased())));
    mActionStateMap.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_OnFire, std::unique_ptr<ActionState>(new StateOnFire())));
    mActionStateMap.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_RidingMount, std::unique_ptr<ActionState>(new StateRidingMount())));
    mActionStateMap.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_MountedCreature, std::unique_ptr<ActionState>(new StateMountedCreature())));
    mActionStateMap.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_PilotingShip, std::unique_ptr<ActionState>(new StatePilotingShip())));
    mActionStateMap.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_ShipOperations, std::unique_ptr<ActionState>(new StateShipOperations())));
    mActionStateMap.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_ShipGunner, std::unique_ptr<ActionState>(new StateShipGunner())));
    mActionStateMap.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_ShipOperations, std::unique_ptr<ActionState>(new StateShipInterior())));
    mActionStateMap.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_PilotingShip, std::unique_ptr<ActionState>(new StatePilotingPobShip())));
}
void StateManager::loadPostureStateMap()
{
    mPostureStateMap.insert(std::make_pair<uint64, std::unique_ptr<PostureState>>(CreaturePosture_Upright, std::unique_ptr<PostureState>(new PostureUpright())));
    mPostureStateMap.insert(std::make_pair<uint64, std::unique_ptr<PostureState>>(CreaturePosture_Crouched, std::unique_ptr<PostureState>(new PostureCrouched())));
    mPostureStateMap.insert(std::make_pair<uint64, std::unique_ptr<PostureState>>(CreaturePosture_Prone, std::unique_ptr<PostureState>(new PostureProne())));
    mPostureStateMap.insert(std::make_pair<uint64, std::unique_ptr<PostureState>>(CreaturePosture_Sneaking, std::unique_ptr<PostureState>(new PostureSneaking())));
    mPostureStateMap.insert(std::make_pair<uint64, std::unique_ptr<PostureState>>(CreaturePosture_Blocking, std::unique_ptr<PostureState>(new PostureBlocking())));
    mPostureStateMap.insert(std::make_pair<uint64, std::unique_ptr<PostureState>>(CreaturePosture_Climbing, std::unique_ptr<PostureState>(new PostureClimbing())));
    mPostureStateMap.insert(std::make_pair<uint64, std::unique_ptr<PostureState>>(CreaturePosture_Flying, std::unique_ptr<PostureState>(new PostureFlying())));
    mPostureStateMap.insert(std::make_pair<uint64, std::unique_ptr<PostureState>>(CreaturePosture_LyingDown, std::unique_ptr<PostureState>(new PostureLyingDown())));
    mPostureStateMap.insert(std::make_pair<uint64, std::unique_ptr<PostureState>>(CreaturePosture_Sitting, std::unique_ptr<PostureState>(new PostureSitting())));
    mPostureStateMap.insert(std::make_pair<uint64, std::unique_ptr<PostureState>>(CreaturePosture_SkillAnimating, std::unique_ptr<PostureState>(new PostureSkillAnimating())));
    mPostureStateMap.insert(std::make_pair<uint64, std::unique_ptr<PostureState>>(CreaturePosture_DrivingVehicle, std::unique_ptr<PostureState>(new PostureDrivingVehicle())));
    mPostureStateMap.insert(std::make_pair<uint64, std::unique_ptr<PostureState>>(CreaturePosture_RidingCreature, std::unique_ptr<PostureState>(new PostureRidingCreature())));
    mPostureStateMap.insert(std::make_pair<uint64, std::unique_ptr<PostureState>>(CreaturePosture_KnockedDown, std::unique_ptr<PostureState>(new PostureKnockedDown())));
    mPostureStateMap.insert(std::make_pair<uint64, std::unique_ptr<PostureState>>(CreaturePosture_Incapacitated, std::unique_ptr<PostureState>(new PostureIncapacitated())));
    mPostureStateMap.insert(std::make_pair<uint64, std::unique_ptr<PostureState>>(CreaturePosture_Dead, std::unique_ptr<PostureState>(new PostureDead())));
}

void StateManager::loadLocomotionStateMap()
{
    mLocomotionStateMap.insert(std::make_pair<int, std::unique_ptr<LocomotionState>>(CreatureLocomotion_Standing, std::unique_ptr<LocomotionState>(new LocomotionStanding())));
    mLocomotionStateMap.insert(std::make_pair<int, std::unique_ptr<LocomotionState>>(CreatureLocomotion_Sneaking, std::unique_ptr<LocomotionState>(new LocomotionSneaking())));
    mLocomotionStateMap.insert(std::make_pair<int, std::unique_ptr<LocomotionState>>(CreatureLocomotion_Walking, std::unique_ptr<LocomotionState>(new LocomotionWalking())));
    mLocomotionStateMap.insert(std::make_pair<int, std::unique_ptr<LocomotionState>>(CreatureLocomotion_Running, std::unique_ptr<LocomotionState>(new LocomotionRunning())));
    mLocomotionStateMap.insert(std::make_pair<int, std::unique_ptr<LocomotionState>>(CreatureLocomotion_Kneeling, std::unique_ptr<LocomotionState>(new LocomotionKneeling())));
    mLocomotionStateMap.insert(std::make_pair<int, std::unique_ptr<LocomotionState>>(CreatureLocomotion_CrouchSneaking, std::unique_ptr<LocomotionState>(new LocomotionCrouchSneaking())));
    mLocomotionStateMap.insert(std::make_pair<int, std::unique_ptr<LocomotionState>>(CreatureLocomotion_CrouchWalking, std::unique_ptr<LocomotionState>(new LocomotionCrouchWalking())));
    mLocomotionStateMap.insert(std::make_pair<int, std::unique_ptr<LocomotionState>>(CreatureLocomotion_Prone, std::unique_ptr<LocomotionState>(new LocomotionProne())));
    mLocomotionStateMap.insert(std::make_pair<int, std::unique_ptr<LocomotionState>>(CreatureLocomotion_Crawling, std::unique_ptr<LocomotionState>(new LocomotionCrawling())));
    mLocomotionStateMap.insert(std::make_pair<int, std::unique_ptr<LocomotionState>>(CreatureLocomotion_ClimbingStationary, std::unique_ptr<LocomotionState>(new LocomotionClimbingStationary())));
    mLocomotionStateMap.insert(std::make_pair<int, std::unique_ptr<LocomotionState>>(CreatureLocomotion_Climbing, std::unique_ptr<LocomotionState>(new LocomotionClimbing())));
    mLocomotionStateMap.insert(std::make_pair<int, std::unique_ptr<LocomotionState>>(CreatureLocomotion_Hovering, std::unique_ptr<LocomotionState>(new LocomotionHovering())));
    mLocomotionStateMap.insert(std::make_pair<int, std::unique_ptr<LocomotionState>>(CreatureLocomotion_Flying, std::unique_ptr<LocomotionState>(new LocomotionFlying())));
    mLocomotionStateMap.insert(std::make_pair<int, std::unique_ptr<LocomotionState>>(CreatureLocomotion_LyingDown, std::unique_ptr<LocomotionState>(new LocomotionLyingDown())));
    mLocomotionStateMap.insert(std::make_pair<int, std::unique_ptr<LocomotionState>>(CreatureLocomotion_Sitting, std::unique_ptr<LocomotionState>(new LocomotionSitting())));
    mLocomotionStateMap.insert(std::make_pair<int, std::unique_ptr<LocomotionState>>(CreatureLocomotion_SkillAnimating, std::unique_ptr<LocomotionState>(new LocomotionSkillAnimating())));
    mLocomotionStateMap.insert(std::make_pair<int, std::unique_ptr<LocomotionState>>(CreatureLocomotion_DrivingVehicle, std::unique_ptr<LocomotionState>(new LocomotionDrivingVehicle())));
    mLocomotionStateMap.insert(std::make_pair<int, std::unique_ptr<LocomotionState>>(CreatureLocomotion_RidingCreature, std::unique_ptr<LocomotionState>(new LocomotionRidingCreature())));
    mLocomotionStateMap.insert(std::make_pair<int, std::unique_ptr<LocomotionState>>(CreatureLocomotion_KnockedDown, std::unique_ptr<LocomotionState>(new LocomotionKnockedDown())));
    mLocomotionStateMap.insert(std::make_pair<int, std::unique_ptr<LocomotionState>>(CreatureLocomotion_Incapacitated, std::unique_ptr<LocomotionState>(new LocomotionIncapacitated())));
    mLocomotionStateMap.insert(std::make_pair<int, std::unique_ptr<LocomotionState>>(CreatureLocomotion_Dead, std::unique_ptr<LocomotionState>(new LocomotionDead())));
    mLocomotionStateMap.insert(std::make_pair<int, std::unique_ptr<LocomotionState>>(CreatureLocomotion_Blocking, std::unique_ptr<LocomotionState>(new LocomotionBlocking())));
}

void StateManager::setCurrentPostureState(CreatureObject* object, CreaturePosture newPosture)
{
    auto posture_update_event = std::make_shared<PostureUpdateEvent>(object, (CreaturePosture)object->getPosture(), newPosture);

    PostureStateMap::iterator iter = mPostureStateMap.find(newPosture);
    if (iter != mPostureStateMap.end())
    {
        if (mPostureStateMap[object->getPosture()]->CanTransition(object, newPosture))
        {
            // EXIT OLD STATE
            mPostureStateMap[object->getPosture()]->Exit(object);

            // ENTER NEW STATE
            mPostureStateMap[newPosture]->Enter(object);
        }
    } 
    gEventDispatcher.Notify(posture_update_event);
}


void StateManager::setCurrentActionState(CreatureObject* object, CreatureState newState)
{    
    auto action_update_event = std::make_shared<ActionStateUpdateEvent>(object, object->getState(), newState);
    ActionStateMap::iterator iter = mActionStateMap.find(newState);
    if (iter != mActionStateMap.end())
    {
        // check if we can transition to the new state
        if (mActionStateMap[newState]->CanTransition(object, newState))
        {
            // Exit old State
            //currState->Exit(object);
            // Enter new State
            mActionStateMap[newState]->Enter(object);
        }
    }
    gEventDispatcher.Notify(action_update_event);
}

void StateManager::setCurrentLocomotionState(CreatureObject* object, CreatureLocomotion newLocomotion)
{
    auto locomotion_update_event = std::make_shared<LocomotionStateUpdateEvent>(object, (CreatureLocomotion)object->getLocomotion(), newLocomotion);
    LocomotionStateMap::iterator iter = mLocomotionStateMap.find(newLocomotion);
    if (iter != mLocomotionStateMap.end())
    {
        // check if we can transition to the new state
        if (mActionStateMap[newLocomotion]->CanTransition(object, newLocomotion))
        {
            // Exit old State
            mLocomotionStateMap[object->getLocomotion()]->Exit(object);
            // Enter new State
            mActionStateMap[newLocomotion]->Enter(object);
        }
    }
    gEventDispatcher.Notify(locomotion_update_event);
}

