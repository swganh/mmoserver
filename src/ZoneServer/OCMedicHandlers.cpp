/*
 This file is part of MMOServer. For more information, visit http://swganh.com
 
 Copyright (c) 2006 - 2011 The SWG:ANH Team

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "ObjectController.h"
#include <boost/regex.hpp>  // NOLINT
#include "MedicManager.h"
#include "ObjectController.h"
#include "ObjectControllerOpcodes.h"
#include "ObjectControllerCommandMap.h"
#include "PlayerObject.h"
#include "WorldManager.h"
#include "MessageLib/MessageLib.h"

#include "DatabaseManager/Database.h"
#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/DatabaseResult.h"
#include "NetworkManager/MessageFactory.h"
#include "NetworkManager/Message.h"
#include "ForageManager.h"

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

    std::string messageResponse = gMedicManager->handleMessage(message,"(action|constitution|health|quickness|stamina|strength)");
    if (messageResponse.length() == 0)
    {
        //you must specify a valid wound type
        gMessageLib->SendSystemMessage(::common::OutOfBand("healing_response", "healing_response_65"), Medic);
    }
    else
    {
        //check Medic has enough Mind
        Ham* ham = Medic->getHam();
        if(ham->checkMainPools(0, 0, 140))
        {
            if (gMedicManager->CheckMedicine(Medic, Target, cmdProperties, messageResponse))
            {
                //call the event
                gMedicManager->startWoundTreatmentEvent(Medic);
                return;
            }
        }
        else
            gMessageLib->SendSystemMessage(::common::OutOfBand("healing_response", "not_enough_mind"), Medic);

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
    PlayerObject* Medic = dynamic_cast<PlayerObject*>(mObject);
    PlayerObject* Target = dynamic_cast<PlayerObject*>(Medic->getHealingTarget(Medic));

    //check Medic has enough Mind
    Ham* ham = Medic->getHam();
    if(ham->checkMainPools(cmdProperties->mHealthCost, cmdProperties->mActionCost, cmdProperties->mMindCost))
    {
        if (gMedicManager->HealDamage(Medic, Target, 0, cmdProperties, "tendDamage"))
        {
            ham->updatePropertyValue(HamBar_Focus ,HamProperty_Wounds, 5);
            ham->updatePropertyValue(HamBar_Willpower ,HamProperty_Wounds, 5);
            ham->updateBattleFatigue(2, true);
            //call the event
            gMedicManager->startInjuryTreatmentEvent(Medic);
            return;
        }
    }
    else
        gMessageLib->SendSystemMessage(::common::OutOfBand("healing_response", "not_enough_mind"), Medic);

}

//=============================================================================================================================
//
// tendwound
//

void ObjectController::_handleTendWound(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    PlayerObject* Medic = dynamic_cast<PlayerObject*>(mObject);
    PlayerObject* Target = dynamic_cast<PlayerObject*>(Medic->getHealingTarget(Medic));
    //TODO:: add medic droid
    /*EMLocationType loc = Medic->getPlayerLocation();
    if(loc != EMLocation_Cantina || loc != EMLocation_Camp || loc != EMLocation_PlayerStructure)
    {
        return;
    }*/
    std::string messageResponse = gMedicManager->handleMessage(message,"(action|constitution|health|quickness|stamina|strength)");
    if (messageResponse.length() == 0)
    {
        //you must specify a valid wound type
        gMessageLib->SendSystemMessage(::common::OutOfBand("healing_response", "healing_response_65"), Medic);
    }
    else
    {
        //check Medic has enough Mind
        Ham* ham = Medic->getHam();
        if(ham->checkMainPools(0, 0, 500))
        {
            if (gMedicManager->HealWound(Medic, Target, 0, cmdProperties, messageResponse + "tendwound"))
            {
                ham->updatePropertyValue(HamBar_Focus ,HamProperty_Wounds, 5);
                ham->updatePropertyValue(HamBar_Willpower ,HamProperty_Wounds, 5);
                ham->updateBattleFatigue(2, true);
                //call the event
                gMedicManager->startWoundTreatmentEvent(Medic);
                return;
            }
        }
        else
            gMessageLib->SendSystemMessage(::common::OutOfBand("healing_response", "not_enough_mind"), Medic);

    }

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
    PlayerObject* Medic = dynamic_cast<PlayerObject*>(mObject);
    PlayerObject* Target = dynamic_cast<PlayerObject*>(Medic->getHealingTarget(Medic));

    //check Medic has enough Mind
    Ham* ham = Medic->getHam();
    if(ham->checkMainPools(0, 0, cmdProperties->mMindCost))
    {
        if (gMedicManager->HealDamage(Medic, Target, 0, cmdProperties, "quickHeal"))
        {
            ham->updatePropertyValue(HamBar_Focus ,HamProperty_Wounds, 10);
            ham->updatePropertyValue(HamBar_Willpower ,HamProperty_Wounds, 10);
            ham->updateBattleFatigue(2, true);
            //call the event
            gMedicManager->startQuickHealInjuryTreatmentEvent(Medic);
            return;
        }
    }
    else
        gMessageLib->SendSystemMessage(::common::OutOfBand("healing_response", "not_enough_mind"), Medic);
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

