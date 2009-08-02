/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_EV_POSTURE_H
#define ANH_ZONESERVER_EV_POSTURE_H

#include "EnqueueValidator.h"


//=======================================================================

class EVPosture : public EnqueueValidator
{
	public:

		EVPosture(ObjectController* controller) : EnqueueValidator(controller){}
		virtual ~EVPosture(){}

		virtual bool validate(uint32 &reply1,uint32 &reply2,uint64 targetId,uint32 opcode,ObjectControllerCmdProperties*& cmdProperties)
		{
			if(CreatureObject* creature = dynamic_cast<CreatureObject*>(mController->getObject()))
			{
				uint32 postureBit = 1 << creature->getPosture();

				// check our posture
				if(cmdProperties && ((cmdProperties->mPostureMask & postureBit) != postureBit))
				{
					reply1 = 0;
					reply2 = 0;
					return(false);
				}
			}

			return(true);
		}
};

//=======================================================================

#endif