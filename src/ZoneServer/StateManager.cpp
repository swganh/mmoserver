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
#include "MessageLib/MessageLib.h"

StateManager::StateManager()
{
	mActionStateMap = loadActionStateMap();
	mPostureStateMap = loadPostureStateMap();
	mLocomotionStateMap = loadLocomotionStateMap();
}
StateManager::~StateManager()
{
	mActionStateMap.empty();
	mPostureStateMap.empty();
	mLocomotionStateMap.empty();
}

ActionStateMap StateManager::loadActionStateMap()
{
	ActionStateMap map;

	//map.insert(std::make_pair<int, std::unique_ptr<IState>>(-1, std::unique_ptr<IState>(new EmptyState())));
	map.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_Cover, std::unique_ptr<ActionState>(new StateCover())));
	map.insert(std::make_pair<CreatureState, std::unique_ptr<ActionState>>(CreatureState_Combat, std::unique_ptr<ActionState>(new StateCombat())));
	/*map.insert(std::make_pair<int, std::unique_ptr<ActionState>>(2, std::unique_ptr<ActionState>(new StatePeace())));
	map.insert(std::make_pair<int, std::unique_ptr<ActionState>>(3, std::unique_ptr<ActionState>(new StateAiming())));*/

	return map;
}
PostureStateMap StateManager::loadPostureStateMap()
{
	PostureStateMap map;

	//map.insert(std::make_pair<int, std::unique_ptr<IState>>(-1, std::unique_ptr<IState>(new EmptyState())));
	//map.insert(std::make_pair<uint64, std::unique_ptr<PostureState>>(0, std::unique_ptr<PostureState>(new PostureUpright())));
	/*map.insert(std::make_pair<int, std::unique_ptr<PostureState>>(1, std::unique_ptr<PostureState>(new PostureCrouched())));
	map.insert(std::make_pair<int, std::unique_ptr<PostureState>>(2, std::unique_ptr<PostureState>(new PostureSneaking())));
	map.insert(std::make_pair<int, std::unique_ptr<PostureState>>(3, std::unique_ptr<PostureState>(new PostureBlocking())));*/

	return map;
}

LocomotionStateMap StateManager::loadLocomotionStateMap()
{
	LocomotionStateMap map;

	//map.insert(std::make_pair<int, std::unique_ptr<IState>>(-1, std::unique_ptr<IState>(new EmptyState())));
	//map.insert(std::make_pair<int, std::unique_ptr<LocomotionState>>(0, std::unique_ptr<LocomotionState>(new LocomotionStanding())));
	/*map.insert(std::make_pair<int, std::unique_ptr<LocomotionState>>(1, std::unique_ptr<LocomotionState>(new LocomotionSneaking())));
	map.insert(std::make_pair<int, std::unique_ptr<LocomotionState>>(2, std::unique_ptr<LocomotionState>(new LocomotionWalking())));
	map.insert(std::make_pair<int, std::unique_ptr<LocomotionState>>(2, std::unique_ptr<LocomotionState>(new LocomotionRunning())));
	map.insert(std::make_pair<int, std::unique_ptr<LocomotionState>>(4, std::unique_ptr<LocomotionState>(new LocomotionKneeling())));*/

	return map;
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

