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
#pragma once
#ifndef ANH_ZONESERVER_STATE_MANAGER_H
#define ANH_ZONESERVER_STATE_MANAGER_H

#include "ActionState.h"
#include "PostureState.h"
#include "LocomotionState.h"
#include "CreatureObject.h"
#include <unordered_map>

#define gStateManager ::utils::Singleton<StateManager>::Instance()

// add a map for each type of State here
typedef std::unordered_map<int64, std::shared_ptr<ActionState>> ActionStateMap;
typedef std::unordered_map<int64, std::shared_ptr<PostureState>> PostureStateMap;
typedef std::unordered_map<int64, std::shared_ptr<LocomotionState>> LocomotionStateMap;

class StateManager
{
public:
    /*	@short State Manager is the state machine system that converts the object to and from a state.
    **
    **
    */
    StateManager();
    ~StateManager();

    void setCurrentActionState(CreatureObject* object, CreatureState);
    void setCurrentPostureState(CreatureObject* object, CreaturePosture);
    void setCurrentLocomotionState(CreatureObject* object, CreatureLocomotion);

    void            loadStateMaps();

    CreatureState		returnCreatureStateFromMap(ActionStateMap* map);
    
    void removeActionState(CreatureObject* object, CreatureState);

    ActionStateMap		    mActionStateMap;
    PostureStateMap		    mPostureStateMap;
    LocomotionStateMap      mLocomotionStateMap;

private:
    void            addActionState(Object* object, ActionState* newState);
    void		    loadActionStateMap();
    void            loadPostureStateMap();
    void    		loadLocomotionStateMap();
};
#endif