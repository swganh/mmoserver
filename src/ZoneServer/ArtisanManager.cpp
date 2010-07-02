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

#include "ArtisanManager.h"
#include "CurrentResource.h"
#include "Item.h"
#include "ObjectController.h"
#include "ObjectControllerOpcodes.h"
#include "ObjectControllerCommandMap.h"
#include "PlayerObject.h"
#include "ResourceManager.h"
#include "ResourceType.h"
#include "SurveyTool.h"
#include "UIManager.h"
#include "Heightmap.h"
#include "WorldConfig.h"
#include "WorldManager.h"
#include "MessageLib/MessageLib.h"
#include "LogManager/LogManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/DatabaseResult.h"
#include "Common/atMacroString.h"
#include "Common/Message.h"
#include "Common/MessageFactory.h"
#include "Utils/clock.h"
#include "ArtisanHeightmapAsyncContainer.h"

bool						ArtisanManager::mInsFlag    = false;
ArtisanManager*				ArtisanManager::mSingleton  = NULL;
//======================================================================================================================
//
// request survey
//

bool ArtisanManager::handleRequestSurvey(Object* player,Object* target,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*		playerObject = dynamic_cast<PlayerObject*>(player);

	// don't allow survey in buildings
	// eventually we will remove all these checks and it will be done in the object controller
	if(playerObject->getParentId())
	{
		gMessageLib->sendSystemMessage(playerObject,L"","error_message","survey_in_structure");
		return false;
	}

	if(playerObject->isDead() || !playerObject->getHam()->checkMainPools(1,1,1))
		return false;
		

	if(playerObject->getPerformingState() != PlayerPerformance_None)
	{
		gMessageLib->sendSystemMessage(playerObject,L"", "error_message", "wrong_state");
		return false;
	}

	if(playerObject->getSurveyState())
	{
		playerObject->getSampleData()->mPendingSample = false;
		gMessageLib->sendSystemMessage(playerObject,L"","survey","survey_sample");
	}

	SurveyTool*			tool			= dynamic_cast<SurveyTool*>(target);
	CurrentResource*	resource		= NULL;
	BString				resourceName;

	message->getStringUnicode16(resourceName);
	resourceName.convert(BSTRType_ANSI);

	resource = reinterpret_cast<CurrentResource*>(gResourceManager->getResourceByNameCRC(resourceName.getCrc()));

	if(tool && resource)
	{
		playerObject->setSurveyState(true);

		// play effect
		BString effect = gWorldManager->getClientEffect(tool->getInternalAttribute<uint32>("survey_effect"));

		gMessageLib->sendPlayClientEffectLocMessage(effect,playerObject->mPosition,playerObject);

		PlayerObjectSet*			playerList	= playerObject->getKnownPlayers();
		PlayerObjectSet::iterator	it			= playerList->begin();

		while(it != playerList->end())
		{
			gMessageLib->sendPlayClientEffectLocMessage(effect,playerObject->mPosition,(*it));

			++it;
		}

		// send system message
		resourceName.convert(BSTRType_Unicode16);
		gMessageLib->sendSystemMessage(playerObject,L"","survey","start_survey","","",resourceName.getUnicode16());

		// schedule execution
		player->getController()->addEvent(new SurveyEvent(tool,resource),5000);
	}
	return true;
}

//======================================================================================================================
//
// request sample
//

bool ArtisanManager::handleRequestCoreSample(Object* player,Object* target, Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*		playerObject = dynamic_cast<PlayerObject*>(player);


	if(playerObject->checkIfMounted())
	{
		gMessageLib->sendSystemMessage(playerObject,L"You cannot take resource samples while mounted.");
		return false;
	}

	if(playerObject->getPerformingState() != PlayerPerformance_None)
	{
		gMessageLib->sendSystemMessage(playerObject,L"", "error_message", "wrong_state");
		return false;
	}

	// don't allow sampling in buildings
	if(playerObject->getParentId())
	{
		gMessageLib->sendSystemMessage(playerObject,L"","error_message","survey_in_structure");
		return false;
	}

	if(playerObject->getPerformingState() != PlayerPerformance_None)
	{
		gMessageLib->sendSystemMessage(playerObject,L"","error_message","sample_cancel");
		return false;
	}

	uint64 localTime = gWorldManager->getServerTime();
	if(!playerObject->getNextSampleTime() || (int32)(playerObject->getNextSampleTime() - localTime) <= 0)
	{
		playerObject->setNextSampleTime(localTime + 30000);
	}
	else
	{
		gMessageLib->sendSystemMessage(playerObject,L"","survey","tool_recharge_time","","",L"",(int32)(playerObject->getNextSampleTime() - localTime) / 1000);
		return false;
	}

	SurveyTool*			tool		= dynamic_cast<SurveyTool*>(target);
	CurrentResource*	resource	= NULL;

	BString resourceName;

	message->getStringUnicode16(resourceName);
	resourceName.convert(BSTRType_ANSI);

	resource = reinterpret_cast<CurrentResource*>(gResourceManager->getResourceByNameCRC(resourceName.getCrc()));

	if(resource == NULL || tool == NULL)
		return false;

	if((resource->getType()->getCategoryId() == 903)||(resource->getType()->getCategoryId() == 904))
	{
		gMessageLib->sendSystemMessage(playerObject,L"","survey","must_have_harvester");
		return false;
	}

	ArtisanHeightmapAsyncContainer* container = new ArtisanHeightmapAsyncContainer(gArtisanManager, HeightmapCallback_ArtisanSurvey);
	container->addToBatch(playerObject->mPosition.x,playerObject->mPosition.z);

	container->playerObject = playerObject;
	container->resource = resource;
	container->resourceName = resourceName;
	container->tool = tool;

	gHeightmap->addNewHeightMapJob(container);

	return true;
}

void ArtisanManager::HeightmapArtisanHandler(HeightmapAsyncContainer* ref)
{
		ArtisanHeightmapAsyncContainer* container = static_cast<ArtisanHeightmapAsyncContainer*>(ref);

		HeightResultMap* mapping = container->getResults();
		HeightResultMap::iterator it = mapping->begin();
		if(it != mapping->end() && it->second != NULL)
		{
			if(it->second->hasWater)
			{
				gMessageLib->sendSystemMessage(container->playerObject,L"","error_message","survey_swimming");
				return;
			}

			// put us into sampling mode
			container->playerObject->setSamplingState(true);

			container->resourceName.convert(BSTRType_Unicode16);
			gMessageLib->sendSystemMessage(container->playerObject,L"","survey","start_sampling","","",container->resourceName.getUnicode16());

			// change posture
			container->playerObject->setPosture(CreaturePosture_Crouched);

			// schedule execution
			container->playerObject->getController()->addEvent(new SampleEvent(container->tool,container->resource),8000);
		}
}

//======================================================================================================================
//
// sent along with requestsurvey, when doing /survey command, contains nothing, not sure what its used for
//
bool ArtisanManager::handleSurvey(Object* player,Object* target,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	//PlayerObject*		playerObject = (PlayerObject*)mObject;

	//gLogger->hexDump(message->getData(),message->getSize());
	return true;
}

//======================================================================================================================
//
// sent along with requestcoresample, when doing /sample command, contains nothing, not sure what its used for
//
bool ArtisanManager::handleSample(Object* player,Object* target,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	//PlayerObject*		playerObject = (PlayerObject*)mObject;

	//gLogger->hexDump(message->getData(),message->getSize());
	return true;
}

//======================================================================================================================

