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


#ifdef ERROR
#undef ERROR
#endif

#include "Utils/logger.h"

using ::common::EventType;

StateManager::StateManager()
{}
StateManager::~StateManager()
{
    mActionStateMap.empty();
    mPostureStateMap.empty();
    mLocomotionStateMap.empty();
}

void StateManager::loadActionStateMap()
{
    mActionStateMap.insert(std::make_pair<CreatureState, std::shared_ptr<ActionState>>(CreatureState_Cover, std::shared_ptr<ActionState>(new StateCover(this))));
    mActionStateMap.insert(std::make_pair<CreatureState, std::shared_ptr<ActionState>>(CreatureState_Combat, std::shared_ptr<ActionState>(new StateCombat(this))));
    mActionStateMap.insert(std::make_pair<CreatureState, std::shared_ptr<ActionState>>(CreatureState_Peace, std::shared_ptr<ActionState>(new StatePeace(this))));
    mActionStateMap.insert(std::make_pair<CreatureState, std::shared_ptr<ActionState>>(CreatureState_Aiming, std::shared_ptr<ActionState>(new StateAiming(this))));
    mActionStateMap.insert(std::make_pair<CreatureState, std::shared_ptr<ActionState>>(CreatureState_Alert, std::shared_ptr<ActionState>(new StateAlert(this))));
    mActionStateMap.insert(std::make_pair<CreatureState, std::shared_ptr<ActionState>>(CreatureState_Berserk, std::shared_ptr<ActionState>(new StateBerserk(this))));
    mActionStateMap.insert(std::make_pair<CreatureState, std::shared_ptr<ActionState>>(CreatureState_FeignDeath, std::shared_ptr<ActionState>(new StateFeign(this))));
    mActionStateMap.insert(std::make_pair<CreatureState, std::shared_ptr<ActionState>>(CreatureState_CombatAttitudeEvasive, std::shared_ptr<ActionState>(new StateCombatEvasive(this))));
    mActionStateMap.insert(std::make_pair<CreatureState, std::shared_ptr<ActionState>>(CreatureState_CombatAttitudeNormal, std::shared_ptr<ActionState>(new StateCombatNormal(this))));
    mActionStateMap.insert(std::make_pair<CreatureState, std::shared_ptr<ActionState>>(CreatureState_CombatAttitudeAggressive, std::shared_ptr<ActionState>(new StateCombatAggressive(this))));
    mActionStateMap.insert(std::make_pair<CreatureState, std::shared_ptr<ActionState>>(CreatureState_Tumbling, std::shared_ptr<ActionState>(new StateTumbling(this))));
    mActionStateMap.insert(std::make_pair<CreatureState, std::shared_ptr<ActionState>>(CreatureState_Rallied, std::shared_ptr<ActionState>(new StateRallied(this))));
    mActionStateMap.insert(std::make_pair<CreatureState, std::shared_ptr<ActionState>>(CreatureState_Stunned, std::shared_ptr<ActionState>(new StateStunned(this))));
    mActionStateMap.insert(std::make_pair<CreatureState, std::shared_ptr<ActionState>>(CreatureState_Blinded, std::shared_ptr<ActionState>(new StateBlinded(this))));
    mActionStateMap.insert(std::make_pair<CreatureState, std::shared_ptr<ActionState>>(CreatureState_Dizzy, std::shared_ptr<ActionState>(new StateDizzy(this))));
    mActionStateMap.insert(std::make_pair<CreatureState, std::shared_ptr<ActionState>>(CreatureState_Intimidated, std::shared_ptr<ActionState>(new StateIntimidated(this))));
    mActionStateMap.insert(std::make_pair<CreatureState, std::shared_ptr<ActionState>>(CreatureState_Immobilized, std::shared_ptr<ActionState>(new StateImmobolized(this))));
    mActionStateMap.insert(std::make_pair<CreatureState, std::shared_ptr<ActionState>>(CreatureState_Frozen, std::shared_ptr<ActionState>(new StateFrozen(this))));
    mActionStateMap.insert(std::make_pair<CreatureState, std::shared_ptr<ActionState>>(CreatureState_Swimming, std::shared_ptr<ActionState>(new StateSwimming(this))));
    mActionStateMap.insert(std::make_pair<CreatureState, std::shared_ptr<ActionState>>(CreatureState_SittingOnChair, std::shared_ptr<ActionState>(new StateSittingOnChair(this))));
    mActionStateMap.insert(std::make_pair<CreatureState, std::shared_ptr<ActionState>>(CreatureState_Crafting, std::shared_ptr<ActionState>(new StateCrafting(this))));
    mActionStateMap.insert(std::make_pair<CreatureState, std::shared_ptr<ActionState>>(CreatureState_GlowingJedi, std::shared_ptr<ActionState>(new StateGlowingJedi(this))));
    mActionStateMap.insert(std::make_pair<CreatureState, std::shared_ptr<ActionState>>(CreatureState_MaskScent, std::shared_ptr<ActionState>(new StateMaskScent(this))));
    mActionStateMap.insert(std::make_pair<CreatureState, std::shared_ptr<ActionState>>(CreatureState_Poisoned, std::shared_ptr<ActionState>(new StatePoisoined(this))));
    mActionStateMap.insert(std::make_pair<CreatureState, std::shared_ptr<ActionState>>(CreatureState_Bleeding, std::shared_ptr<ActionState>(new StateBleeding(this))));
    mActionStateMap.insert(std::make_pair<CreatureState, std::shared_ptr<ActionState>>(CreatureState_Diseased, std::shared_ptr<ActionState>(new StateDiseased(this))));
    mActionStateMap.insert(std::make_pair<CreatureState, std::shared_ptr<ActionState>>(CreatureState_OnFire, std::shared_ptr<ActionState>(new StateOnFire(this))));
    mActionStateMap.insert(std::make_pair<CreatureState, std::shared_ptr<ActionState>>(CreatureState_RidingMount, std::shared_ptr<ActionState>(new StateRidingMount(this))));
    mActionStateMap.insert(std::make_pair<CreatureState, std::shared_ptr<ActionState>>(CreatureState_MountedCreature, std::shared_ptr<ActionState>(new StateMountedCreature(this))));
    mActionStateMap.insert(std::make_pair<CreatureState, std::shared_ptr<ActionState>>(CreatureState_PilotingShip, std::shared_ptr<ActionState>(new StatePilotingShip(this))));
    mActionStateMap.insert(std::make_pair<CreatureState, std::shared_ptr<ActionState>>(CreatureState_ShipOperations, std::shared_ptr<ActionState>(new StateShipOperations(this))));
    mActionStateMap.insert(std::make_pair<CreatureState, std::shared_ptr<ActionState>>(CreatureState_ShipGunner, std::shared_ptr<ActionState>(new StateShipGunner(this))));
    mActionStateMap.insert(std::make_pair<CreatureState, std::shared_ptr<ActionState>>(CreatureState_ShipOperations, std::shared_ptr<ActionState>(new StateShipInterior(this))));
    mActionStateMap.insert(std::make_pair<CreatureState, std::shared_ptr<ActionState>>(CreatureState_PilotingPobShip, std::shared_ptr<ActionState>(new StatePilotingPobShip(this))));
    mActionStateMap.insert(std::make_pair<CreatureState, std::shared_ptr<ActionState>>(CreatureState_ClearState, std::shared_ptr<ActionState>(new StateClear(this))));
}
void StateManager::loadPostureStateMap()
{
    mPostureStateMap.insert(std::make_pair<CreaturePosture, std::shared_ptr<PostureState>>(CreaturePosture_Upright, std::shared_ptr<PostureState>(new PostureUpright(this))));
    mPostureStateMap.insert(std::make_pair<CreaturePosture, std::shared_ptr<PostureState>>(CreaturePosture_Crouched, std::shared_ptr<PostureState>(new PostureCrouched(this))));
    mPostureStateMap.insert(std::make_pair<CreaturePosture, std::shared_ptr<PostureState>>(CreaturePosture_Prone, std::shared_ptr<PostureState>(new PostureProne(this))));
    mPostureStateMap.insert(std::make_pair<CreaturePosture, std::shared_ptr<PostureState>>(CreaturePosture_Sneaking, std::shared_ptr<PostureState>(new PostureSneaking(this))));
    mPostureStateMap.insert(std::make_pair<CreaturePosture, std::shared_ptr<PostureState>>(CreaturePosture_Blocking, std::shared_ptr<PostureState>(new PostureBlocking(this))));
    mPostureStateMap.insert(std::make_pair<CreaturePosture, std::shared_ptr<PostureState>>(CreaturePosture_Climbing, std::shared_ptr<PostureState>(new PostureClimbing(this))));
    mPostureStateMap.insert(std::make_pair<CreaturePosture, std::shared_ptr<PostureState>>(CreaturePosture_Flying, std::shared_ptr<PostureState>(new PostureFlying(this))));
    mPostureStateMap.insert(std::make_pair<CreaturePosture, std::shared_ptr<PostureState>>(CreaturePosture_LyingDown, std::shared_ptr<PostureState>(new PostureLyingDown(this))));
    mPostureStateMap.insert(std::make_pair<CreaturePosture, std::shared_ptr<PostureState>>(CreaturePosture_Sitting, std::shared_ptr<PostureState>(new PostureSitting(this))));
    mPostureStateMap.insert(std::make_pair<CreaturePosture, std::shared_ptr<PostureState>>(CreaturePosture_SkillAnimating, std::shared_ptr<PostureState>(new PostureSkillAnimating(this))));
    mPostureStateMap.insert(std::make_pair<CreaturePosture, std::shared_ptr<PostureState>>(CreaturePosture_DrivingVehicle, std::shared_ptr<PostureState>(new PostureDrivingVehicle(this))));
    mPostureStateMap.insert(std::make_pair<CreaturePosture, std::shared_ptr<PostureState>>(CreaturePosture_RidingCreature, std::shared_ptr<PostureState>(new PostureRidingCreature(this))));
    mPostureStateMap.insert(std::make_pair<CreaturePosture, std::shared_ptr<PostureState>>(CreaturePosture_KnockedDown, std::shared_ptr<PostureState>(new PostureKnockedDown(this))));
    mPostureStateMap.insert(std::make_pair<CreaturePosture, std::shared_ptr<PostureState>>(CreaturePosture_Incapacitated, std::shared_ptr<PostureState>(new PostureIncapacitated(this))));
    mPostureStateMap.insert(std::make_pair<CreaturePosture, std::shared_ptr<PostureState>>(CreaturePosture_Dead, std::shared_ptr<PostureState>(new PostureDead(this))));
}

void StateManager::loadLocomotionStateMap()
{
   	mLocomotionStateMap.insert(std::make_pair<CreatureLocomotion, std::shared_ptr<LocomotionState>>(CreatureLocomotion_Standing, std::shared_ptr<LocomotionState>(new LocomotionStanding(this))));
    mLocomotionStateMap.insert(std::make_pair<CreatureLocomotion, std::shared_ptr<LocomotionState>>(CreatureLocomotion_Sneaking, std::shared_ptr<LocomotionState>(new LocomotionSneaking(this))));
    mLocomotionStateMap.insert(std::make_pair<CreatureLocomotion, std::shared_ptr<LocomotionState>>(CreatureLocomotion_Walking, std::shared_ptr<LocomotionState>(new LocomotionWalking(this))));
    mLocomotionStateMap.insert(std::make_pair<CreatureLocomotion, std::shared_ptr<LocomotionState>>(CreatureLocomotion_Running, std::shared_ptr<LocomotionState>(new LocomotionRunning(this))));
    mLocomotionStateMap.insert(std::make_pair<CreatureLocomotion, std::shared_ptr<LocomotionState>>(CreatureLocomotion_Kneeling, std::shared_ptr<LocomotionState>(new LocomotionKneeling(this))));
    mLocomotionStateMap.insert(std::make_pair<CreatureLocomotion, std::shared_ptr<LocomotionState>>(CreatureLocomotion_CrouchSneaking, std::shared_ptr<LocomotionState>(new LocomotionCrouchSneaking(this))));
    mLocomotionStateMap.insert(std::make_pair<CreatureLocomotion, std::shared_ptr<LocomotionState>>(CreatureLocomotion_CrouchWalking, std::shared_ptr<LocomotionState>(new LocomotionCrouchWalking(this))));
    mLocomotionStateMap.insert(std::make_pair<CreatureLocomotion, std::shared_ptr<LocomotionState>>(CreatureLocomotion_Prone, std::shared_ptr<LocomotionState>(new LocomotionProne(this))));
    mLocomotionStateMap.insert(std::make_pair<CreatureLocomotion, std::shared_ptr<LocomotionState>>(CreatureLocomotion_Crawling, std::shared_ptr<LocomotionState>(new LocomotionCrawling(this))));
    mLocomotionStateMap.insert(std::make_pair<CreatureLocomotion, std::shared_ptr<LocomotionState>>(CreatureLocomotion_ClimbingStationary, std::shared_ptr<LocomotionState>(new LocomotionClimbingStationary(this))));
    mLocomotionStateMap.insert(std::make_pair<CreatureLocomotion, std::shared_ptr<LocomotionState>>(CreatureLocomotion_Climbing, std::shared_ptr<LocomotionState>(new LocomotionClimbing(this))));
    mLocomotionStateMap.insert(std::make_pair<CreatureLocomotion, std::shared_ptr<LocomotionState>>(CreatureLocomotion_Hovering, std::shared_ptr<LocomotionState>(new LocomotionHovering(this))));
    mLocomotionStateMap.insert(std::make_pair<CreatureLocomotion, std::shared_ptr<LocomotionState>>(CreatureLocomotion_Flying, std::shared_ptr<LocomotionState>(new LocomotionFlying(this))));
    mLocomotionStateMap.insert(std::make_pair<CreatureLocomotion, std::shared_ptr<LocomotionState>>(CreatureLocomotion_LyingDown, std::shared_ptr<LocomotionState>(new LocomotionLyingDown(this))));
    mLocomotionStateMap.insert(std::make_pair<CreatureLocomotion, std::shared_ptr<LocomotionState>>(CreatureLocomotion_Sitting, std::shared_ptr<LocomotionState>(new LocomotionSitting(this))));
    mLocomotionStateMap.insert(std::make_pair<CreatureLocomotion, std::shared_ptr<LocomotionState>>(CreatureLocomotion_SkillAnimating, std::shared_ptr<LocomotionState>(new LocomotionSkillAnimating(this))));
    mLocomotionStateMap.insert(std::make_pair<CreatureLocomotion, std::shared_ptr<LocomotionState>>(CreatureLocomotion_DrivingVehicle, std::shared_ptr<LocomotionState>(new LocomotionDrivingVehicle(this))));
    mLocomotionStateMap.insert(std::make_pair<CreatureLocomotion, std::shared_ptr<LocomotionState>>(CreatureLocomotion_RidingCreature, std::shared_ptr<LocomotionState>(new LocomotionRidingCreature(this))));
    mLocomotionStateMap.insert(std::make_pair<CreatureLocomotion, std::shared_ptr<LocomotionState>>(CreatureLocomotion_KnockedDown, std::shared_ptr<LocomotionState>(new LocomotionKnockedDown(this))));
    mLocomotionStateMap.insert(std::make_pair<CreatureLocomotion, std::shared_ptr<LocomotionState>>(CreatureLocomotion_Incapacitated, std::shared_ptr<LocomotionState>(new LocomotionIncapacitated(this))));
    mLocomotionStateMap.insert(std::make_pair<CreatureLocomotion, std::shared_ptr<LocomotionState>>(CreatureLocomotion_Dead, std::shared_ptr<LocomotionState>(new LocomotionDead(this))));
    mLocomotionStateMap.insert(std::make_pair<CreatureLocomotion, std::shared_ptr<LocomotionState>>(CreatureLocomotion_Blocking, std::shared_ptr<LocomotionState>(new LocomotionBlocking(this))));
}

void StateManager::setCurrentPostureState(CreatureObject* object, CreaturePosture newPosture)
{
    auto posture_update_event = std::make_shared<PostureUpdateEvent>(object->getId(), (CreaturePosture)object->states.getPosture(), newPosture);

    PostureStateMap::iterator iter = mPostureStateMap.find(newPosture);
    if (iter != mPostureStateMap.end())
    {
        if (mPostureStateMap[object->states.getPosture()]->CanTransition(object, newPosture))
        {
            // EXIT OLD STATE
            mPostureStateMap[object->states.getPosture()]->Exit(object);

            // ENTER NEW STATE
            mPostureStateMap[newPosture]->Enter(object);
        }
        else
        {
            DLOG(WARNING) << "unable to transition from " << object->states.getPosture() << " to" << newPosture;
            gMessageLib->SendSystemMessage(L"You cannot transition from this Posture state");
        }
        gEventDispatcher.Notify(posture_update_event);
    } 
}

void StateManager::setCurrentActionState(CreatureObject* object, CreatureState newState)
{    
    auto action_update_event = std::make_shared<ActionStateUpdateEvent>(object->getId(), object->states.getAction(), newState);
    ActionStateMap::iterator iter = mActionStateMap.find(newState);
    if (iter != mActionStateMap.end())
    {
        // check if we can transition to the new state
        if (mActionStateMap[newState]->CanTransition(object, newState))
        {
            // check which states to remove
            std::vector<uint64>::const_iterator states_it = mActionStateMap[newState]->statesToRemove().begin();
            while(states_it != mActionStateMap[newState]->statesToRemove().end())
            {
                mActionStateMap[*states_it]->Exit(object);
                ++states_it;
            }
            // Enter new State
            mActionStateMap[newState]->Enter(object);
        }
        else
        {
            DLOG(WARNING) << "unable to transition from " << object->states.getAction() << " to" << static_cast<uint64_t>(newState);
            gMessageLib->SendSystemMessage(L"You cannot transition from this Action state");
        }
        gEventDispatcher.Notify(action_update_event);
    }
}

void StateManager::setCurrentLocomotionState(CreatureObject* object, CreatureLocomotion newLocomotion)
{
    auto locomotion_update_event = std::make_shared<LocomotionStateUpdateEvent>(object->getId(), (CreatureLocomotion)object->states.getLocomotion(), newLocomotion);
    LocomotionStateMap::iterator iter = mLocomotionStateMap.find(newLocomotion);
    if (iter != mLocomotionStateMap.end())
    {
        // check if we can transition to the new state
        if (mActionStateMap[newLocomotion]->CanTransition(object, newLocomotion))
        {
            // Exit old State
            mLocomotionStateMap[object->states.getLocomotion()]->Exit(object);
            // Enter new State
            mLocomotionStateMap[newLocomotion]->Enter(object);
        }
        else
        {
            DLOG(WARNING) << "unable to transition from " << object->states.getLocomotion() << " to" << static_cast<uint64_t>(newLocomotion);
            gMessageLib->SendSystemMessage(L"You cannot transition from this Locomotion state");
        }
        gEventDispatcher.Notify(locomotion_update_event);
    }
}
void StateManager::removeActionState(CreatureObject* obj, CreatureState stateToRemove)
{
        auto action_update_event = std::make_shared<ActionStateUpdateEvent>(obj->getId(), obj->states.getAction(), stateToRemove);
        // check if we can transition out of this action state
        if (mActionStateMap[stateToRemove]->CanTransition(obj, stateToRemove))
        {
            // Exit old State
            obj->states.toggleActionOff(stateToRemove);
        }
        else
        {
            DLOG(WARNING) << "unable to remove action state " << static_cast<uint64_t>(stateToRemove);
            gMessageLib->SendSystemMessage(L"You cannot remove this Action state");
        }
        gEventDispatcher.Notify(action_update_event);
}
void StateManager::loadStateMaps()
{
    loadActionStateMap();
    loadPostureStateMap();
    loadLocomotionStateMap();
}