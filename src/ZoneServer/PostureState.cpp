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

#define insertAction mTransitionList.insert(std::make_pair(State_Action 
#define insertPosture mTransitionList.insert(std::make_pair(State_Posture
#define insertLocomotion mTransitionList.insert(std::make_pair(State_Locomotion

#define actionMap mStateManager->mActionStateMap
#define postureMap mStateManager->mPostureStateMap
#define locomotionMap mStateManager->mLocomotionStateMap

PostureState::PostureState(StateManager* const sm) : IState(),
    mStateManager(sm), mStateID(0){}

PostureState::~PostureState(void)
{
    mStateID = 0;
    mTransitionList.clear();
}

void PostureState::Enter(CreatureObject* obj)
{
    obj->states.setPosture(mStateID);
}
void PostureState::Exit(CreatureObject* obj)
{
    obj->states.setPosture(0);
}
bool PostureState::CanTransition(CreatureObject* obj, uint64 newPosture)
{ 
    // check to see if the layer is blocked
    if (obj->states.blockPosture)
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
        if (obj->states.checkPosture(pos))
        {
            return false;
        }
        ++itPosture;
    }
    while (itAction != mTransitionList.end())
    {
        uint64 state = (*itAction).second;
        if (obj->states.checkStates(state))
        {
            return false;
        }
        ++itAction;
    }
    while (itLocomotion != mTransitionList.end())
    {
       uint32 locomotion = (*itLocomotion).second;
        if (obj->states.checkLocomotion(locomotion))
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
    insertAction,CreatureState_Frozen));
    insertAction,CreatureState_Immobilized));
    insertAction,CreatureState_MountedCreature));
    insertAction,CreatureState_RidingMount));
    insertPosture,CreaturePosture_Climbing));
    insertPosture,CreaturePosture_Dead));
    insertPosture,CreaturePosture_Flying));
    insertPosture,CreaturePosture_DrivingVehicle));
    insertPosture,CreaturePosture_RidingCreature));
    insertPosture,CreaturePosture_Incapacitated));
    insertLocomotion,CreatureLocomotion_Climbing));
    insertLocomotion,CreatureLocomotion_ClimbingStationary));
    insertLocomotion,CreatureLocomotion_Dead));
    insertLocomotion,CreatureLocomotion_DrivingVehicle));
    insertLocomotion,CreatureLocomotion_Flying));
    insertLocomotion,CreatureLocomotion_Hovering));
    insertLocomotion,CreatureLocomotion_Incapacitated));
}
void PostureUpright::Enter(CreatureObject* obj)
{
    // exit states we shouldn't be in
    actionMap[CreatureState_RidingMount]->Exit(obj);
    actionMap[CreatureState_Cover]->Exit(obj);
    actionMap[CreatureState_SittingOnChair]->Exit(obj);
    actionMap[CreatureState_Tumbling]->Exit(obj);

    obj->states.setPosture(mStateID);
}

//	Posture Crouched
PostureCrouched::PostureCrouched(StateManager* const sm) : PostureState(sm)
{
    mStateID = CreaturePosture_Crouched;
    // deny transition list
    insertAction,CreatureState_Frozen));
    insertAction,CreatureState_Immobilized));
    insertAction,CreatureState_MountedCreature));
    insertAction,CreatureState_RidingMount));
    insertPosture,CreaturePosture_Climbing));
    insertPosture,CreaturePosture_Dead));
    insertPosture,CreaturePosture_Flying));
    insertPosture,CreaturePosture_DrivingVehicle));
    insertPosture,CreaturePosture_RidingCreature));
    insertPosture,CreaturePosture_Incapacitated));
    insertLocomotion,CreatureLocomotion_Climbing));
    insertLocomotion,CreatureLocomotion_ClimbingStationary));
    insertLocomotion,CreatureLocomotion_Dead));
    insertLocomotion,CreatureLocomotion_DrivingVehicle));
    insertLocomotion,CreatureLocomotion_Flying));
    insertLocomotion,CreatureLocomotion_Hovering));
    insertLocomotion,CreatureLocomotion_Incapacitated));
}
void PostureCrouched::Enter(CreatureObject* obj)
{
    // exit states we shouldn't be in
    actionMap[CreatureState_RidingMount]->Exit(obj);
    actionMap[CreatureState_Cover]->Exit(obj);
    actionMap[CreatureState_SittingOnChair]->Exit(obj);
    actionMap[CreatureState_Tumbling]->Exit(obj);
    
    obj->states.setPosture(mStateID);
}
//	Posture Prone
PostureProne::PostureProne(StateManager* const sm) : PostureState(sm)
{
    mStateID = CreaturePosture_Prone;
    // deny transition list
    insertAction,CreatureState_Frozen));
    insertAction,CreatureState_Immobilized));
    insertAction,CreatureState_MountedCreature));
    insertAction,CreatureState_RidingMount));
    insertPosture,CreaturePosture_Climbing));
    insertPosture,CreaturePosture_Dead));
    insertPosture,CreaturePosture_Flying));
    insertPosture,CreaturePosture_DrivingVehicle));
    insertPosture,CreaturePosture_RidingCreature));
    insertPosture,CreaturePosture_Incapacitated));
    insertLocomotion,CreatureLocomotion_Climbing));
    insertLocomotion,CreatureLocomotion_ClimbingStationary));
    insertLocomotion,CreatureLocomotion_Dead));
    insertLocomotion,CreatureLocomotion_DrivingVehicle));
    insertLocomotion,CreatureLocomotion_Flying));
    insertLocomotion,CreatureLocomotion_Hovering));
    insertLocomotion,CreatureLocomotion_Incapacitated));
}
void PostureProne::Enter(CreatureObject* obj)
{
    // exit states we shouldn't be in
    actionMap[CreatureState_RidingMount]->Exit(obj);
    actionMap[CreatureState_Cover]->Exit(obj);
    actionMap[CreatureState_SittingOnChair]->Exit(obj);
    actionMap[CreatureState_Tumbling]->Exit(obj);
    
    obj->states.setPosture(mStateID);
}
//	Posture Sneaking
PostureSneaking::PostureSneaking(StateManager* const sm) : PostureState(sm)
{
    mStateID = CreaturePosture_Sneaking;
    // deny transition list
    insertAction,CreatureState_Frozen));
    insertAction,CreatureState_Immobilized));
    insertAction,CreatureState_MountedCreature));
    insertAction,CreatureState_RidingMount));
    insertPosture,CreaturePosture_Climbing));
    insertPosture,CreaturePosture_Dead));
    insertPosture,CreaturePosture_Flying));
    insertPosture,CreaturePosture_DrivingVehicle));
    insertPosture,CreaturePosture_RidingCreature));
    insertPosture,CreaturePosture_Incapacitated));
    insertLocomotion,CreatureLocomotion_Climbing));
    insertLocomotion,CreatureLocomotion_ClimbingStationary));
    insertLocomotion,CreatureLocomotion_Dead));
    insertLocomotion,CreatureLocomotion_DrivingVehicle));
    insertLocomotion,CreatureLocomotion_Flying));
    insertLocomotion,CreatureLocomotion_Hovering));
    insertLocomotion,CreatureLocomotion_Incapacitated));
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
    insertAction,CreatureState_Frozen));
    insertAction,CreatureState_Immobilized));
    insertAction,CreatureState_MountedCreature));
    insertAction,CreatureState_RidingMount));
    insertPosture,CreaturePosture_Climbing));
    insertPosture,CreaturePosture_Dead));
    insertPosture,CreaturePosture_Flying));
    insertPosture,CreaturePosture_DrivingVehicle));
    insertPosture,CreaturePosture_RidingCreature));
    insertPosture,CreaturePosture_Incapacitated));
    insertLocomotion,CreatureLocomotion_Climbing));
    insertLocomotion,CreatureLocomotion_ClimbingStationary));
    insertLocomotion,CreatureLocomotion_Dead));
    insertLocomotion,CreatureLocomotion_DrivingVehicle));
    insertLocomotion,CreatureLocomotion_Flying));
    insertLocomotion,CreatureLocomotion_Hovering));
    insertLocomotion,CreatureLocomotion_Incapacitated));
}
void PostureLyingDown::Enter(CreatureObject* obj)
{
    // exit states we shouldn't be in
    actionMap[CreatureState_RidingMount]->Exit(obj);
    actionMap[CreatureState_Cover]->Exit(obj);
    actionMap[CreatureState_SittingOnChair]->Exit(obj);
    actionMap[CreatureState_Tumbling]->Exit(obj);
    
    obj->states.setPosture(mStateID);
}
//	Posture Sitting
PostureSitting::PostureSitting(StateManager* const sm) : PostureState(sm)
{
    mStateID = CreaturePosture_Sitting;
    // deny transition list
    insertAction,CreatureState_Frozen));
    insertAction,CreatureState_Immobilized));
    insertAction,CreatureState_MountedCreature));
    insertAction,CreatureState_RidingMount));
    insertPosture,CreaturePosture_Climbing));
    insertPosture,CreaturePosture_Dead));
    insertPosture,CreaturePosture_Flying));
    insertPosture,CreaturePosture_DrivingVehicle));
    insertPosture,CreaturePosture_RidingCreature));
    insertPosture,CreaturePosture_Incapacitated));
    insertLocomotion,CreatureLocomotion_Climbing));
    insertLocomotion,CreatureLocomotion_ClimbingStationary));
    insertLocomotion,CreatureLocomotion_Dead));
    insertLocomotion,CreatureLocomotion_DrivingVehicle));
    insertLocomotion,CreatureLocomotion_Flying));
    insertLocomotion,CreatureLocomotion_Hovering));
    insertLocomotion,CreatureLocomotion_Incapacitated));
}
void PostureSitting::Enter(CreatureObject* obj)
{
    // exit states we shouldn't be in
    actionMap[CreatureState_RidingMount]->Exit(obj);
    actionMap[CreatureState_Cover]->Exit(obj);
    actionMap[CreatureState_Tumbling]->Exit(obj);
    actionMap[CreatureState_Swimming]->Exit(obj);
    
    obj->states.setPosture(mStateID);
}
//	Posture Skill Animating
PostureSkillAnimating::PostureSkillAnimating(StateManager* const sm) : PostureState(sm)
{
    mStateID = CreaturePosture_SkillAnimating;
    // deny transition list
    insertAction,CreatureState_Frozen));
    insertAction,CreatureState_Immobilized));
    insertAction,CreatureState_MountedCreature));
    insertAction,CreatureState_RidingMount));
    insertPosture,CreaturePosture_Climbing));
    insertPosture,CreaturePosture_Dead));
    insertPosture,CreaturePosture_Flying));
    insertPosture,CreaturePosture_DrivingVehicle));
    insertPosture,CreaturePosture_RidingCreature));
    insertPosture,CreaturePosture_Incapacitated));
    insertLocomotion,CreatureLocomotion_Climbing));
    insertLocomotion,CreatureLocomotion_ClimbingStationary));
    insertLocomotion,CreatureLocomotion_Dead));
    insertLocomotion,CreatureLocomotion_DrivingVehicle));
    insertLocomotion,CreatureLocomotion_Flying));
    insertLocomotion,CreatureLocomotion_Hovering));
    insertLocomotion,CreatureLocomotion_Incapacitated));
}
void PostureSkillAnimating::Enter(CreatureObject* obj)
{
    // exit states we shouldn't be in
    actionMap[CreatureState_RidingMount]->Exit(obj);
    actionMap[CreatureState_Cover]->Exit(obj);
    
    obj->states.setPosture(mStateID);
}
//	Posture Driving Vehicle
PostureDrivingVehicle::PostureDrivingVehicle(StateManager* const sm) : PostureState(sm)
{
    mStateID = CreaturePosture_DrivingVehicle;
    // deny transition list
    insertAction,CreatureState_Frozen));
    insertAction,CreatureState_Immobilized));
    insertAction,CreatureState_MountedCreature));
    insertAction,CreatureState_RidingMount));
    insertPosture,CreaturePosture_Climbing));
    insertPosture,CreaturePosture_Dead));
    insertPosture,CreaturePosture_Flying));
    insertPosture,CreaturePosture_DrivingVehicle));
    insertPosture,CreaturePosture_RidingCreature));
    insertPosture,CreaturePosture_Incapacitated));
    insertLocomotion,CreatureLocomotion_Climbing));
    insertLocomotion,CreatureLocomotion_ClimbingStationary));
    insertLocomotion,CreatureLocomotion_Dead));
    insertLocomotion,CreatureLocomotion_DrivingVehicle));
    insertLocomotion,CreatureLocomotion_Flying));
    insertLocomotion,CreatureLocomotion_Hovering));
    insertLocomotion,CreatureLocomotion_Incapacitated));
}
void PostureDrivingVehicle::Enter(CreatureObject* obj)
{
    // exit states we shouldn't be in
    actionMap[CreatureState_Cover]->Exit(obj);
    actionMap[CreatureState_SittingOnChair]->Exit(obj);
    actionMap[CreatureState_Tumbling]->Exit(obj);
    
    obj->states.setPosture(mStateID);
}
//	Posture Riding Creature
PostureRidingCreature::PostureRidingCreature(StateManager* const sm) : PostureState(sm)
{
    mStateID = CreaturePosture_RidingCreature;
    // deny transition list
    insertAction,CreatureState_Frozen));
    insertAction,CreatureState_Immobilized));
    insertAction,CreatureState_MountedCreature));
    insertAction,CreatureState_RidingMount));
    insertPosture,CreaturePosture_Climbing));
    insertPosture,CreaturePosture_Dead));
    insertPosture,CreaturePosture_Flying));
    insertPosture,CreaturePosture_DrivingVehicle));
    insertPosture,CreaturePosture_RidingCreature));
    insertPosture,CreaturePosture_Incapacitated));
    insertLocomotion,CreatureLocomotion_Climbing));
    insertLocomotion,CreatureLocomotion_ClimbingStationary));
    insertLocomotion,CreatureLocomotion_Dead));
    insertLocomotion,CreatureLocomotion_DrivingVehicle));
    insertLocomotion,CreatureLocomotion_Flying));
    insertLocomotion,CreatureLocomotion_Hovering));
    insertLocomotion,CreatureLocomotion_Incapacitated));
}
void PostureRidingCreature::Enter(CreatureObject* obj)
{
    // exit states we shouldn't be in
    actionMap[CreatureState_Cover]->Exit(obj);
    actionMap[CreatureState_SittingOnChair]->Exit(obj);
    actionMap[CreatureState_Tumbling]->Exit(obj);
    
    obj->states.setPosture(mStateID);
}
//	Posture Knocked Down
PostureKnockedDown::PostureKnockedDown(StateManager* const sm) : PostureState(sm)
{
    mStateID = CreaturePosture_KnockedDown;
    // deny transition list
    insertAction,CreatureState_Frozen));
    insertAction,CreatureState_Immobilized));
    insertAction,CreatureState_MountedCreature));
    insertAction,CreatureState_RidingMount));
    insertPosture,CreaturePosture_Climbing));
    insertPosture,CreaturePosture_Dead));
    insertPosture,CreaturePosture_Flying));
    insertPosture,CreaturePosture_DrivingVehicle));
    insertPosture,CreaturePosture_RidingCreature));
    insertPosture,CreaturePosture_Incapacitated));
    insertLocomotion,CreatureLocomotion_Climbing));
    insertLocomotion,CreatureLocomotion_ClimbingStationary));
    insertLocomotion,CreatureLocomotion_Dead));
    insertLocomotion,CreatureLocomotion_DrivingVehicle));
    insertLocomotion,CreatureLocomotion_Flying));
    insertLocomotion,CreatureLocomotion_Hovering));
    insertLocomotion,CreatureLocomotion_Incapacitated));
}
void PostureKnockedDown::Enter(CreatureObject* obj)
{
    // exit states we shouldn't be in
    actionMap[CreatureState_RidingMount]->Exit(obj);
    actionMap[CreatureState_Cover]->Exit(obj);
    actionMap[CreatureState_SittingOnChair]->Exit(obj);
    actionMap[CreatureState_Tumbling]->Exit(obj);
    
    obj->states.setPosture(mStateID);
}
//	Posture Incapacitated
PostureIncapacitated::PostureIncapacitated(StateManager* const sm) : PostureState(sm)
{
    mStateID = CreaturePosture_Incapacitated;
    // deny transition list
    insertPosture,CreaturePosture_Dead));
    insertPosture,CreaturePosture_Incapacitated));
    insertLocomotion,CreatureLocomotion_Dead));
    insertLocomotion,CreatureLocomotion_Incapacitated));
}
void PostureIncapacitated::Enter(CreatureObject* obj)
{  
    obj->states.blockLayers();
    obj->states.setPosture(mStateID);
    // clear all states
    obj->states.toggleActionOn(CreatureState_ClearState);
}
//	Posture Dead
PostureDead::PostureDead(StateManager* const sm) : PostureState(sm)
{
    mStateID = CreaturePosture_Dead;
    // deny transition list
    insertPosture,CreaturePosture_Dead));
    insertLocomotion,CreatureLocomotion_Dead));
}
void PostureDead::Enter(CreatureObject* obj)
{
    obj->states.blockLayers();
    obj->states.setPosture(mStateID);
    // clear all states
    obj->states.toggleActionOn(CreatureState_ClearState);
}
