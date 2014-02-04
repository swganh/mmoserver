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
//#include "ZoneServer/Objects/BankTerminal.h"
//#include "Zoneserver/GameSystemManagers/Structure Manager/BuildingObject.h"
//#include "ZoneServer/GameSystemManagers/Structure Manager/CellObject.h"
//#include "ZoneServer/Objects/CraftingTool.h"
//#include "ZoneServer/GameSystemManagers/Resource Manager/CurrentResource.h"
#include "Zoneserver/Objects/Datapad.h"
#include "Zoneserver/Objects/Item.h"
#include "ZoneServer/GameSystemManagers/NPC Manager/NPCObject.h"
#include "Zoneserver/ObjectController/ObjectController.h"
#include "ZoneServer/ObjectController/ObjectControllerOpcodes.h"
#include "ZoneServer/ObjectController/ObjectControllerCommandMap.h"
#include "ZoneServer/Objects/Player Object/PlayerObject.h"
#include "ZoneServer/GameSystemManagers/Travel Manager/TravelMapHandler.h"
//#include "Zoneserver/Objects/SurveyTool.h"
#include "ZoneServer/GameSystemManagers/UI Manager/UIManager.h"
#include "Zoneserver/Objects/waypoints/WaypointObject.h"
//#include "ZoneServer/Objects/wearable.h"
#include "ZoneServer/WorldConfig.h"
#include "ZoneServer/WorldManager.h"

#include "MessageLib/MessageLib.h"
#include "DatabaseManager/Database.h"
//#include "DatabaseManager/DatabaseResult.h"
//#include "DatabaseManager/DataBinding.h"
#include "NetworkManager/Message.h"
#include "NetworkManager/MessageFactory.h"

//======================================================================================================================
//
// waypoint request from datapad and planetmap
//

void ObjectController::_handleRequestWaypointAtPosition(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);
    Datapad* datapad			= player->getDataPad();

    //if(!datapad->getCapacity())
    //{
    //	gMessageLib->sendSystemMessage(player,L"","base_player","too_many_waypoints");
    //	return;
    //}

    BStringVector	dataElements;
    BString			dataStr;
    std::string			nameStr;

    message->getStringUnicode16(dataStr);

    // Have to convert BEFORE using split, since the conversion done there is removed It will assert().. evil grin...
    // Either do the conversion HERE, or better fix the split so it handles unicoe also.
    dataStr.convert(BSTRType_ANSI);
    uint32 elementCount = dataStr.split(dataElements,' ');

    if(elementCount < 5)
    {
        if(elementCount < 4)
        {
            return;
        }
        else
        {
            nameStr = gWorldManager->getPlanetNameThis();
            std::transform(nameStr.begin(), nameStr.end(), nameStr.begin(), &tolower);
        }
    }
    else
    {
        for(uint i = 4; i < elementCount; i++)
        {
            nameStr.append(dataElements[i].getAnsi());

            if(i + 1 < elementCount)
                nameStr.append(" ");
        }
    }

    std::string	planetStr	= dataElements[0].getAnsi();
    //gLogger->log(LogManager::DEBUG,"ObjController::handleCreateWaypointAtPosition: planet %s",planetStr.getAnsi());
    float	x			= static_cast<float>(atof(dataElements[1].getAnsi()));
    float	y			= static_cast<float>(atof(dataElements[2].getAnsi()));
    float	z			= static_cast<float>(atof(dataElements[3].getAnsi()));

    int32 planetId = gWorldManager->getPlanetIdByName(planetStr);

    if(planetId == -1)
    {
        return;
    }

	std::u16string name_unicode(nameStr.begin(), nameStr.end());
    datapad->requestNewWaypoint(name_unicode, glm::vec3(x,y,z),static_cast<uint16>(planetId),Waypoint_blue);
}

//======================================================================================================================
//
// toggles waypoint active status
//

void ObjectController::_handleSetWaypointActiveStatus(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    PlayerObject*	player		= dynamic_cast<PlayerObject*>(mObject);
    Datapad* datapad			= player->getDataPad();

    std::shared_ptr<WaypointObject>	waypoint = datapad->getWaypointById(targetId);

    if(!waypoint)
    {
		DLOG(info) << "ObjController::handleSetWaypointStatus: could not find waypoint " << targetId;
		return;
	}
    
	waypoint->toggleActive();
	std::stringstream sql;
	sql << "UPDATE " << mDatabase->galaxy() << ".waypoints set active=" << (waypoint->getActive() ? 1 : 0) << " WHERE waypoint_id=" << targetId << ";";
    mDatabase->executeSqlAsync(0,0,sql.str());
    
}

//======================================================================================================================
//
// waypoint request from the commandline
//

void ObjectController::_handleWaypoint(uint64 targetId, Message* message, ObjectControllerCmdProperties* cmdProperties)
{
    PlayerObject*	player			= dynamic_cast<PlayerObject*>(mObject);
    Datapad* datapad			= player->getDataPad();
    BString			waypoint_data;
    glm::vec3       waypoint_position;

    // Before anything else verify the datapad can hold another waypoint.
    if(! datapad->getCapacity()) {
        gMessageLib->SendSystemMessage(::common::OutOfBand("base_player", "too_many_waypoints"), player);
        return;
    }

    // Read in any waypoint data that may have been sent:
    //  [SYNTAX] /waypoint <x> <z> or /waypoint <x> <y> <z>
    message->getStringUnicode16(waypoint_data);

    // Check and see if any parameters were passed to the /waypoint command. For
    // immediate purposes the length can be used to tell if anything or nothing was passed.
    if (waypoint_data.getLength()) {
        int count = swscanf(waypoint_data.getUnicode16(), L"%f %f %f", &waypoint_position.x, &waypoint_position.y, &waypoint_position.z);

        // If there are an invalid number of items then disregard and notify the player of the correct
        // format for the /waypoint command.
        if (count < 2 || count > 3) {
            gMessageLib->SendSystemMessage(L"[SYNTAX] /waypoint <x> <z> or /waypoint <x> <y> <z>", player);
            return;
        }

        // If the item count is 2 it means no y value was set in the /waypoint command so
        // update the waypoint_position data values accordingly.
        if (count == 2) {
            waypoint_position.z = waypoint_position.y;
            waypoint_position.y = 0;
        }

        // Validate the position values.
        if (waypoint_position.x < -8192 || waypoint_position.x > 8192 ||
                waypoint_position.y < -500 || waypoint_position.y > 500 ||
                waypoint_position.z < -8192 || waypoint_position.z > 8192) {
            gMessageLib->SendSystemMessage( L"[SYNTAX] Invalid range for /waypoint. x = -8192/8192 y = -500/500 z = -8192/8192", player);
            return;
        }
    } else {
        // If no parameters were passed to the /waypoint command use the current world position.
        waypoint_position = player->getWorldPosition();
    }
	
	std::string n("Waypoint");
	std::u16string name(n.begin(), n.end());

	datapad->requestNewWaypoint(name, waypoint_position, static_cast<uint16>(gWorldManager->getZoneId()), Waypoint_blue);
}

//======================================================================================================================
//
// update a waypoints name
//

void ObjectController::_handleSetWaypointName(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    PlayerObject*	player		= dynamic_cast<PlayerObject*>(mObject);
    std::u16string	name;
	
    Datapad* datapad			= player->getDataPad();
    std::shared_ptr<WaypointObject>	waypoint	= datapad->getWaypointById(targetId);

    if(waypoint == NULL)    {
        DLOG(info) << "ObjController::handlesetwaypointname: could not find waypoint "<< targetId;
        return;
    }

    name = message->getStringUnicode16();
	std::string		name_ansi(name.begin(), name.end());
	LOG(info) << "ObjectController::_handleSetWaypointName new name : " << name_ansi << "for id : " << waypoint->getId();

    if(!(name.length()))
        return;

	waypoint->setName(name);

	datapad->updateWaypoint(waypoint);
}

//======================================================================================================================

