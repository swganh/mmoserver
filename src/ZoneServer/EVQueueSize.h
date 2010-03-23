/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_EV_QUEUESIZE_H
#define ANH_ZONESERVER_EV_QUEUESIZE_H

#include "EnqueueValidator.h"

class ObjectController;
class ObjectControllerCmdProperties;

//=======================================================================

class EVQueueSize : public EnqueueValidator
{
public:
	explicit EVQueueSize(ObjectController* controller);
	virtual ~EVQueueSize();

	virtual bool validate(uint32 &reply1, uint32 &reply2, uint64 targetId, uint32 opcode, ObjectControllerCmdProperties*& cmdProperties);
};

//=======================================================================

#endif