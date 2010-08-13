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
#include "abstractstate.h"
#include "CreatureObject.h"
class ActionState :
	public AbstractState
{
public:
	ActionState(void);
	~ActionState(void);

	/* Activates the Enter process for the character state
	*  
	*/
	virtual bool Enter();
	/* Activates the Exit process for the given state
	*  
	*/
	virtual bool Exit();
	/* Determines if the player can transition to the state
	*  
	*/
	virtual bool CanTransition();

};
/*	Cover State
*
*/
class StateCover:
	public ActionState
{
public:
	virtual bool Enter();
	virtual bool Exit();
	virtual bool CanTransition();
};
/*	Combat State
*
*/
class StateCombat:
	public ActionState
{
public:
	virtual bool Enter();
	virtual bool Exit();
	virtual bool CanTransition();
};
/*	Peace State
*
*/
class StatePeace:
	public ActionState
{
public:
	virtual bool Enter();
	virtual bool Exit();
	virtual bool CanTransition();
};
/*	Aiming State
*
*/
class StateAiming:
	public ActionState
{
public:
	virtual bool Enter();
	virtual bool Exit();
	virtual bool CanTransition();
};
/*	Alert State
*
*/
class StateAlert:
	public ActionState
{
public:
	virtual bool Enter();
	virtual bool Exit();
	virtual bool CanTransition();
};
/*	Bersker State
*
*/
class StateBerserk:
	public ActionState
{
public:
	virtual bool Enter();
	virtual bool Exit();
	virtual bool CanTransition();
};
/*	Feign Death State
*
*/
class StateFeign:
	public ActionState
{
public:
	virtual bool Enter();
	virtual bool Exit();
	virtual bool CanTransition();
};
/*	CombatAttitudeEvasive State
*
*/
class StateCombatEvasive:
	public ActionState
{
public:
	virtual bool Enter();
	virtual bool Exit();
	virtual bool CanTransition();
};
/*	CombatAttitudeNormal State
*
*/
class StateCombatNormal:
	public ActionState
{
public:
	virtual bool Enter();
	virtual bool Exit();
	virtual bool CanTransition();
};
/*	CombatAttitudeAggressive State
*
*/
class StateCombatAggressive:
	public ActionState
{
public:
	virtual bool Enter();
	virtual bool Exit();
	virtual bool CanTransition();
};
/*	Tumbling State
*
*/
class StateTumbling:
	public ActionState
{
public:
	virtual bool Enter();
	virtual bool Exit();
	virtual bool CanTransition();
};
/*	Rallied State
*
*/
class StateRallied:
	public ActionState
{
public:
	virtual bool Enter();
	virtual bool Exit();
	virtual bool CanTransition();
};
/*	Stunned State
*
*/
class StateStunned:
	public ActionState
{
public:
	virtual bool Enter();
	virtual bool Exit();
	virtual bool CanTransition();
};
/*	Blinded State
*
*/
class StateBlinded:
	public ActionState
{
public:
	virtual bool Enter();
	virtual bool Exit();
	virtual bool CanTransition();
};
/*	Dizzy State
*
*/
class StateDizzy:
	public ActionState
{
public:
	virtual bool Enter();
	virtual bool Exit();
	virtual bool CanTransition();
};
/*	Intimidated State
*
*/
class StateIntimidated:
	public ActionState
{
public:
	virtual bool Enter();
	virtual bool Exit();
	virtual bool CanTransition();
};
/*	Immobolized State
*
*/
class StateImmobolized:
	public ActionState
{
public:
	virtual bool Enter();
	virtual bool Exit();
	virtual bool CanTransition();
};
/*	Frozen State
*
*/
class StateFrozen:
	public ActionState
{
public:
	virtual bool Enter();
	virtual bool Exit();
	virtual bool CanTransition();
};
/*	Swimming State
*
*/
class StateSwimming:
	public ActionState
{
public:
	virtual bool Enter();
	virtual bool Exit();
	virtual bool CanTransition();
};
/*	SittingOnChair State
*
*/
class StateSittingOnChair:
	public ActionState
{
public:
	virtual bool Enter();
	virtual bool Exit();
	virtual bool CanTransition();
};
/*	Crafting State
*
*/
class StateCrafting:
	public ActionState
{
public:
	virtual bool Enter();
	virtual bool Exit();
	virtual bool CanTransition();
};
/*	GlowingJedi State
*
*/
class StateGlowingJedi:
	public ActionState
{
public:
	virtual bool Enter();
	virtual bool Exit();
	virtual bool CanTransition();
};
/*	MaskScent State
*
*/
class StateMaskScent:
	public ActionState
{
public:
	virtual bool Enter();
	virtual bool Exit();
	virtual bool CanTransition();
};
/*	Poisoined State
*
*/
class StatePoisoined:
	public ActionState
{
public:
	virtual bool Enter();
	virtual bool Exit();
	virtual bool CanTransition();
};
/*	Bleeding State
*
*/
class StateBleeding:
	public ActionState
{
public:
	virtual bool Enter();
	virtual bool Exit();
	virtual bool CanTransition();
};
/*	Diseased State
*
*/
class StateDiseased:
	public ActionState
{
public:
	virtual bool Enter();
	virtual bool Exit();
	virtual bool CanTransition();
};
/*	OnFire State
*
*/
class StateOnFire:
	public ActionState
{
public:
	virtual bool Enter();
	virtual bool Exit();
	virtual bool CanTransition();
};
/*	RidingMount State
*
*/
class StateRidingMount:
	public ActionState
{
public:
	virtual bool Enter();
	virtual bool Exit();
	virtual bool CanTransition();
};
/*	MountedCreature State
*
*/
class StateMountedCreature:
	public ActionState
{
public:
	virtual bool Enter();
	virtual bool Exit();
	virtual bool CanTransition();
};
/*	PilotingShip State
*
*/
class StatePilotingShip:
	public ActionState
{
public:
	virtual bool Enter();
	virtual bool Exit();
	virtual bool CanTransition();
};
/*	ShipOperations State
*
*/
class StateShipOperations:
	public ActionState
{
public:
	virtual bool Enter();
	virtual bool Exit();
	virtual bool CanTransition();
};
/*	ShipGunner State
*
*/
class StateShipGunner:
	public ActionState
{
public:
	virtual bool Enter();
	virtual bool Exit();
	virtual bool CanTransition();
};
/*	ShipInterior State
*
*/
class StateShipInterior:
	public ActionState
{
public:
	virtual bool Enter();
	virtual bool Exit();
	virtual bool CanTransition();
};
/*	PilotingPobShip State
*
*/
class StatePilotingPobShip:
	public ActionState
{
public:
	virtual bool Enter();
	virtual bool Exit();
	virtual bool CanTransition();
};
#endif