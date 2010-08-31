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

ActionState::ActionState(void)
{
    mBlocked = false;
}

void ActionState::Enter(CreatureObject* obj)
{
    obj->toggleStateOn(mStateID);
    mTransitionList.clear();
}
void ActionState::Exit(CreatureObject* obj)
{
    obj->toggleStateOff(mStateID);
}
bool ActionState::CanTransition(CreatureObject* obj, uint64 newState)
{
    bool transitionPosture    = true;
    bool transitionAction     = true;
    bool transitionLocomotion = true;
    // check to see if the layer is blocked
    if (mBlocked)
        return false;
    transitionList::iterator itPosture    = mTransitionList.find(State_Posture);
    transitionList::iterator itAction     = mTransitionList.find(State_Action);
    transitionList::iterator itLocomotion = mTransitionList.find(State_Locomotion);
    // check each state type
    while (itPosture != mTransitionList.end())
    {
        // are we allowed to transition based on posture?
        CreaturePosture pos = (CreaturePosture)(*itPosture).second;
        if (pos != obj->getPosture())
        {
            transitionPosture = false;
            break;
        }
    }
    while (itAction != mTransitionList.end())
    {
        uint64 state = (*itAction).second;
        if ((state & obj->getState()) == obj->getState())
        {
            transitionAction = false;
            break;
        }
    }
    while (itLocomotion != mTransitionList.end())
    {
        CreatureLocomotion locomotion = (CreatureLocomotion)(*itAction).second;
        if (locomotion != obj->getLocomotion())
        {
            transitionLocomotion = false;
            break;
        }
    }
    return transitionAction && transitionPosture && transitionLocomotion;
}
void ActionState::insertIntoTransitionList(std::pair<StateTypes, uint64> types)
{
    mTransitionList.insert(types);
}

// StateCover
StateCover::StateCover() : ActionState()
{
    mStateID = CreatureState_Cover;
}

// State Combat
StateCombat::StateCombat() : ActionState()
{
    mStateID = CreatureState_Combat;
}

// State Peace
StatePeace::StatePeace() : ActionState()
{
    mStateID = CreatureState_Combat;
}

// State Aiming
StateAiming::StateAiming() : ActionState()
{
    mStateID = CreatureState_Aiming;
}
// State Alert
StateAlert::StateAlert() : ActionState()
{
    mStateID = CreatureState_Alert;
}
// Berserk State
StateBerserk::StateBerserk() : ActionState()
{
    mStateID = CreatureState_Berserk;
}
// Feign State
StateFeign::StateFeign() : ActionState()
{
    mStateID = CreatureState_FeignDeath;
}
// Combat Attitude Evasive State
StateCombatEvasive::StateCombatEvasive() : ActionState()
{
    mStateID = CreatureState_CombatAttitudeEvasive;
}
// Combat Attitude Normal State
StateCombatNormal::StateCombatNormal() : ActionState()
{
    mStateID = CreatureState_CombatAttitudeNormal;
}
// Combat Attitude Aggressive State
StateCombatAggressive::StateCombatAggressive() : ActionState()
{
    mStateID = CreatureState_CombatAttitudeAggressive;
}
// Tumbling State
StateTumbling::StateTumbling() : ActionState()
{
    mStateID = CreatureState_Tumbling;
}
// Rallied State
StateRallied::StateRallied() : ActionState()
{
    mStateID = CreatureState_Rallied;
}
// Stunned State
StateStunned::StateStunned() : ActionState()
{
    mStateID = CreatureState_Stunned;
}
// Blinded State
StateBlinded::StateBlinded() : ActionState()
{
    mStateID = CreatureState_Blinded;
}
// Dizzy State
StateDizzy::StateDizzy() : ActionState()
{
    mStateID = CreatureState_Dizzy;
}
// Intimidated State
StateIntimidated::StateIntimidated() : ActionState()
{
    mStateID = CreatureState_Intimidated;
}
// Immobolized State
StateImmobolized::StateImmobolized() : ActionState()
{
    mStateID = CreatureState_Immobilized;
}
// Frozen State
StateFrozen::StateFrozen() : ActionState()
{
    mStateID = CreatureState_Frozen;
}
// Swimming State
StateSwimming::StateSwimming() : ActionState()
{
    mStateID = CreatureState_Swimming;
}
// SittingOnaChair State
StateSittingOnChair::StateSittingOnChair() : ActionState()
{
    mStateID = CreatureState_SittingOnChair;
}
// Crafting State
StateCrafting::StateCrafting() : ActionState()
{
    mStateID = CreatureState_Crafting;
}
// GlowingJedi State
StateGlowingJedi::StateGlowingJedi() : ActionState()
{
    mStateID = CreatureState_GlowingJedi;
}
// Mask Scent State
StateMaskScent::StateMaskScent() : ActionState()
{
    mStateID = CreatureState_MaskScent;
}
// Poisoned State
StatePoisoined::StatePoisoined() : ActionState()
{
    mStateID = CreatureState_Poisoned;
}
// Bleeding State
StateBleeding::StateBleeding() : ActionState()
{
    mStateID = CreatureState_Bleeding;
}
// Diseased State
StateDiseased::StateDiseased() : ActionState()
{
    mStateID = CreatureState_Diseased;
}
// OnFire State
StateOnFire::StateOnFire() : ActionState()
{
    mStateID = CreatureState_OnFire;
}
// RidingMount State
StateRidingMount::StateRidingMount() : ActionState()
{
    mStateID = CreatureState_RidingMount;
}
// MountedCreature State
StateMountedCreature::StateMountedCreature() : ActionState()
{
    mStateID = CreatureState_MountedCreature;
}
// PilotingShip State
StatePilotingShip::StatePilotingShip() : ActionState()
{
    mStateID = CreatureState_PilotingShip;
}
// ShipOperations State
StateShipOperations::StateShipOperations() : ActionState()
{
    mStateID = CreatureState_ShipOperations;
}
// ShipGunner State
StateShipGunner::StateShipGunner() : ActionState()
{
    mStateID = CreatureState_ShipGunner;
}
// ShipInterior State
StateShipInterior::StateShipInterior() : ActionState()
{
    mStateID = CreatureState_ShipOperations;
}
// PilotingPobShip State
StatePilotingPobShip::StatePilotingPobShip() : ActionState()
{
    mStateID = CreatureState_PilotingShip;
}