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

	if(!playerObject)
		return;

	if(!target)
		gMessageLib->sendSystemMessage(playerObject, L"", "internal_command_string","target_not_creature");

	BString input;
	message->getStringAnsi(input);
	
	input.toLower();

	if(input.getAnsi() == "meat")
		gScoutManager->handleHarvestCorpse(playerObject, dynamic_cast<CreatureObject*>(playerObject->getTarget()), HARVEST_MEAT);
	else if(input.getAnsi() == "bone")
		gScoutManager->handleHarvestCorpse(playerObject, dynamic_cast<CreatureObject*>(playerObject->getTarget()), HARVEST_BONE);
	else if(input.getAnsi() == "hide")
		gScoutManager->handleHarvestCorpse(playerObject, dynamic_cast<CreatureObject*>(playerObject->getTarget()), HARVEST_HIDE);
	else
		gScoutManager->handleHarvestCorpse(playerObject, dynamic_cast<CreatureObject*>(playerObject->getTarget()), HARVEST_ANY);

	
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