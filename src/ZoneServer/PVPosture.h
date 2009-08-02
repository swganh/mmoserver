/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_PV_POSTURE_H
#define ANH_ZONESERVER_PV_POSTURE_H

#include "ProcessValidator.h"


//=======================================================================

class PVPosture : public ProcessValidator
{
	public:

		PVPosture(ObjectController* controller) : ProcessValidator(controller){}
		virtual ~PVPosture(){}

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