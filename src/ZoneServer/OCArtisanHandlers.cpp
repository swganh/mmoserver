/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/
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

//======================================================================================================================
//
// request survey
//

void ObjectController::_handleRequestSurvey(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*		playerObject = dynamic_cast<PlayerObject*>(mObject);

	// don't allow survey in buildings
	if(playerObject->getParentId())
	{
		gMessageLib->sendSystemMessage(playerObject,L"","error_message","survey_in_structure");
		return;
	}

	SurveyTool*			tool			= dynamic_cast<SurveyTool*>(gWorldManager->getObjectById(targetId));
	CurrentResource*	resource		= NULL;
	string				resourceName;

	message->getStringUnicode16(resourceName);
	resourceName.convert(BSTRType_ANSI);

	resource = reinterpret_cast<CurrentResource*>(gResourceManager->getResourceByNameCRC(resourceName.getCrc()));

	if(tool && resource)
	{
		playerObject->setSurveyState(true);

		// play effect
		string effect = gWorldManager->getClientEffect(tool->getInternalAttribute<uint32>("survey_effect"));

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
		gMessageLib->sendSystemMessage(playerObject,L"","survey","start_survey","","",resourceName);

		// schedule execution
		addEvent(new SurveyEvent(tool,resource),5000);
	}
}

//======================================================================================================================
//
// request sample
//

void ObjectController::_handleRequestCoreSample(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*		playerObject = dynamic_cast<PlayerObject*>(mObject);



	// don't allow sampling in buildings
	if(playerObject->getParentId())
	{
		gMessageLib->sendSystemMessage(playerObject,L"","error_message","survey_in_structure");
		return;
	}

	if(Heightmap::Instance()->hasWater(playerObject->mPosition.mX,playerObject->mPosition.mZ))
	{
		gMessageLib->sendSystemMessage(playerObject,L"","error_message","survey_swimming");
		return;
	}

	if(playerObject->getPerformingState() != PlayerPerformance_None)
	{
		gMessageLib->sendSystemMessage(playerObject,L"","error_message","sample_cancel");
		return;
	}

	uint64 localTime = Anh_Utils::Clock::getSingleton()->getLocalTime();
	if(!playerObject->getNextSampleTime() || (int32)(playerObject->getNextSampleTime() - localTime) <= 0)
	{
		playerObject->setNextSampleTime(localTime + 30000);
	}
	else
	{
		gMessageLib->sendSystemMessage(playerObject,L"","survey","tool_recharge_time","","",L"",(int32)(playerObject->getNextSampleTime() - localTime) / 1000);
		return;
	}

	SurveyTool*			tool		= dynamic_cast<SurveyTool*>(gWorldManager->getObjectById(targetId));
	CurrentResource*	resource	= NULL;

	string resourceName;

	message->getStringUnicode16(resourceName);
	resourceName.convert(BSTRType_ANSI);

	resource = reinterpret_cast<CurrentResource*>(gResourceManager->getResourceByNameCRC(resourceName.getCrc()));

	if(resource == NULL || tool == NULL)
		return;

	if((resource->getType()->getCategoryId() == 903)||(resource->getType()->getCategoryId() == 904))
	{
		gMessageLib->sendSystemMessage(playerObject,L"","survey","must_have_harvester");
		return;
	}

	// put us into sampling mode
	playerObject->setSamplingState(true);

	resourceName.convert(BSTRType_Unicode16);
	gMessageLib->sendSystemMessage(playerObject,L"","survey","start_sampling","","",resourceName);

	// change posture
	_handleKneel(0,NULL,NULL);

	// schedule execution
	addEvent(new SampleEvent(tool,resource),8000);
}

//======================================================================================================================
//
// sent along with requestsurvey, when doing /survey command, contains nothing, not sure what its used for
//
void ObjectController::_handleSurvey(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	//PlayerObject*		playerObject = (PlayerObject*)mObject;

	//gLogger->hexDump(message->getData(),message->getSize());
}

//======================================================================================================================
//
// sent along with requestcoresample, when doing /sample command, contains nothing, not sure what its used for
//
void ObjectController::_handleSample(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	//PlayerObject*		playerObject = (PlayerObject*)mObject;

	//gLogger->hexDump(message->getData(),message->getSize());
}

//======================================================================================================================



