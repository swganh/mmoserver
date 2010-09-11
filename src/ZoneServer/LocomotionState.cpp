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

#include "LocomotionState.h"


LocomotionState::LocomotionState(StateManager* const sm) : IState(),
    mStateManager(sm), mStateID(0), mBlocked(false){}

LocomotionState::~LocomotionState(void)
{
    mStateID = 0;
    mBlocked = false;
    mTransitionList.clear();
}

void LocomotionState::Enter(CreatureObject* obj)
{
    obj->setLocomotion(mStateID);
}
void LocomotionState::Exit(CreatureObject* obj)
{
    obj->setLocomotion(0);
}
bool LocomotionState::CanTransition(CreatureObject* obj, uint64 newLocomotionState)
{ 
    bool transitionPosture    = true;
    bool transitionAction     = true;
    bool transitionLocomotion = true;
    // check to see if the layer is blocked
    if (mBlocked)
        return false;
    transitionList::iterator itPosture    = mTransitionList.find(State_Posture);
    transitionList::iterator itAction     = mTransitionList.find(State_Action);
    transitionList::iterator itLocomotion = mTransitionList.find(State_Locomotion);
    // check each state type
    while (itPosture != mTransitionList.end())
    {
        // are we allowed to transition based on posture?
        CreaturePosture pos = (CreaturePosture)(*itPosture).second;
        if (pos != obj->getPosture())
        {
            transitionPosture = false;
            break;
        }
        ++itPosture;
    }
    while (itAction != mTransitionList.end())
    {
        uint64 state = (*itAction).second;
        if ((state & obj->getState()) == obj->getState())
        {
            transitionAction = false;
            break;
        }
        ++itAction;
    }
    while (itLocomotion != mTransitionList.end())
    {
        CreatureLocomotion locomotion = (CreatureLocomotion)(*itAction).second;
        if (locomotion != obj->getLocomotion())
        {
            transitionLocomotion = false;
            break;
        }
        ++itLocomotion;
    }
    return transitionAction && transitionPosture && transitionLocomotion;
}
void LocomotionState::insertIntoTransitionList(const std::pair<StateTypes, uint64>& types)
{
    mTransitionList.insert(types);
}

LocomotionStanding::LocomotionStanding(StateManager* const sm) : LocomotionState(sm)
{
    mStateID = CreatureLocomotion_Standing;
}
LocomotionSneaking::LocomotionSneaking(StateManager* const sm) : LocomotionState(sm)
{
    mStateID = CreatureLocomotion_Sneaking;
}
LocomotionWalking::LocomotionWalking(StateManager* const sm) : LocomotionState(sm)
{
    mStateID = CreatureLocomotion_Walking;
}
LocomotionRunning::LocomotionRunning(StateManager* const sm) : LocomotionState(sm)
{
    mStateID = CreatureLocomotion_Running;
}
LocomotionKneeling::LocomotionKneeling(StateManager* const sm) : LocomotionState(sm)
{
    mStateID = CreatureLocomotion_Kneeling;
}
LocomotionCrouchSneaking::LocomotionCrouchSneaking(StateManager* const sm) : LocomotionState(sm)
{
    mStateID = CreatureLocomotion_CrouchSneaking;
}
LocomotionCrouchWalking::LocomotionCrouchWalking(StateManager* const sm) : LocomotionState(sm)
{
    mStateID = CreatureLocomotion_CrouchWalking;
}
LocomotionProne::LocomotionProne(StateManager* const sm) : LocomotionState(sm)
{
    mStateID = CreatureLocomotion_Prone;
}
LocomotionCrawling::LocomotionCrawling(StateManager* const sm) : LocomotionState(sm)
{
    mStateID = CreatureLocomotion_Crawling;
}
LocomotionClimbingStationary::LocomotionClimbingStationary(StateManager* const sm) : LocomotionState(sm)
{
    mStateID = CreatureLocomotion_ClimbingStationary;
}
LocomotionClimbing::LocomotionClimbing(StateManager* const sm) : LocomotionState(sm)
{
    mStateID = CreatureLocomotion_Climbing;
}
LocomotionHovering::LocomotionHovering(StateManager* const sm) : LocomotionState(sm)
{
    mStateID = CreatureLocomotion_Hovering;
}
LocomotionFlying::LocomotionFlying(StateManager* const sm) : LocomotionState(sm)
{
    mStateID = CreatureLocomotion_Flying;
}
LocomotionLyingDown::LocomotionLyingDown(StateManager* const sm) : LocomotionState(sm)
{
    mStateID = CreatureLocomotion_LyingDown;
}
LocomotionSitting::LocomotionSitting(StateManager* const sm) : LocomotionState(sm)
{
    mStateID = CreatureLocomotion_Sitting;
}
LocomotionSkillAnimating::LocomotionSkillAnimating(StateManager* const sm) : LocomotionState(sm)
{
    mStateID = CreatureLocomotion_SkillAnimating;
}
LocomotionDrivingVehicle::LocomotionDrivingVehicle(StateManager* const sm) : LocomotionState(sm)
{
    mStateID = CreatureLocomotion_DrivingVehicle;
}
LocomotionRidingCreature::LocomotionRidingCreature(StateManager* const sm) : LocomotionState(sm)
{
    mStateID = CreatureLocomotion_RidingCreature;
}
LocomotionKnockedDown::LocomotionKnockedDown(StateManager* const sm) : LocomotionState(sm)
{
    mStateID = CreatureLocomotion_KnockedDown;
}
LocomotionIncapacitated::LocomotionIncapacitated(StateManager* const sm) : LocomotionState(sm)
{
    mStateID = CreatureLocomotion_Incapacitated;
}
LocomotionDead::LocomotionDead(StateManager* const sm) : LocomotionState(sm)
{
    mStateID = CreatureLocomotion_Dead;
}
LocomotionBlocking::LocomotionBlocking(StateManager* const sm) : LocomotionState(sm)
{
    mStateID = CreatureLocomotion_Blocking;
}