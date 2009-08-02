/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_EV_STATE_H
#define ANH_ZONESERVER_EV_STATE_H

#include "EnqueueValidator.h"


//=======================================================================

class EVState : public EnqueueValidator
{
	public:

		EVState(ObjectController* controller) : EnqueueValidator(controller){}
		virtual ~EVState(){}

		virtual bool validate(uint32 &reply1,uint32 &reply2,uint64 targetId,uint32 opcode,ObjectControllerCmdProperties*& cmdProperties)
		{
			CreatureObject* creature = dynamic_cast<CreatureObject*>(mController->getObject());

			// check our states
			if(creature && cmdProperties && (creature->getState() & cmdProperties->mStates) != 0)
			{
				reply1 = 0;
				reply2 = 0;
				return(false);
			}

			return(true);
		}
};

//=======================================================================

#endif