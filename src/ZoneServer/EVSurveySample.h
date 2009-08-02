/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_EV_SURVEYSAMPLE_H
#define ANH_ZONESERVER_EV_SURVEYSAMPLE_H

#include "EnqueueValidator.h"


//=======================================================================

class EVSurveySample : public EnqueueValidator
{
	public:

		EVSurveySample(ObjectController* controller) : EnqueueValidator(controller){}
		virtual ~EVSurveySample(){}

		virtual bool validate(uint32 &reply1,uint32 &reply2,uint64 targetId,uint32 opcode,ObjectControllerCmdProperties*& cmdProperties)
		{
			PlayerObject* player = dynamic_cast<PlayerObject*>(mController->getObject());

			//check survey/sampling states
			if(player && (opcode == opOCrequestsurvey || opcode == opOCrequestcoresample))
			{
				if(player->getSurveyState() || player->getSamplingState())
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