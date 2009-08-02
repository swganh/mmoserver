/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_PV_HAM_H
#define ANH_ZONESERVER_PV_HAM_H

#include "ProcessValidator.h"
#include "Ham.h"


//=======================================================================

class PVHam : public ProcessValidator
{
	public:

		PVHam(ObjectController* controller) : ProcessValidator(controller){}
		virtual ~PVHam(){}

		virtual bool validate(uint32 &reply1,uint32 &reply2,uint64 targetId,uint32 opcode,ObjectControllerCmdProperties*& cmdProperties)
		{
			if(CreatureObject* creature	= dynamic_cast<CreatureObject*>(mController->getObject())) 
			{
				if(Ham*	ham = creature->getHam())
				{
					if(!ham->checkMainPools(cmdProperties->mHealthCost,cmdProperties->mActionCost,cmdProperties->mMindCost))
					{
						reply1 = 0;
						reply2 = 0;

						return(false);
					}
				}
			}

			return(true);
		}
};

//=======================================================================

#endif