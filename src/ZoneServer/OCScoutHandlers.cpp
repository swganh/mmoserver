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

	//Check for Inside building
	if(player->mParentId != 0)
	{
		gScoutManager->failForage(player, NOT_OUTSIDE);
		return;
	}

	//Check for combat
	if(player->checkState(CreatureState_Combat))
	{
		gScoutManager->failForage(player, IN_COMBAT);
		return;
	}

	//Check for action being too low
	if(player->getHam()->mAction.getCurrentHitPoints() < 101)
	{
		gScoutManager->failForage(player, ACTION_LOW);
		return;
	}

	//Check for skill being too low
	if(!player->checkSkill(45)) //Scout -> Survival 1
	{
		gScoutManager->failForage(player, NO_SKILL);
		return;
	}

	//Already foraging
	if(player->isForaging())
	{
		gScoutManager->failForage(player, ALREADY_FORAGING);
		return;
	}

	gScoutManager->startForage(player);

} 

//=============================================================================================================================
//
// throwtrap
//

void ObjectController::_handleThrowTrap(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{

}