/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_PV_HAM_H
#define ANH_ZONESERVER_PV_HAM_H

#include "ProcessValidator.h"

class ObjectController;
class ObjectControllerCmdProperties;

//=======================================================================

class PVHam : public ProcessValidator
{
public:
    explicit PVHam(ObjectController* controller);
    virtual ~PVHam();

    virtual bool validate(uint32 &reply1, uint32 &reply2, uint64 targetId, uint32 opcode, ObjectControllerCmdProperties*& cmdProperties);
};

//=======================================================================

#endif