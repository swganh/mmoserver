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


LocomotionState::LocomotionState(void)
{
    mStateID = 0;
    mTransitionList.clear();
}

LocomotionState::~LocomotionState(void)
{
}

void LocomotionState::Enter(CreatureObject* obj)
{
    obj->setLocomotion(mStateID);
}
void LocomotionState::Exit(CreatureObject* obj)
{
    obj->setLocomotion(0);
}
bool LocomotionState::CanTransition(uint64 newLocomotionState)
{
    transitionList::iterator itPosture = mTransitionList.find(State_Posture);
    transitionList::iterator itAction = mTransitionList.find(State_Action);
    transitionList::iterator itLocomotion = mTransitionList.find(State_Locomotion);
    // check each state type
    if (itPosture != mTransitionList.end())
    {
        // check the bitmask to see if we're allowed
        uint32 bitmask = itPosture->second;
        if((newLocomotionState & bitmask) == 0)
            return false;
    }
    else if (itAction != mTransitionList.end())
    {
        // check the bitmask to see if we're allowed
        uint32 bitmask = itAction->second;
        if((newLocomotionState & bitmask) != 0)
            return false;
    }
    else if (itLocomotion != mTransitionList.end())
    {
        // check the bitmask to see if we're allowed
        uint32 bitmask = itLocomotion->second;
        if((newLocomotionState & bitmask) != 0)
            return false;
    }

    return true;
}

LocomotionStanding::LocomotionStanding()
{
    mStateID = CreatureLocomotion_Standing;
}
LocomotionSneaking::LocomotionSneaking()
{
    mStateID = CreatureLocomotion_Sneaking;

}
LocomotionWalking::LocomotionWalking()
{
    mStateID = CreatureLocomotion_Walking;
}
LocomotionRunning::LocomotionRunning()
{
    mStateID = CreatureLocomotion_Running;
}
LocomotionKneeling::LocomotionKneeling()
{
    mStateID = CreatureLocomotion_Kneeling;
}
LocomotionCrouchSneaking::LocomotionCrouchSneaking()
{
    mStateID = CreatureLocomotion_CrouchSneaking;
}
LocomotionCrouchWalking::LocomotionCrouchWalking()
{
    mStateID = CreatureLocomotion_CrouchWalking;
}
LocomotionProne::LocomotionProne()
{
    mStateID = CreatureLocomotion_Prone;
}
LocomotionCrawling::LocomotionCrawling()
{
    mStateID = CreatureLocomotion_Crawling;
}
LocomotionClimbingStationary::LocomotionClimbingStationary()
{
    mStateID = CreatureLocomotion_ClimbingStationary;
}
LocomotionClimbing::LocomotionClimbing()
{
    mStateID = CreatureLocomotion_Climbing;
}
LocomotionHovering::LocomotionHovering()
{
    mStateID = CreatureLocomotion_Hovering;
}
LocomotionFlying::LocomotionFlying()
{
    mStateID = CreatureLocomotion_Flying;
}
LocomotionLyingDown::LocomotionLyingDown()
{
    mStateID = CreatureLocomotion_LyingDown;
}
LocomotionSitting::LocomotionSitting()
{
    mStateID = CreatureLocomotion_Sitting;
}
LocomotionSkillAnimating::LocomotionSkillAnimating()
{
    mStateID = CreatureLocomotion_SkillAnimating;
}
LocomotionDrivingVehicle::LocomotionDrivingVehicle()
{
    mStateID = CreatureLocomotion_DrivingVehicle;
}
LocomotionRidingCreature::LocomotionRidingCreature()
{
    mStateID = CreatureLocomotion_RidingCreature;
}
LocomotionKnockedDown::LocomotionKnockedDown()
{
    mStateID = CreatureLocomotion_KnockedDown;
}
LocomotionIncapacitated::LocomotionIncapacitated()
{
    mStateID = CreatureLocomotion_Incapacitated;
}
LocomotionDead::LocomotionDead()
{
    mStateID = CreatureLocomotion_Dead;
}
LocomotionBlocking::LocomotionBlocking()
{
    mStateID = CreatureLocomotion_Blocking;
}