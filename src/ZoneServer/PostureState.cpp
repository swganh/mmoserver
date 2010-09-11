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
#include "StateManager.h"

#define insertAction mTransitionList.insert(std::make_pair<StateTypes, uint64>(State_Action, mStateManager->mActionStateMap
#define insertPosture mTransitionList.insert(std::make_pair<StateTypes, uint64>(State_Posture, mStateManager->mPostureStateMap
#define insertLocomotion mTransitionList.insert(std::make_pair<StateTypes, uint64>(State_Locomotion, mStateManager->mLocomotionStateMap

#define actionMap mStateManager->mActionStateMap
#define postureMap mStateManager->mPostureStateMap
#define locomotionMap mStateManager->mLocomotionStateMap

PostureState::PostureState(StateManager* const sm) : IState(),
    mStateManager(sm), mStateID(0), mBlocked(false){}

PostureState::~PostureState(void)
{
    mStateID = 0;
    mTransitionList.clear();
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
    // check to see if the layer is blocked
    if (mBlocked)
        return false;
    transitionList::iterator itPosture    = mTransitionList.find(State_Posture);
    transitionList::iterator itAction     = mTransitionList.find(State_Action);
    transitionList::iterator itLocomotion = mTransitionList.find(State_Locomotion);
    // check each state type
    // transition list is based on which states we can't transition to.
    while (itPosture != mTransitionList.end())
    {
        // are we allowed to transition based on posture?
        CreaturePosture pos = (CreaturePosture)(*itPosture).second;
        if (pos == obj->getPosture())
        {
            return false;
        }
        ++itPosture;
    }
    while (itAction != mTransitionList.end())
    {
        uint64 state = (*itAction).second;
        if ((state & obj->getState()) != obj->getState())
        {
            return false;
        }
        ++itAction;
    }
    while (itLocomotion != mTransitionList.end())
    {
        CreatureLocomotion locomotion = (CreatureLocomotion)(*itAction).second;
        if (locomotion == obj->getLocomotion())
        {
            return false;
        }
        ++itLocomotion;
    }
    return true;
}
void PostureState::insertIntoTransitionList(const std::pair<StateTypes, uint64>& types)
{
    mTransitionList.insert(types);
}

// Posture Upright
PostureUpright::PostureUpright( StateManager* const sm) : PostureState(sm)
{
    mStateID = CreaturePosture_Upright;
    // deny transition list
    insertAction[CreatureState_Frozen]->getID()));
    insertAction[CreatureState_Immobilized]->getID()));
    insertAction[CreatureState_MountedCreature]->getID()));
    insertAction[CreatureState_RidingMount]->getID()));
    insertPosture[CreaturePosture_Climbing]->getID()));
    insertPosture[CreaturePosture_Dead]->getID()));
    insertPosture[CreaturePosture_Flying]->getID()));
    insertPosture[CreaturePosture_DrivingVehicle]->getID()));
    insertPosture[CreaturePosture_RidingCreature]->getID()));
    insertPosture[CreaturePosture_Incapacitated]->getID()));
    insertLocomotion[CreatureLocomotion_Climbing]->getID()));
    insertLocomotion[CreatureLocomotion_ClimbingStationary]->getID()));
    insertLocomotion[CreatureLocomotion_Dead]->getID()));
    insertLocomotion[CreatureLocomotion_DrivingVehicle]->getID()));
    insertLocomotion[CreatureLocomotion_Flying]->getID()));
    insertLocomotion[CreatureLocomotion_Hovering]->getID()));
    insertLocomotion[CreatureLocomotion_Incapacitated]->getID()));
}
void PostureUpright::Enter(CreatureObject* obj)
{
    // exit states we shouldn't be in
    actionMap[CreatureState_RidingMount]->Exit(obj);
    actionMap[CreatureState_Cover]->Exit(obj);
    actionMap[CreatureState_SittingOnChair]->Exit(obj);
    actionMap[CreatureState_Tumbling]->Exit(obj);

    obj->setPosture(mStateID);
}

//	Posture Crouched
PostureCrouched::PostureCrouched(StateManager* const sm) : PostureState(sm)
{
    mStateID = CreaturePosture_Crouched;
    // deny transition list
    insertAction[CreatureState_Frozen]->getID()));
    insertAction[CreatureState_Immobilized]->getID()));
    insertAction[CreatureState_MountedCreature]->getID()));
    insertAction[CreatureState_RidingMount]->getID()));
    insertPosture[CreaturePosture_Climbing]->getID()));
    insertPosture[CreaturePosture_Dead]->getID()));
    insertPosture[CreaturePosture_Flying]->getID()));
    insertPosture[CreaturePosture_DrivingVehicle]->getID()));
    insertPosture[CreaturePosture_RidingCreature]->getID()));
    insertPosture[CreaturePosture_Incapacitated]->getID()));
    insertLocomotion[CreatureLocomotion_Climbing]->getID()));
    insertLocomotion[CreatureLocomotion_ClimbingStationary]->getID()));
    insertLocomotion[CreatureLocomotion_Dead]->getID()));
    insertLocomotion[CreatureLocomotion_DrivingVehicle]->getID()));
    insertLocomotion[CreatureLocomotion_Flying]->getID()));
    insertLocomotion[CreatureLocomotion_Hovering]->getID()));
    insertLocomotion[CreatureLocomotion_Incapacitated]->getID()));
}
void PostureCrouched::Enter(CreatureObject* obj)
{
    // exit states we shouldn't be in
    actionMap[CreatureState_RidingMount]->Exit(obj);
    actionMap[CreatureState_Cover]->Exit(obj);
    actionMap[CreatureState_SittingOnChair]->Exit(obj);
    actionMap[CreatureState_Tumbling]->Exit(obj);
    
    obj->setPosture(mStateID);
}
//	Posture Prone
PostureProne::PostureProne(StateManager* const sm) : PostureState(sm)
{
    mStateID = CreaturePosture_Prone;
    // deny transition list
    insertAction[CreatureState_Frozen]->getID()));
    insertAction[CreatureState_Immobilized]->getID()));
    insertAction[CreatureState_MountedCreature]->getID()));
    insertAction[CreatureState_RidingMount]->getID()));
    insertPosture[CreaturePosture_Climbing]->getID()));
    insertPosture[CreaturePosture_Dead]->getID()));
    insertPosture[CreaturePosture_Flying]->getID()));
    insertPosture[CreaturePosture_DrivingVehicle]->getID()));
    insertPosture[CreaturePosture_RidingCreature]->getID()));
    insertPosture[CreaturePosture_Incapacitated]->getID()));
    insertLocomotion[CreatureLocomotion_Climbing]->getID()));
    insertLocomotion[CreatureLocomotion_ClimbingStationary]->getID()));
    insertLocomotion[CreatureLocomotion_Dead]->getID()));
    insertLocomotion[CreatureLocomotion_DrivingVehicle]->getID()));
    insertLocomotion[CreatureLocomotion_Flying]->getID()));
    insertLocomotion[CreatureLocomotion_Hovering]->getID()));
    insertLocomotion[CreatureLocomotion_Incapacitated]->getID()));
}
void PostureProne::Enter(CreatureObject* obj)
{
    // exit states we shouldn't be in
    actionMap[CreatureState_RidingMount]->Exit(obj);
    actionMap[CreatureState_Cover]->Exit(obj);
    actionMap[CreatureState_SittingOnChair]->Exit(obj);
    actionMap[CreatureState_Tumbling]->Exit(obj);
    
    obj->setPosture(mStateID);
}
//	Posture Sneaking
PostureSneaking::PostureSneaking(StateManager* const sm) : PostureState(sm)
{
    mStateID = CreaturePosture_Sneaking;
        // deny transition list
    insertAction[CreatureState_Frozen]->getID()));
    insertAction[CreatureState_Immobilized]->getID()));
    insertAction[CreatureState_MountedCreature]->getID()));
    insertAction[CreatureState_RidingMount]->getID()));
    insertPosture[CreaturePosture_Climbing]->getID()));
    insertPosture[CreaturePosture_Dead]->getID()));
    insertPosture[CreaturePosture_Flying]->getID()));
    insertPosture[CreaturePosture_DrivingVehicle]->getID()));
    insertPosture[CreaturePosture_RidingCreature]->getID()));
    insertPosture[CreaturePosture_Incapacitated]->getID()));
    insertLocomotion[CreatureLocomotion_Climbing]->getID()));
    insertLocomotion[CreatureLocomotion_ClimbingStationary]->getID()));
    insertLocomotion[CreatureLocomotion_Dead]->getID()));
    insertLocomotion[CreatureLocomotion_DrivingVehicle]->getID()));
    insertLocomotion[CreatureLocomotion_Flying]->getID()));
    insertLocomotion[CreatureLocomotion_Hovering]->getID()));
    insertLocomotion[CreatureLocomotion_Incapacitated]->getID()));
}
//Posture Blocking
PostureBlocking::PostureBlocking(StateManager* const sm) : PostureState(sm)
{
    mStateID = CreaturePosture_Blocking;
}
//	Posture Climbing
PostureClimbing::PostureClimbing(StateManager* const sm) : PostureState(sm)
{
    mStateID = CreaturePosture_Climbing;
}
//	Posture Flying
PostureFlying::PostureFlying(StateManager* const sm) : PostureState(sm)
{
    mStateID = CreaturePosture_Flying;
}
//	Posture Lying Down
PostureLyingDown::PostureLyingDown(StateManager* const sm) : PostureState(sm)
{
    mStateID = CreaturePosture_LyingDown;
        // deny transition list
    insertAction[CreatureState_Frozen]->getID()));
    insertAction[CreatureState_Immobilized]->getID()));
    insertAction[CreatureState_MountedCreature]->getID()));
    insertAction[CreatureState_RidingMount]->getID()));
    insertPosture[CreaturePosture_Climbing]->getID()));
    insertPosture[CreaturePosture_Dead]->getID()));
    insertPosture[CreaturePosture_Flying]->getID()));
    insertPosture[CreaturePosture_DrivingVehicle]->getID()));
    insertPosture[CreaturePosture_RidingCreature]->getID()));
    insertPosture[CreaturePosture_Incapacitated]->getID()));
    insertLocomotion[CreatureLocomotion_Climbing]->getID()));
    insertLocomotion[CreatureLocomotion_ClimbingStationary]->getID()));
    insertLocomotion[CreatureLocomotion_Dead]->getID()));
    insertLocomotion[CreatureLocomotion_DrivingVehicle]->getID()));
    insertLocomotion[CreatureLocomotion_Flying]->getID()));
    insertLocomotion[CreatureLocomotion_Hovering]->getID()));
    insertLocomotion[CreatureLocomotion_Incapacitated]->getID()));
}
void PostureLyingDown::Enter(CreatureObject* obj)
{
    // exit states we shouldn't be in
    actionMap[CreatureState_RidingMount]->Exit(obj);
    actionMap[CreatureState_Cover]->Exit(obj);
    actionMap[CreatureState_SittingOnChair]->Exit(obj);
    actionMap[CreatureState_Tumbling]->Exit(obj);
    
    obj->setPosture(mStateID);
}
//	Posture Sitting
PostureSitting::PostureSitting(StateManager* const sm) : PostureState(sm)
{
    mStateID = CreaturePosture_Sitting;
        // deny transition list
    insertAction[CreatureState_Frozen]->getID()));
    insertAction[CreatureState_Immobilized]->getID()));
    insertAction[CreatureState_MountedCreature]->getID()));
    insertAction[CreatureState_RidingMount]->getID()));
    insertPosture[CreaturePosture_Climbing]->getID()));
    insertPosture[CreaturePosture_Dead]->getID()));
    insertPosture[CreaturePosture_Flying]->getID()));
    insertPosture[CreaturePosture_DrivingVehicle]->getID()));
    insertPosture[CreaturePosture_RidingCreature]->getID()));
    insertPosture[CreaturePosture_Incapacitated]->getID()));
    insertLocomotion[CreatureLocomotion_Climbing]->getID()));
    insertLocomotion[CreatureLocomotion_ClimbingStationary]->getID()));
    insertLocomotion[CreatureLocomotion_Dead]->getID()));
    insertLocomotion[CreatureLocomotion_DrivingVehicle]->getID()));
    insertLocomotion[CreatureLocomotion_Flying]->getID()));
    insertLocomotion[CreatureLocomotion_Hovering]->getID()));
    insertLocomotion[CreatureLocomotion_Incapacitated]->getID()));
}
void PostureSitting::Enter(CreatureObject* obj)
{
    // exit states we shouldn't be in
    actionMap[CreatureState_RidingMount]->Exit(obj);
    actionMap[CreatureState_Cover]->Exit(obj);
    actionMap[CreatureState_Tumbling]->Exit(obj);
    actionMap[CreatureState_Swimming]->Exit(obj);
    
    obj->setPosture(mStateID);
}
//	Posture Skill Animating
PostureSkillAnimating::PostureSkillAnimating(StateManager* const sm) : PostureState(sm)
{
    mStateID = CreaturePosture_SkillAnimating;
    // deny transition list
    insertAction[CreatureState_Frozen]->getID()));
    insertAction[CreatureState_Immobilized]->getID()));
    insertAction[CreatureState_MountedCreature]->getID()));
    insertAction[CreatureState_RidingMount]->getID()));
    insertPosture[CreaturePosture_Climbing]->getID()));
    insertPosture[CreaturePosture_Dead]->getID()));
    insertPosture[CreaturePosture_Flying]->getID()));
    insertPosture[CreaturePosture_DrivingVehicle]->getID()));
    insertPosture[CreaturePosture_RidingCreature]->getID()));
    insertPosture[CreaturePosture_Incapacitated]->getID()));
    insertLocomotion[CreatureLocomotion_Climbing]->getID()));
    insertLocomotion[CreatureLocomotion_ClimbingStationary]->getID()));
    insertLocomotion[CreatureLocomotion_Dead]->getID()));
    insertLocomotion[CreatureLocomotion_DrivingVehicle]->getID()));
    insertLocomotion[CreatureLocomotion_Flying]->getID()));
    insertLocomotion[CreatureLocomotion_Hovering]->getID()));
    insertLocomotion[CreatureLocomotion_Incapacitated]->getID()));
}
void PostureSkillAnimating::Enter(CreatureObject* obj)
{
    // exit states we shouldn't be in
    actionMap[CreatureState_RidingMount]->Exit(obj);
    actionMap[CreatureState_Cover]->Exit(obj);
    
    obj->setPosture(mStateID);
}
//	Posture Driving Vehicle
PostureDrivingVehicle::PostureDrivingVehicle(StateManager* const sm) : PostureState(sm)
{
    mStateID = CreaturePosture_DrivingVehicle;
    // deny transition list
    insertAction[CreatureState_Frozen]->getID()));
    insertAction[CreatureState_Immobilized]->getID()));
    insertAction[CreatureState_MountedCreature]->getID()));
    insertAction[CreatureState_RidingMount]->getID()));
    insertPosture[CreaturePosture_Climbing]->getID()));
    insertPosture[CreaturePosture_Dead]->getID()));
    insertPosture[CreaturePosture_Flying]->getID()));
    insertPosture[CreaturePosture_DrivingVehicle]->getID()));
    insertPosture[CreaturePosture_RidingCreature]->getID()));
    insertPosture[CreaturePosture_Incapacitated]->getID()));
    insertLocomotion[CreatureLocomotion_Climbing]->getID()));
    insertLocomotion[CreatureLocomotion_ClimbingStationary]->getID()));
    insertLocomotion[CreatureLocomotion_Dead]->getID()));
    insertLocomotion[CreatureLocomotion_DrivingVehicle]->getID()));
    insertLocomotion[CreatureLocomotion_Flying]->getID()));
    insertLocomotion[CreatureLocomotion_Hovering]->getID()));
    insertLocomotion[CreatureLocomotion_Incapacitated]->getID()));
}
void PostureDrivingVehicle::Enter(CreatureObject* obj)
{
    // exit states we shouldn't be in
    actionMap[CreatureState_Cover]->Exit(obj);
    actionMap[CreatureState_SittingOnChair]->Exit(obj);
    actionMap[CreatureState_Tumbling]->Exit(obj);
    
    obj->setPosture(mStateID);
}
//	Posture Riding Creature
PostureRidingCreature::PostureRidingCreature(StateManager* const sm) : PostureState(sm)
{
    mStateID = CreaturePosture_RidingCreature;
    // deny transition list
    insertAction[CreatureState_Frozen]->getID()));
    insertAction[CreatureState_Immobilized]->getID()));
    insertAction[CreatureState_MountedCreature]->getID()));
    insertAction[CreatureState_RidingMount]->getID()));
    insertPosture[CreaturePosture_Climbing]->getID()));
    insertPosture[CreaturePosture_Dead]->getID()));
    insertPosture[CreaturePosture_Flying]->getID()));
    insertPosture[CreaturePosture_DrivingVehicle]->getID()));
    insertPosture[CreaturePosture_RidingCreature]->getID()));
    insertPosture[CreaturePosture_Incapacitated]->getID()));
    insertLocomotion[CreatureLocomotion_Climbing]->getID()));
    insertLocomotion[CreatureLocomotion_ClimbingStationary]->getID()));
    insertLocomotion[CreatureLocomotion_Dead]->getID()));
    insertLocomotion[CreatureLocomotion_DrivingVehicle]->getID()));
    insertLocomotion[CreatureLocomotion_Flying]->getID()));
    insertLocomotion[CreatureLocomotion_Hovering]->getID()));
    insertLocomotion[CreatureLocomotion_Incapacitated]->getID()));
}
void PostureRidingCreature::Enter(CreatureObject* obj)
{
    // exit states we shouldn't be in
    actionMap[CreatureState_Cover]->Exit(obj);
    actionMap[CreatureState_SittingOnChair]->Exit(obj);
    actionMap[CreatureState_Tumbling]->Exit(obj);
    
    obj->setPosture(mStateID);
}
//	Posture Knocked Down
PostureKnockedDown::PostureKnockedDown(StateManager* const sm) : PostureState(sm)
{
    mStateID = CreaturePosture_KnockedDown;
        // deny transition list
    insertAction[CreatureState_Frozen]->getID()));
    insertAction[CreatureState_Immobilized]->getID()));
    insertAction[CreatureState_MountedCreature]->getID()));
    insertAction[CreatureState_RidingMount]->getID()));
    insertPosture[CreaturePosture_Climbing]->getID()));
    insertPosture[CreaturePosture_Dead]->getID()));
    insertPosture[CreaturePosture_Flying]->getID()));
    insertPosture[CreaturePosture_DrivingVehicle]->getID()));
    insertPosture[CreaturePosture_RidingCreature]->getID()));
    insertPosture[CreaturePosture_Incapacitated]->getID()));
    insertLocomotion[CreatureLocomotion_Climbing]->getID()));
    insertLocomotion[CreatureLocomotion_ClimbingStationary]->getID()));
    insertLocomotion[CreatureLocomotion_Dead]->getID()));
    insertLocomotion[CreatureLocomotion_DrivingVehicle]->getID()));
    insertLocomotion[CreatureLocomotion_Flying]->getID()));
    insertLocomotion[CreatureLocomotion_Hovering]->getID()));
    insertLocomotion[CreatureLocomotion_Incapacitated]->getID()));
}
void PostureKnockedDown::Enter(CreatureObject* obj)
{
    // exit states we shouldn't be in
    actionMap[CreatureState_RidingMount]->Exit(obj);
    actionMap[CreatureState_Cover]->Exit(obj);
    actionMap[CreatureState_SittingOnChair]->Exit(obj);
    actionMap[CreatureState_Tumbling]->Exit(obj);
    
    obj->setPosture(mStateID);
}
//	Posture Incapacitated
PostureIncapacitated::PostureIncapacitated(StateManager* const sm) : PostureState(sm)
{
    mStateID = CreaturePosture_Incapacitated;
        // deny transition list
    insertPosture[CreaturePosture_Dead]->getID()));
    insertPosture[CreaturePosture_Incapacitated]->getID()));
    insertLocomotion[CreatureLocomotion_Dead]->getID()));
    insertLocomotion[CreatureLocomotion_Incapacitated]->getID()));
}
void PostureIncapacitated::Enter(CreatureObject* obj)
{
    // exit states we shouldn't be in
    actionMap[CreatureState_Aiming]->Exit(obj);
    actionMap[CreatureState_Alert]->Exit(obj);
    actionMap[CreatureState_Berserk]->Exit(obj);
    actionMap[CreatureState_Bleeding]->Exit(obj);
    actionMap[CreatureState_Blinded]->Exit(obj);
    actionMap[CreatureState_Combat]->Exit(obj);
    actionMap[CreatureState_CombatAttitudeAggressive]->Exit(obj);
    actionMap[CreatureState_CombatAttitudeEvasive]->Exit(obj);
    actionMap[CreatureState_CombatAttitudeNormal]->Exit(obj);
    actionMap[CreatureState_Crafting]->Exit(obj);
    actionMap[CreatureState_Cover]->Exit(obj);
    actionMap[CreatureState_Diseased]->Exit(obj);
    actionMap[CreatureState_Dizzy]->Exit(obj);
    actionMap[CreatureState_FeignDeath]->Exit(obj);
    //actionMap[CreatureState_Frozen]->Exit(obj);
    actionMap[CreatureState_Intimidated]->Exit(obj);
    actionMap[CreatureState_MaskScent]->Exit(obj);
    actionMap[CreatureState_MountedCreature]->Exit(obj);
    actionMap[CreatureState_OnFire]->Exit(obj);
    actionMap[CreatureState_Peace]->Exit(obj);
    actionMap[CreatureState_PilotingPobShip]->Exit(obj);
    actionMap[CreatureState_PilotingShip]->Exit(obj);
    actionMap[CreatureState_Poisoned]->Exit(obj);
    actionMap[CreatureState_Rallied]->Exit(obj);
    actionMap[CreatureState_RidingMount]->Exit(obj);
    actionMap[CreatureState_ShipGunner]->Exit(obj);
    actionMap[CreatureState_ShipInterior]->Exit(obj);
    actionMap[CreatureState_ShipOperations]->Exit(obj);
    actionMap[CreatureState_SittingOnChair]->Exit(obj);
    actionMap[CreatureState_Stunned]->Exit(obj);
    //actionMap[CreatureState_Swimming]->Exit(obj);
    actionMap[CreatureState_Tumbling]->Exit(obj);
    
    obj->setPosture(mStateID);
}
//	Posture Dead
PostureDead::PostureDead(StateManager* const sm) : PostureState(sm)
{
    mStateID = CreaturePosture_Dead;
    // deny transition list
    insertPosture[CreaturePosture_Dead]->getID()));
    insertLocomotion[CreatureLocomotion_Dead]->getID()));
    Block();
}
void PostureDead::Enter(CreatureObject* obj)
{
    // exit states we shouldn't be in
    actionMap[CreatureState_Aiming]->Exit(obj);
    actionMap[CreatureState_Alert]->Exit(obj);
    actionMap[CreatureState_Berserk]->Exit(obj);
    actionMap[CreatureState_Bleeding]->Exit(obj);
    actionMap[CreatureState_Blinded]->Exit(obj);
    actionMap[CreatureState_Combat]->Exit(obj);
    actionMap[CreatureState_CombatAttitudeAggressive]->Exit(obj);
    actionMap[CreatureState_CombatAttitudeEvasive]->Exit(obj);
    actionMap[CreatureState_CombatAttitudeNormal]->Exit(obj);
    actionMap[CreatureState_Crafting]->Exit(obj);
    actionMap[CreatureState_Cover]->Exit(obj);
    actionMap[CreatureState_Diseased]->Exit(obj);
    actionMap[CreatureState_Dizzy]->Exit(obj);
    actionMap[CreatureState_FeignDeath]->Exit(obj);
    //actionMap[CreatureState_Frozen]->Exit(obj);
    actionMap[CreatureState_Intimidated]->Exit(obj);
    actionMap[CreatureState_MaskScent]->Exit(obj);
    actionMap[CreatureState_MountedCreature]->Exit(obj);
    actionMap[CreatureState_OnFire]->Exit(obj);
    actionMap[CreatureState_Peace]->Exit(obj);
    actionMap[CreatureState_PilotingPobShip]->Exit(obj);
    actionMap[CreatureState_PilotingShip]->Exit(obj);
    actionMap[CreatureState_Poisoned]->Exit(obj);
    actionMap[CreatureState_Rallied]->Exit(obj);
    actionMap[CreatureState_RidingMount]->Exit(obj);
    actionMap[CreatureState_ShipGunner]->Exit(obj);
    actionMap[CreatureState_ShipInterior]->Exit(obj);
    actionMap[CreatureState_ShipOperations]->Exit(obj);
    actionMap[CreatureState_SittingOnChair]->Exit(obj);
    actionMap[CreatureState_Stunned]->Exit(obj);
    //actionMap[CreatureState_Swimming]->Exit(obj);
    actionMap[CreatureState_Tumbling]->Exit(obj);
    
    obj->setPosture(mStateID);
}
