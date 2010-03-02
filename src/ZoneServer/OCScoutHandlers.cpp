/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "ObjectController.h"
#include "ObjectControllerOpcodes.h"
#include "ObjectControllerCommandMap.h"
#include "MessageLib/MessageLib.h"
#include "LogManager/LogManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/DatabaseResult.h"
#include "Common/MessageFactory.h"
#include "Common/Message.h"
#include "ScoutManager.h"
#include "ForageManager.h"
#include "PlayerObject.h"



//=============================================================================================================================
//
// harvestCorpse
//

void ObjectController::_handleHarvestCorpse(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
} 

//=============================================================================================================================
//
// maskscent
//

void ObjectController::_handleMaskScent(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
} 

//=============================================================================================================================
//
// forage
//

void ObjectController::_handleForage(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject* player = dynamic_cast<PlayerObject*>(mObject);
	if(player)
		gForageManager->startForage(player, ForageClass_Scout);
} 

//=============================================================================================================================
//
// throwtrap
//

void ObjectController::_handleThrowTrap(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{

}