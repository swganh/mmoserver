/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_EV_CMDPROPERTY_H
#define ANH_ZONESERVER_EV_CMDPROPERTY_H

#include "EnqueueValidator.h"


//=======================================================================

class EVCmdProperty : public EnqueueValidator
{
	public:

		EVCmdProperty(ObjectController* controller) : EnqueueValidator(controller){}
		virtual ~EVCmdProperty(){}

		virtual bool validate(uint32 &reply1,uint32 &reply2,uint64 targetId,uint32 opcode,ObjectControllerCmdProperties*& cmdProperties)
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
};

//=======================================================================

#endif