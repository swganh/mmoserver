/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "EVSurveySample.h"
#include "PlayerObject.h"
#include "ObjectController.h"
#include "ObjectControllerCommandMap.h"
#include "ObjectControllerOpcodes.h"

EVSurveySample::EVSurveySample(ObjectController* controller) 
: EnqueueValidator(controller)
{}

EVSurveySample::~EVSurveySample()
{}

bool EVSurveySample::validate(uint32 &reply1,uint32 &reply2,uint64 targetId,uint32 opcode,ObjectControllerCmdProperties*& cmdProperties)
{
    PlayerObject* player = dynamic_cast<PlayerObject*>(mController->getObject());

    //check survey/sampling states
    if(player && (opcode == opOCrequestsurvey || opcode == opOCrequestcoresample))
    {
        if(player->getSurveyState() || player->getSamplingState())
        {
            reply1 = 0;
            reply2 = 0;

            return false;
        }
    }

    return true;
}

