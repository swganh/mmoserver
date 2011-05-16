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

#include "EVQueueSize.h"
#include "ObjectController.h"
#include "ObjectControllerCommandMap.h"

EVQueueSize::EVQueueSize(ObjectController* controller)
    : EnqueueValidator(controller)
{}

EVQueueSize::~EVQueueSize()
{}

bool EVQueueSize::validate(uint32 &reply1, uint32 &reply2, uint64 targetId, uint32 opcode, ObjectControllerCmdProperties*& cmdProperties)
{
    // if we exceed the allowed queue size, drop it
    if(mController->getCommandQueue()->size() >= COMMAND_QUEUE_MAX_SIZE)
    {
        reply1 = 0;
        reply2 = 0;

        return false;
    }

    return true;
}

