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

bool PostureState::Enter(CreatureObject* obj)
{
    obj->setPosture(mStateID);
    return true;
}
bool PostureState::Exit(CreatureObject* obj)
{
    return true;
}
bool PostureState::CanTransition(CreatureObject* obj)
{
    return true;
}

PostureUpright::PostureUpright() : PostureState()
{
    mStateID = CreaturePosture_Upright;
}
bool PostureUpright::Enter(CreatureObject* obj)
{
    obj->setPosture(this->mStateID);

    PlayerObject*  player = dynamic_cast<PlayerObject*>(obj);

    if(player)
    {
        // see if we need to get out of sampling mode
        if(player->getSamplingState())
        {
            gMessageLib->SendSystemMessage(::common::OutOfBand("survey", "sample_cancel"), player);
            player->setSamplingState(false);
        }

        if(player->checkPlayerCustomFlag(PlayerCustomFlag_LogOut))
        {
            player->togglePlayerCustomFlagOff(PlayerCustomFlag_LogOut);
            gMessageLib->SendSystemMessage(::common::OutOfBand("logout", "aborted"), player);	
        }

        //if player is seated on an a chair, hack-fix clientside bug by manually sending client message
        bool IsSeatedOnChair = player->checkState(CreatureState_SittingOnChair);
        if(IsSeatedOnChair)
        {
            gMessageLib->SendSystemMessage(::common::OutOfBand("shared", "player_stand"), player);	
        }
    }

    return true;
}
bool PostureUpright::Exit(CreatureObject* obj)
{
    return true;
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
bool PostureCrouched::Enter(CreatureObject* obj)
{
    //Get whether player is seated on a chair before we toggle it
    bool IsSeatedOnChair = obj->checkState(CreatureState_SittingOnChair);
  
    obj->setPosture(CreaturePosture_Crouched);

    //if player is seated on an a chair, hack-fix clientside bug by manually sending client message
    PlayerObject*  player = dynamic_cast<PlayerObject*>(obj);
    if(IsSeatedOnChair && player)
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("shared", "player_kneel"), player);
    }

    return true;
}
bool PostureCrouched::Exit(CreatureObject* obj)
{
    return true;
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
bool PostureProne::Enter(CreatureObject* obj)
{
    obj->setPosture(mStateID);

    //if player is seated on an a chair, hack-fix clientside bug by manually sending client message
    bool IsSeatedOnChair = obj->checkState(CreatureState_SittingOnChair);
    PlayerObject*  player = dynamic_cast<PlayerObject*>(obj);
    if(IsSeatedOnChair && player)
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("shared", "player_prone"), player);

        // see if we need to get out of sampling mode
        if(player->getSamplingState())
        {
            gMessageLib->SendSystemMessage(::common::OutOfBand("survey", "sample_cancel"), player);
            player->setSamplingState(false);
        }

        if(player->checkPlayerCustomFlag(PlayerCustomFlag_LogOut))
        {
            player->togglePlayerCustomFlagOff(PlayerCustomFlag_LogOut);
            gMessageLib->SendSystemMessage(::common::OutOfBand("logout", "aborted"), player);	
        }
    }

    return true;
}
bool PostureProne::Exit(CreatureObject* obj)
{
    return true;
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
bool PostureSneaking::Enter(CreatureObject* obj)
{
    obj->setPosture(mStateID);
    return true;
}
bool PostureSneaking::Exit(CreatureObject* obj)
{
    return true;
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
bool PostureBlocking::Enter(CreatureObject* obj)
{
    obj->setPosture(mStateID);
    return true;
}
bool PostureBlocking::Exit(CreatureObject* obj)
{
    return true;
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
bool PostureClimbing::Enter(CreatureObject* obj)
{
    obj->setPosture(mStateID);
    return true;
}
bool PostureClimbing::Exit(CreatureObject* obj)
{
    return true;
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
bool PostureFlying::Enter(CreatureObject* obj)
{
    obj->setPosture(mStateID);
    return true;
}
bool PostureFlying::Exit(CreatureObject* obj)
{
    return true;
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
bool PostureLyingDown::Enter(CreatureObject* obj)
{
    obj->setPosture(mStateID);
    return true;
}
bool PostureLyingDown::Exit(CreatureObject* obj)
{
    return true;
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
bool PostureSitting::Enter(CreatureObject* obj)
{
    obj->setPosture(mStateID);
    return true;
}
bool PostureSitting::Exit(CreatureObject* obj)
{
    return true;
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
bool PostureSkillAnimating::Enter(CreatureObject* obj)
{
    obj->setPosture(mStateID);
    return true;
}
bool PostureSkillAnimating::Exit(CreatureObject* obj)
{
    return true;
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
bool PostureDrivingVehicle::Enter(CreatureObject* obj)
{
    obj->setPosture(mStateID);
    return true;
}
bool PostureDrivingVehicle::Exit(CreatureObject* obj)
{
    return true;
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
bool PostureRidingCreature::Enter(CreatureObject* obj)
{
    obj->setPosture(mStateID);
    return true;
}
bool PostureRidingCreature::Exit(CreatureObject* obj)
{
    return true;
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
bool PostureKnockedDown::Enter(CreatureObject* obj)
{
    obj->setPosture(mStateID);
    return true;
}
bool PostureKnockedDown::Exit(CreatureObject* obj)
{
    return true;
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
bool PostureIncapacitated::Enter(CreatureObject* obj)
{
    obj->setPosture(mStateID);
    return true;
}
bool PostureIncapacitated::Exit(CreatureObject* obj)
{
    return true;
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
bool PostureDead::Enter(CreatureObject* obj)
{
    obj->setPosture(mStateID);
    return true;
}
bool PostureDead::Exit(CreatureObject* obj)
{
    return true;
}
bool PostureDead::CanTransition(CreatureObject* obj)
{
    return true;
}