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

#ifndef ZONESERVER_ABSTRACT_STATE_H_
#define ZONESERVER_ABSTRACT_STATE_H_

#include "AbstractState.h"

class StateRules
{
public:
    StateRules(std::string db_server, std::string db_user, std::string db_pass, std::string db_name);
    ~StateRules();
    /*
    *
    */
    void loadActionStateRules();
    void loadPostureStateRules();
    void loadLocomotionStateRules();
    void loadActionStatesToRemove();
private:
    StateTypes types;
    uint64 stateClass;
    uint32 validStates;

    uint64 removal_state_class;
    uint64 state_to_remove;

};

#endif  // ZONESERVER_ABSTRACT_STATE_H_
