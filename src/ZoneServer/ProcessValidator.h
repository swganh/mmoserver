/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_PROCESS_VALIDATOR_H
#define ANH_ZONESERVER_PROCESS_VALIDATOR_H

#include "Utils/typedefs.h"

class ObjectController;
class ObjectControllerCmdProperties;

//=======================================================================

class ProcessValidator
{
public:
    explicit ProcessValidator(ObjectController* controller);
    virtual ~ProcessValidator();

    virtual bool validate(uint32 &reply1, uint32 &reply2, uint64 targetId, uint32 opcode, ObjectControllerCmdProperties*& cmdProperties) = 0;

protected:
    ObjectController* mController;
};

//=======================================================================

#endif


