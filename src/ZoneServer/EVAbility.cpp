/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "EVAbility.h"
#include "CreatureObject.h"
#include "ObjectController.h"
#include "ObjectControllerCommandMap.h"

EVAbility::EVAbility(ObjectController* controller) 
: EnqueueValidator(controller)
{}

EVAbility::~EVAbility()
{}

bool EVAbility::validate(uint32 &reply1, uint32 &reply2, uint64 targetId, uint32 opcode, ObjectControllerCmdProperties*& cmdProperties)
{
    CreatureObject* creature = dynamic_cast<CreatureObject*>(mController->getObject());

	// check if we need to have an ability
	if(creature && cmdProperties && cmdProperties->mAbilityCrc != 0)
	{
		// check if the player has it
		if(!(creature->verifyAbility(cmdProperties->mAbilityCrc)))
		{
			reply1 = 2;
			reply2 = 0;

			return(false);
		}
	}

	return(true);
}

