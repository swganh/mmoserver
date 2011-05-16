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

#include "PVPosture.h"
#include "CreatureObject.h"
#include "ObjectController.h"
#include "ObjectControllerCommandMap.h"

PVPosture::PVPosture(ObjectController* controller)
    : ProcessValidator(controller)
{}

PVPosture::~PVPosture()
{}

bool PVPosture::validate(uint32 &reply1,uint32 &reply2,uint64 targetId,uint32 opcode,ObjectControllerCmdProperties*& cmdProperties)
{
    if(CreatureObject* creature = dynamic_cast<CreatureObject*>(mController->getObject()))
    {
        uint32 postureBit = 1 << creature->states.getPosture();

        // check our posture
        if(cmdProperties && ((cmdProperties->mPostureMask & postureBit) != postureBit))
        {
            reply1 = kCannotDoWhileLocomotion;
            reply2 = mController->getLowestCommonBit(creature->states.getPosture(),cmdProperties->mPostureMask);
			return false;
		}
    }

    return true;
}

