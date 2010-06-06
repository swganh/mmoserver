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
#include "BankTerminal.h"
#include "BuildingObject.h"
#include "CellObject.h"
#include "CraftingTool.h"
#include "CurrentResource.h"
#include "Datapad.h"
#include "Item.h"
#include "NPCObject.h"
#include "ObjectController.h"
#include "ObjectControllerOpcodes.h"
#include "ObjectControllerCommandMap.h"
#include "PlayerObject.h"
#include "TravelMapHandler.h"
#include "SurveyTool.h"
#include "UIManager.h"
#include "WaypointObject.h"
#include "Wearable.h"
#include "WorldConfig.h"
#include "WorldManager.h"

#include "MessageLib/MessageLib.h"
#include "LogManager/LogManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include "Common/Message.h"
#include "Common/MessageFactory.h"

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
	string			dataStr;
	string			nameStr;

	message->getStringUnicode16(dataStr);

	// Have to convert BEFORE using split, since the conversion done there is removed It will assert().. evil grin...
	// Either do the conversion HERE, or better fix the split so it handles unicoe also.
	dataStr.convert(BSTRType_ANSI);
	uint32 elementCount = dataStr.split(dataElements,' ');

	if(elementCount < 5)
	{
		if(elementCount < 4)
		{
			gLogger->log(LogManager::DEBUG,"ObjController::handleCreateWaypointAtPosition: Error in parameters(count %u)",elementCount);
			return;
		}
		else
		{
			nameStr = gWorldManager->getPlanetNameThis();
			nameStr.getAnsi()[0] = toupper(nameStr.getAnsi()[0]);
		}
	}
	else
	{
		for(uint i = 4;i < elementCount;i++)
		{
			nameStr	<< dataElements[i].getAnsi();

			if(i + 1 < elementCount)
				nameStr << " ";
		}
	}

	string	planetStr	= dataElements[0].getAnsi();
	//gLogger->log(LogManager::DEBUG,"ObjController::handleCreateWaypointAtPosition: planet %s",planetStr.getAnsi());
	float	x			= static_cast<float>(atof(dataElements[1].getAnsi()));
	float	y			= static_cast<float>(atof(dataElements[2].getAnsi()));
	float	z			= static_cast<float>(atof(dataElements[3].getAnsi()));

	int32 planetId = gWorldManager->getPlanetIdByName(planetStr);

	if(planetId == -1)
	{
		gLogger->log(LogManager::DEBUG,"ObjController::handleCreateWaypointAtPosition: could not find planet id for %s",planetStr.getAnsi());
		return;
	}

    datapad->requestNewWaypoint(nameStr, glm::vec3(x,y,z),static_cast<uint16>(planetId),Waypoint_blue);
}

//======================================================================================================================
//
// toggles waypoint active status
//

void ObjectController::_handleSetWaypointActiveStatus(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*	player		= dynamic_cast<PlayerObject*>(mObject);
	WaypointObject*	waypoint	= NULL;
	Datapad* datapad			= player->getDataPad();

	waypoint = datapad->getWaypointById(targetId);

	if(waypoint)
	{
		waypoint->toggleActive();
		mDatabase->ExecuteSqlAsync(0,0,"UPDATE waypoints set active=%u WHERE waypoint_id=%"PRIu64"",(uint8)waypoint->getActive(),targetId);
	}
	else
	{
		gLogger->log(LogManager::DEBUG,"ObjController::handleSetWaypointStatus: could not find waypoint %"PRIu64"",targetId);
	}
}

//======================================================================================================================
//
// waypoint request from the commandline
//

void ObjectController::_handleWaypoint(uint64 targetId, Message* message, ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*	player			= dynamic_cast<PlayerObject*>(mObject);
	Datapad* datapad			= player->getDataPad();
	string			waypoint_data;
    glm::vec3       waypoint_position;
					
    // Before anything else verify the datapad can hold another waypoint.
	if(! datapad->getCapacity()) {
		gMessageLib->sendSystemMessage(player, L"", "base_player", "too_many_waypoints");
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
            gMessageLib->sendSystemMessage(player,L"[SYNTAX] /waypoint <x> <z> or /waypoint <x> <y> <z>");
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
		    gMessageLib->sendSystemMessage(player, L"[SYNTAX] Invalid range for /waypoint. x = -8192/8192 y = -500/500 z = -8192/8192");
            return;
        }
    } else {
        // If no parameters were passed to the /waypoint command use the current world position.
        waypoint_position = player->getWorldPosition();
    }
					
    datapad->requestNewWaypoint("Waypoint", waypoint_position, static_cast<uint16>(gWorldManager->getZoneId()), Waypoint_blue);
}

//======================================================================================================================
//
// update a waypoints name
//

void ObjectController::_handleSetWaypointName(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*	player		= dynamic_cast<PlayerObject*>(mObject);
	string			name;
	Datapad* datapad			= player->getDataPad();
	WaypointObject*	waypoint	= datapad->getWaypointById(targetId);
	int8			sql[1024],restStr[64],*sqlPointer;

	if(waypoint == NULL)
	{
		gLogger->log(LogManager::DEBUG,"ObjController::handlesetwaypointname: could not find waypoint %"PRIu64"",targetId);
		return;
	}

	message->getStringUnicode16(name);

	if(!(name.getLength()))
		return;

	waypoint->setName(name);

	name.convert(BSTRType_ANSI);

	sprintf(sql,"UPDATE waypoints SET name='");
	sqlPointer = sql + strlen(sql);
	sqlPointer += mDatabase->Escape_String(sqlPointer,name.getAnsi(),name.getLength());
	sprintf(restStr,"' WHERE waypoint_id=%"PRIu64"",targetId);
	strcat(sql,restStr);

	mDatabase->ExecuteSqlAsync(NULL,NULL,sql);

	gMessageLib->sendUpdateWaypoint(waypoint,ObjectUpdateChange,player);
}

//======================================================================================================================

