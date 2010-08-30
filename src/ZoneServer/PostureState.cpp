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

#include "PostureState.h"


PostureState::PostureState(void)
{
    mStateID = 0;
    mTransitionList->clear();
}

PostureState::~PostureState(void)
{
    mStateID = 0;
    mTransitionList->clear();
}

void PostureState::Enter(CreatureObject* obj)
{
    obj->setPosture(mStateID);
}
void PostureState::Exit(CreatureObject* obj)
{
    obj->setPosture(0);
}
bool PostureState::CanTransition(CreatureObject* obj, uint64 newPosture)
{
    bool transitionPosture    = false;
    bool transitionAction     = false;
    bool transitionLocomotion = false;
    // check to see if the layer is blocked
    if (mBlocked)
        return false;
    transitionList::iterator itPosture    = mTransitionList->find(State_Posture);
    transitionList::iterator itAction     = mTransitionList->find(State_Action);
    transitionList::iterator itLocomotion = mTransitionList->find(State_Locomotion);
    // check each state type
    while (itPosture != mTransitionList->end())
    {
        // are we allowed to transition based on posture?
        CreaturePosture pos = (CreaturePosture)(*itPosture).second;
        if (pos == obj->getPosture())
        {
            transitionPosture = true;
            break;
        }
    }
    while (itAction != mTransitionList->end())
    {
        // function here to go through states and pull one at a time to check

        CreatureState state = (CreatureState)(*itAction).second;
        if (state == obj->getState())
        {
            transitionAction = true;
            break;
        }
    }
    while (itLocomotion != mTransitionList->end())
    {
        CreatureLocomotion locomotion = (CreatureLocomotion)(*itAction).second;
        if (locomotion == obj->getLocomotion())
        {
            transitionLocomotion = true;
            break;
        }
    }
    return transitionAction && transitionPosture && transitionLocomotion;
}

PostureUpright::PostureUpright() : PostureState()
{
    mStateID = CreaturePosture_Upright;
}

//	Posture Crouched
PostureCrouched::PostureCrouched() : PostureState()
{
    mStateID = CreaturePosture_Crouched;
}
//	Posture Prone
PostureProne::PostureProne() : PostureState()
{
    mStateID = CreaturePosture_Prone;
}
//	Posture Sneaking
PostureSneaking::PostureSneaking() : PostureState()
{
    mStateID = CreaturePosture_Sneaking;
}
//Posture Blocking
PostureBlocking::PostureBlocking() : PostureState()
{
    mStateID = CreaturePosture_Blocking;
}
//	Posture Climbing
PostureClimbing::PostureClimbing() : PostureState()
{
    mStateID = CreaturePosture_Climbing;
}
//	Posture Flying
PostureFlying::PostureFlying() : PostureState()
{
    mStateID = CreaturePosture_Flying;
}
//	Posture Lying Down
PostureLyingDown::PostureLyingDown() : PostureState()
{
    mStateID = CreaturePosture_LyingDown;
}
//	Posture Sitting
PostureSitting::PostureSitting() : PostureState()
{
    mStateID = CreaturePosture_Sitting;
}
//	Posture Skill Animating
PostureSkillAnimating::PostureSkillAnimating() : PostureState()
{
    mStateID = CreaturePosture_SkillAnimating;
}
//	Posture Driving Vehicle
PostureDrivingVehicle::PostureDrivingVehicle() : PostureState()
{
    mStateID = CreaturePosture_DrivingVehicle;
}
//	Posture Riding Creature
PostureRidingCreature::PostureRidingCreature() : PostureState()
{
    mStateID = CreaturePosture_RidingCreature;
}
//	Posture Knocked Down
PostureKnockedDown::PostureKnockedDown() : PostureState()
{
    mStateID = CreaturePosture_KnockedDown;
}
//	Posture Incapacitated
PostureIncapacitated::PostureIncapacitated() : PostureState()
{
    mStateID = CreaturePosture_Incapacitated;
}
//	Posture Dead
PostureDead::PostureDead() : PostureState()
{
    mStateID = CreaturePosture_Dead;
}