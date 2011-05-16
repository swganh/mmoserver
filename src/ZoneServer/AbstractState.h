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
#include "CreatureObject.h"
#include "CreatureEnums.h"
#include <map>

enum StateTypes
{
    State_Action     = 0,
    State_Posture    = 1,
    State_Locomotion = 2
};
typedef std::vector<uint64> stateRemove;
typedef std::map<StateTypes, uint32> transitionList;
class IState
{
public:
    virtual ~IState(void){};	

    /* Activates the Enter process for the given state
    *  
    */
    virtual void Enter(CreatureObject* obj) = 0 ;
    /* Activates the Exit process for the given state
    *  
    */
    virtual void Exit(CreatureObject* obj) = 0;
    /* Determines if the player can transition to the state
    *  
    */
    virtual bool CanTransition(CreatureObject* obj, uint64 newState) = 0;

    virtual const transitionList& returnTransitionList(){return mTransitionList;}

    virtual void insertIntoTransitionList(const std::pair<StateTypes, uint64>& pair)=0;

    void        loadCommonLocomotionList(transitionList& mList)
    {
        mList.insert(std::make_pair(State_Locomotion,CreatureLocomotion_Climbing));
        mList.insert(std::make_pair(State_Locomotion,CreatureLocomotion_ClimbingStationary));
        mList.insert(std::make_pair(State_Locomotion,CreatureLocomotion_Hovering));
        mList.insert(std::make_pair(State_Locomotion,CreatureLocomotion_Flying));
        mList.insert(std::make_pair(State_Locomotion,CreatureLocomotion_LyingDown));
        mList.insert(std::make_pair(State_Locomotion,CreatureLocomotion_SkillAnimating));
        mList.insert(std::make_pair(State_Locomotion,CreatureLocomotion_DrivingVehicle));
        mList.insert(std::make_pair(State_Locomotion,CreatureLocomotion_RidingCreature));
        mList.insert(std::make_pair(State_Locomotion,CreatureLocomotion_KnockedDown));
        mList.insert(std::make_pair(State_Locomotion,CreatureLocomotion_Incapacitated));
        mList.insert(std::make_pair(State_Locomotion,CreatureLocomotion_Dead));
    }
    void        loadCommonActionList(transitionList& mList)
    {
        mList.insert(std::make_pair(State_Action,CreatureState_Frozen));
        mList.insert(std::make_pair(State_Action,CreatureState_Immobilized));
        mList.insert(std::make_pair(State_Action,CreatureState_GlowingJedi));
    }




    //virtual bool hidden(){return mHidden;}

protected:
    transitionList          mTransitionList;
};