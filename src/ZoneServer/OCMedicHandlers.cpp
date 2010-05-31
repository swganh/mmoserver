/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/
#include "MedicManager.h"
#include "ObjectController.h"
#include "ObjectControllerOpcodes.h"
#include "ObjectControllerCommandMap.h"
#include "PlayerObject.h"
#include "WorldManager.h"
#include "MessageLib/MessageLib.h"
#include "LogManager/LogManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/DatabaseResult.h"
#include "Common/MessageFactory.h"
#include "Common/Message.h"
#include "ForageManager.h"
#include "Utils/clock.h"


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
	CreatureObject* Target = dynamic_cast<CreatureObject*>(Medic->getHealingTarget(Medic));

	mHandlerCompleted = gMedicManager->CheckStim(Medic, Target, cmdProperties);
	if (mHandlerCompleted)
	{
		//call the event
		gMedicManager->startInjuryTreatmentEvent(Medic);
	}
}

//=============================================================================================================================
//
// healwound
//

void ObjectController::_handleHealWound(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	//PlayerObject* Medic = dynamic_cast<PlayerObject*>(mObject);
	//gMessageLib->sendSystemMessage(Medic, "Heal Wound has not been implemented yet. Sorry.");
}

//=============================================================================================================================
//
// medicalforage
//

void ObjectController::_handleMedicalForage(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
		PlayerObject* player = dynamic_cast<PlayerObject*>(mObject);
	if(player)
		gForageManager->startForage(player, ForageClass_Medic);
}

//=============================================================================================================================
//
// tenddamage
//

void ObjectController::_handleTendDamage(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	//PlayerObject* Medic = dynamic_cast<PlayerObject*>(mObject);
	//gMessageLib->sendSystemMessage(Medic, "Tend Damage has not been implemented yet. Sorry.");
}

//=============================================================================================================================
//
// tendwound
//

void ObjectController::_handleTendWound(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	//PlayerObject* Medic = dynamic_cast<PlayerObject*>(mObject);
	//gMessageLib->sendSystemMessage(Medic, "Tend Wound has not been implemented yet. Sorry.");
}

//=============================================================================================================================
//
// firstaid
//

void ObjectController::_handleFirstAid(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	//PlayerObject* Medic = dynamic_cast<PlayerObject*>(mObject);
	//gMessageLib->sendSystemMessage(Medic, "First Aid has not been implemented yet. Sorry.");
}

//=============================================================================================================================
//
// quickheal
//

void ObjectController::_handleQuickHeal(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	//PlayerObject* Medic = dynamic_cast<PlayerObject*>(mObject);
	//gMessageLib->sendSystemMessage(Medic, "Quick Heal has not been implemented yet. Sorry.");
}

//=============================================================================================================================
//
// dragincapacitatedplayer
//

void ObjectController::_handleDragIncapacitatedPlayer(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	//PlayerObject* Medic = dynamic_cast<PlayerObject*>(mObject);
	//gMessageLib->sendSystemMessage(Medic, "Drag has not been implemented yet. Sorry.");
}

//=============================================================================================================================



