/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
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

