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
	AttackableCreature* target = dynamic_cast<AttackableCreature*>(gWorldManager->getObjectById(targetId));
	PlayerObject* playerObject = dynamic_cast<PlayerObject*>(mObject);

	if(!playerObject || !playerObject->isConnected())
		return;

	if(!target)
		gMessageLib->sendSystemMessage(playerObject, L"", "internal_command_string","target_not_creature");

	string cmdString;
	message->getStringAnsi(cmdString);

	int8 rawData[128];

	int32 elementCount = sscanf(cmdString.getAnsi(), "%80s", rawData);

	if(elementCount == 0)
	{
		gScoutManager->handleHarvestCorpse(playerObject, target, HARVEST_ANY);
	}
	else if(elementCount == 1)
	{
		string data(rawData);
		data.toLower();
		
		if(data == "meat")
			gScoutManager->handleHarvestCorpse(playerObject, target, HARVEST_MEAT);
		else if(data == "bone")
			gScoutManager->handleHarvestCorpse(playerObject, target, HARVEST_BONE);
		else if(data == "hide")
			gScoutManager->handleHarvestCorpse(playerObject, target, HARVEST_HIDE);
	}
	else
	{
		gMessageLib->sendSystemMessage(playerObject, L"", "internal_command_string","no_resource");
	}

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