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
}


PostureState::~PostureState(void)
{
}

void PostureState::Enter(CreatureObject* obj)
{
    obj->setPosture(mStateID);
}
void PostureState::Exit(CreatureObject* obj)
{
    obj->setPosture(0);
}
bool PostureState::CanTransition(CreatureObject* obj)
{
    return true;
}

PostureUpright::PostureUpright() : PostureState()
{
    mStateID = CreaturePosture_Upright;
}

bool PostureUpright::CanTransition(CreatureObject* obj)
{
    return true;
}
//	Posture Crouched
PostureCrouched::PostureCrouched() : PostureState()
{
    mStateID = CreaturePosture_Crouched;
}

bool PostureCrouched::CanTransition(CreatureObject* obj)
{
    return true;
}
//	Posture Prone
PostureProne::PostureProne() : PostureState()
{
    mStateID = CreaturePosture_Prone;
}

bool PostureProne::CanTransition(CreatureObject* obj)
{
    return true;
}
//	Posture Sneaking
PostureSneaking::PostureSneaking() : PostureState()
{
    mStateID = CreaturePosture_Sneaking;
}

bool PostureSneaking::CanTransition(CreatureObject* obj)
{
    return true;
}
//	Posture Blocking
PostureBlocking::PostureBlocking() : PostureState()
{
    mStateID = CreaturePosture_Blocking;
}

bool PostureBlocking::CanTransition(CreatureObject* obj)
{
    return true;
}
//	Posture Climbing
PostureClimbing::PostureClimbing() : PostureState()
{
    mStateID = CreaturePosture_Climbing;
}

bool PostureClimbing::CanTransition(CreatureObject* obj)
{
    return true;
}
//	Posture Flying
PostureFlying::PostureFlying() : PostureState()
{
    mStateID = CreaturePosture_Flying;
}

bool PostureFlying::CanTransition(CreatureObject* obj)
{
    return true;
}
//	Posture Lying Down
PostureLyingDown::PostureLyingDown() : PostureState()
{
    mStateID = CreaturePosture_LyingDown;
}

bool PostureLyingDown::CanTransition(CreatureObject* obj)
{
    return true;
}
//	Posture Sitting
PostureSitting::PostureSitting() : PostureState()
{
    mStateID = CreaturePosture_Sitting;
}

bool PostureSitting::CanTransition(CreatureObject* obj)
{
    return true;
}
//	Posture Skill Animating
PostureSkillAnimating::PostureSkillAnimating() : PostureState()
{
    mStateID = CreaturePosture_SkillAnimating;
}

bool PostureSkillAnimating::CanTransition(CreatureObject* obj)
{
    return true;
}
//	Posture Driving Vehicle
PostureDrivingVehicle::PostureDrivingVehicle() : PostureState()
{
    mStateID = CreaturePosture_DrivingVehicle;
}

bool PostureDrivingVehicle::CanTransition(CreatureObject* obj)
{
    return true;
}
//	Posture Riding Creature
PostureRidingCreature::PostureRidingCreature() : PostureState()
{
    mStateID = CreaturePosture_RidingCreature;
}

bool PostureRidingCreature::CanTransition(CreatureObject* obj)
{
    return true;
}
//	Posture Knocked Down
PostureKnockedDown::PostureKnockedDown() : PostureState()
{
    mStateID = CreaturePosture_KnockedDown;
}

bool PostureKnockedDown::CanTransition(CreatureObject* obj)
{
    return true;
}
//	Posture Incapacitated
PostureIncapacitated::PostureIncapacitated() : PostureState()
{
    mStateID = CreaturePosture_Incapacitated;
}

bool PostureIncapacitated::CanTransition(CreatureObject* obj)
{
    return true;
}
//	Posture Dead
PostureDead::PostureDead() : PostureState()
{
    mStateID = CreaturePosture_Dead;
}
bool PostureDead::CanTransition(CreatureObject* obj)
{
    return true;
}