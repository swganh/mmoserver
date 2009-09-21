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
#include "TravelMapHandler.h"
#include "BankTerminal.h"
#include "CurrentResource.h"
#include "WorldConfig.h"
#include "UIManager.h"
#include "Item.h"
#include "SurveyTool.h"
#include "Wearable.h"
#include "NPCObject.h"
#include "CraftingTool.h"


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
	float	x			= atof(dataElements[1].getAnsi());
	float	y			= atof(dataElements[2].getAnsi());
	float	z			= atof(dataElements[3].getAnsi());

	int32 planetId = gWorldManager->getPlanetIdByName(planetStr);

	if(planetId == -1)
	{
		gLogger->logMsgF("ObjController::handleCreateWaypointAtPosition: could not find planet id for %s",MSG_NORMAL,planetStr.getAnsi());
		return;
	}

	datapad->requestNewWaypoint(nameStr,Anh_Math::Vector3(x,y,z),planetId,Waypoint_blue);
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
		mDatabase->ExecuteSqlAsync(0,0,"UPDATE waypoints set active=%u WHERE waypoint_id=%lld",(uint8)waypoint->getActive(),targetId);
	}
	else
	{
		gLogger->logMsgF("ObjController::handleSetWaypointStatus: could not find waypoint %lld",MSG_LOW,targetId);
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
		x = (int32)target->mPosition.mX;
		y = (int32)target->mPosition.mY;
		z = (int32)target->mPosition.mZ;

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
					x = (int32)player->mPosition.mX;
					y = (int32)player->mPosition.mY;
					z = (int32)player->mPosition.mZ;
				}
				else
				{
					CellObject*	cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(player->getParentId()));

					if(cell == NULL)
						return;

					BuildingObject* building = dynamic_cast<BuildingObject*>(gWorldManager->getObjectById(cell->getParentId()));

					if(building == NULL)
						return;

					// Well, this is only half the job. We need to add our position inside the building too.
					// As a starter, let's get the orientation of the building.

					// Anh_Math::Quaternion	direction(building->mDirection);
					// Anh_Math::Vector3		position(building->mPosition);

					// Get the length of the vector inside building.
					float length = sqrt((player->mPosition.mX * player->mPosition.mX) + (player->mPosition.mZ * player->mPosition.mZ));
					
					// Get the angle to the object.
					// float alpha = atan(player->mPosition.mX/player->mPosition.mZ);
					float alpha = atan(player->mPosition.mX/player->mPosition.mZ);

					if (alpha < 0.0f)
					{
						if (player->mPosition.mX > 0.0f)
						{
							// add 180
							alpha += 3.1415936539f;
						}
					}
					else
					{
						if (player->mPosition.mX < 0.0f)
						{
							// add 180
							alpha += 3.1415936539f;
						}
					}
					
					// gLogger->logMsgF("Player %d",MSG_NORMAL, (int32)((alpha / (2.0f * 3.1415936539f)) * 360));


					// Now get the angle of the building.
					float wDir = building->mDirection.mW;
					// float yDir = 1.0f;

					if (wDir > 0.0f)
					{
						if (building->mDirection.mY < 0.0)
						{
							wDir *= -1.0f;
							// yDir = -1.0f;
						}
					}

					float angle = 2.0f*acos(wDir);
					// gLogger->logMsgF("Building: %d",MSG_NORMAL, (int32)((angle / (2.0f * 3.1415936539f)) * 360));

					angle -= alpha;

					if (angle > (2.0f * 3.1415936539f))
					{
						angle -= (2.0f * 3.1415936539f);
					}
					else if (angle < 0.0f)
					{
						angle += (2.0f * 3.1415936539f);
					}

					// gLogger->logMsgF("In world: %d",MSG_NORMAL, (int32)((angle / (2.0f * 3.1415936539f)) * 360));

					// gLogger->logMsgF("Player pos: %.1f, %.1f", MSG_NORMAL, player->mPosition.mX, player->mPosition.mZ);

					// gLogger->logMsgF("Base pos: %.0f, %.0f", MSG_NORMAL, building->mPosition.mX, building->mPosition.mZ);

					float xDelta = building->mPosition.mX;
					float zDelta = building->mPosition.mZ;

					if (angle <= 3.1415936539/2.0)
					{
						xDelta += (sin(angle) * length);
						zDelta -= (cos(angle) * length);
					}
					else if (angle <= 3.1415936539f)
					{
						angle = (float)(3.1415936539) - angle;
						xDelta += (sin(angle) * length);
						zDelta += (cos(angle) * length);
					}
					else if (angle <= (float)((3.0 * 3.1415936539)/2.0))
					{
						angle = angle - (float)(3.1415936539f);
						xDelta -= (sin(angle) * length);
						zDelta += (cos(angle) * length);
					}
					else
					{
						angle = (float)(2.0 * 3.1415936539) - angle;
						xDelta -= (sin(angle) * length);
						zDelta -= (cos(angle) * length);
					}

					// gLogger->logMsgF("Delta pos: %.0f, %.0f", MSG_NORMAL, xDelta, zDelta);
					
					if (xDelta > 0.0f )
					{
						// x = (int32)(ceil(xDelta));
						x = (int32)(xDelta + 0.5f);
					}
					else
					{
						// x = (int32)(floor(xDelta));
						x = (int32)(xDelta - 0.5f);
					}


					if ((player->mPosition.mY + building->mPosition.mY) > 0.0f)
					{
						// y = (int32)(ceil(player->mPosition.mY));
						y = (int32)(player->mPosition.mY + building->mPosition.mY + 0.5f);
					}
					else
					{
						// y = (int32)(floor(player->mPosition.mY));
						y = (int32)(player->mPosition.mY + building->mPosition.mY - 0.5f);
					}

					if (zDelta > 0.0f )
					{
						// z = (int32)(ceil(zDelta));
						z = (int32)(zDelta + 0.5f);
					}
					else
					{
						// z = (int32)(floor(zDelta));
						z = (int32)(zDelta - 0.5f);
					}
					// gLogger->logMsgF("Position is: %d, %d", MSG_NORMAL, x, z);
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

		datapad->requestNewWaypoint("Waypoint",Anh_Math::Vector3((float)x,(float)y,(float)z),gWorldManager->getZoneId(),Waypoint_blue);
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
		gLogger->logMsgF("ObjController::handlesetwaypointname: could not find waypoint %lld",MSG_NORMAL,targetId);
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
	sprintf(restStr,"' WHERE waypoint_id=%lld",targetId);
	strcat(sql,restStr);

	mDatabase->ExecuteSqlAsync(NULL,NULL,sql);

	gMessageLib->sendUpdateWaypoint(waypoint,ObjectUpdateChange,player);
}

//======================================================================================================================

