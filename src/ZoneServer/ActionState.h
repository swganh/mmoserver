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
#pragma once
#ifndef ANH_ZONESERVER_ACTION_STATE_H
#define ANH_ZONESERVER_ACTION_STATE_H

#include "AbstractState.h"

class ActionState :
	public IState
{
public:
	ActionState(void);
	virtual ~ActionState(void){};

	/* Activates the Enter process for the character state
	*  
	*/
	virtual bool Enter(CreatureObject* obj);
	/* Activates the Exit process for the given state
	*  
	*/
	virtual bool Exit(CreatureObject* obj);
	/* Determines if the player can transition to the state
	*  
	*/
	virtual bool CanTransition(CreatureObject* obj);

protected:
	CreatureState	mStateID;
};
#endif

/*	Cover State
*
*/
class StateCover:
	public ActionState
{
public:
	StateCover();
	virtual bool Enter(CreatureObject* obj);
	virtual bool Exit(CreatureObject* obj);
	virtual bool CanTransition(CreatureObject* obj);
};
/*
*	Combat State
*
*/
class StateCombat:
	public ActionState
{
public:
	StateCombat();
	virtual bool Enter(CreatureObject* obj);
	virtual bool Exit(CreatureObject* obj);
	virtual bool CanTransition(CreatureObject* obj);
};
/*
*	Peace State
*
*/
class StatePeace:
	public ActionState
{
public:
	StatePeace();
	virtual bool Enter(CreatureObject* obj);
	virtual bool Exit(CreatureObject* obj);
	virtual bool CanTransition(CreatureObject* obj);
};
/*	Aiming State
*
*/
class StateAiming:
	public ActionState
{
public:
	StateAiming();
	virtual bool Enter(CreatureObject* obj);
	virtual bool Exit(CreatureObject* obj);
	virtual bool CanTransition(CreatureObject* obj);
};
/*	Alert State
*
*/
class StateAlert:
	public ActionState
{
public:
	StateAlert();
	virtual bool Enter(CreatureObject* obj);
	virtual bool Exit(CreatureObject* obj);
	virtual bool CanTransition(CreatureObject* obj);
};
/*	Bersker State
*
*/
class StateBerserk:
	public ActionState
{
public:
	StateBerserk();
	virtual bool Enter(CreatureObject* obj);
	virtual bool Exit(CreatureObject* obj);
	virtual bool CanTransition(CreatureObject* obj);
};
/*	Feign Death State
*
*/
class StateFeign:
	public ActionState
{
public:
	StateFeign();
	virtual bool Enter(CreatureObject* obj);
	virtual bool Exit(CreatureObject* obj);
	virtual bool CanTransition(CreatureObject* obj);
};
/*	CombatAttitudeEvasive State
*
*/
class StateCombatEvasive:
	public ActionState
{
public:
	StateCombatEvasive();
	virtual bool Enter(CreatureObject* obj);
	virtual bool Exit(CreatureObject* obj);
	virtual bool CanTransition(CreatureObject* obj);
};
/*	CombatAttitudeNormal State
*
*/
class StateCombatNormal:
	public ActionState
{
public:
	StateCombatNormal();
	virtual bool Enter(CreatureObject* obj);
	virtual bool Exit(CreatureObject* obj);
	virtual bool CanTransition(CreatureObject* obj);
};
/*	CombatAttitudeAggressive State
*
*/
class StateCombatAggressive:
	public ActionState
{
public:
	StateCombatAggressive();
	virtual bool Enter(CreatureObject* obj);
	virtual bool Exit(CreatureObject* obj);
	virtual bool CanTransition(CreatureObject* obj);
};
/*	Tumbling State
*
*/
class StateTumbling:
	public ActionState
{
public:
	StateTumbling();
	virtual bool Enter(CreatureObject* obj);
	virtual bool Exit(CreatureObject* obj);
	virtual bool CanTransition(CreatureObject* obj);
};
/*	Rallied State
*
*/
class StateRallied:
	public ActionState
{
public:
	StateRallied();
	virtual bool Enter(CreatureObject* obj);
	virtual bool Exit(CreatureObject* obj);
	virtual bool CanTransition(CreatureObject* obj);
};
/*	Stunned State
*
*/
class StateStunned:
	public ActionState
{
public:
	StateStunned();
	virtual bool Enter(CreatureObject* obj);
	virtual bool Exit(CreatureObject* obj);
	virtual bool CanTransition(CreatureObject* obj);
};
/*	Blinded State
*
*/
class StateBlinded:
	public ActionState
{
public:
	StateBlinded();
	virtual bool Enter(CreatureObject* obj);
	virtual bool Exit(CreatureObject* obj);
	virtual bool CanTransition(CreatureObject* obj);
};
/*	Dizzy State
*
*/
class StateDizzy:
	public ActionState
{
public:
	StateDizzy();
	virtual bool Enter(CreatureObject* obj);
	virtual bool Exit(CreatureObject* obj);
	virtual bool CanTransition(CreatureObject* obj);
};
/*	Intimidated State
*
*/
class StateIntimidated:
	public ActionState
{
public:
	StateIntimidated();
	virtual bool Enter(CreatureObject* obj);
	virtual bool Exit(CreatureObject* obj);
	virtual bool CanTransition(CreatureObject* obj);
};
/*	Immobolized State
*
*/
class StateImmobolized:
	public ActionState
{
public:
	StateImmobolized();
	virtual bool Enter(CreatureObject* obj);
	virtual bool Exit(CreatureObject* obj);
	virtual bool CanTransition(CreatureObject* obj);
};
/*	Frozen State
*
*/
class StateFrozen:
	public ActionState
{
public:
	StateFrozen();
	virtual bool Enter(CreatureObject* obj);
	virtual bool Exit(CreatureObject* obj);
	virtual bool CanTransition(CreatureObject* obj);
};
/*	Swimming State
*
*/
class StateSwimming:
	public ActionState
{
public:
	StateSwimming();
	virtual bool Enter(CreatureObject* obj);
	virtual bool Exit(CreatureObject* obj);
	virtual bool CanTransition(CreatureObject* obj);
};
/*	SittingOnChair State
*
*/
class StateSittingOnChair:
	public ActionState
{
public:
	StateSittingOnChair();
	virtual bool Enter(CreatureObject* obj);
	virtual bool Exit(CreatureObject* obj);
	virtual bool CanTransition(CreatureObject* obj);
};
/*	Crafting State
*
*/
class StateCrafting:
	public ActionState
{
public:
	StateCrafting();
	virtual bool Enter(CreatureObject* obj);
	virtual bool Exit(CreatureObject* obj);
	virtual bool CanTransition(CreatureObject* obj);
};
/*	GlowingJedi State
*
*/
class StateGlowingJedi:
	public ActionState
{
public:
	StateGlowingJedi();
	virtual bool Enter(CreatureObject* obj);
	virtual bool Exit(CreatureObject* obj);
	virtual bool CanTransition(CreatureObject* obj);
};
/*	MaskScent State
*
*/
class StateMaskScent:
	public ActionState
{
public:
	StateMaskScent();
	virtual bool Enter(CreatureObject* obj);
	virtual bool Exit(CreatureObject* obj);
	virtual bool CanTransition(CreatureObject* obj);
};
/*	Poisoined State
*
*/
class StatePoisoined:
	public ActionState
{
public:
	StatePoisoined();
	virtual bool Enter(CreatureObject* obj);
	virtual bool Exit(CreatureObject* obj);
	virtual bool CanTransition(CreatureObject* obj);
};
/*	Bleeding State
*
*/
class StateBleeding:
	public ActionState
{
public:
	StateBleeding();
	virtual bool Enter(CreatureObject* obj);
	virtual bool Exit(CreatureObject* obj);
	virtual bool CanTransition(CreatureObject* obj);
};
/*	Diseased State
*
*/
class StateDiseased:
	public ActionState
{
public:
	StateDiseased();
	virtual bool Enter(CreatureObject* obj);
	virtual bool Exit(CreatureObject* obj);
	virtual bool CanTransition(CreatureObject* obj);
};
/*	OnFire State
*
*/
class StateOnFire:
	public ActionState
{
public:
	StateOnFire();
	virtual bool Enter(CreatureObject* obj);
	virtual bool Exit(CreatureObject* obj);
	virtual bool CanTransition(CreatureObject* obj);
};
/*	RidingMount State
*
*/
class StateRidingMount:
	public ActionState
{
public:
	StateRidingMount();
	virtual bool Enter(CreatureObject* obj);
	virtual bool Exit(CreatureObject* obj);
	virtual bool CanTransition(CreatureObject* obj);
};
/*	MountedCreature State
*
*/
class StateMountedCreature:
	public ActionState
{
public:
	StateMountedCreature();
	virtual bool Enter(CreatureObject* obj);
	virtual bool Exit(CreatureObject* obj);
	virtual bool CanTransition(CreatureObject* obj);
};
/*	PilotingShip State
*
*/
class StatePilotingShip:
	public ActionState
{
public:
	StatePilotingShip();
	virtual bool Enter(CreatureObject* obj);
	virtual bool Exit(CreatureObject* obj);
	virtual bool CanTransition(CreatureObject* obj);
};
/*	ShipOperations State
*
*/
class StateShipOperations:
	public ActionState
{
public:
	StateShipOperations();
	virtual bool Enter(CreatureObject* obj);
	virtual bool Exit(CreatureObject* obj);
	virtual bool CanTransition(CreatureObject* obj);
};
/*	ShipGunner State
*
*/
class StateShipGunner:
	public ActionState
{
public:
	StateShipGunner();
	virtual bool Enter(CreatureObject* obj);
	virtual bool Exit(CreatureObject* obj);
	virtual bool CanTransition(CreatureObject* obj);
};
/*	ShipInterior State
*
*/
class StateShipInterior:
	public ActionState
{
public:
	StateShipInterior();
	virtual bool Enter(CreatureObject* obj);
	virtual bool Exit(CreatureObject* obj);
	virtual bool CanTransition(CreatureObject* obj);
};
/*	PilotingPobShip State
*
*/
class StatePilotingPobShip:
	public ActionState
{
public:
	StatePilotingPobShip();
	virtual bool Enter(CreatureObject* obj);
	virtual bool Exit(CreatureObject* obj);
	virtual bool CanTransition(CreatureObject* obj);
};