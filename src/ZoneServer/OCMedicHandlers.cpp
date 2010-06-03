/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2010 The SWG:ANH Team
---------------------------------------------------------------------------------------
Use of this source code is governed by the GPL v3 license that can be found
in the COPYING file or at http://www.gnu.org/licenses/gpl-3.0.html

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
---------------------------------------------------------------------------------------
*/
#include "ObjectController.h"
#include <cstdint>
#ifdef _MSC_VER
#include <regex>  // NOLINT
#else
#endif

#include <boost/regex.hpp>  // NOLINT

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

// GCC doesn't fully support regex yet (some methods are unimplemented at this
// time), so fall back to the boost regex (which is essentially the same
// interface, different namespace). Once GCC finishes their implementation all
// boost/regex references can be removed.
#ifdef WIN32
using ::std::regex;
using ::std::smatch;
using ::std::regex_search;
using ::std::sregex_token_iterator;
#else
using ::boost::regex;
using ::boost::smatch;
using ::boost::regex_search;
using ::boost::sregex_token_iterator;
#endif

	//consts
	const char* const woundpack = "woundpack";
	const char* const stim = "stim";
	const char* const rangedstim = "ranged";
	const char* const self = "self";
	const char* const action = "action";
	const char* const constitution = "constitution";
	const char* const health = "health";
	const char* const quickness = "quickness";
	const char* const stamina = "stamina";
	const char* const strength = "strength";
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
	PlayerObject* Target = dynamic_cast<PlayerObject*>(Medic->getHealingTarget(Medic));

	mHandlerCompleted = gMedicManager->CheckMedicine(Medic, Target, cmdProperties, stim);
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
	PlayerObject* Medic = dynamic_cast<PlayerObject*>(mObject);
	PlayerObject* Target = dynamic_cast<PlayerObject*>(Medic->getHealingTarget(Medic));

// Read the message out of the packet.
  string tmp;
  message->getStringUnicode16(tmp);

  // If the string has no length the message is ill-formatted, send the
  // proper format to the client.
  if (!tmp.getLength()) {
	 if(gMedicManager->CheckMedicine(Medic, Target, cmdProperties, woundpack))
	 { 
		//call the event
		gMedicManager->startWoundTreatmentEvent(Medic);
		return;
	 }
  }

  // Convert the string to an ansi string for ease with the regex.
  tmp.convert(BSTRType_ANSI);
  std::string input_string(tmp.getAnsi());

  static const regex pattern("(action|constitution|health|quickness|stamina|strength)");
  smatch result;

  regex_search(input_string, result, pattern);
  
  // Gather the results of the pattern for validation and use.
  std::string medpackType(result[1]);

	if (gMedicManager->CheckMedicine(Medic, Target, cmdProperties, medpackType))
	{
		//call the event
		gMedicManager->startWoundTreatmentEvent(Medic);
		return;
	}
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



