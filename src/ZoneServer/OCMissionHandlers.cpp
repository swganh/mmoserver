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
#include "MissionManager.h"
#include "ObjectController.h"
#include "ObjectControllerOpcodes.h"
#include "ObjectControllerCommandMap.h"
#include "PlayerObject.h"
#include "WorldConfig.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/DatabaseResult.h"
#include "NetworkManager/Message.h"
#include "NetworkManager/MessageFactory.h"





//=============================================================================================================================
//
// Missions were switched off by the original team, with no reason recorded.
// Missions_Enabled (zone config, default on) switches them off again.
//
namespace {
bool missionsEnabled(PlayerObject* player)
{
    if(!player) {
        return false;
    }
    // gMissionManager is not created in the tutorial
    if(gMissionManager && gWorldConfig->getConfiguration<bool>("Missions_Enabled", true)) {
        return true;
    }
    gMessageLib->SendSystemMessage(L"Missions are currently disabled on this server.", player);
    return false;
}
}

//=============================================================================================================================
//
// mission list request
//

void ObjectController::handleMissionListRequest(Message* message)
{
    PlayerObject*   player  = dynamic_cast<PlayerObject*>(mObject);
    if(!missionsEnabled(player)) {
        return;
    }

    /*uint8           unknown     = */
    message->getUint8();
    uint8           stale_flag  = message->getUint8();
    uint64          terminal_id = message->getUint64();

    gMissionManager->listRequest(player, terminal_id,stale_flag);
}

//=============================================================================================================================

//NOTE: Client never sends this... - meanmon13
void ObjectController::handleMissionDetailsRequest(Message* message)
{
    PlayerObject*   player  = dynamic_cast<PlayerObject*>(mObject);
    if(!missionsEnabled(player)) {
        return;
    }

    gMissionManager->detailsRequest(player);


}

//=============================================================================================================================

void ObjectController::handleMissionCreateRequest(Message* message)
{
    PlayerObject*   player  = dynamic_cast<PlayerObject*>(mObject);
    if(!missionsEnabled(player)) {
        return;
    }

    gMissionManager->createRequest(player);

}

//=============================================================================================================================

void ObjectController::handleGenericMissionRequest(Message* message)
{
    PlayerObject*   player  = dynamic_cast<PlayerObject*>(mObject);
    if(!missionsEnabled(player)) {
        return;
    }
    uint64 mission_id = message->getUint64();

    gMissionManager->missionRequest(player, mission_id);

}

//=============================================================================================================================

void ObjectController::handleMissionAbort(Message* message)
{
    PlayerObject*   player  = dynamic_cast<PlayerObject*>(mObject);
    if(!missionsEnabled(player)) {
        return;
    }
    uint64 mission_id = message->getUint64();

    gMissionManager->missionAbort(player, mission_id);


}

//=============================================================================================================================

