/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/
//#include "CellObject.h"
//#include "Item.h"
#include "ObjectController.h"
#include "ObjectControllerOpcodes.h"
#include "ObjectControllerCommandMap.h"
#include "PlayerObject.h"
//#include "QuadTree.h"
//#include "QTRegion.h"
//#include "WorldConfig.h"
//#include "WorldManager.h"
//#include "ZoneTree.h"

//#include "MessageLib/MessageLib.h"
//#include "LogManager/LogManager.h"
//#include "DatabaseManager/Database.h"
//#include "DatabaseManager/DataBinding.h"
//#include "DatabaseManager/DatabaseResult.h"
//#include "Common/Message.h"
//#include "Common/MessageFactory.h"


//=============================================================================
//
// sit
//

void ObjectController::_handleSitServer(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	
	PlayerObject*	playerObject	= dynamic_cast<PlayerObject*>(mObject);

	if(playerObject)
		playerObject->setSitting(message);
}

//=============================================================================
//
// stand
//

void ObjectController::_handleStand(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	// FIXME: for now assume only players send chat
	//???LOL WUT???
	PlayerObject*	playerObject = dynamic_cast<PlayerObject*>(mObject);

	if(playerObject)
		playerObject->setUpright();
}

//=============================================================================
//
// prone
//

void ObjectController::_handleProne(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*	playerObject	= dynamic_cast<PlayerObject*>(mObject);

	if(playerObject)
		playerObject->setProne();
}

//=============================================================================
//
// kneel
//

void ObjectController::_handleKneel(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*	playerObject = dynamic_cast<PlayerObject*>(mObject);

	if(playerObject) 
		playerObject->setCrouched();
}

//=============================================================================



