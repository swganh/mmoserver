/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "EVCmdProperty.h"
#include "ObjectController.h"
#include "ObjectControllerCommandMap.h"

EVCmdProperty::EVCmdProperty(ObjectController* controller) 
: EnqueueValidator(controller)
{}

EVCmdProperty::~EVCmdProperty()
{}

bool EVCmdProperty::validate(uint32 &reply1,uint32 &reply2,uint64 targetId,uint32 opcode,ObjectControllerCmdProperties*& cmdProperties)
{
    // get the command properties
    CmdPropertyMap::iterator it = gObjControllerCmdPropertyMap.find(opcode);

    if(it == gObjControllerCmdPropertyMap.end())
    {
        reply1 = 0;
        reply2 = 0;

        return(false);
    }

    cmdProperties = ((*it).second);

    return(true);
}

