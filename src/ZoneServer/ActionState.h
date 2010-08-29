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
    virtual void Enter(CreatureObject* obj);
    /* Activates the Exit process for the given state
    *  
    */
    virtual void Exit(CreatureObject* obj);
    /* Determines if the player can transition to the state
    *  
    */
    virtual bool CanTransition(uint64 newState);

protected:
    transitionList  mTransitionList;
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
    virtual bool CanTransition(uint64 newState);
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
    
    
    virtual bool CanTransition(uint64 newState);
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
    
    
    virtual bool CanTransition(uint64 newState);
};
/*	Aiming State
*
*/
class StateAiming:
    public ActionState
{
public:
    StateAiming();
    
    
    virtual bool CanTransition(uint64 newState);
};
/*	Alert State
*
*/
class StateAlert:
    public ActionState
{
public:
    StateAlert();
    
    
    virtual bool CanTransition(uint64 newState);
};
/*	Bersker State
*
*/
class StateBerserk:
    public ActionState
{
public:
    StateBerserk();
    
    
    virtual bool CanTransition(uint64 newState);
};
/*	Feign Death State
*
*/
class StateFeign:
    public ActionState
{
public:
    StateFeign();
    
    
    virtual bool CanTransition(uint64 newState);
};
/*	CombatAttitudeEvasive State
*
*/
class StateCombatEvasive:
    public ActionState
{
public:
    StateCombatEvasive();
    
    
    virtual bool CanTransition(uint64 newState);
};
/*	CombatAttitudeNormal State
*
*/
class StateCombatNormal:
    public ActionState
{
public:
    StateCombatNormal();
    
    
    virtual bool CanTransition(uint64 newState);
};
/*	CombatAttitudeAggressive State
*
*/
class StateCombatAggressive:
    public ActionState
{
public:
    StateCombatAggressive();
    
    
    virtual bool CanTransition(uint64 newState);
};
/*	Tumbling State
*
*/
class StateTumbling:
    public ActionState
{
public:
    StateTumbling();
    
    
    virtual bool CanTransition(uint64 newState);
};
/*	Rallied State
*
*/
class StateRallied:
    public ActionState
{
public:
    StateRallied();
    
    
    virtual bool CanTransition(uint64 newState);
};
/*	Stunned State
*
*/
class StateStunned:
    public ActionState
{
public:
    StateStunned();
    
    
    virtual bool CanTransition(uint64 newState);
};
/*	Blinded State
*
*/
class StateBlinded:
    public ActionState
{
public:
    StateBlinded();
    
    
    virtual bool CanTransition(uint64 newState);
};
/*	Dizzy State
*
*/
class StateDizzy:
    public ActionState
{
public:
    StateDizzy();
    
    
    virtual bool CanTransition(uint64 newState);
};
/*	Intimidated State
*
*/
class StateIntimidated:
    public ActionState
{
public:
    StateIntimidated();
    
    
    virtual bool CanTransition(uint64 newState);
};
/*	Immobolized State
*
*/
class StateImmobolized:
    public ActionState
{
public:
    StateImmobolized();
    
    
    virtual bool CanTransition(uint64 newState);
};
/*	Frozen State
*
*/
class StateFrozen:
    public ActionState
{
public:
    StateFrozen();
    
    
    virtual bool CanTransition(uint64 newState);
};
/*	Swimming State
*
*/
class StateSwimming:
    public ActionState
{
public:
    StateSwimming();
    
    
    virtual bool CanTransition(uint64 newState);
};
/*	SittingOnChair State
*
*/
class StateSittingOnChair:
    public ActionState
{
public:
    StateSittingOnChair();
    
    
    virtual bool CanTransition(uint64 newState);
};
/*	Crafting State
*
*/
class StateCrafting:
    public ActionState
{
public:
    StateCrafting();
    
    
    virtual bool CanTransition(uint64 newState);
};
/*	GlowingJedi State
*
*/
class StateGlowingJedi:
    public ActionState
{
public:
    StateGlowingJedi();
    
    
    virtual bool CanTransition(uint64 newState);
};
/*	MaskScent State
*
*/
class StateMaskScent:
    public ActionState
{
public:
    StateMaskScent();
    
    
    virtual bool CanTransition(uint64 newState);
};
/*	Poisoined State
*
*/
class StatePoisoined:
    public ActionState
{
public:
    StatePoisoined();
    
    
    virtual bool CanTransition(uint64 newState);
};
/*	Bleeding State
*
*/
class StateBleeding:
    public ActionState
{
public:
    StateBleeding();
    
    
    virtual bool CanTransition(uint64 newState);
};
/*	Diseased State
*
*/
class StateDiseased:
    public ActionState
{
public:
    StateDiseased();
    
    
    virtual bool CanTransition(uint64 newState);
};
/*	OnFire State
*
*/
class StateOnFire:
    public ActionState
{
public:
    StateOnFire();
    
    
    virtual bool CanTransition(uint64 newState);
};
/*	RidingMount State
*
*/
class StateRidingMount:
    public ActionState
{
public:
    StateRidingMount();
    
    
    virtual bool CanTransition(uint64 newState);
};
/*	MountedCreature State
*
*/
class StateMountedCreature:
    public ActionState
{
public:
    StateMountedCreature();
    
    
    virtual bool CanTransition(uint64 newState);
};
/*	PilotingShip State
*
*/
class StatePilotingShip:
    public ActionState
{
public:
    StatePilotingShip();
    
    
    virtual bool CanTransition(uint64 newState);
};
/*	ShipOperations State
*
*/
class StateShipOperations:
    public ActionState
{
public:
    StateShipOperations();
    
    
    virtual bool CanTransition(uint64 newState);
};
/*	ShipGunner State
*
*/
class StateShipGunner:
    public ActionState
{
public:
    StateShipGunner();
    
    
    virtual bool CanTransition(uint64 newState);
};
/*	ShipInterior State
*
*/
class StateShipInterior:
    public ActionState
{
public:
    StateShipInterior();
    
    
    virtual bool CanTransition(uint64 newState);
};
/*	PilotingPobShip State
*
*/
class StatePilotingPobShip:
    public ActionState
{
public:
    StatePilotingPobShip();
    
    
    virtual bool CanTransition(uint64 newState);
};