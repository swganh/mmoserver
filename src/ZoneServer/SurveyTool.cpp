/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "SurveyTool.h"
#include "ObjectControllerOpcodes.h"
#include "ObjectFactory.h"
#include "PlayerObject.h"
#include "ResourceCategory.h"
#include "ResourceManager.h"
#include "ResourceType.h"
#include "UIManager.h"
#include "WorldManager.h"
#include "ZoneOpcodes.h"
#include "MessageLib/MessageLib.h"
#include "LogManager/LogManager.h"
#include "DatabaseManager/Database.h"
#include "Common/MessageFactory.h"
#include "Common/MessageOpcodes.h"
#include "MathLib/Quaternion.h"
#include <boost/lexical_cast.hpp>


//=============================================================================

SurveyTool::SurveyTool() : Item()
{
	mRadialMenu	= RadialMenuPtr(new RadialMenu());

	mRadialMenu->addItem(1,0,radId_itemUse,radAction_ObjCallback);
	mRadialMenu->addItem(2,0,radId_examine,radAction_ObjCallback);
	mRadialMenu->addItem(3,0,radId_itemDestroy,radAction_Default);
	mRadialMenu->addItem(4,0,radId_serverItemOptions,radAction_ObjCallback,"@sui:tool_options");
	mRadialMenu->addItem(5,4,radId_serverSurveyToolRange,radAction_ObjCallback,"@sui:survey_range");
}

//=============================================================================

SurveyTool::~SurveyTool()
{
}

//=============================================================================

void SurveyTool::handleObjectMenuSelect(uint8 messageType,Object* srcObject)
{
	PlayerObject* playerObject = dynamic_cast<PlayerObject*>(srcObject);

	if(!(playerObject->isConnected()))
		return;

	// bring up the surve ui
	switch(messageType)
	{
		case radId_itemUse:
		{
			// verify we are able to use this
			if(!(playerObject->verifyAbility(opOCsurvey)))
			{
				gMessageLib->sendSystemMessage(playerObject,L"You lack the skill to use thís tool.");

				return;
			}

			if(playerObject->getParentId())
			{
				gMessageLib->sendSystemMessage(playerObject,L"","error_message","survey_in_structure");
				return;
			}	

			//check whether the tool is initialized already - if not initialize

			int32	range	= getInternalAttribute<int32>("survey_range");
			if(range < 0 )
			{
				_createRangeMenu(playerObject, true);
				return;
			}

			StartUsing(playerObject);


		}
		break;

		case radId_serverSurveyToolRange:
		{
			if(!(playerObject->verifyAbility(opOCsurvey)))
			{
				gMessageLib->sendSystemMessage(playerObject,L"You lack the skill to use thís tool.");

				return;
			}

			_createRangeMenu(playerObject);
		}
		break;
	}
}

//=============================================================================
//
// Starts the sampling
//

void SurveyTool::StartUsing(PlayerObject* player)
{
	Message*			message;
	ResourceCategory*	mainCat = gResourceManager->getResourceCategoryById(getInternalAttribute<uint32>("survey_tool_resource_category"));
	ResourceList		resourceList;

	mainCat->getResources(resourceList,true);
	
	gMessageFactory->StartMessage();       
	gMessageFactory->addUint32(opResourceListForSurveyMessage);   
	
	gMessageFactory->addUint32(resourceList.size());

	ResourceList::iterator resourceIt = resourceList.begin();

	while(resourceIt != resourceList.end())
	{
		Resource* tmpResource = (*resourceIt);

		gMessageFactory->addString(tmpResource->getName());  
		gMessageFactory->addUint64(tmpResource->getId());
		gMessageFactory->addString((tmpResource->getType())->getDescriptor());	

		++resourceIt;
	}

	gMessageFactory->addString(mainCat->getName());
	gMessageFactory->addUint64(mId);
    
	message = gMessageFactory->EndMessage();

	(player->getClient())->SendChannelA(message, player->getAccountId(),CR_Client,4);
}

//=============================================================================
//
// TODO: verify with skillmod
//
void SurveyTool::handleUIEvent(uint32 action,int32 element,string inputStr,UIWindow* window)
{
	if(!action && element != -1)
	{
		uint32	range  = (element + 1) * 64;
		uint32	points = 3;

		switch(element)
		{
			case 1: case 2: points = 4; break;
			case 3: case 4: points = 5; break;
		}

		setInternalAttribute("survey_range",boost::lexical_cast<std::string>(range));
		setInternalAttribute("survey_points",boost::lexical_cast<std::string>(points));

		gWorldManager->getDatabase()->ExecuteSqlAsync(NULL,NULL,"UPDATE item_attributes SET value=%u WHERE item_id=%lld AND attribute_id=6",range,mId);
		gWorldManager->getDatabase()->ExecuteSqlAsync(NULL,NULL,"UPDATE item_attributes SET value=%u WHERE item_id=%lld AND attribute_id=7",points,mId);

		
	}

	PlayerObject*			playerObject	= window->getOwner();
	
	if(mSampleAfterSet)
	{
		StartUsing(playerObject);
		mSampleAfterSet = false;
	}
}

//=============================================================================

void SurveyTool::_createRangeMenu(PlayerObject* playerObject, bool sample)
{
	int32 surveyMod = playerObject->getSkillModValue(SMod_surveying);

	BStringVector availableRanges;

	availableRanges.push_back("64m x 3pts");

	if(surveyMod >= 35)
		availableRanges.push_back("128m x 4pts");

	if(surveyMod >= 55)
		availableRanges.push_back("192m x 4pts");

	if(surveyMod >= 75)
		availableRanges.push_back("256m x 5pts");

	if(surveyMod >= 100)
		availableRanges.push_back("320m x 5pts");

	if(!sample)
		mSampleAfterSet = false;	
	else
		mSampleAfterSet = true;

	gUIManager->createNewListBox(this,"handleSetRange","Survey Device","@survey:select_range",availableRanges,playerObject);
}

//=============================================================================

