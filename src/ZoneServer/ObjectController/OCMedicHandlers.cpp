/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2014 The SWG:ANH Team
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
#include "Zoneserver/ObjectController/ObjectController.h"
#include <boost/regex.hpp>  // NOLINT
#include "Zoneserver/ProfessionManagers/Medic Manager/MedicManager.h"
#include "Zoneserver/ObjectController/ObjectController.h"
#include "ZoneServer/ObjectController/ObjectControllerOpcodes.h"
#include "ZoneServer/ObjectController/ObjectControllerCommandMap.h"
#include "ZoneServer/Objects/Player Object/PlayerObject.h"
#include "ZoneServer/WorldManager.h"
#include "MessageLib/MessageLib.h"

#include "ZoneServer\Services\ham\ham_service.h"

//#include "NetworkManager/MessageFactory.h"
//#include "NetworkManager/Message.h"
#include "ZoneServer/GameSystemManagers/Forage Manager/ForageManager.h"

#include "ZoneServer\Services\ham\ham_service.h"
#include "anh/app/swganh_kernel.h"
#include "anh\service\service_manager.h"

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
    CreatureObject* creature  = dynamic_cast<CreatureObject*>(mObject); PlayerObject* Medic = creature->GetGhost();
    PlayerObject* Target = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(targetId));
    if(Target != 0)
    {
        gMedicManager->Diagnose(Medic, Target);
    } else {
        gMessageLib->SendSystemMessage(::common::OutOfBand("healing_response", "healing_response_b6"), Medic);
        return;
    }
}

//=============================================================================================================================
//
// healdamage
//

void ObjectController::_handleHealDamage(uint64 targetId, Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    CreatureObject* creature  = dynamic_cast<CreatureObject*>(mObject); PlayerObject* Medic = creature->GetGhost();
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
    CreatureObject* creature  = dynamic_cast<CreatureObject*>(mObject); PlayerObject* Medic = creature->GetGhost();
    PlayerObject* Target = dynamic_cast<PlayerObject*>(Medic->getHealingTarget(Medic));

    std::string messageResponse = gMedicManager->handleMessage(message,"(action|constitution|health|quickness|stamina|strength)");
    if (messageResponse.length() == 0)
    {
        //you must specify a valid wound type
        gMessageLib->SendSystemMessage(::common::OutOfBand("healing_response", "healing_response_65"), Medic);
    }
    else
    {
        //check Medic has enough Mind
		auto ham = gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::ham::HamService>("HamService");

        if(!ham->checkMainPool(Medic->GetCreature(), HamBar_Mind, 140))        {
			gMessageLib->SendSystemMessage(::common::OutOfBand("healing_response", "not_enough_mind"), Medic);
			return;
		}
            
		if (gMedicManager->CheckMedicine(Medic, Target, cmdProperties, messageResponse))
        {
            //call the event
            gMedicManager->startWoundTreatmentEvent(Medic);
            return;
        }    

    }
}

//=============================================================================================================================
//
// medicalforage
//

void ObjectController::_handleMedicalForage(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    CreatureObject* creature  = dynamic_cast<CreatureObject*>(mObject); PlayerObject* player = creature->GetGhost();
    if(player)
        gForageManager->startForage(player, ForageClass_Medic);
}

//=============================================================================================================================
//
// tenddamage
//

void ObjectController::_handleTendDamage(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    CreatureObject* creature  = dynamic_cast<CreatureObject*>(mObject); PlayerObject* Medic = creature->GetGhost();
    PlayerObject* Target = dynamic_cast<PlayerObject*>(Medic->getHealingTarget(Medic));

    //check Medic has enough Mind
    auto ham = gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::ham::HamService>("HamService");

    if(!ham->checkMainPools(Medic->GetCreature(), cmdProperties->mHealthCost, cmdProperties->mActionCost, cmdProperties->mMindCost)){
		gMessageLib->SendSystemMessage(::common::OutOfBand("healing_response", "not_enough_mind"), Medic);
		return;
	}

    if (gMedicManager->HealDamage(Medic, Target, 0, cmdProperties, "tendDamage"))
    {
		ham->ApplyWound(Medic->GetCreature(), HamBar_Focus, 5);
        ham->ApplyWound(Medic->GetCreature(), HamBar_Willpower, 5);
		Medic->GetCreature()->AddBattleFatigue(2);
        
		//call the event
        gMedicManager->startInjuryTreatmentEvent(Medic);
        return;
    }

}

//=============================================================================================================================
//
// tendwound
//

void ObjectController::_handleTendWound(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    CreatureObject* creature  = dynamic_cast<CreatureObject*>(mObject); PlayerObject* Medic = creature->GetGhost();
    PlayerObject* Target = dynamic_cast<PlayerObject*>(Medic->getHealingTarget(Medic));
    //TODO:: add medic droid
    /*EMLocationType loc = Medic->getPlayerLocation();
    if(loc != EMLocation_Cantina || loc != EMLocation_Camp || loc != EMLocation_PlayerStructure)
    {
        return;
    }*/
    std::string messageResponse = gMedicManager->handleMessage(message,"(action|constitution|health|quickness|stamina|strength)");
    if (messageResponse.length() == 0)    {
        //you must specify a valid wound type
        gMessageLib->SendSystemMessage(::common::OutOfBand("healing_response", "healing_response_65"), Medic);
		return;
    }
    
    //check Medic has enough Mind
    auto ham = gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::ham::HamService>("HamService");
    if(!ham->checkMainPools(Medic->GetCreature(), 0, 0, 500))    {
		gMessageLib->SendSystemMessage(::common::OutOfBand("healing_response", "not_enough_mind"), Medic);
		return;
	}
        
	if (gMedicManager->HealWound(Medic, Target, 0, cmdProperties, messageResponse + "tendwound"))
    {
        ham->ApplyWound(Medic->GetCreature(), HamBar_Focus, 5);
        ham->ApplyWound(Medic->GetCreature(), HamBar_Willpower, 5);
		Medic->GetCreature()->AddBattleFatigue(2);
        //call the event
        gMedicManager->startWoundTreatmentEvent(Medic);
        return;
	}

}

//=============================================================================================================================
//
// firstaid
//

void ObjectController::_handleFirstAid(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    //CreatureObject* creature  = dynamic_cast<CreatureObject*>(mObject); PlayerObject* Medic = creature->GetGhost();
    //gMessageLib->sendSystemMessage(Medic, "First Aid has not been implemented yet. Sorry.");
}

//=============================================================================================================================
//
// quickheal
//

void ObjectController::_handleQuickHeal(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    CreatureObject* creature  = dynamic_cast<CreatureObject*>(mObject); PlayerObject* Medic = creature->GetGhost();
    PlayerObject* Target = dynamic_cast<PlayerObject*>(Medic->getHealingTarget(Medic));

    //check Medic has enough Mind
    auto ham = gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::ham::HamService>("HamService");
    if(!ham->checkMainPools(Medic->GetCreature(), 0, 0, cmdProperties->mMindCost))    {
		gMessageLib->SendSystemMessage(::common::OutOfBand("healing_response", "not_enough_mind"), Medic);
		return;
	}
    
	if (gMedicManager->HealDamage(Medic, Target, 0, cmdProperties, "quickHeal"))
    {
        ham->ApplyWound(Medic->GetCreature(), HamBar_Focus, 5);
        ham->ApplyWound(Medic->GetCreature(), HamBar_Willpower, 5);
		Medic->GetCreature()->AddBattleFatigue(2);
        //call the event
        gMedicManager->startQuickHealInjuryTreatmentEvent(Medic);
        return;
    }
       
}

//=============================================================================================================================
//
// dragincapacitatedplayer
//

void ObjectController::_handleDragIncapacitatedPlayer(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    //CreatureObject* creature  = dynamic_cast<CreatureObject*>(mObject); PlayerObject* Medic = creature->GetGhost();
    //gMessageLib->sendSystemMessage(Medic, "Drag has not been implemented yet. Sorry.");
}

//=============================================================================================================================

