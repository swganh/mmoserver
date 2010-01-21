/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

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

    // check our states
    if(creature && cmdProperties && (creature->getState() & cmdProperties->mStates) != 0)
    {
        reply1 = 0;
        reply2 = 0;
        return false;
    }

    return true;
}

