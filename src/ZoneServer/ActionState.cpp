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

ActionState::ActionState(StateManager* const sm) : IState(),
    mStateManager(sm){}

void ActionState::Enter(CreatureObject* obj)
{
    obj->states.toggleActionOn(mStateID);
    mTransitionList.clear();
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
        if (pos == obj->states.getPosture())
        {
            return false;
        }
        ++itPosture;
    }
    while (itAction != mTransitionList.end())
    {
        uint64 state = (*itAction).second;
        if ((state & obj->states.getAction()) != obj->states.getAction())
        {
            return false;
        }
        ++itAction;
    }
    while (itLocomotion != mTransitionList.end())
    {
       uint32 locomotion = (*itLocomotion).second;
        if (locomotion == obj->states.getLocomotion())
        {
            return false;
        }
        ++itLocomotion;
    }
    return true;
}
void ActionState::loadTransitionList()
{}
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
    mStateID = CreatureState_Cover;
}

// State Combat
StateCombat::StateCombat(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_Combat;
}

// State Peace
StatePeace::StatePeace(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_Combat;
}

// State Aiming
StateAiming::StateAiming(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_Aiming;
}
// State Alert
StateAlert::StateAlert(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_Alert;
}
// Berserk State
StateBerserk::StateBerserk(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_Berserk;
}
// Feign State
StateFeign::StateFeign(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_FeignDeath;
}
// Combat Attitude Evasive State
StateCombatEvasive::StateCombatEvasive(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_CombatAttitudeEvasive;
}
// Combat Attitude Normal State
StateCombatNormal::StateCombatNormal(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_CombatAttitudeNormal;
}
// Combat Attitude Aggressive State
StateCombatAggressive::StateCombatAggressive(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_CombatAttitudeAggressive;
}
// Tumbling State
StateTumbling::StateTumbling(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_Tumbling;
}
// Rallied State
StateRallied::StateRallied(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_Rallied;
}
// Stunned State
StateStunned::StateStunned(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_Stunned;
}
// Blinded State
StateBlinded::StateBlinded(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_Blinded;
}
// Dizzy State
StateDizzy::StateDizzy(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_Dizzy;
}
// Intimidated State
StateIntimidated::StateIntimidated(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_Intimidated;
}
// Immobolized State
StateImmobolized::StateImmobolized(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_Immobilized;
}
// Frozen State
StateFrozen::StateFrozen(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_Frozen;
}
// Swimming State
StateSwimming::StateSwimming(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_Swimming;
}
// SittingOnaChair State
StateSittingOnChair::StateSittingOnChair(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_SittingOnChair;
}
// Crafting State
StateCrafting::StateCrafting(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_Crafting;
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
}
// Poisoned State
StatePoisoined::StatePoisoined(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_Poisoned;
}
// Bleeding State
StateBleeding::StateBleeding(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_Bleeding;
}
// Diseased State
StateDiseased::StateDiseased(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_Diseased;
}
// OnFire State
StateOnFire::StateOnFire(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_OnFire;
}
// RidingMount State
StateRidingMount::StateRidingMount(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_RidingMount;
}
// MountedCreature State
StateMountedCreature::StateMountedCreature(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_MountedCreature;
}
// PilotingShip State
StatePilotingShip::StatePilotingShip(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_PilotingShip;
}
// ShipOperations State
StateShipOperations::StateShipOperations(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_ShipOperations;
}
// ShipGunner State
StateShipGunner::StateShipGunner(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_ShipGunner;
}
// ShipInterior State
StateShipInterior::StateShipInterior(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_ShipOperations;
}
// PilotingPobShip State
StatePilotingPobShip::StatePilotingPobShip(StateManager* const sm) : ActionState(sm)
{
    mStateID = CreatureState_PilotingShip;
}