/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

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

#include <glm/gtx/fast_trigonometry.hpp>

using namespace glm::gtx;

//======================================================================================================================
//
// waypoint request from datapad and planetmap
//

void ObjectController::_handleRequestWaypointAtPosition(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);
	Datapad*		datapad = dynamic_cast<Datapad*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Datapad));

	if(!datapad->getCapacity())
	{
		gMessageLib->sendSystemMessage(player,L"","base_player","too_many_waypoints");
		return;
	}

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
			gLogger->logMsgF("ObjController::handleCreateWaypointAtPosition: Error in parameters(count %u)",MSG_NORMAL,elementCount);
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
	gLogger->logMsgF("ObjController::handleCreateWaypointAtPosition: planet %s",MSG_NORMAL,planetStr.getAnsi());
	float	x			= static_cast<float>(atof(dataElements[1].getAnsi()));
	float	y			= static_cast<float>(atof(dataElements[2].getAnsi()));
	float	z			= static_cast<float>(atof(dataElements[3].getAnsi()));

	int32 planetId = gWorldManager->getPlanetIdByName(planetStr);

	if(planetId == -1)
	{
		gLogger->logMsgF("ObjController::handleCreateWaypointAtPosition: could not find planet id for %s",MSG_NORMAL,planetStr.getAnsi());
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
	Datapad*		datapad		= dynamic_cast<Datapad*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Datapad));

	waypoint = datapad->getWaypointById(targetId);

	if(waypoint)
	{
		waypoint->toggleActive();
		mDatabase->ExecuteSqlAsync(0,0,"UPDATE waypoints set active=%u WHERE waypoint_id=%"PRIu64"",(uint8)waypoint->getActive(),targetId);
	}
	else
	{
		gLogger->logMsgF("ObjController::handleSetWaypointStatus: could not find waypoint %"PRIu64"",MSG_LOW,targetId);
	}
}

//======================================================================================================================
//
// waypoint request from the commandline
//

void ObjectController::_handleWaypoint(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*	player			= dynamic_cast<PlayerObject*>(mObject);
	Datapad*		datapad			= dynamic_cast<Datapad*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Datapad));
	string			name;
	int32			x,y,z;
	uint32			elementCount	= 0;
	bool			verified		= false;

	
					
	if(!datapad->getCapacity())
	{
		gMessageLib->sendSystemMessage(player,L"","base_player","too_many_waypoints");
		return;
	}
					

	// create waypoint for current target, don't care about passed parameters
	
	// NOTE by ERU: I don't know why or when this code is to be used.
	// The code makes it impossible to set a Waypoint unless you untarget everything first.
	// And you don't even know about that Ticket collector you targeted when you travelled the last time.
	// And thats why we get the wierd Waypoints 6000 m away...
	/*
	if(Object* target = player->getTarget())
	{
		x = (int32)target->mPosition.x;
		y = (int32)target->mPosition.y;
		z = (int32)target->mPosition.z;

		verified = true;
	}
	// create waypoint, no object targeted
	else
	*/
	{
		message->getStringUnicode16(name);

		if(name.getLength())
			elementCount = swscanf(name.getUnicode16(),L"%i %i %i",&x,&y,&z);

		switch(elementCount)
		{
			case 0:
			{
				if(!(player->getParentId()))
				{
					x = static_cast<int32>(player->mPosition.x);
					y = static_cast<int32>(player->mPosition.y);
					z = static_cast<int32>(player->mPosition.z);
				}
				else
				{
					CellObject*	cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(player->getParentId()));

					if(cell == NULL)
						return;

					BuildingObject* building = dynamic_cast<BuildingObject*>(gWorldManager->getObjectById(cell->getParentId()));

					if(building == NULL)
						return;

                    // Get the length of the vector inside building.
                    float length = glm::length(player->mPosition);

                    // Determine the translation angle.
                    float theta = quaternion::angle(building->mDirection) - fast_trigonometry::fastAtan(player->mPosition.x, player->mPosition.z);

                    // Calculate the player position relative to building position.
                    x = static_cast<int32>(building->mPosition.x + (sin(theta) * length));
                    z = static_cast<int32>(building->mPosition.z - (cos(theta) * length));
                    y = static_cast<int32>(building->mPosition.y + player->mPosition.y);

				}

				verified = true;
			}
			break;

			case 2:
			{
				z = y;
				y = 0;

				if(x < -8192 || x > 8192)
					break;

				if(z < -8192 || z > 8192)
					break;

				verified = true;
			}
			break;

			case 3:
			{
				if(x < -8192 || x > 8192)
					break;

				if(y < -500 || y > 500)
					break;

				if(z < -8192 || z > 8192)
					break;

				verified = true;
			}
			break;

			default:
			{
				gMessageLib->sendSystemMessage(player,L"[SYNTAX] /waypoint <x> <z> or /waypoint <x> <y> <z>");

				return;
			}
			break;
		}
	}

	if(verified)
	{

        datapad->requestNewWaypoint("Waypoint", glm::vec3((float)x,(float)y,(float)z),static_cast<uint16>(gWorldManager->getZoneId()),Waypoint_blue);
	}
	else
	{
		gMessageLib->sendSystemMessage(player,L"Error parsing coordinates.");
	}
}

//======================================================================================================================
//
// update a waypoints name
//

void ObjectController::_handleSetWaypointName(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*	player		= dynamic_cast<PlayerObject*>(mObject);
	string			name;
	Datapad*		datapad		= dynamic_cast<Datapad*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Datapad));
	WaypointObject*	waypoint	= datapad->getWaypointById(targetId);
	int8			sql[1024],restStr[64],*sqlPointer;

	if(waypoint == NULL)
	{
		gLogger->logMsgF("ObjController::handlesetwaypointname: could not find waypoint %"PRIu64"",MSG_NORMAL,targetId);
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

