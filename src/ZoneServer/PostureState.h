/*
 This file is part of MMOServer. For more information, visit http://swganh.com
 
 Copyright (c) 2006 - 2011 The SWG:ANH Team

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#pragma once
#ifndef ANH_ZONESERVER_POSTURE_STATE_H
#define ANH_ZONESERVER_POSTURE_STATE_H

#include "AbstractState.h"
#include "PlayerObject.h"

class StateManager;
class PostureState :
    public IState
{
public:
    explicit PostureState(StateManager* const sm);
    virtual ~PostureState(void);
    /* Activates the Enter process for the character Posture
    *  
    */
    virtual void Enter(CreatureObject* obj);
    /* Activates the Exit process for the given Posture
    *  
    */
    virtual void Exit(CreatureObject* obj);
    /* Determines if the player can transition to the Posture
    *  
    */
    virtual bool CanTransition(CreatureObject* obj, uint64 newPosture);

    virtual const transitionList& returnTransitionList(){return mTransitionList;}

    virtual void insertIntoTransitionList(const std::pair<StateTypes, uint64>& pair);

    virtual uint64 getID() {return mStateID;}
    
protected:
    StateManager*           mStateManager;
    uint64					mStateID;
    bool					mHidden;
    // think of this as a can't transition list
    transitionList          mTransitionList;
    

};
/*	Upright Posture
*
*/
class PostureUpright:
    public PostureState
{
public:
    explicit PostureUpright(StateManager* const sm);
    void Enter(CreatureObject* obj);
};
#endif
/*	Crouched Posture
*
*/
class PostureCrouched:
    public PostureState
{
public:
    explicit PostureCrouched(StateManager* const sm);
    void Enter(CreatureObject* obj);
};
/*	Prone Posture
*
*/
class PostureProne:
    public PostureState
{
public:
  explicit PostureProne(StateManager* const sm);
  void Enter(CreatureObject* obj);
};
/*	Sneaking Posture
*
*/
class PostureSneaking:
    public PostureState
{
public:
   explicit PostureSneaking(StateManager* const sm);
};
/*	Blocking Posture
*
*/
class PostureBlocking:
    public PostureState
{
public:
    explicit PostureBlocking(StateManager* const sm);
};
/*	Climbing Posture
*
*/
class PostureClimbing:
    public PostureState
{
public:
    explicit PostureClimbing(StateManager* const sm);
};
/*	Flying Posture
*
*/
class PostureFlying:
    public PostureState
{
public:
    explicit PostureFlying(StateManager* const sm);
};
/*	LyingDown Posture
*
*/
class PostureLyingDown:
    public PostureState
{
public:
    explicit PostureLyingDown(StateManager* const sm);
    void Enter(CreatureObject* obj);
};
/*	Sitting Posture
*
*/
class PostureSitting:
    public PostureState
{
public:
    explicit PostureSitting(StateManager* const sm);
    void Enter(CreatureObject* obj);
};
/*	SkillAnimating Posture
*
*/
class PostureSkillAnimating:
    public PostureState
{
public:
    explicit PostureSkillAnimating(StateManager* const sm);
    void Enter(CreatureObject* obj);
};
/*	DrivingVehicle Posture
*
*/
class PostureDrivingVehicle:
    public PostureState
{
public:
    explicit PostureDrivingVehicle(StateManager* const sm);
    void Enter(CreatureObject* obj);
};
/*	RidingCreature Posture
*
*/
class PostureRidingCreature:
    public PostureState
{
public:
    explicit PostureRidingCreature(StateManager* const sm);
    void Enter(CreatureObject* obj);
};
/*	KnockedDown Posture
*
*/
class PostureKnockedDown:
    public PostureState
{
public:
    explicit PostureKnockedDown(StateManager* const sm);
    void Enter(CreatureObject* obj);
};
/*	Incapacitated Posture
*
*/
class PostureIncapacitated:
    public PostureState
{
public:
   explicit PostureIncapacitated(StateManager* const sm);
   void Enter(CreatureObject* obj);
};
/*	Dead Posture
*
*/
class PostureDead:
    public PostureState
{
public:
    explicit PostureDead(StateManager* const sm);
    void Enter(CreatureObject* obj);
};
