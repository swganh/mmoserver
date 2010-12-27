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
#include "ActionState.h"
#include "StateManager.h"
#include "Common/EventDispatcher.h"
#include "Common/Event.h"



#define insertAction mTransitionList.insert(std::make_pair(State_Action 
#define insertPosture mTransitionList.insert(std::make_pair(State_Posture
#define insertLocomotion mTransitionList.insert(std::make_pair(State_Locomotion

#define actionMap mStateManager->mActionStateMap
#define postureMap mStateManager->mPostureStateMap
#define locomotionMap mStateManager->mLocomotionStateMap

using ::common::SimpleEvent;
using ::common::EventType;

ActionState::ActionState(StateManager* const sm) : IState(),
    mStateManager(sm){}

void ActionState::Enter(CreatureObject* obj)
{
    obj->states.toggleActionOn(mStateID);
}
void ActionState::Exit(CreatureObject* obj)
{
    obj->states.toggleActionOff(mStateID);
}
bool ActionState::CanTransition(CreatureObject* obj, uint64 newState)
{ 
    // check to see if the layer is blocked
    if (obj->states.blockAction)
        return false;
    transitionList::iterator itPosture    = mTransitionList.find(State_Posture);
    transitionList::iterator itAction     = mTransitionList.find(State_Action);
    transitionList::iterator itLocomotion = mTransitionList.find(State_Locomotion);
    // check each state type
    // transition list is based on which states we can't transition to.
    while (itPosture != mTransitionList.end())
    {
        // are we allowed to transition based on posture?
        CreaturePosture pos = (CreaturePosture)(*itPosture).second;
        if (obj->states.checkPosture(pos))
        {
            return false;
        }
        ++itPosture;
    }
    while (itAction != mTransitionList.end())
    {
        uint64 state = (*itAction).second;
        if (obj->states.checkStates(state))
        {
            return false;
        }
        ++itAction;
    }
    while (itLocomotion != mTransitionList.end())
    {
       uint32 locomotion = (*itLocomotion).second;
        if (obj->states.checkLocomotion(locomotion))
        {
            return false;
        }
        ++itLocomotion;
    }
    return true;
}
void ActionState::insertIntoStateRemovalList(const uint64& state)
{
    mStatesRemovalList.push_back(state);
}
void ActionState::insertIntoTransitionList(const std::pair<StateTypes, uint64>& types)
{
    mTransitionList.insert(types);
}

// StateCover
StateCover::StateCover(StateManager* const sm) : ActionState(sm)
{
    mStateID            = CreatureState_Cover;
    //mClientEffect       = "clienteffect/combat_special_attacker_cover.cef";

    loadCommonLocomotionList(mTransitionList);
    insertLocomotion,CreatureLocomotion_Sitting));
    insertLocomotion,CreatureLocomotion_Sneaking));
    insertLocomotion,CreatureLocomotion_CrouchSneaking));
    insertLocomotion,CreatureLocomotion_Blocking));
   
    loadCommonActionList(mTransitionList);
    insertAction,CreatureState_Cover));
    insertAction,CreatureState_Alert));
    insertAction,CreatureState_Swimming));
    insertAction,CreatureState_MountedCreature));
    insertAction,CreatureState_RidingMount));
}

// State Combat
StateCombat::StateCombat(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_Combat;

    insertLocomotion,CreatureLocomotion_CrouchSneaking));
    insertLocomotion,CreatureLocomotion_Climbing));
    insertLocomotion,CreatureLocomotion_ClimbingStationary));
    insertLocomotion,CreatureLocomotion_Hovering));
    insertLocomotion,CreatureLocomotion_Flying));
    insertLocomotion,CreatureLocomotion_LyingDown));
    insertLocomotion,CreatureLocomotion_Sitting));
    insertLocomotion,CreatureLocomotion_SkillAnimating));
    insertLocomotion,CreatureLocomotion_RidingCreature));
    insertLocomotion,CreatureLocomotion_KnockedDown));
    insertLocomotion,CreatureLocomotion_Blocking));

    loadCommonActionList(mTransitionList);
    insertAction,CreatureState_Alert));
    insertAction,CreatureState_Swimming));
    insertAction,CreatureState_MountedCreature));
    insertAction,CreatureState_RidingMount));

    insertPosture,CreaturePosture_Incapacitated));
    insertPosture,CreaturePosture_Dead));
}
void StateCombat::Enter(CreatureObject* obj)
{
    actionMap[CreatureState_Peace]->Exit(obj);

    obj->states.toggleActionOn(mStateID);
}

// State Peace
StatePeace::StatePeace(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_Peace;

    insertPosture,CreaturePosture_Incapacitated));
    insertLocomotion,CreatureLocomotion_Incapacitated));

    insertAction,CreatureState_Berserk));
    insertAction,CreatureState_Immobilized));
    insertAction,CreatureState_GlowingJedi));
}
void StatePeace::Enter(CreatureObject* obj)
{

    actionMap[CreatureState_Combat]->Exit(obj);
    
    obj->states.toggleActionOn(mStateID);

    // turn it back off after 5 seconds
    // yay for Lambdas
    std::shared_ptr<SimpleEvent> start_peace_delay_event = std::make_shared<SimpleEvent>(EventType("start_peace"),0, 5000, [=] 
    {
         obj->states.toggleActionOff(CreatureState_Peace); 
         gMessageLib->sendPostureAndStateUpdate(obj);
    });
    gEventDispatcher.Notify(start_peace_delay_event);
}

// State Aiming
StateAiming::StateAiming(StateManager* const sm) : ActionState(sm)
{
    mStateID            = CreatureState_Aiming;

    loadCommonLocomotionList(mTransitionList);
    insertLocomotion,CreatureLocomotion_Sitting));
    insertLocomotion,CreatureLocomotion_Blocking));

    loadCommonActionList(mTransitionList);
    insertAction,CreatureState_Swimming));
    
}
// State Alert
StateAlert::StateAlert(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_Alert;
    
    insertLocomotion,CreatureLocomotion_Blocking));
    insertLocomotion,CreatureLocomotion_Incapacitated));
    insertLocomotion,CreatureLocomotion_Dead));

    loadCommonActionList(mTransitionList);
    insertAction,CreatureState_Swimming));
}
// Berserk State
StateBerserk::StateBerserk(StateManager* const sm) : ActionState(sm)
{
    mStateID            = CreatureState_Berserk;

    loadCommonLocomotionList(mTransitionList);
    insertLocomotion,CreatureLocomotion_Sitting));
    insertLocomotion,CreatureLocomotion_Sneaking));
    insertLocomotion,CreatureLocomotion_CrouchSneaking));
    insertLocomotion,CreatureLocomotion_Crawling));
    insertLocomotion,CreatureLocomotion_Blocking));

    loadCommonActionList(mTransitionList);
    insertAction,CreatureState_Alert));
    insertAction,CreatureState_Berserk));
    insertAction,CreatureState_Tumbling));
    insertAction,CreatureState_Swimming));
    
}
// Feign State
StateFeign::StateFeign(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_FeignDeath;

    loadCommonLocomotionList(mTransitionList);
    insertLocomotion,CreatureLocomotion_Sitting));
    insertLocomotion,CreatureLocomotion_Sneaking));
    insertLocomotion,CreatureLocomotion_Walking));
    insertLocomotion,CreatureLocomotion_Running));
    insertLocomotion,CreatureLocomotion_CrouchSneaking));
    insertLocomotion,CreatureLocomotion_CrouchWalking));
    insertLocomotion,CreatureLocomotion_Blocking));

    loadCommonActionList(mTransitionList);
    insertAction,CreatureState_Alert));
    insertAction,CreatureState_Tumbling));
    insertAction,CreatureState_Swimming));

}
// Combat Attitude Evasive State
StateCombatEvasive::StateCombatEvasive(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_CombatAttitudeEvasive;

    insertLocomotion,CreatureLocomotion_Incapacitated));
    insertLocomotion,CreatureLocomotion_Dead));
    insertLocomotion,CreatureLocomotion_Blocking));

    loadCommonActionList(mTransitionList);
}
// Combat Attitude Normal State
StateCombatNormal::StateCombatNormal(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_CombatAttitudeNormal;

    insertLocomotion,CreatureLocomotion_Incapacitated));
    insertLocomotion,CreatureLocomotion_Dead));
    insertLocomotion,CreatureLocomotion_Blocking));

    loadCommonActionList(mTransitionList);
}
// Combat Attitude Aggressive State
StateCombatAggressive::StateCombatAggressive(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_CombatAttitudeAggressive;

    insertLocomotion,CreatureLocomotion_Incapacitated));
    insertLocomotion,CreatureLocomotion_Dead));
    insertLocomotion,CreatureLocomotion_Blocking));

    loadCommonActionList(mTransitionList);
}
// Tumbling State
StateTumbling::StateTumbling(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_Tumbling;

    loadCommonLocomotionList(mTransitionList);
    insertLocomotion,CreatureLocomotion_Sitting));
    insertLocomotion,CreatureLocomotion_Sneaking));

    insertAction,CreatureState_Cover));
    insertAction,CreatureState_Alert));
    insertAction,CreatureState_Swimming));
    insertAction,CreatureState_Frozen));
    insertAction,CreatureState_GlowingJedi));
}
// Rallied State
StateRallied::StateRallied(StateManager* const sm) : ActionState(sm)
{
    mStateID      = CreatureState_Rallied;
    //mClientEffect = "clienteffect/combat_special_defender_rally.cef";

    loadCommonLocomotionList(mTransitionList);
    insertLocomotion,CreatureLocomotion_Sitting));
    insertLocomotion,CreatureLocomotion_Blocking));
}
// Stunned State
StateStunned::StateStunned(StateManager* const sm) : ActionState(sm)
{
    mStateID        = CreatureState_Stunned;
    //mClientEffect   = "clienteffect/combat_special_defender_stun.cef";

    insertLocomotion,CreatureLocomotion_Dead));
    insertLocomotion,CreatureLocomotion_Incapacitated));
    insertLocomotion,CreatureLocomotion_Blocking));

    loadCommonActionList(mTransitionList);
}
// Blinded State
StateBlinded::StateBlinded(StateManager* const sm) : ActionState(sm)
{
    mStateID        = CreatureState_Blinded;
    //mClientEffect   = "clienteffect/combat_special_defender_blind.cef";

    insertLocomotion,CreatureLocomotion_Dead));
    insertLocomotion,CreatureLocomotion_Incapacitated));
    insertLocomotion,CreatureLocomotion_Blocking));

    loadCommonActionList(mTransitionList);
}
// Dizzy State
StateDizzy::StateDizzy(StateManager* const sm) : ActionState(sm)
{
    mStateID        = CreatureState_Dizzy;
    //mClientEffect   = "clienteffect/combat_special_defender_dizzy.cef";
    
    insertLocomotion,CreatureLocomotion_Dead));
    insertLocomotion,CreatureLocomotion_Incapacitated));
    insertLocomotion,CreatureLocomotion_Blocking));

    loadCommonActionList(mTransitionList);
}
// Intimidated State
StateIntimidated::StateIntimidated(StateManager* const sm) : ActionState(sm)
{
    mStateID        = CreatureState_Intimidated;
    //mClientEffect   = "clienteffect/combat_special_defender_intimidate.cef";
    //349, clienteffect/combat_special_attacker_intimidate.cef
    
    insertLocomotion,CreatureLocomotion_Dead));
    insertLocomotion,CreatureLocomotion_Incapacitated));
    insertLocomotion,CreatureLocomotion_Blocking));

    loadCommonActionList(mTransitionList);
}
// Immobolized State
StateImmobolized::StateImmobolized(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_Immobilized;

    insertLocomotion,CreatureLocomotion_Dead));
    
    loadCommonActionList(mTransitionList);
}
// Frozen State
StateFrozen::StateFrozen(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_Frozen;
    
    insertLocomotion,CreatureLocomotion_Dead));
    
    loadCommonActionList(mTransitionList);
}
// Swimming State
StateSwimming::StateSwimming(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_Swimming;

    loadCommonLocomotionList(mTransitionList);
    insertLocomotion,CreatureLocomotion_Sitting));

    loadCommonActionList(mTransitionList);
}
void StateSwimming::Enter(CreatureObject* obj)
{
    actionMap[CreatureState_OnFire]->Exit(obj);
    obj->states.toggleActionOn(mStateID);
}
// SittingOnaChair State
StateSittingOnChair::StateSittingOnChair(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_SittingOnChair;

    insertLocomotion,CreatureLocomotion_LyingDown));
    insertLocomotion,CreatureLocomotion_Sitting));
    insertLocomotion,CreatureLocomotion_SkillAnimating));
    insertLocomotion,CreatureLocomotion_DrivingVehicle));
    insertLocomotion,CreatureLocomotion_RidingCreature));
    insertLocomotion,CreatureLocomotion_KnockedDown));
    insertLocomotion,CreatureLocomotion_Incapacitated));
    insertLocomotion,CreatureLocomotion_Dead));

    loadCommonActionList(mTransitionList);
    insertAction,CreatureState_Combat));
    insertAction,CreatureState_Aiming));
    insertAction,CreatureState_Berserk));
    insertAction,CreatureState_CombatAttitudeEvasive));
    insertAction,CreatureState_CombatAttitudeNormal));
    insertAction,CreatureState_CombatAttitudeAggressive));
    insertAction,CreatureState_Tumbling));
    insertAction,CreatureState_Rallied));
    insertAction,CreatureState_Stunned));
    insertAction,CreatureState_Blinded));
    insertAction,CreatureState_Dizzy));
    insertAction,CreatureState_Intimidated));
    insertAction,CreatureState_Swimming));
}
// Crafting State
StateCrafting::StateCrafting(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_Crafting;

    insertLocomotion,CreatureLocomotion_Sneaking));
    insertLocomotion,CreatureLocomotion_Hovering));
    insertLocomotion,CreatureLocomotion_Flying));
    insertLocomotion,CreatureLocomotion_SkillAnimating));
    insertLocomotion,CreatureLocomotion_DrivingVehicle));
    insertLocomotion,CreatureLocomotion_RidingCreature));
    insertLocomotion,CreatureLocomotion_KnockedDown));
    insertLocomotion,CreatureLocomotion_Incapacitated));
    insertLocomotion,CreatureLocomotion_Dead));
    insertLocomotion,CreatureLocomotion_Blocking));

    loadCommonActionList(mTransitionList);
    insertAction,CreatureState_Cover));
    insertAction,CreatureState_Combat));
    insertAction,CreatureState_Aiming));
    insertAction,CreatureState_Berserk));
    insertAction,CreatureState_FeignDeath));
    insertAction,CreatureState_CombatAttitudeEvasive));
    insertAction,CreatureState_CombatAttitudeNormal));
    insertAction,CreatureState_CombatAttitudeAggressive));
    insertAction,CreatureState_Tumbling));
    insertAction,CreatureState_Rallied));
    insertAction,CreatureState_Stunned));
    insertAction,CreatureState_Blinded));
    insertAction,CreatureState_Dizzy));
    insertAction,CreatureState_Intimidated));
    insertAction,CreatureState_Swimming));
    insertAction,CreatureState_Crafting));
    insertAction,CreatureState_OnFire));
}
// GlowingJedi State
StateGlowingJedi::StateGlowingJedi(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_GlowingJedi;
}
// Mask Scent State
StateMaskScent::StateMaskScent(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_MaskScent;

    loadCommonLocomotionList(mTransitionList);

    loadCommonActionList(mTransitionList);
    insertAction,CreatureState_Cover));
    insertAction,CreatureState_Combat));
    insertAction,CreatureState_Aiming));
    insertAction,CreatureState_Alert));
    insertAction,CreatureState_Berserk));
    insertAction,CreatureState_FeignDeath));
    insertAction,CreatureState_Tumbling));
    insertAction,CreatureState_Rallied));
    insertAction,CreatureState_Stunned));
    insertAction,CreatureState_Blinded));
    insertAction,CreatureState_Dizzy));
    insertAction,CreatureState_Intimidated));
    insertAction,CreatureState_Swimming));
}
// Poisoned State
StatePoisoined::StatePoisoined(StateManager* const sm) : ActionState(sm)
{
    mStateID        = CreatureState_Poisoned;
    //mClientEffect   = "clienteffect/dot_poisoned.cef";

    insertLocomotion,CreatureLocomotion_Dead));
    insertLocomotion,CreatureLocomotion_Incapacitated));
    insertLocomotion,CreatureLocomotion_Blocking));

    loadCommonActionList(mTransitionList);
}
// Bleeding State
StateBleeding::StateBleeding(StateManager* const sm) : ActionState(sm)
{
    mStateID        = CreatureState_Bleeding;
    //mClientEffect   = "clienteffect/dot_bleeding.cef";

    insertLocomotion,CreatureLocomotion_Dead));
    insertLocomotion,CreatureLocomotion_Incapacitated));
    insertLocomotion,CreatureLocomotion_Blocking));

    loadCommonActionList(mTransitionList);
}
// Diseased State
StateDiseased::StateDiseased(StateManager* const sm) : ActionState(sm)
{
    mStateID        = CreatureState_Diseased;
    //mClientEffect   = "clienteffect/dot_diseased.cef";

    insertLocomotion,CreatureLocomotion_Dead));
    insertLocomotion,CreatureLocomotion_Incapacitated));
    insertLocomotion,CreatureLocomotion_Blocking));

    loadCommonActionList(mTransitionList);
}
// OnFire State
StateOnFire::StateOnFire(StateManager* const sm) : ActionState(sm)
{
    mStateID        = CreatureState_OnFire;
    //mClientEffect   = "clienteffect/dot_fire.cef";

    insertLocomotion,CreatureLocomotion_Dead));
    insertLocomotion,CreatureLocomotion_Incapacitated));
    insertLocomotion,CreatureLocomotion_Blocking));

    loadCommonActionList(mTransitionList);
}
//void StateOnFire::Enter(CreatureObject* obj)
//{
//    actionMap[CreatureState_Peace]->Exit(obj);
//}
// RidingMount State
StateRidingMount::StateRidingMount(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_RidingMount;

    insertLocomotion,CreatureLocomotion_Dead));
    insertLocomotion,CreatureLocomotion_Incapacitated));
    insertLocomotion,CreatureLocomotion_Blocking));
    insertAction,CreatureState_MountedCreature));

    loadCommonActionList(mTransitionList);
}
// MountedCreature State
StateMountedCreature::StateMountedCreature(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_MountedCreature;

    insertLocomotion,CreatureLocomotion_Dead));
    insertLocomotion,CreatureLocomotion_Incapacitated));
    insertLocomotion,CreatureLocomotion_Blocking));
    insertAction,CreatureState_RidingMount));

    loadCommonActionList(mTransitionList);
}
// PilotingShip State
StatePilotingShip::StatePilotingShip(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_PilotingShip;

    insertLocomotion,CreatureLocomotion_Dead));
    insertLocomotion,CreatureLocomotion_Incapacitated));
    insertLocomotion,CreatureLocomotion_Blocking));

    loadCommonActionList(mTransitionList);
}
// ShipOperations State
StateShipOperations::StateShipOperations(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_ShipOperations;

    insertLocomotion,CreatureLocomotion_Dead));
    insertLocomotion,CreatureLocomotion_Incapacitated));
    insertLocomotion,CreatureLocomotion_Blocking));

    loadCommonActionList(mTransitionList);
}
// ShipGunner State
StateShipGunner::StateShipGunner(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_ShipGunner;

    insertLocomotion,CreatureLocomotion_Dead));
    insertLocomotion,CreatureLocomotion_Incapacitated));
    insertLocomotion,CreatureLocomotion_Blocking));

    loadCommonActionList(mTransitionList);
}
// ShipInterior State
StateShipInterior::StateShipInterior(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_ShipOperations;

    insertLocomotion,CreatureLocomotion_Dead));
    insertLocomotion,CreatureLocomotion_Incapacitated));
    insertLocomotion,CreatureLocomotion_Blocking));

    loadCommonActionList(mTransitionList);
}
// PilotingPobShip State
StatePilotingPobShip::StatePilotingPobShip(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_PilotingShip;

    insertLocomotion,CreatureLocomotion_Dead));
    insertLocomotion,CreatureLocomotion_Incapacitated));
    insertLocomotion,CreatureLocomotion_Blocking));

    loadCommonActionList(mTransitionList);
}
// StateClear State
StateClear::StateClear(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_ClearState;
}
void StateClear::Enter(CreatureObject* obj)
{
    obj->states.unblock();
    obj->states.clearAllStates();
}