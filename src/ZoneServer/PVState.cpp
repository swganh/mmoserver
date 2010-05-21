/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "PVState.h"
#include "CreatureObject.h"
#include "ObjectController.h"
#include "ObjectControllerCommandMap.h"

PVState::PVState(ObjectController* controller)
: ProcessValidator(controller)
{}

PVState::~PVState()
{}

bool PVState::validate(uint32 &reply1, uint32 &reply2, uint64 targetId, uint32 opcode, ObjectControllerCmdProperties*& cmdProperties)
{
    CreatureObject* creature = dynamic_cast<CreatureObject*>(mController->getObject());

    // check our states
    if(!creature || !cmdProperties || (creature->getState() & cmdProperties->mStates) != 0)
    {

		if(!creature)
			gLogger->log(LogManager::DEBUG,"ObjController::PVState::validate: creature not found %"PRIu64"",mController->getObject()->getId());

		if((creature->getState() & cmdProperties->mStates) != 0)
			gLogger->log(LogManager::DEBUG,"ObjController::PVState::validate: state denial state :  %"PRIu64"",((creature->getState() & cmdProperties->mStates)));

        reply1 = 0;
        reply2 = 0;

		gLogger->log(LogManager::DEBUG,"ObjController::PVState::validate: state denial state Command crc :  %u",cmdProperties->mCmdCrc);
		//handle canceling of crafting session if it was denied
		
        
        return false;
    }
    
    return true;
}

