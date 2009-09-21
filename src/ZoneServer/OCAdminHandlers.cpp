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


//=============================================================================================================================
//
// system message
//

void ObjectController::_handleAdminSysMsg(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*		playerObject	= dynamic_cast<PlayerObject*>(mObject);
	string				dataStr;

	message->getStringUnicode16(dataStr);

	if(dataStr.getLength())
	{
		PlayerAccMap::const_iterator it = gWorldManager->getPlayerAccMap()->begin();

		while(it != gWorldManager->getPlayerAccMap()->end())
		{
			const PlayerObject* const player = (*it).second;

			if(player->isConnected())
			{
				gMessageLib->sendSystemMessage((PlayerObject*)player,dataStr);
			}

			++it;
		}
	}
}

//======================================================================================================================
//
// warp self
//

void ObjectController::_handleAdminWarpSelf(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*		player		= dynamic_cast<PlayerObject*>(mObject);
	string				dataStr;
	string				planet;
	int32				planetId	= 0;
	int32				x,z;

	message->getStringUnicode16(dataStr);

	int32 elementCount = swscanf(dataStr.getUnicode16(),L"%i %i %S",&x,&z,planet.getAnsi());

	switch(elementCount)
	{
		// warp on current planet
		case 2:
		{
			// make sure we in bounds
			if(x < -8192 || x > 8192 || z < -8192 || z > 8192)
				break;

			gWorldManager->warpPlanet(player,Anh_Math::Vector3(x,0.0f,z),0);
		}
		return;

		// warp to other or current planet
		case 3:
		{
			// make sure we in bounds
			if(x < -8192 || x > 8192 || z < -8192 || z > 8192)
				break;

			planetId = gWorldManager->getPlanetIdByName(planet);

			if(planetId == -1)
				break;

			// warp on this planet
			if(planetId == gWorldManager->getZoneId())
			{
				gWorldManager->warpPlanet(player,Anh_Math::Vector3(x,0.0f,z),0);
			}
			// zone transfer request
			else
			{
				gMessageLib->sendSystemMessage(player,L"Requesting zone transfer...");

				gMessageLib->sendClusterZoneTransferRequestByPosition(player,Anh_Math::Vector3(x,0.0f,z),planetId);
			}
		}
		return;

		default:
		{
			gMessageLib->sendSystemMessage(player,L"[SYNTAX] /admin_warp_self <x> <z> <planet>");
		}
		return;
	}

	gMessageLib->sendSystemMessage(player,L"Error parsing parameters.");
}

//======================================================================================================================


