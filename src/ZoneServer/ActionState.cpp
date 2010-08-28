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
#include "PlayerObject.h"


ActionState::ActionState(void)
{
}

void ActionState::Enter(CreatureObject* obj)
{
    obj->toggleStateOn(mStateID);
}
void ActionState::Exit(CreatureObject* obj)
{
    obj->toggleStateOff(mStateID);
}
bool ActionState::CanTransition(CreatureObject* obj)
{
    return true;
}

// StateCover
StateCover::StateCover() : ActionState()
{
    mStateID = CreatureState_Cover;
}
bool StateCover::CanTransition(CreatureObject* obj)
{
    // check the transition list
    return true;
}

// State Combat
StateCombat::StateCombat() : ActionState()
{
    mStateID = CreatureState_Combat;
}
bool StateCombat::CanTransition(CreatureObject* obj)
{
    // check the transition list
    return true;
}

// State Peace
StatePeace::StatePeace() : ActionState()
{
    mStateID = CreatureState_Combat;
}
bool StatePeace::CanTransition(CreatureObject* obj)
{
    // check the transition list
    return true;
}

// State Aiming
StateAiming::StateAiming() : ActionState()
{
    mStateID = CreatureState_Aiming;
}
bool StateAiming::CanTransition(CreatureObject* obj)
{
    // check the transition list
    return true;
}

// State Alert
StateAlert::StateAlert() : ActionState()
{
    mStateID = CreatureState_Alert;
}

bool StateAlert::CanTransition(CreatureObject* obj)
{
    // check the transition list
    return true;
}

// Berserk State
StateBerserk::StateBerserk() : ActionState()
{
    mStateID = CreatureState_Berserk;
}

bool StateBerserk::CanTransition(CreatureObject* obj)
{
    // check the transition list
    return true;
}

// Feign State
StateFeign::StateFeign() : ActionState()
{
    mStateID = CreatureState_FeignDeath;
}

bool StateFeign::CanTransition(CreatureObject* obj)
{
    // check the transition list
    return true;
}

// Combat Attitude Evasive State
StateCombatEvasive::StateCombatEvasive() : ActionState()
{
    mStateID = CreatureState_CombatAttitudeEvasive;
}

bool StateCombatEvasive::CanTransition(CreatureObject* obj)
{
    // check the transition list
    return true;
}

// Combat Attitude Normal State
StateCombatNormal::StateCombatNormal() : ActionState()
{
    mStateID = CreatureState_CombatAttitudeNormal;
}
bool StateCombatNormal::CanTransition(CreatureObject* obj)
{
    // check the transition list
    return true;
}

// Combat Attitude Aggressive State
StateCombatAggressive::StateCombatAggressive() : ActionState()
{
    mStateID = CreatureState_CombatAttitudeAggressive;
}
bool StateCombatAggressive::CanTransition(CreatureObject* obj)
{
    // check the transition list
    return true;
}

// Tumbling State
StateTumbling::StateTumbling() : ActionState()
{
    mStateID = CreatureState_Tumbling;
}
bool StateTumbling::CanTransition(CreatureObject* obj)
{
    // check the transition list
    return true;
}
// Rallied State
StateRallied::StateRallied() : ActionState()
{
    mStateID = CreatureState_Rallied;
}
bool StateRallied::CanTransition(CreatureObject* obj)
{
    // check the transition list
    return true;
}
// Stunned State
StateStunned::StateStunned() : ActionState()
{
    mStateID = CreatureState_Stunned;
}
bool StateStunned::CanTransition(CreatureObject* obj)
{
    // check the transition list
    return true;
}
// Blinded State
StateBlinded::StateBlinded() : ActionState()
{
    mStateID = CreatureState_Blinded;
}
bool StateBlinded::CanTransition(CreatureObject* obj)
{
    // check the transition list
    return true;
}
// Dizzy State
StateDizzy::StateDizzy() : ActionState()
{
    mStateID = CreatureState_Dizzy;
}
bool StateDizzy::CanTransition(CreatureObject* obj)
{
    // check the transition list
    return true;
}
// Intimidated State
StateIntimidated::StateIntimidated() : ActionState()
{
    mStateID = CreatureState_Intimidated;
}
bool StateIntimidated::CanTransition(CreatureObject* obj)
{
    // check the transition list
    return true;
}
// Immobolized State
StateImmobolized::StateImmobolized() : ActionState()
{
    mStateID = CreatureState_Immobilized;
}
bool StateImmobolized::CanTransition(CreatureObject* obj)
{
    // check the transition list
    return true;
}
// Frozen State
StateFrozen::StateFrozen() : ActionState()
{
    mStateID = CreatureState_Frozen;
}
bool StateFrozen::CanTransition(CreatureObject* obj)
{
    // check the transition list
    return true;
}
// Swimming State
StateSwimming::StateSwimming() : ActionState()
{
    mStateID = CreatureState_Swimming;
}
bool StateSwimming::CanTransition(CreatureObject* obj)
{
    // check the transition list
    return true;
}
// SittingOnaChair State
StateSittingOnChair::StateSittingOnChair() : ActionState()
{
    mStateID = CreatureState_SittingOnChair;
}
bool StateSittingOnChair::CanTransition(CreatureObject* obj)
{
    // check the transition list
    return true;
}
// Crafting State
StateCrafting::StateCrafting() : ActionState()
{
    mStateID = CreatureState_Crafting;
}
bool StateCrafting::CanTransition(CreatureObject* obj)
{
    // check the transition list
    return true;
}
// GlowingJedi State
StateGlowingJedi::StateGlowingJedi() : ActionState()
{
    mStateID = CreatureState_GlowingJedi;
}
bool StateGlowingJedi::CanTransition(CreatureObject* obj)
{
    // check the transition list
    return true;
}
// Mask Scent State
StateMaskScent::StateMaskScent() : ActionState()
{
    mStateID = CreatureState_MaskScent;
}
bool StateMaskScent::CanTransition(CreatureObject* obj)
{
    // check the transition list
    return true;
}
// Poisoned State
StatePoisoined::StatePoisoined() : ActionState()
{
    mStateID = CreatureState_Poisoned;
}
bool StatePoisoined::CanTransition(CreatureObject* obj)
{
    // check the transition list
    return true;
}
// Bleeding State
StateBleeding::StateBleeding() : ActionState()
{
    mStateID = CreatureState_Bleeding;
}
bool StateBleeding::CanTransition(CreatureObject* obj)
{
    // check the transition list
    return true;
}
// Diseased State
StateDiseased::StateDiseased() : ActionState()
{
    mStateID = CreatureState_Diseased;
}
bool StateDiseased::CanTransition(CreatureObject* obj)
{
    // check the transition list
    return true;
}
// OnFire State
StateOnFire::StateOnFire() : ActionState()
{
    mStateID = CreatureState_OnFire;
}
bool StateOnFire::CanTransition(CreatureObject* obj)
{
    // check the transition list
    return true;
}
// RidingMount State
StateRidingMount::StateRidingMount() : ActionState()
{
    mStateID = CreatureState_RidingMount;
}
bool StateRidingMount::CanTransition(CreatureObject* obj)
{
    // check the transition list
    return true;
}
// MountedCreature State
StateMountedCreature::StateMountedCreature() : ActionState()
{
    mStateID = CreatureState_MountedCreature;
}
bool StateMountedCreature::CanTransition(CreatureObject* obj)
{
    // check the transition list
    return true;
}
// PilotingShip State
StatePilotingShip::StatePilotingShip() : ActionState()
{
    mStateID = CreatureState_PilotingShip;
}
bool StatePilotingShip::CanTransition(CreatureObject* obj)
{
    // check the transition list
    return true;
}
// ShipOperations State
StateShipOperations::StateShipOperations() : ActionState()
{
    mStateID = CreatureState_ShipOperations;
}
bool StateShipOperations::CanTransition(CreatureObject* obj)
{
    // check the transition list
    return true;
}
// ShipGunner State
StateShipGunner::StateShipGunner() : ActionState()
{
    mStateID = CreatureState_ShipGunner;
}
bool StateShipGunner::CanTransition(CreatureObject* obj)
{
    // check the transition list
    return true;
}
// ShipInterior State
StateShipInterior::StateShipInterior() : ActionState()
{
    mStateID = CreatureState_ShipOperations;
}
bool StateShipInterior::CanTransition(CreatureObject* obj)
{
    // check the transition list
    return true;
}
// PilotingPobShip State
StatePilotingPobShip::StatePilotingPobShip() : ActionState()
{
    mStateID = CreatureState_PilotingShip;
}
bool StatePilotingPobShip::CanTransition(CreatureObject* obj)
{
    // check the transition list
    return true;
}