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

class LocomotionState :
    public IState
{
public:
    LocomotionState(void);
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

    virtual transitionList returnTransitionList(){return mTransitionList;}

    virtual void insertIntoTransitionList(std::pair<StateTypes, uint64> pair);
protected:
    transitionList  mTransitionList;
};
#endif
/* Standing Locomotion
*
*/
class LocomotionStanding:
    public LocomotionState
{
public:
    LocomotionStanding();
};
/*	Sneaking Locomotion
*
*/
class LocomotionSneaking:
    public LocomotionState
{
public:
    LocomotionSneaking();
};
/*	Walking Locomotion
*
*/
class LocomotionWalking:
    public LocomotionState
{
public:
    LocomotionWalking();
};
/*	Running Locomotion
*
*/
class LocomotionRunning:
    public LocomotionState
{
public:
    LocomotionRunning();
};
/*  Kneeling Locomotion
*
*/
class LocomotionKneeling:
    public LocomotionState
{
public:
    LocomotionKneeling();
};
/*	CrouchSneaking Locomotion
*
*/
class LocomotionCrouchSneaking:
    public LocomotionState
{
public:
    LocomotionCrouchSneaking();
};
/*	CrouchWalking Locomotion
*
*/
class LocomotionCrouchWalking:
    public LocomotionState
{
public:
    LocomotionCrouchWalking();
};
/*	Prone Locomotion
*
*/
class LocomotionProne:
    public LocomotionState
{
public:
    LocomotionProne();
};
/*	Crawling Locomotion
*
*/
class LocomotionCrawling:
    public LocomotionState
{
public:
    LocomotionCrawling();
};
/*	ClimbingStationary Locomotion
*
*/
class LocomotionClimbingStationary:
    public LocomotionState
{
public:
    LocomotionClimbingStationary();
};
/*	Climbing Locomotion
*
*/
class LocomotionClimbing:
    public LocomotionState
{
public:
    LocomotionClimbing();
};
/*	Hovering Locomotion
*
*/
class LocomotionHovering:
    public LocomotionState
{
public:
    LocomotionHovering();
};
/*	Flying Locomotion
*
*/
class LocomotionFlying:
    public LocomotionState
{
public:
    LocomotionFlying();
};
/*	LyingDown Locomotion
*
*/
class LocomotionLyingDown:
    public LocomotionState
{
public:
    LocomotionLyingDown();
};
/*	Sitting Locomotion
*
*/
class LocomotionSitting:
    public LocomotionState
{
public:
    LocomotionSitting();
};
/*	SkillAnimating Locomotion
*
*/
class LocomotionSkillAnimating:
    public LocomotionState
{
public:
    LocomotionSkillAnimating();
};
/*	DrivingVehicle Locomotion
*
*/
class LocomotionDrivingVehicle:
    public LocomotionState
{
public:
    LocomotionDrivingVehicle();
};
/*	RidingCreature Locomotion
*
*/
class LocomotionRidingCreature:
    public LocomotionState
{
public:
    LocomotionRidingCreature();
};
/*	KnockedDown Locomotion
*
*/
class LocomotionKnockedDown:
    public LocomotionState
{
public:
    LocomotionKnockedDown();
};
/*	Incapacitated Locomotion
*
*/
class LocomotionIncapacitated:
    public LocomotionState
{
public:
    LocomotionIncapacitated();
};
/*	Dead Locomotion
*
*/
class LocomotionDead:
    public LocomotionState
{
public:
    LocomotionDead();
};
/*	Blocking Locomotion
*
*/
class LocomotionBlocking:
    public LocomotionState
{
public:
    LocomotionBlocking();
};