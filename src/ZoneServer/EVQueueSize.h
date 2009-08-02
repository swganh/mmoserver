/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_EV_QUEUESIZE_H
#define ANH_ZONESERVER_EV_QUEUESIZE_H

#include "EnqueueValidator.h"


//=======================================================================

class EVQueueSize : public EnqueueValidator
{
	public:

		EVQueueSize(ObjectController* controller) : EnqueueValidator(controller){}
		virtual ~EVQueueSize(){}

		virtual bool validate(uint32 &reply1,uint32 &reply2,uint64 targetId,uint32 opcode,ObjectControllerCmdProperties*& cmdProperties)
		{
			// if we exceed the allowed queue size, drop it
			if(mController->getCommandQueue()->size() >= COMMAND_QUEUE_MAX_SIZE)
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