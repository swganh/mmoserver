/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "PVHam.h"
#include "ObjectController.h"
#include "CreatureObject.h"
#include "Ham.h"
#include "ObjectControllerCommandMap.h"

PVHam::PVHam(ObjectController* controller)
: ProcessValidator(controller)
{}

PVHam::~PVHam()
{}

bool PVHam::validate(uint32 &reply1, uint32 &reply2, uint64 targetId, uint32 opcode, ObjectControllerCmdProperties*& cmdProperties)
{
    if(CreatureObject* creature	= dynamic_cast<CreatureObject*>(mController->getObject())) 
    {
        if(Ham*	ham = creature->getHam()) 
        {
            if(!ham->checkMainPools(cmdProperties->mHealthCost, cmdProperties->mActionCost, cmdProperties->mMindCost)) 
            {
                reply1 = 0;
                reply2 = 0;

                return false;
            }
        }
    }

    return true;
}

