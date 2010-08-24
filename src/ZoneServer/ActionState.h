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
	virtual bool Enter(Object* obj);
	/* Activates the Exit process for the given state
	*  
	*/
	virtual bool Exit(Object* obj);
	/* Determines if the player can transition to the state
	*  
	*/
	virtual bool CanTransition(Object* obj);

};
#endif

/*	Cover State
*
*/
class StateCover:
	public ActionState
{
public:
	virtual bool Enter(Object* obj);
	virtual bool Exit(Object* obj);
	virtual bool CanTransition(Object* obj);
private:
	uint32 mStateID;
};
//
///*	Combat State
//*
//*/
//class StateCombat:
//	public ActionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	Peace State
//*
//*/
//class StatePeace:
//	public ActionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	Aiming State
//*
//*/
//class StateAiming:
//	public ActionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	Alert State
//*
//*/
//class StateAlert:
//	public ActionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	Bersker State
//*
//*/
//class StateBerserk:
//	public ActionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	Feign Death State
//*
//*/
//class StateFeign:
//	public ActionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	CombatAttitudeEvasive State
//*
//*/
//class StateCombatEvasive:
//	public ActionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	CombatAttitudeNormal State
//*
//*/
//class StateCombatNormal:
//	public ActionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	CombatAttitudeAggressive State
//*
//*/
//class StateCombatAggressive:
//	public ActionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	Tumbling State
//*
//*/
//class StateTumbling:
//	public ActionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	Rallied State
//*
//*/
//class StateRallied:
//	public ActionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	Stunned State
//*
//*/
//class StateStunned:
//	public ActionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	Blinded State
//*
//*/
//class StateBlinded:
//	public ActionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	Dizzy State
//*
//*/
//class StateDizzy:
//	public ActionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	Intimidated State
//*
//*/
//class StateIntimidated:
//	public ActionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	Immobolized State
//*
//*/
//class StateImmobolized:
//	public ActionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	Frozen State
//*
//*/
//class StateFrozen:
//	public ActionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	Swimming State
//*
//*/
//class StateSwimming:
//	public ActionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	SittingOnChair State
//*
//*/
//class StateSittingOnChair:
//	public ActionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	Crafting State
//*
//*/
//class StateCrafting:
//	public ActionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	GlowingJedi State
//*
//*/
//class StateGlowingJedi:
//	public ActionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	MaskScent State
//*
//*/
//class StateMaskScent:
//	public ActionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	Poisoined State
//*
//*/
//class StatePoisoined:
//	public ActionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	Bleeding State
//*
//*/
//class StateBleeding:
//	public ActionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	Diseased State
//*
//*/
//class StateDiseased:
//	public ActionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	OnFire State
//*
//*/
//class StateOnFire:
//	public ActionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	RidingMount State
//*
//*/
//class StateRidingMount:
//	public ActionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	MountedCreature State
//*
//*/
//class StateMountedCreature:
//	public ActionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	PilotingShip State
//*
//*/
//class StatePilotingShip:
//	public ActionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	ShipOperations State
//*
//*/
//class StateShipOperations:
//	public ActionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	ShipGunner State
//*
//*/
//class StateShipGunner:
//	public ActionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	ShipInterior State
//*
//*/
//class StateShipInterior:
//	public ActionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	PilotingPobShip State
//*
//*/
//class StatePilotingPobShip:
//	public ActionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};