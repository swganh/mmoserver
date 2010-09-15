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
#ifndef ANH_ZONESERVER_LOCOMOTION_STATE_H
#define ANH_ZONESERVER_LOCOMOTION_STATE_H

#include "AbstractState.h"
#include "CreatureObject.h"

class StateManager;
class LocomotionState :
    public IState
{
public:
    LocomotionState(StateManager* const sm);
    virtual ~LocomotionState(void);
    /* Activates the Enter process for the character Locomotion
    *  
    */
    virtual void Enter(CreatureObject* obj);
    /* Activates the Exit process for the given Locomotion
    *  
    */
    virtual void Exit(CreatureObject* obj);
    /* Determines if the player can transition to the Locomotion
    *  
    */
    virtual bool CanTransition(CreatureObject* obj, uint64 newLocomotionState);

    virtual const transitionList& returnTransitionList(){return mTransitionList;}

    virtual void insertIntoTransitionList(const std::pair<StateTypes, uint64>& pair);

    virtual uint64 getID() {return mStateID;}

protected:
    StateManager*           mStateManager;
    uint64					mStateID;
    bool					mHidden;
    transitionList          mTransitionList;
};
#endif
/* Standing Locomotion
*
*/
class LocomotionStanding:
    public LocomotionState
{
public:
    explicit LocomotionStanding(StateManager* const sm);
    //void Enter(CreatureObject* obj);
};
/*	Sneaking Locomotion
*
*/
class LocomotionSneaking:
    public LocomotionState
{
public:
    explicit LocomotionSneaking(StateManager* const sm);
};
/*	Walking Locomotion
*
*/
class LocomotionWalking:
    public LocomotionState
{
public:
    explicit LocomotionWalking(StateManager* const sm);
    //void Enter(CreatureObject* obj);
};
/*	Running Locomotion
*
*/
class LocomotionRunning:
    public LocomotionState
{
public:
    explicit LocomotionRunning(StateManager* const sm);
    //void Enter(CreatureObject* obj);
};
/*  Kneeling Locomotion
*
*/
class LocomotionKneeling:
    public LocomotionState
{
public:
    explicit LocomotionKneeling(StateManager* const sm);
    //void Enter(CreatureObject* obj);
};
/*	CrouchSneaking Locomotion
*
*/
class LocomotionCrouchSneaking:
    public LocomotionState
{
public:
    LocomotionCrouchSneaking(StateManager* const sm);
};
/*	CrouchWalking Locomotion
*
*/
class LocomotionCrouchWalking:
    public LocomotionState
{
public:
    LocomotionCrouchWalking(StateManager* const sm);
};
/*	Prone Locomotion
*
*/
class LocomotionProne:
    public LocomotionState
{
public:
    explicit LocomotionProne(StateManager* const sm);
    //void Enter(CreatureObject* obj);
};
/*	Crawling Locomotion
*
*/
class LocomotionCrawling:
    public LocomotionState
{
public:
    LocomotionCrawling(StateManager* const sm);
};
/*	ClimbingStationary Locomotion
*
*/
class LocomotionClimbingStationary:
    public LocomotionState
{
public:
    LocomotionClimbingStationary(StateManager* const sm);
};
/*	Climbing Locomotion
*
*/
class LocomotionClimbing:
    public LocomotionState
{
public:
    LocomotionClimbing(StateManager* const sm);
};
/*	Hovering Locomotion
*
*/
class LocomotionHovering:
    public LocomotionState
{
public:
    LocomotionHovering(StateManager* const sm);
};
/*	Flying Locomotion
*
*/
class LocomotionFlying:
    public LocomotionState
{
public:
    LocomotionFlying(StateManager* const sm);
};
/*	LyingDown Locomotion
*
*/
class LocomotionLyingDown:
    public LocomotionState
{
public:
    LocomotionLyingDown(StateManager* const sm);
};
/*	Sitting Locomotion
*
*/
class LocomotionSitting:
    public LocomotionState
{
public:
    explicit LocomotionSitting(StateManager* const sm);
    //void Enter(CreatureObject* obj);
};
/*	SkillAnimating Locomotion
*
*/
class LocomotionSkillAnimating:
    public LocomotionState
{
public:
    explicit LocomotionSkillAnimating(StateManager* const sm);
    //void Enter(CreatureObject* obj);
};
/*	DrivingVehicle Locomotion
*
*/
class LocomotionDrivingVehicle:
    public LocomotionState
{
public:
    explicit LocomotionDrivingVehicle(StateManager* const sm);
    //void Enter(CreatureObject* obj);
};
/*	RidingCreature Locomotion
*
*/
class LocomotionRidingCreature:
    public LocomotionState
{
public:
    explicit LocomotionRidingCreature(StateManager* const sm);
    //void Enter(CreatureObject* obj);
};
/*	KnockedDown Locomotion
*
*/
class LocomotionKnockedDown:
    public LocomotionState
{
public:
    explicit LocomotionKnockedDown(StateManager* const sm);
    //void Enter(CreatureObject* obj);
};
/*	Incapacitated Locomotion
*
*/
class LocomotionIncapacitated:
    public LocomotionState
{
public:
    explicit LocomotionIncapacitated(StateManager* const sm);
    //void Enter(CreatureObject* obj);
};
/*	Dead Locomotion
*
*/
class LocomotionDead:
    public LocomotionState
{
public:
    explicit LocomotionDead(StateManager* const sm);
    //void Enter(CreatureObject* obj);
};
/*	Blocking Locomotion
*
*/
class LocomotionBlocking:
    public LocomotionState
{
public:
    LocomotionBlocking(StateManager* const sm);
};