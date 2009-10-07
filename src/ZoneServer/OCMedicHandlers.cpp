/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

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
#include "MedicManager.h"
#include "PlayerObject.h"

//=============================================================================================================================
//
// diagnose
//

void ObjectController::_handleDiagnose(uint64 targetId, Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject* Medic = dynamic_cast<PlayerObject*>(mObject);
	PlayerObject* Target = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(targetId));
	if(Target != 0)
	{
		gMedicManager->Diagnose(Medic, Target);
	} else {
		gMessageLib->sendSystemMessage(Medic,L"","healing_response","healing_response_b6");
		return;
	}
} 

//=============================================================================================================================
//
// healdamage
//

void ObjectController::_handleHealDamage(uint64 targetId, Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject* Medic = dynamic_cast<PlayerObject*>(mObject);
	CreatureObject* Target = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById(targetId));
	if(Target == 0) //If target is not a PlayerObject
	{
		gMessageLib->sendSystemMessage(Medic,L"","healing_response","healing_response_62");
		return;
	}

	mHandlerCompleted = gMedicManager->HealDamage(Medic, Target, cmdProperties);
} 

//=============================================================================================================================
//
// healwound
//

void ObjectController::_handleHealWound(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject* Medic = dynamic_cast<PlayerObject*>(mObject);
	//gMessageLib->sendSystemMessage(Medic, "Heal Wound has not been implemented yet. Sorry.");
} 

//=============================================================================================================================
//
// medicalforage
//

void ObjectController::_handleMedicalForage(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject* Medic = dynamic_cast<PlayerObject*>(mObject);
	//gMessageLib->sendSystemMessage(Medic, "Medical Forage has not been implemented yet. Sorry.");
} 

//=============================================================================================================================
//
// tenddamage
//

void ObjectController::_handleTendDamage(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject* Medic = dynamic_cast<PlayerObject*>(mObject);
	//gMessageLib->sendSystemMessage(Medic, "Tend Damage has not been implemented yet. Sorry.");
} 

//=============================================================================================================================
//
// tendwound
//

void ObjectController::_handleTendWound(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject* Medic = dynamic_cast<PlayerObject*>(mObject);
	//gMessageLib->sendSystemMessage(Medic, "Tend Wound has not been implemented yet. Sorry.");
} 

//=============================================================================================================================
//
// firstaid
//

void ObjectController::_handleFirstAid(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject* Medic = dynamic_cast<PlayerObject*>(mObject);
	//gMessageLib->sendSystemMessage(Medic, "First Aid has not been implemented yet. Sorry.");
} 

//=============================================================================================================================
//
// quickheal
//

void ObjectController::_handleQuickHeal(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject* Medic = dynamic_cast<PlayerObject*>(mObject);
	//gMessageLib->sendSystemMessage(Medic, "Quick Heal has not been implemented yet. Sorry.");
} 

//=============================================================================================================================
//
// dragincapacitatedplayer
//

void ObjectController::_handleDragIncapacitatedPlayer(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject* Medic = dynamic_cast<PlayerObject*>(mObject);
	//gMessageLib->sendSystemMessage(Medic, "Drag has not been implemented yet. Sorry.");
} 

//=============================================================================================================================



