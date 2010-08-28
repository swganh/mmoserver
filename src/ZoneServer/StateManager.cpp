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
#include "StateManager.h"
#include "PostureEvent.h"
#include "MessageLib/MessageLib.h"
#include "Common/EventDispatcher.h"

using ::common::SimpleEvent;
using ::common::EventType;

StateManager::StateManager()
{
    mActionStateMap = loadActionStateMap();
    mPostureStateMap = loadPostureStateMap();
}
StateManager::~StateManager()
{
    mActionStateMap.empty();
    mPostureStateMap.empty();
}

ActionStateMap StateManager::loadActionStateMap()
{
    ActionStateMap map;

    map.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_Cover, std::unique_ptr<ActionState>(new StateCover())));
    map.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_Combat, std::unique_ptr<ActionState>(new StateCombat())));
    map.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_Peace, std::unique_ptr<ActionState>(new StatePeace())));
    map.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_Aiming, std::unique_ptr<ActionState>(new StateAiming())));
    map.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_Alert, std::unique_ptr<ActionState>(new StateAlert())));
    map.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_Berserk, std::unique_ptr<ActionState>(new StateBerserk())));
    map.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_FeignDeath, std::unique_ptr<ActionState>(new StateFeign())));
    map.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_CombatAttitudeEvasive, std::unique_ptr<ActionState>(new StateCombatEvasive())));
    map.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_CombatAttitudeNormal, std::unique_ptr<ActionState>(new StateCombatNormal())));
    map.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_CombatAttitudeAggressive, std::unique_ptr<ActionState>(new StateCombatAggressive())));
    map.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_Tumbling, std::unique_ptr<ActionState>(new StateTumbling())));
    map.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_Rallied, std::unique_ptr<ActionState>(new StateRallied())));
    map.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_Stunned, std::unique_ptr<ActionState>(new StateStunned())));
    map.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_Blinded, std::unique_ptr<ActionState>(new StateBlinded())));
    map.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_Dizzy, std::unique_ptr<ActionState>(new StateDizzy())));
    map.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_Intimidated, std::unique_ptr<ActionState>(new StateIntimidated())));
    map.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_Immobilized, std::unique_ptr<ActionState>(new StateImmobolized())));
    map.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_Frozen, std::unique_ptr<ActionState>(new StateFrozen())));
    map.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_Swimming, std::unique_ptr<ActionState>(new StateSwimming())));
    map.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_SittingOnChair, std::unique_ptr<ActionState>(new StateSittingOnChair())));
    map.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_Crafting, std::unique_ptr<ActionState>(new StateCrafting())));
    map.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_GlowingJedi, std::unique_ptr<ActionState>(new StateGlowingJedi())));
    map.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_MaskScent, std::unique_ptr<ActionState>(new StateMaskScent())));
    map.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_Poisoned, std::unique_ptr<ActionState>(new StatePoisoined())));
    map.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_Bleeding, std::unique_ptr<ActionState>(new StateBleeding())));
    map.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_Diseased, std::unique_ptr<ActionState>(new StateDiseased())));
    map.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_OnFire, std::unique_ptr<ActionState>(new StateOnFire())));
    map.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_RidingMount, std::unique_ptr<ActionState>(new StateRidingMount())));
    map.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_MountedCreature, std::unique_ptr<ActionState>(new StateMountedCreature())));
    map.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_PilotingShip, std::unique_ptr<ActionState>(new StatePilotingShip())));
    map.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_ShipOperations, std::unique_ptr<ActionState>(new StateShipOperations())));
    map.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_ShipGunner, std::unique_ptr<ActionState>(new StateShipGunner())));
    //map.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_ShipOperations, std::unique_ptr<ActionState>(new StateShipInterior())));
    //map.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_PilotingShip, std::unique_ptr<ActionState>(new StatePilotingPobShip())));
    

    return map;
}
PostureStateMap StateManager::loadPostureStateMap()
{
    PostureStateMap map;

    map.insert(std::make_pair<uint64, std::unique_ptr<PostureState>>(CreaturePosture_Upright, std::unique_ptr<PostureState>(new PostureUpright())));
    map.insert(std::make_pair<uint64, std::unique_ptr<PostureState>>(CreaturePosture_Crouched, std::unique_ptr<PostureState>(new PostureCrouched())));
    map.insert(std::make_pair<uint64, std::unique_ptr<PostureState>>(CreaturePosture_Prone, std::unique_ptr<PostureState>(new PostureProne())));
    map.insert(std::make_pair<uint64, std::unique_ptr<PostureState>>(CreaturePosture_Sneaking, std::unique_ptr<PostureState>(new PostureSneaking())));
    map.insert(std::make_pair<uint64, std::unique_ptr<PostureState>>(CreaturePosture_Blocking, std::unique_ptr<PostureState>(new PostureBlocking())));
    map.insert(std::make_pair<uint64, std::unique_ptr<PostureState>>(CreaturePosture_Climbing, std::unique_ptr<PostureState>(new PostureClimbing())));
    map.insert(std::make_pair<uint64, std::unique_ptr<PostureState>>(CreaturePosture_Flying, std::unique_ptr<PostureState>(new PostureFlying())));
    map.insert(std::make_pair<uint64, std::unique_ptr<PostureState>>(CreaturePosture_LyingDown, std::unique_ptr<PostureState>(new PostureLyingDown())));
    map.insert(std::make_pair<uint64, std::unique_ptr<PostureState>>(CreaturePosture_Sitting, std::unique_ptr<PostureState>(new PostureSitting())));
    map.insert(std::make_pair<uint64, std::unique_ptr<PostureState>>(CreaturePosture_SkillAnimating, std::unique_ptr<PostureState>(new PostureSkillAnimating())));
    map.insert(std::make_pair<uint64, std::unique_ptr<PostureState>>(CreaturePosture_DrivingVehicle, std::unique_ptr<PostureState>(new PostureDrivingVehicle())));
    map.insert(std::make_pair<uint64, std::unique_ptr<PostureState>>(CreaturePosture_RidingCreature, std::unique_ptr<PostureState>(new PostureRidingCreature())));
    map.insert(std::make_pair<uint64, std::unique_ptr<PostureState>>(CreaturePosture_KnockedDown, std::unique_ptr<PostureState>(new PostureKnockedDown())));
    map.insert(std::make_pair<uint64, std::unique_ptr<PostureState>>(CreaturePosture_Incapacitated, std::unique_ptr<PostureState>(new PostureIncapacitated())));
    map.insert(std::make_pair<uint64, std::unique_ptr<PostureState>>(CreaturePosture_Dead, std::unique_ptr<PostureState>(new PostureDead())));

    return map;
}

void StateManager::setCurrentPostureState(CreatureObject* object, CreaturePosture newPosture)
{
    auto posture_update_event = std::make_shared<PostureUpdateEvent>(object, (CreaturePosture)object->getPosture(), newPosture);

    PostureStateMap::iterator iter = mPostureStateMap.find(object->getPosture());
    if (iter != mPostureStateMap.end())
    {
        if (mPostureStateMap[object->getPosture()]->CanTransition(object))
        {
            // EXIT OLD STATE
            mPostureStateMap[object->getPosture()]->Exit(object);

            // ENTER NEW STATE
            mPostureStateMap[newPosture]->Enter(object);
        }
    } 
    gEventDispatcher.Notify(posture_update_event);
}


void StateManager::setCurrentActionState(CreatureObject* object, CreatureState newState)
{
    bool canTransition = false;
    // get our current states
    uint64 currStates = object->getState();
    
    // nothing to check out transition against
    if (currStates = 0)
        canTransition = true;

    while (currStates)
    {
        uint64 currState = 1 << currStates;
        ActionStateMap::iterator iter = mActionStateMap.find(currState);
        if (iter != mActionStateMap.end())
        {
            // check if we can transition to the new state
            if ((*iter).second->CanTransition(object))
            {
                canTransition = true;
            }
            else
                canTransition = false;
        }

    }
    // Exit old State
    //currState->Exit(object);
    if (canTransition)
    {
        // Enter new State
        mActionStateMap[newState]->Enter(object);

        //notify the client
        gMessageLib->sendPostureAndStateUpdate(object);
    }

}

