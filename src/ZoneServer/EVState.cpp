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

#include "EVState.h"
#include "CreatureObject.h"
#include "ObjectController.h"
#include "ObjectControllerCommandMap.h"

EVState::EVState(ObjectController* controller)
    : EnqueueValidator(controller)
{}

EVState::~EVState()
{}

bool EVState::validate(uint32 &reply1, uint32 &reply2, uint64 targetId, uint32 opcode, ObjectControllerCmdProperties*& cmdProperties)
{
    CreatureObject* creature = dynamic_cast<CreatureObject*>(mController->getObject());
	if(creature && cmdProperties)
	{
		//gLogger->log(LogManager::DEBUG, "locomotionMask(%u) & creature->getLocomotion (%u) ==%u",
			//cmdProperties->mLocomotionMask,mController->getLocoValidator(creature->states.getLocomotion()), cmdProperties->mLocomotionMask & creature->states.getLocomotion());
		// skip over commands with no state requirement and check the rest
		if((cmdProperties->mStates != 0) && creature->states.checkStates(cmdProperties->mStates))
		{
			reply1 = kCannotDoWhileState;
			reply2 = this->mController->getLowestCommonBit(creature->states.getAction(), cmdProperties->mStates);
			return false;
		}
		if (cmdProperties->mLocomotionMask !=0 && ((cmdProperties->mLocomotionMask & creature->states.getPosture()) != 0))
		{
			reply1 = kCannotDoWhileLocomotion;
			reply2 = mController->getPostureValidator(creature->states.getLocomotion());
			return false;
		}
	}
    return true;
}


