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

bool ActionState::Enter(CreatureObject* obj)
{
	return true;
}
bool ActionState::Exit(CreatureObject* obj)
{
	return true;
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
bool StateCover::Enter(CreatureObject* obj)
{
	// check take cover mod
	// do appropriate actions
	obj->toggleStateOn(mStateID);
	return true;
}
bool StateCover::Exit(CreatureObject* obj)
{
	obj->toggleStateOff(mStateID);
	return true;
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
bool StateCombat::Enter(CreatureObject* obj)
{
	obj->toggleStateOn(mStateID);

	return true;
}
bool StateCombat::Exit(CreatureObject* obj)
{
	obj->toggleStateOff(mStateID);

	return true;
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
bool StatePeace::Enter(CreatureObject* obj)
{
	obj->toggleStateOn(mStateID);

	return true;
}
bool StatePeace::Exit(CreatureObject* obj)
{
	obj->toggleStateOff(mStateID);

	return true;
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
bool StateAiming::Enter(CreatureObject* obj)
{
	obj->toggleStateOn(mStateID);

	return true;
}
bool StateAiming::Exit(CreatureObject* obj)
{
	obj->toggleStateOff(mStateID);

	return true;
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
bool StateAlert::Enter(CreatureObject* obj)
{
	obj->toggleStateOn(mStateID);

	return true;
}
bool StateAlert::Exit(CreatureObject* obj)
{
	obj->toggleStateOff(mStateID);

	return true;
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
bool StateBerserk::Enter(CreatureObject* obj)
{
	obj->toggleStateOn(mStateID);

	return true;
}
bool StateBerserk::Exit(CreatureObject* obj)
{
	obj->toggleStateOff(mStateID);

	return true;
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
bool StateFeign::Enter(CreatureObject* obj)
{
	obj->toggleStateOn(mStateID);

	return true;
}
bool StateFeign::Exit(CreatureObject* obj)
{
	obj->toggleStateOff(mStateID);

	return true;
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
bool StateCombatEvasive::Enter(CreatureObject* obj)
{
	obj->toggleStateOn(mStateID);

	return true;
}
bool StateCombatEvasive::Exit(CreatureObject* obj)
{
	obj->toggleStateOff(mStateID);

	return true;
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
bool StateCombatNormal::Enter(CreatureObject* obj)
{
	obj->toggleStateOn(mStateID);

	return true;
}
bool StateCombatNormal::Exit(CreatureObject* obj)
{
	obj->toggleStateOff(mStateID);

	return true;
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
bool StateCombatAggressive::Enter(CreatureObject* obj)
{
	obj->toggleStateOn(mStateID);

	return true;
}
bool StateCombatAggressive::Exit(CreatureObject* obj)
{
	obj->toggleStateOff(mStateID);

	return true;
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
bool StateTumbling::Enter(CreatureObject* obj)
{
	obj->toggleStateOn(mStateID);

	return true;
}
bool StateTumbling::Exit(CreatureObject* obj)
{
	obj->toggleStateOff(mStateID);

	return true;
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
bool StateRallied::Enter(CreatureObject* obj)
{
	obj->toggleStateOn(mStateID);

	return true;
}
bool StateRallied::Exit(CreatureObject* obj)
{
	obj->toggleStateOff(mStateID);

	return true;
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
bool StateStunned::Enter(CreatureObject* obj)
{
	obj->toggleStateOn(mStateID);

	return true;
}
bool StateStunned::Exit(CreatureObject* obj)
{
	obj->toggleStateOff(mStateID);

	return true;
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
bool StateBlinded::Enter(CreatureObject* obj)
{
	obj->toggleStateOn(mStateID);

	return true;
}
bool StateBlinded::Exit(CreatureObject* obj)
{
	obj->toggleStateOff(mStateID);

	return true;
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
bool StateDizzy::Enter(CreatureObject* obj)
{
	obj->toggleStateOn(mStateID);

	return true;
}
bool StateDizzy::Exit(CreatureObject* obj)
{
	obj->toggleStateOff(mStateID);

	return true;
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
bool StateIntimidated::Enter(CreatureObject* obj)
{
	obj->toggleStateOn(mStateID);

	return true;
}
bool StateIntimidated::Exit(CreatureObject* obj)
{
	obj->toggleStateOff(mStateID);

	return true;
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
bool StateImmobolized::Enter(CreatureObject* obj)
{
	obj->toggleStateOn(mStateID);

	return true;
}
bool StateImmobolized::Exit(CreatureObject* obj)
{
	obj->toggleStateOff(mStateID);

	return true;
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
bool StateFrozen::Enter(CreatureObject* obj)
{
	obj->toggleStateOn(mStateID);

	return true;
}
bool StateFrozen::Exit(CreatureObject* obj)
{
	obj->toggleStateOff(mStateID);

	return true;
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
bool StateSwimming::Enter(CreatureObject* obj)
{
	obj->toggleStateOn(mStateID);

	return true;
}
bool StateSwimming::Exit(CreatureObject* obj)
{
	obj->toggleStateOff(mStateID);

	return true;
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
bool StateSittingOnChair::Enter(CreatureObject* obj)
{
	obj->toggleStateOn(mStateID);

	return true;
}
bool StateSittingOnChair::Exit(CreatureObject* obj)
{
	obj->toggleStateOff(mStateID);

	return true;
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
bool StateCrafting::Enter(CreatureObject* obj)
{
	obj->toggleStateOn(mStateID);

	return true;
}
bool StateCrafting::Exit(CreatureObject* obj)
{
	obj->toggleStateOff(mStateID);

	return true;
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
bool StateGlowingJedi::Enter(CreatureObject* obj)
{
	obj->toggleStateOn(mStateID);

	return true;
}
bool StateGlowingJedi::Exit(CreatureObject* obj)
{
	obj->toggleStateOff(mStateID);

	return true;
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
bool StateMaskScent::Enter(CreatureObject* obj)
{
	obj->toggleStateOn(mStateID);

	return true;
}
bool StateMaskScent::Exit(CreatureObject* obj)
{
	obj->toggleStateOff(mStateID);

	return true;
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
bool StatePoisoined::Enter(CreatureObject* obj)
{
	obj->toggleStateOn(mStateID);

	return true;
}
bool StatePoisoined::Exit(CreatureObject* obj)
{
	obj->toggleStateOff(mStateID);

	return true;
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
bool StateBleeding::Enter(CreatureObject* obj)
{
	obj->toggleStateOn(mStateID);

	return true;
}
bool StateBleeding::Exit(CreatureObject* obj)
{
	obj->toggleStateOff(mStateID);

	return true;
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
bool StateDiseased::Enter(CreatureObject* obj)
{
	obj->toggleStateOn(mStateID);

	return true;
}
bool StateDiseased::Exit(CreatureObject* obj)
{
	obj->toggleStateOff(mStateID);

	return true;
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
bool StateOnFire::Enter(CreatureObject* obj)
{
	obj->toggleStateOn(mStateID);

	return true;
}
bool StateOnFire::Exit(CreatureObject* obj)
{
	obj->toggleStateOff(mStateID);

	return true;
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
bool StateRidingMount::Enter(CreatureObject* obj)
{
	obj->toggleStateOn(mStateID);

	return true;
}
bool StateRidingMount::Exit(CreatureObject* obj)
{
	obj->toggleStateOff(mStateID);

	return true;
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
bool StateMountedCreature::Enter(CreatureObject* obj)
{
	obj->toggleStateOn(mStateID);

	return true;
}
bool StateMountedCreature::Exit(CreatureObject* obj)
{
	obj->toggleStateOff(mStateID);

	return true;
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
bool StatePilotingShip::Enter(CreatureObject* obj)
{
	obj->toggleStateOn(mStateID);

	return true;
}
bool StatePilotingShip::Exit(CreatureObject* obj)
{
	obj->toggleStateOff(mStateID);

	return true;
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
bool StateShipOperations::Enter(CreatureObject* obj)
{
	obj->toggleStateOn(mStateID);

	return true;
}
bool StateShipOperations::Exit(CreatureObject* obj)
{
	obj->toggleStateOff(mStateID);

	return true;
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
bool StateShipGunner::Enter(CreatureObject* obj)
{
	obj->toggleStateOn(mStateID);

	return true;
}
bool StateShipGunner::Exit(CreatureObject* obj)
{
	obj->toggleStateOff(mStateID);

	return true;
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
bool StateShipInterior::Enter(CreatureObject* obj)
{
	obj->toggleStateOn(mStateID);

	return true;
}
bool StateShipInterior::Exit(CreatureObject* obj)
{
	obj->toggleStateOff(mStateID);

	return true;
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
bool StatePilotingPobShip::Enter(CreatureObject* obj)
{
	obj->toggleStateOn(mStateID);

	return true;
}
bool StatePilotingPobShip::Exit(CreatureObject* obj)
{
	obj->toggleStateOff(mStateID);

	return true;
}
bool StatePilotingPobShip::CanTransition(CreatureObject* obj)
{
	// check the transition list
	return true;
}