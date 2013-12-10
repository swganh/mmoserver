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
#include "Heightmap.h"
#include "MessageLib/MessageLib.h"

#include "DatabaseManager/Database.h"
#include "NetworkManager/MessageFactory.h"
#include "NetworkManager/MessageOpcodes.h"

#include <boost/lexical_cast.hpp>


//=============================================================================

SurveyTool::SurveyTool() : Item()
{

}

//=============================================================================

SurveyTool::~SurveyTool()
{
}

void SurveyTool::prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount)
{
    if (creatureObject->isDead() || creatureObject->isIncapacitated())
    {
        mRadialMenu	= RadialMenuPtr(new RadialMenu());
        mRadialMenu->addItem(1,0,radId_examine,radAction_ObjCallback);
    }
    else
    {
        mRadialMenu	= RadialMenuPtr(new RadialMenu());

        mRadialMenu->addItem(1,0,radId_itemUse,radAction_ObjCallback);
        mRadialMenu->addItem(2,0,radId_examine,radAction_ObjCallback);
        mRadialMenu->addItem(3,0,radId_itemDestroy,radAction_Default);
        mRadialMenu->addItem(4,0,radId_serverItemOptions,radAction_ObjCallback,"@sui:tool_options");
        mRadialMenu->addItem(5,4,radId_serverSurveyToolRange,radAction_ObjCallback,"@sui:survey_range");
    }
}

//=============================================================================

void SurveyTool::handleObjectMenuSelect(uint8 messageType,Object* srcObject)
{
    PlayerObject* playerObject = dynamic_cast<PlayerObject*>(srcObject);

    if( !(playerObject->isConnected()) || playerObject->isDead())
        return;

    // bring up the surve ui
    switch(messageType)
    {
    case radId_itemUse:
    {
        //We only need to check this when using the tool's functions!

        if(playerObject->getPerformingState() != PlayerPerformance_None || playerObject->isDead())
        {
            gMessageLib->SendSystemMessage(::common::OutOfBand("error_message", "survey_cant"), playerObject);
            return;
        }

        // verify we are able to use this
        if(!(playerObject->verifyAbility(opOCsurvey)))
        {
            gMessageLib->SendSystemMessage(::common::OutOfBand("error_message", "insufficient_skill"), playerObject);
            return;
        }

        if(playerObject->getParentId())
        {
            gMessageLib->SendSystemMessage(::common::OutOfBand("error_message", "survey_in_structure"), playerObject);
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
            gMessageLib->SendSystemMessage(::common::OutOfBand("error_message", "insufficient_skill"), playerObject);
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
void SurveyTool::handleUIEvent(uint32 action,int32 element,BString inputStr,UIWindow* window)
{
    if(!action && element != -1)
    {
        uint32	range  = (element + 1) * 64;
        uint32	points = 3;

        switch(element)
        {
        case 1:
        case 2:
            points = 4;
            break;
        case 3:
        case 4:
            points = 5;
            break;
        }

        setInternalAttribute("survey_range",boost::lexical_cast<std::string>(range));
        setInternalAttribute("survey_points",boost::lexical_cast<std::string>(points));

        gWorldManager->getDatabase()->executeSqlAsync(NULL,NULL,"UPDATE %s.item_attributes SET value=%u WHERE item_id=%" PRIu64 " AND attribute_id=6",gWorldManager->getDatabase()->galaxy(),range,mId);
        
        gWorldManager->getDatabase()->executeSqlAsync(NULL,NULL,"UPDATE %s.item_attributes SET value=%u WHERE item_id=%" PRIu64 " AND attribute_id=7",gWorldManager->getDatabase()->galaxy(),points,mId);
        
    }
    else
        // make the player set the range before they can use the tool
        return;

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

