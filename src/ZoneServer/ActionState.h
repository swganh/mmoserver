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

    virtual void loadTransitionList();

    virtual uint64 getID() {return mStateID;}

protected:
    stateRemove             mStatesRemovalList;
    StateManager*     mStateManager;
    bool                    mBlocked;
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
    StateCover(StateManager* const sm);
    
};
/*
*	Combat State
*
*/
class StateCombat:
    public ActionState
{
public:
    StateCombat(StateManager* const sm);
    
    
    
};
/*
*	Peace State
*
*/
class StatePeace:
    public ActionState
{
public:
    StatePeace(StateManager* const sm);
    
    
    
};
/*	Aiming State
*
*/
class StateAiming:
    public ActionState
{
public:
    StateAiming(StateManager* const sm);
    
    
    
};
/*	Alert State
*
*/
class StateAlert:
    public ActionState
{
public:
    StateAlert(StateManager* const sm);
    
    
    
};
/*	Bersker State
*
*/
class StateBerserk:
    public ActionState
{
public:
    StateBerserk(StateManager* const sm);
    
    
    
};
/*	Feign Death State
*
*/
class StateFeign:
    public ActionState
{
public:
    StateFeign(StateManager* const sm);
    
    
    
};
/*	CombatAttitudeEvasive State
*
*/
class StateCombatEvasive:
    public ActionState
{
public:
    StateCombatEvasive(StateManager* const sm);
    
    
    
};
/*	CombatAttitudeNormal State
*
*/
class StateCombatNormal:
    public ActionState
{
public:
    StateCombatNormal(StateManager* const sm);
    
    
    
};
/*	CombatAttitudeAggressive State
*
*/
class StateCombatAggressive:
    public ActionState
{
public:
    StateCombatAggressive(StateManager* const sm);
    
    
    
};
/*	Tumbling State
*
*/
class StateTumbling:
    public ActionState
{
public:
    StateTumbling(StateManager* const sm);
    
    
    
};
/*	Rallied State
*
*/
class StateRallied:
    public ActionState
{
public:
    StateRallied(StateManager* const sm);
    
    
    
};
/*	Stunned State
*
*/
class StateStunned:
    public ActionState
{
public:
    StateStunned(StateManager* const sm);
    
    
    
};
/*	Blinded State
*
*/
class StateBlinded:
    public ActionState
{
public:
    StateBlinded(StateManager* const sm);
    
    
    
};
/*	Dizzy State
*
*/
class StateDizzy:
    public ActionState
{
public:
    StateDizzy(StateManager* const sm);
    
    
    
};
/*	Intimidated State
*
*/
class StateIntimidated:
    public ActionState
{
public:
    StateIntimidated(StateManager* const sm);
    
    
    
};
/*	Immobolized State
*
*/
class StateImmobolized:
    public ActionState
{
public:
    StateImmobolized(StateManager* const sm);
    
    
    
};
/*	Frozen State
*
*/
class StateFrozen:
    public ActionState
{
public:
    StateFrozen(StateManager* const sm);
    
    
    
};
/*	Swimming State
*
*/
class StateSwimming:
    public ActionState
{
public:
    StateSwimming(StateManager* const sm);
    
    
    
};
/*	SittingOnChair State
*
*/
class StateSittingOnChair:
    public ActionState
{
public:
    StateSittingOnChair(StateManager* const sm);
    
    
    
};
/*	Crafting State
*
*/
class StateCrafting:
    public ActionState
{
public:
    StateCrafting(StateManager* const sm);
    
    
    
};
/*	GlowingJedi State
*
*/
class StateGlowingJedi:
    public ActionState
{
public:
    StateGlowingJedi(StateManager* const sm);
    
    
    
};
/*	MaskScent State
*
*/
class StateMaskScent:
    public ActionState
{
public:
    StateMaskScent(StateManager* const sm);
    
    
    
};
/*	Poisoined State
*
*/
class StatePoisoined:
    public ActionState
{
public:
    StatePoisoined(StateManager* const sm);
    
    
    
};
/*	Bleeding State
*
*/
class StateBleeding:
    public ActionState
{
public:
    StateBleeding(StateManager* const sm);
    
    
    
};
/*	Diseased State
*
*/
class StateDiseased:
    public ActionState
{
public:
    StateDiseased(StateManager* const sm);
    
    
    
};
/*	OnFire State
*
*/
class StateOnFire:
    public ActionState
{
public:
    StateOnFire(StateManager* const sm);
    
    
    
};
/*	RidingMount State
*
*/
class StateRidingMount:
    public ActionState
{
public:
    StateRidingMount(StateManager* const sm);
    
    
    
};
/*	MountedCreature State
*
*/
class StateMountedCreature:
    public ActionState
{
public:
    StateMountedCreature(StateManager* const sm);
    
    
    
};
/*	PilotingShip State
*
*/
class StatePilotingShip:
    public ActionState
{
public:
    StatePilotingShip(StateManager* const sm);
    
    
    
};
/*	ShipOperations State
*
*/
class StateShipOperations:
    public ActionState
{
public:
    StateShipOperations(StateManager* const sm);
    
    
    
};
/*	ShipGunner State
*
*/
class StateShipGunner:
    public ActionState
{
public:
    StateShipGunner(StateManager* const sm);
    
    
    
};
/*	ShipInterior State
*
*/
class StateShipInterior:
    public ActionState
{
public:
    StateShipInterior(StateManager* const sm);
    
    
    
};
/*	PilotingPobShip State
*
*/
class StatePilotingPobShip:
    public ActionState
{
public:
    StatePilotingPobShip(StateManager* const sm);
    
    
    
};