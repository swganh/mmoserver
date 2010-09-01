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
    virtual bool CanTransition(CreatureObject* obj, uint64 newState);

    virtual const std::vector<uint64>& statesToRemove(){return mStatesRemovalList;}
    
    virtual void insertIntoStateRemovalList(const uint64& state);

    virtual const transitionList& returnTransitionList(){return mTransitionList;}

    virtual void insertIntoTransitionList(const std::pair<StateTypes, uint64>& pair);

protected:
    stateRemove mStatesRemovalList;
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
    
    
    
};
/*	Aiming State
*
*/
class StateAiming:
    public ActionState
{
public:
    StateAiming();
    
    
    
};
/*	Alert State
*
*/
class StateAlert:
    public ActionState
{
public:
    StateAlert();
    
    
    
};
/*	Bersker State
*
*/
class StateBerserk:
    public ActionState
{
public:
    StateBerserk();
    
    
    
};
/*	Feign Death State
*
*/
class StateFeign:
    public ActionState
{
public:
    StateFeign();
    
    
    
};
/*	CombatAttitudeEvasive State
*
*/
class StateCombatEvasive:
    public ActionState
{
public:
    StateCombatEvasive();
    
    
    
};
/*	CombatAttitudeNormal State
*
*/
class StateCombatNormal:
    public ActionState
{
public:
    StateCombatNormal();
    
    
    
};
/*	CombatAttitudeAggressive State
*
*/
class StateCombatAggressive:
    public ActionState
{
public:
    StateCombatAggressive();
    
    
    
};
/*	Tumbling State
*
*/
class StateTumbling:
    public ActionState
{
public:
    StateTumbling();
    
    
    
};
/*	Rallied State
*
*/
class StateRallied:
    public ActionState
{
public:
    StateRallied();
    
    
    
};
/*	Stunned State
*
*/
class StateStunned:
    public ActionState
{
public:
    StateStunned();
    
    
    
};
/*	Blinded State
*
*/
class StateBlinded:
    public ActionState
{
public:
    StateBlinded();
    
    
    
};
/*	Dizzy State
*
*/
class StateDizzy:
    public ActionState
{
public:
    StateDizzy();
    
    
    
};
/*	Intimidated State
*
*/
class StateIntimidated:
    public ActionState
{
public:
    StateIntimidated();
    
    
    
};
/*	Immobolized State
*
*/
class StateImmobolized:
    public ActionState
{
public:
    StateImmobolized();
    
    
    
};
/*	Frozen State
*
*/
class StateFrozen:
    public ActionState
{
public:
    StateFrozen();
    
    
    
};
/*	Swimming State
*
*/
class StateSwimming:
    public ActionState
{
public:
    StateSwimming();
    
    
    
};
/*	SittingOnChair State
*
*/
class StateSittingOnChair:
    public ActionState
{
public:
    StateSittingOnChair();
    
    
    
};
/*	Crafting State
*
*/
class StateCrafting:
    public ActionState
{
public:
    StateCrafting();
    
    
    
};
/*	GlowingJedi State
*
*/
class StateGlowingJedi:
    public ActionState
{
public:
    StateGlowingJedi();
    
    
    
};
/*	MaskScent State
*
*/
class StateMaskScent:
    public ActionState
{
public:
    StateMaskScent();
    
    
    
};
/*	Poisoined State
*
*/
class StatePoisoined:
    public ActionState
{
public:
    StatePoisoined();
    
    
    
};
/*	Bleeding State
*
*/
class StateBleeding:
    public ActionState
{
public:
    StateBleeding();
    
    
    
};
/*	Diseased State
*
*/
class StateDiseased:
    public ActionState
{
public:
    StateDiseased();
    
    
    
};
/*	OnFire State
*
*/
class StateOnFire:
    public ActionState
{
public:
    StateOnFire();
    
    
    
};
/*	RidingMount State
*
*/
class StateRidingMount:
    public ActionState
{
public:
    StateRidingMount();
    
    
    
};
/*	MountedCreature State
*
*/
class StateMountedCreature:
    public ActionState
{
public:
    StateMountedCreature();
    
    
    
};
/*	PilotingShip State
*
*/
class StatePilotingShip:
    public ActionState
{
public:
    StatePilotingShip();
    
    
    
};
/*	ShipOperations State
*
*/
class StateShipOperations:
    public ActionState
{
public:
    StateShipOperations();
    
    
    
};
/*	ShipGunner State
*
*/
class StateShipGunner:
    public ActionState
{
public:
    StateShipGunner();
    
    
    
};
/*	ShipInterior State
*
*/
class StateShipInterior:
    public ActionState
{
public:
    StateShipInterior();
    
    
    
};
/*	PilotingPobShip State
*
*/
class StatePilotingPobShip:
    public ActionState
{
public:
    StatePilotingPobShip();
    
    
    
};