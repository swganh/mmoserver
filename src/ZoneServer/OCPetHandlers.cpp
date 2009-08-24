/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

//common includes
#include "ObjectController.h"
#include "ObjectControllerOpcodes.h"
#include "ObjectControllerCommandMap.h"
#include "WorldManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/DatabaseResult.h"
#include "Common/MessageFactory.h"
#include "Common/Message.h"
#include "MessageLib/MessageLib.h"
#include "LogManager/LogManager.h"
#include "WorldConfig.h"
#include "UIManager.h"
#include "Item.h"


//specific includes
#include "VehicleFactory.h"

void ObjectController::_handleMount(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{

	PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);
	CreatureObject* pet		= dynamic_cast<CreatureObject*>(gWorldManager->getObjectById(targetId));
	
	if(player->getMount())
	{
		if(!player->checkIfMounted())
		{
			// verify its player's mount
			if(pet->getOwner() == player->getId())
			{
				// get the mount Vehicle object by the id (Creature object id - 1 )
				if(Vehicle* vehicle = dynamic_cast<Vehicle*>(gWorldManager->getObjectById(pet->getId() - 1)))
				{
					vehicle->mountPlayer();
				}
				else
				{
					gLogger->logMsg("ObjectController::_handleMount : Cannot find vehicle\n");
				}
			}
		}
		else
		{
			gMessageLib->sendSystemMessage(player,L"You cannot mount this because you are already mounted.");
		}
	}

	
}

//===============================================================================================

void ObjectController::_handleDismount(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{

	PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);
	CreatureObject* pet		= dynamic_cast<CreatureObject*>(gWorldManager->getObjectById(targetId));

	if(player->getMount())
	{
		if(player->checkIfMounted())
		{
			// verify its player's mount
			if(pet->getOwner() == player->getId())
			{
				// get the mount Vehicle object by the id (Creature object id - 1 )
				if(Vehicle* vehicle = dynamic_cast<Vehicle*>(gWorldManager->getObjectById(pet->getId() - 1)))
				{
					vehicle->dismountPlayer();
				}
				
			}
			 
		}
		else
		{
			gMessageLib->sendSystemMessage(player,L"You are not drivng a vehicle at this time.");
		}
	}
}

//===============================================================================================