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

#include "StateRules.h"
#include <cstdlib>
#include <iostream>

#include <mysql_connection.h>
#include <mysql_driver.h>

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

#include "Utils/typedefs.h"

#include "StateManager.h"

StateRules::StateRules(std::string db_server, std::string db_user, std::string db_pass, std::string db_name)
{
    std::unique_ptr<sql::Connection>sql_connection_;
    sql::Driver* driver = sql::mysql::get_driver_instance();
    sql_connection_.reset(driver->connect(db_server, db_user, db_pass));
    sql_connection_->setSchema(db_name);

    std::unique_ptr<sql::Statement> statement(sql_connection_->createStatement());
    sql::ResultSet* res;
    res = statement->executeQuery("select statetype, stateclass, validstates from staterules");

    while (res->next())
    {
        types = (StateTypes)res->getInt("statetype");
        stateClass = res->getInt("stateclass");
        validStates = res->getInt("validstates");

        switch(types)
        {
        case State_Action:
            loadActionStateRules();
            break;
        case State_Posture:
            loadPostureStateRules();
            break;
        case State_Locomotion:
            loadLocomotionStateRules();
            break;
        }
    }
    res = statement->executeQuery("select state_class,state_to_remove from staterules_removalstates");

    while (res->next())
    {
        removal_state_class = res->getInt64("state_class");
        state_to_remove = res->getInt64("state_to_remove");
        loadActionStatesToRemove();
    }

    res = nullptr;
    delete res;
}


StateRules::~StateRules()
{
}

void StateRules::loadActionStateRules()
{
    ActionStateMap::iterator iter = gStateManager.mActionStateMap.find(stateClass);
    if (iter != gStateManager.mActionStateMap.end())
    {
        gStateManager.mActionStateMap[stateClass]->insertIntoTransitionList(
            std::make_pair(State_Action, validStates));
    }

}
void StateRules::loadPostureStateRules()
{
    PostureStateMap::iterator iter = gStateManager.mPostureStateMap.find(stateClass);
    if (iter != gStateManager.mPostureStateMap.end())
    {
        gStateManager.mPostureStateMap[stateClass]->insertIntoTransitionList(
            std::make_pair(State_Posture, validStates));
    }
}
void StateRules::loadLocomotionStateRules()
{
    LocomotionStateMap::const_iterator iter = gStateManager.mLocomotionStateMap.find(stateClass);
    if (iter != gStateManager.mLocomotionStateMap.end())
    {
        gStateManager.mLocomotionStateMap[stateClass]->insertIntoTransitionList(
            std::make_pair(State_Locomotion, validStates));
    }
}
void StateRules::loadActionStatesToRemove()
{
    ActionStateMap::iterator iter = gStateManager.mActionStateMap.find(stateClass);
    if (iter != gStateManager.mActionStateMap.end())
    {
        gStateManager.mActionStateMap[stateClass]->insertIntoStateRemovalList(state_to_remove);
    }
}