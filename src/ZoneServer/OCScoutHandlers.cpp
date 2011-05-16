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
#include "ObjectControllerOpcodes.h"
#include "ObjectControllerCommandMap.h"
#include "MessageLib/MessageLib.h"

#include "DatabaseManager/Database.h"
#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/DatabaseResult.h"
#include "NetworkManager/MessageFactory.h"
#include "NetworkManager/Message.h"
#include "ScoutManager.h"
#include "ForageManager.h"
#include "WorldManager.h"
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
        gMessageLib->SendSystemMessage(::common::OutOfBand("internal_command_string", "target_not_creature"), playerObject);

    BString cmdString;
    message->getStringAnsi(cmdString);

    int8 rawData[128];

    int32 elementCount = sscanf(cmdString.getAnsi(), "%80s", rawData);

    if(elementCount == 0)
    {
        gScoutManager->handleHarvestCorpse(playerObject, target, HARVEST_ANY);
    }
    else if(elementCount == 1)
    {
        BString data(rawData);
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
        gMessageLib->SendSystemMessage(::common::OutOfBand("internal_command_string", "no_resource"), playerObject);
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
