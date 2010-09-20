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
#include "CreatureObject.h"

typedef std::vector<uint64> stateRemove;
class StateManager;
class ActionState :
    public IState
{
public:
    ActionState(StateManager* const sm);
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
    virtual bool CanTransition(CreatureObject* obj, uint64 newState);

    virtual const std::vector<uint64>& statesToRemove(){return mStatesRemovalList;}
    
    virtual void insertIntoStateRemovalList(const uint64& state);

    virtual const transitionList& returnTransitionList(){return mTransitionList;}

    virtual void insertIntoTransitionList(const std::pair<StateTypes, uint64>& pair);

    virtual uint64 getID() {return mStateID;}

protected:
    stateRemove             mStatesRemovalList;
    StateManager*           mStateManager;
    bool					mHidden;
    transitionList          mTransitionList;
    CreatureState	        mStateID;
};
#endif

/*	Cover State
*
*/
class StateCover:
    public ActionState
{
public:
    explicit StateCover(StateManager* const sm);
    //void Enter(CreatureObject* obj);
};
/*
*	Combat State
*
*/
class StateCombat:
    public ActionState
{
public:
    explicit StateCombat(StateManager* const sm);
    void Enter(CreatureObject* obj);
};
/*
*	Peace State
*
*/
class StatePeace:
    public ActionState
{
public:
    explicit StatePeace(StateManager* const sm);
    void Enter(CreatureObject* obj);
};
/*	Aiming State
*
*/
class StateAiming:
    public ActionState
{
public:
    explicit StateAiming(StateManager* const sm);
    //void Enter(CreatureObject* obj);
};
/*	Alert State
*
*/
class StateAlert:
    public ActionState
{
public:
    explicit StateAlert(StateManager* const sm);
    //void Enter(CreatureObject* obj);
};
/*	Bersker State
*
*/
class StateBerserk:
    public ActionState
{
public:
    explicit StateBerserk(StateManager* const sm);
    //void Enter(CreatureObject* obj);
};
/*	Feign Death State
*
*/
class StateFeign:
    public ActionState
{
public:
    explicit StateFeign(StateManager* const sm);
    //void Enter(CreatureObject* obj);
};
/*	CombatAttitudeEvasive State
*
*/
class StateCombatEvasive:
    public ActionState
{
public:
    explicit StateCombatEvasive(StateManager* const sm);
    //void Enter(CreatureObject* obj);
};
/*	CombatAttitudeNormal State
*
*/
class StateCombatNormal:
    public ActionState
{
public:
    explicit StateCombatNormal(StateManager* const sm);
    //void Enter(CreatureObject* obj);
};
/*	CombatAttitudeAggressive State
*
*/
class StateCombatAggressive:
    public ActionState
{
public:
    explicit StateCombatAggressive(StateManager* const sm);
    //void Enter(CreatureObject* obj);
};
/*	Tumbling State
*
*/
class StateTumbling:
    public ActionState
{
public:
    explicit StateTumbling(StateManager* const sm);
    //void Enter(CreatureObject* obj);
};
/*	Rallied State
*
*/
class StateRallied:
    public ActionState
{
public:
    explicit StateRallied(StateManager* const sm);
    //void Enter(CreatureObject* obj);
};
/*	Stunned State
*
*/
class StateStunned:
    public ActionState
{
public:
    explicit StateStunned(StateManager* const sm);
    //void Enter(CreatureObject* obj);
};
/*	Blinded State
*
*/
class StateBlinded:
    public ActionState
{
public:
    explicit StateBlinded(StateManager* const sm);
    //void Enter(CreatureObject* obj);
};
/*	Dizzy State
*
*/
class StateDizzy:
    public ActionState
{
public:
    explicit StateDizzy(StateManager* const sm);
    //void Enter(CreatureObject* obj);
};
/*	Intimidated State
*
*/
class StateIntimidated:
    public ActionState
{
public:
    explicit StateIntimidated(StateManager* const sm);
    //void Enter(CreatureObject* obj);
};
/*	Immobolized State
*
*/
class StateImmobolized:
    public ActionState
{
public:
    explicit StateImmobolized(StateManager* const sm);
    //void Enter(CreatureObject* obj);
};
/*	Frozen State
*
*/
class StateFrozen:
    public ActionState
{
public:
    explicit StateFrozen(StateManager* const sm);
    //void Enter(CreatureObject* obj);
};
/*	Swimming State
*
*/
class StateSwimming:
    public ActionState
{
public:
    explicit StateSwimming(StateManager* const sm);
    void Enter(CreatureObject* obj);
};
/*	SittingOnChair State
*
*/
class StateSittingOnChair:
    public ActionState
{
public:
    explicit StateSittingOnChair(StateManager* const sm);
    //void Enter(CreatureObject* obj);
};
/*	Crafting State
*
*/
class StateCrafting:
    public ActionState
{
public:
    explicit StateCrafting(StateManager* const sm);
    //void Enter(CreatureObject* obj);
};
/*	GlowingJedi State
*
*/
class StateGlowingJedi:
    public ActionState
{
public:
    explicit StateGlowingJedi(StateManager* const sm);
    //void Enter(CreatureObject* obj);
};
/*	MaskScent State
*
*/
class StateMaskScent:
    public ActionState
{
public:
    explicit StateMaskScent(StateManager* const sm);
    //void Enter(CreatureObject* obj);
};
/*	Poisoined State
*
*/
class StatePoisoined:
    public ActionState
{
public:
    explicit StatePoisoined(StateManager* const sm);
    //void Enter(CreatureObject* obj);
};
/*	Bleeding State
*
*/
class StateBleeding:
    public ActionState
{
public:
    explicit StateBleeding(StateManager* const sm);
    //void Enter(CreatureObject* obj);
};
/*	Diseased State
*
*/
class StateDiseased:
    public ActionState
{
public:
    explicit StateDiseased(StateManager* const sm);
    //void Enter(CreatureObject* obj);
};
/*	OnFire State
*
*/
class StateOnFire:
    public ActionState
{
public:
    explicit StateOnFire(StateManager* const sm);
    //void Enter(CreatureObject* obj);
};
/*	RidingMount State
*
*/
class StateRidingMount:
    public ActionState
{
public:
    explicit StateRidingMount(StateManager* const sm);
    //void Enter(CreatureObject* obj);
};
/*	MountedCreature State
*
*/
class StateMountedCreature:
    public ActionState
{
public:
    explicit StateMountedCreature(StateManager* const sm);
    //void Enter(CreatureObject* obj);
};
/*	PilotingShip State
*
*/
class StatePilotingShip:
    public ActionState
{
public:
    explicit StatePilotingShip(StateManager* const sm);
    //void Enter(CreatureObject* obj);
};
/*	ShipOperations State
*
*/
class StateShipOperations:
    public ActionState
{
public:
    explicit StateShipOperations(StateManager* const sm);
    //void Enter(CreatureObject* obj);
};
/*	ShipGunner State
*
*/
class StateShipGunner:
    public ActionState
{
public:
    explicit StateShipGunner(StateManager* const sm);
    //void Enter(CreatureObject* obj);
};
/*	ShipInterior State
*
*/
class StateShipInterior:
    public ActionState
{
public:
    explicit StateShipInterior(StateManager* const sm);
    //void Enter(CreatureObject* obj);
};
/*	PilotingPobShip State
*
*/
class StatePilotingPobShip:
    public ActionState
{
public:
    explicit StatePilotingPobShip(StateManager* const sm);
    //void Enter(CreatureObject* obj);
};
/*  ClearStates State
*
*/
class StateClear:
    public ActionState
{
public:
    explicit StateClear(StateManager* const sm);
    void Enter(CreatureObject* obj);
};