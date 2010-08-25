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
#include "ActionState.h"
#include "PlayerObject.h"


ActionState::ActionState(void)
{
}

bool ActionState::Enter(CreatureObject* obj)
{
	return true;
}
bool ActionState::Exit(CreatureObject* obj)
{
	return true;
}
bool ActionState::CanTransition(CreatureObject* obj)
{
	return true;
}

// StateCover
StateCover::StateCover() : ActionState()
{
	mStateID = CreatureState_Cover;
}
bool StateCover::Enter(CreatureObject* obj)
{
	// check take cover mod
	// do appropriate actions
	obj->toggleStateOn(mStateID);
	return true;
}
bool StateCover::Exit(CreatureObject* obj)
{
	obj->toggleStateOff(mStateID);
	return true;
}
bool StateCover::CanTransition(CreatureObject* obj)
{
	// check the transition list
	return true;
}

// State Combat
StateCombat::StateCombat() : ActionState()
{
	mStateID = CreatureState_Combat;
}
bool StateCombat::Enter(CreatureObject* obj)
{
	obj->toggleStateOn(mStateID);

	return true;
}
bool StateCombat::Exit(CreatureObject* obj)
{
	obj->toggleStateOff(mStateID);

	return true;
}
bool StateCombat::CanTransition(CreatureObject* obj)
{
	// check the transition list
	return true;
}