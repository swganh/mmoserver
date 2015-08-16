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

#include "ZoneServer/ArtisanManager.h"

#include <algorithm>

#include "Utils/rand.h"
#include "Utils/clock.h"

#include "Common/atMacroString.h"
#include "Common/EventDispatcher.h"
#include "Common/Event.h"

#include "DatabaseManager/Database.h"
#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/DatabaseResult.h"

#include "NetworkManager/Message.h"
#include "NetworkManager/MessageFactory.h"

#include "MessageLib/MessageLib.h"

#include "ZoneServer/ArtisanHeightmapAsyncContainer.h"
#include "ZoneServer/Buff.h"
#include "ZoneServer/ContainerManager.h"
#include "ZoneServer/CurrentResource.h"
#include "ZoneServer/Datapad.h"
#include "ZoneServer/Heightmap.h"
#include "ZoneServer/Inventory.h"
#include "ZoneServer/Item.h"
#include "ZoneServer/MissionManager.h"
#include "ZoneServer/ObjectController.h"
#include "ZoneServer/ObjectControllerOpcodes.h"
#include "ZoneServer/ObjectControllerCommandMap.h"
#include "ZoneServer/ObjectFactory.h"
#include "ZoneServer/PlayerObject.h"
#include "ZoneServer/ResourceContainer.h"
#include "ZoneServer/ResourceManager.h"
#include "ZoneServer/ResourceType.h"
#include "ZoneServer/SampleEvent.h"
#include "ZoneServer/SurveyEvent.h"
#include "ZoneServer/SurveyTool.h"
#include "ZoneServer/StateManager.h"
#include "ZoneServer/UIManager.h"
#include "ZoneServer/WaypointObject.h"
#include "ZoneServer/WorldConfig.h"
#include "ZoneServer/WorldManager.h"

using std::stringstream;
using common::SimpleEvent;
using common::EventType;

ArtisanManager::ArtisanManager(): mSurveyMindCost(0),mSampleActionCost(0) {}
ArtisanManager::~ArtisanManager() {}

//======================================================================================================================
//
// request survey
//

bool ArtisanManager::handleRequestSurvey(Object* playerObject,Object* target,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    PlayerObject*		player = dynamic_cast<PlayerObject*>(playerObject);
    std::shared_ptr<SimpleEvent> start_survey_event = nullptr;

    if(cmdProperties)
        mSurveyMindCost = cmdProperties->mMindCost;

    // don't allow survey in buildings
    if(player->getParentId())
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("error_message", "survey_in_structure"), player);
        return false;
    }
    if(player->getPerformingState() != PlayerPerformance_None)
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("error_message", "wrong_state"), player);
        return false;
    }
    if(player->getSurveyState())
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("error_message", "survey_cant"), player);
        return false;
    }
    if(player->getSamplingState())
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("error_message", "survey_sample"), player);
        return false;
    }
	// checks if we are in combat, dead or incapacitated
	if (player->states.checkState(CreatureState_Combat) || player->states.checkPosture(CreaturePosture_Dead) || player->states.checkLocomotion(CreatureLocomotion_Incapacitated))
	{
		gMessageLib->SendSystemMessage(::common::OutOfBand("error_message", "wrong_state"), player);
		return false;
	}

    SurveyTool*			tool			= dynamic_cast<SurveyTool*>(target);
    CurrentResource*	resource		= NULL;
    BString				resourceName;

    message->getStringUnicode16(resourceName);
    resourceName.convert(BSTRType_ANSI);

    resource = reinterpret_cast<CurrentResource*>(gResourceManager->getResourceByNameCRC(resourceName.getCrc()));

    if(tool && resource)
    {
        player->setSurveyState(true);

        // play effect
        std::string effect = gWorldManager->getClientEffect(tool->getInternalAttribute<uint32>("survey_effect"));
        gMessageLib->sendPlayClientEffectLocMessage(effect,player->mPosition,player);

        gContainerManager->sendToRegisteredWatchers(player, [effect, player] (PlayerObject* const recipient) {
            gMessageLib->sendPlayClientEffectLocMessage(effect, player->mPosition, recipient);
        });

        uint32 mindCost = mSurveyMindCost;
        Ham* hamz = player->getHam();
        //are we able to sample in the first place ??
        if(!hamz->checkMainPools(0,0,mindCost))
        {

            int32 myMind = hamz->mAction.getCurrentHitPoints();

            //return message for sampling cancel based on HAM
            if(myMind < (int32)mindCost)
            {
                gMessageLib->SendSystemMessage(::common::OutOfBand("error_message", "sample_mind"), player);
            }

            //message for stop sampling
            gMessageLib->SendSystemMessage(::common::OutOfBand("survey", "sample_cancel"), player);

            player->getSampleData()->mPendingSurvey = false;

            hamz->updateRegenRates();
            player->updateMovementProperties();
            return false;
        }

        hamz->performSpecialAction(0,0,(float)mindCost,HamProperty_CurrentHitpoints);
        // send system message
        resourceName.convert(BSTRType_Unicode16);
        gMessageLib->SendSystemMessage(::common::OutOfBand("survey", "start_survey", L"", L"", resourceName.getUnicode16()), player);

        // schedule execution
        start_survey_event = std::make_shared<SimpleEvent>(EventType("start_survey"),0, 5000,
                             std::bind(&ArtisanManager::surveyEvent, this, player, resource, tool));

    }
    else
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("ui","survey_nothingfound"));
        return false;
    }
    // notify any listeners
    if (start_survey_event)
        gEventDispatcher.Notify(start_survey_event);
    return true;
}

//======================================================================================================================
//
// request sample
//

bool ArtisanManager::handleRequestCoreSample(Object* player,Object* target, Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    PlayerObject*		playerObject = dynamic_cast<PlayerObject*>(player);
    if(cmdProperties)
        // unfortunately it's not in this opcode
        // hardcode for now
        //mSampleActionCost = cmdProperties->mActionCost;
        mSampleActionCost = 150;

    if(playerObject->getPerformingState() != PlayerPerformance_None || playerObject->checkIfMounted() || playerObject->isDead() || playerObject->states.checkState(CreatureState_Combat))
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("error_message", "wrong_state"), playerObject);
        return false;
    }
    // can't sample while surveying
    if(playerObject->getSurveyState())
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("survey", "sample_survey"), playerObject);
        return false;
    }
    // don't allow sampling in buildings
    if(playerObject->getParentId())
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("error_message", "survey_in_structure"), playerObject);
        return false;
    }

    uint64 localTime = Anh_Utils::Clock::getSingleton()->getLocalTime();
    // don't allow more than one sample at a time
    if(playerObject->getSamplingState())
    {
        playerObject->getSampleData()->mPendingSample = false;
        playerObject->setNextSampleTime(localTime + 18000);
        gMessageLib->SendSystemMessage(::common::OutOfBand("survey", "tool_recharge_time", 0, 0, 0, (int32)(playerObject->getNextSampleTime() - localTime) / 1000), playerObject);
        return false;
    }

    if(!playerObject->getNextSampleTime() || (int32)(playerObject->getNextSampleTime() - localTime) <= 0)
    {
        playerObject->getSampleData()->mPendingSample = false;
        playerObject->setNextSampleTime(localTime + 18000);
    }
    else
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("survey", "tool_recharge_time", 0, 0, 0, (int32)(playerObject->getNextSampleTime() - localTime) / 1000), playerObject);
        return false;
    }

    SurveyTool*			tool		= dynamic_cast<SurveyTool*>(target);
    CurrentResource*	resource	= NULL;

    BString resourceName;

    message->getStringUnicode16(resourceName);
    resourceName.convert(BSTRType_ANSI);

    resource = reinterpret_cast<CurrentResource*>(gResourceManager->getResourceByNameCRC(resourceName.getCrc()));

    if(resource == NULL || tool == NULL)
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("ui","survey_noresource"), playerObject);
        return false;
    }

    if((resource->getType()->getCategoryId() == 903)||(resource->getType()->getCategoryId() == 904))
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("survey", "must_have_harvester"), playerObject);
        return false;
    }
    playerObject->setSamplingState(true);
    ArtisanHeightmapAsyncContainer* container = new ArtisanHeightmapAsyncContainer(this, HeightmapCallback_ArtisanSurvey);
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
    std::shared_ptr<SimpleEvent> start_sample_event = nullptr;

    HeightResultMap* mapping = container->getResults();
    HeightResultMap::iterator it = mapping->begin();
    if(it != mapping->end() && it->second != NULL)
    {
        if(it->second->hasWater)
        {
            gMessageLib->SendSystemMessage(::common::OutOfBand("error_message", "survey_swimming"), container->playerObject);
            return;
        }

        // put us into sampling mode
        container->playerObject->setSamplingState(true);

        container->resourceName.convert(BSTRType_Unicode16);
        gMessageLib->SendSystemMessage(::common::OutOfBand("survey", "start_sampling", L"", L"", container->resourceName.getUnicode16()), container->playerObject);

        // change posture
        gStateManager.setCurrentPostureState(container->playerObject, CreaturePosture_Crouched);
        // play animation
        gWorldManager->getClientEffect(container->tool->getInternalAttribute<uint32>("sample_effect"));
        // schedule execution
        //container->playerObject->getController()->addEvent(new SampleEvent(container->playerObject,container->tool,container->resource),2000);
        start_sample_event = std::make_shared<SimpleEvent>(EventType("start_sample"), 0, 2000,
                             std::bind(&ArtisanManager::sampleEvent,this, container->playerObject, container->resource, container->tool));
    }
    // notify any listeners
    if(start_sample_event)
        gEventDispatcher.Notify(start_sample_event);

}

//======================================================================================================================
//
// sent along with requestsurvey, when doing /survey command, contains nothing, not sure what its used for
//
bool ArtisanManager::handleSurvey(Object* player,Object* target,Message* message,ObjectControllerCmdProperties* cmdProperties)
{

    return true;
}

//======================================================================================================================
//
// sent along with requestcoresample, when doing /sample command, contains nothing, not sure what its used for
// I have a feeling this is 'dosample' and should probably be used for every sample after the first, but I'll need to check
// into this...
//
bool ArtisanManager::handleSample(Object* player,Object* target,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    return true;
}

//======================================================================================================================
void ArtisanManager::sampleEvent(PlayerObject* player, CurrentResource* resource, SurveyTool* tool)
{
    if (!player->isConnected())
        return;

    Ham*				ham			= player->getHam();

    //====================================================
    //check whether we are able to sample in the first place
    //
    if (stopSampling(player, resource, tool))
        return;

    std::string				effect			= gWorldManager->getClientEffect(tool->getInternalAttribute<uint32>("sample_effect"));
    float					ratio			= (resource->getDistribution((int)player->mPosition.x + 8192,(int)player->mPosition.z + 8192));
    int32					surveyMod		= player->getSkillModValue(SMod_surveying);
    uint32					sampleAmount	= 0;
    BString					resName			= resource->getName().getAnsi();
    uint32					resType			= resource->getType()->getCategoryId();
    uint16					resPE			= resource->getAttribute(ResAttr_PE);
    bool					successSample	= false;
    bool					resAvailable	= true;
    resName.convert(BSTRType_Unicode16);

    if (getRadioactiveSample(player, resource, tool))
    {
        // now we're done sampling for this go around, finish up or set a flag so we know to finish up
        if(stopSampling(player, resource, tool))
        {
            return;
        }
    }

    // calculate sample size if concentration >0 and surveyMod exists for player

    //original implementation sampleAmount = (uint32)floor((double)((15.0f * ratio) * (surveyMod / 100.0f)));
    float ratio_100			= ratio*100;
    float successChance	    = static_cast<float>((surveyMod/4)+45);
//	float maxSuccessChance  = 70;   // == 70, but showing the formula so it is obvious where it comes from
    float failureChance		= 100-successChance;
    float dieRoll			= static_cast<float>((gRandom->getRand()%100)+1);        // random value from 1 to 100
    float adjSkill		    = surveyMod/successChance;  //100 skill == 1;  85 skill = 0.9464; etc
    float minSample			= ratio_100/20*adjSkill;
    float maxSample		    = ratio_100/10*adjSkill;
    float minConcentration  = static_cast<float>(-1.12*successChance+88);   // attempting to simplify this to "-1*sC+90" or whatever will break it
    uint32 actionCost		= mSampleActionCost;

    if(player->getSampleData()->mSampleNodeFlag)
    {
        if(setupForNodeSampleRecovery(player))
        {
            ratio	= 100.0;
            dieRoll = 200;
        }
        else
        {
            stopSampling(player, resource, tool);
            return;
        }
    }

    // let's make sure their ratio is good enough to sample this
    if(ratio <= 0.0f)
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("survey", "density_below_threshold", L"", L"", resName.getUnicode16()), player);
        player->getSampleData()->mPendingSample = false;
        return;

    }

    //If previous call triggered a sample event, set the roll to ensure critical success
    if(player->getSampleData()->mSampleEventFlag)
    {
        player->getSampleData()->mSampleEventFlag = false;
        //set so a critical success happens -- A critical failure can also happen.  In either case, this took a significant amount of action (300 points???)
        // This appears to have been what the commented out gambling code below was supposed to do.
        dieRoll = 100;
    }

    if(ratio_100 >= minConcentration)
    {
        //inside calculation section
        if(dieRoll <= failureChance)
        {
            // FAILED ATTEMPT
            sampleAmount = 0;
            successSample = false;
            gMessageLib->SendSystemMessage(::common::OutOfBand("survey", "sample_failed", L"", L"", resName.getUnicode16()), player);
        }
        else if((dieRoll > 96)&&(dieRoll < 98))
        {
            //Sample event
            if (setupSampleEvent(player, resource, tool))
                return;
        }
        else
        {
            successSample = true;
            if(dieRoll == 200)
            {
                sampleAmount = (static_cast<uint32>(3*maxSample));
                sampleAmount = std::max<uint>(sampleAmount,static_cast<uint>(1));
                gMessageLib->SendSystemMessage(::common::OutOfBand("survey", "node_recovery"), player);
                gMessageLib->SendSystemMessage(::common::OutOfBand("survey", "sample_located", L"", L"", resName.getUnicode16(), sampleAmount), player);
                player->getSampleData()->mSampleEventFlag = false;
                player->getSampleData()->mSampleNodeFlag = false;
            }
            else
                // was set to == 100
                if(dieRoll >= 99)
                {
                    if(player->getSampleData()->mSampleGambleFlag)
                    {
                        gMessageLib->SendSystemMessage(::common::OutOfBand("survey", "gamble_success"), player);
                        sampleAmount = (static_cast<uint32>(3*maxSample));
                        sampleAmount = std::max<uint>(sampleAmount, static_cast<uint>(1));
                        actionCost = 300; //300 action
                        gMessageLib->SendSystemMessage(::common::OutOfBand("survey", "sample_located", L"", L"", resName.getUnicode16(), sampleAmount), player);
                        player->getSampleData()->mSampleGambleFlag = false;
                        player->getSampleData()->mSampleEventFlag = false;
                    }
                    else
                    {
                        //CRITICAL SUCCESS
                        sampleAmount = (static_cast<uint32>(2*maxSample));
                        sampleAmount = std::max<uint>(sampleAmount, static_cast<uint>(1));
                        gMessageLib->SendSystemMessage(::common::OutOfBand("survey", "critical_success", L"", L"", resName.getUnicode16()), player);
                        gMessageLib->SendSystemMessage(::common::OutOfBand("survey", "sample_located", L"", L"", resName.getUnicode16(), sampleAmount), player);

                    }
                }
                else
                {
                    //NORMAL SUCCESS
                    sampleAmount = (static_cast<uint32>(floor(static_cast<float>((maxSample-minSample)*(dieRoll-failureChance)/(90-failureChance)+minSample))));         // floor == round down, so 9.9 == 9
                    sampleAmount = std::max<uint>(sampleAmount, static_cast<uint>(1));
                    gMessageLib->SendSystemMessage(::common::OutOfBand("survey", "sample_located", L"", L"", resName.getUnicode16(), sampleAmount), player);
                }
        }
    }
    else
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("survey", "density_below_threshold", L"", L"", resName.getUnicode16()), player);
        player->setSamplingState(false);
        return;
    }

    // show the effects always
    gContainerManager->sendToRegisteredWatchers(player, [effect, player] (PlayerObject* const recipient) {
        gMessageLib->sendPlayClientEffectLocMessage(effect, player->mPosition,recipient);
    });

    if (sampleAmount > 0 && successSample)
    {
        finishSampling(player, resource, tool, sampleAmount);
    }

    // check our ham and keep sampling
    if(!stopSampling(player, resource, tool))
    {
        player->getSampleData()->mNextSampleTime = Anh_Utils::Clock::getSingleton()->getLocalTime() + 18000;
        std::shared_ptr<SimpleEvent> start_sample_event = std::make_shared<SimpleEvent>(EventType("start_sample"),0, 18000,
                std::bind(&ArtisanManager::sampleEvent,this, player, resource, tool));
        gEventDispatcher.Notify(start_sample_event);
    }

    player->getHam()->performSpecialAction(0, (float)actionCost, 0, HamProperty_CurrentHitpoints);
}

bool	ArtisanManager::setupSampleEvent(PlayerObject* player, CurrentResource* resource, SurveyTool* tool)
{
    //EVENT WINDOW CASE
    int32 eventRoll = int(gRandom->getRand()%2)+1;
    //eventRoll = 2;
    //do event 1 or event 2 based on roll
    if(eventRoll == 1)
    {
        // setup gamble event
        //GAMBLE Event
        WindowAsyncContainerCommand* asyncContainer = new  WindowAsyncContainerCommand(Window_Query_Radioactive_Sample);
        asyncContainer->PlayerId		= player->getId();
        asyncContainer->ToolId			= tool->getId();
        asyncContainer->CurrentResource	= resource;

        BStringVector items;
        items.push_back("Ignore the concentration and continue working.");
        items.push_back("Attempt to recover the resources. (300 Action)");
        gUIManager->createNewListBox(this,"gambleSample","@survey:gnode_t","@survey:gnode_d",items,player,SUI_Window_SmplGamble_ListBox,SUI_LB_OKCANCEL,0,0,asyncContainer);

        player->getSampleData()->mPendingSample = false;
        return true;
    }
    else
    {
        WindowAsyncContainerCommand* asyncContainer = new  WindowAsyncContainerCommand(Window_Query_Radioactive_Sample);
        asyncContainer->PlayerId		= player->getId();
        asyncContainer->ToolId			= tool->getId();
        asyncContainer->CurrentResource	= resource;

        //WAYP CONCENTRATION
        BStringVector items;
        items.push_back("Ignore the concentration and continue working.");
        items.push_back("Focus the device on the concentration");
        gUIManager->createNewListBox(this,"waypNodeSample","@survey:cnode_t","@survey:cnode_d",items,player,SUI_Window_SmplWaypNode_ListBox,SUI_LB_OKCANCEL,0,0,asyncContainer);

        //Pause sampling
        player->getSampleData()->mPendingSample = false;
        player->getSampleData()->mSampleEventFlag = true;

        return true;
    }
    return false;
}
bool	ArtisanManager::setupForNodeSampleRecovery(PlayerObject* player)
{
    glm::vec2 playerPos;
    playerPos.x = player->mPosition.x;
    playerPos.y = player->mPosition.z;
    glm::vec2 nodePos;
    nodePos.x = player->getSampleData()->Position.x;
    nodePos.y = player->getSampleData()->Position.z;

    // need to be in a 5m radius of the node
    if(glm::distance(playerPos,nodePos) <= 5.0)
    {
        player->getSampleData()->mSampleNodeRecovery = true;
        player->getSampleData()->mSampleEventFlag = false;
    }
    else
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("survey", "node_not_close"), player);
        player->getSampleData()->mPendingSample		= false;
        player->getSampleData()->mSampleNodeFlag	= false;
        player->getSampleData()->mSampleNodeRecovery= false;
        player->getSampleData()->mSampleEventFlag = false;
        return false;
    }

    return true;
}
bool	ArtisanManager::getRadioactiveSample(PlayerObject* player, CurrentResource* resource, SurveyTool* tool)
{
    uint32					resType			= resource->getType()->getCategoryId();
    uint16					resPE			= resource->getAttribute(ResAttr_PE);

    //these are the radioactive types
    if(resType == 477 || resType == 476 /* || resType == 475*/)
    {
        //did we already warn the player on the wounds ???
        if(!player->getSampleData()->mPassRadioactive)
        {
            //UI Integration

            WindowAsyncContainerCommand* asyncContainer = new  WindowAsyncContainerCommand(Window_Query_Radioactive_Sample);
            asyncContainer->PlayerId		= player->getId();
            asyncContainer->ToolId			= tool->getId();
            asyncContainer->CurrentResource	= resource;

            gUIManager->createNewMessageBox(this,"radioactiveSample","@survey:radioactive_sample_t","@survey:radioactive_sample_d",player,SUI_Window_SmplRadioactive_MsgBox, SUI_MB_YESNO,asyncContainer);
            //Pause Sampling
            player->getSampleData()->mPendingSample = false;
            return true;
        }
        Ham* hamz = player->getHam();
        uint32 playerBF = hamz->getBattleFatigue();

        uint32 woundDmg = 50*(1 + (playerBF/100)) + (50*(1 + (resPE/1000)));
        uint32 bfDmg    = static_cast<uint32>(0.075*resPE);
        uint32 hamReduc = 100*(2+ (resPE/1000));

        if(resPE >= 500)
        {
            //wound and BF dmg
            hamz->updateBattleFatigue(bfDmg);
            hamz->updatePropertyValue(HamBar_Health,HamProperty_Wounds, woundDmg);
            hamz->updatePropertyValue(HamBar_Action,HamProperty_Wounds, woundDmg);
            hamz->updatePropertyValue(HamBar_Mind,HamProperty_Wounds, woundDmg);
        }

        //this should be a timed debuff per instance -- Do not cause wounds unless potential energy >= 500
        // each time a radioactive is sampled, there is a 5 minute debuff
        // this currently doesn't work properly as when the debuff wears off, the buff class doesn't ensure
        // we don't have more ham than we should.

        BuffAttribute* healthdebuffAttribute = new BuffAttribute(attr_health, -(int)hamReduc,0,hamReduc);
        Buff* healthdebuff = Buff::SimpleBuff(player, player, 300000,0, gWorldManager->GetCurrentGlobalTick());
        healthdebuff->AddAttribute(healthdebuffAttribute);
        player->AddBuff(healthdebuff,true);

        healthdebuffAttribute = new BuffAttribute(attr_action, -(int)hamReduc,0,hamReduc);
        healthdebuff = Buff::SimpleBuff(player, player, 300000, 0, gWorldManager->GetCurrentGlobalTick());
        healthdebuff->AddAttribute(healthdebuffAttribute);
        player->AddBuff(healthdebuff,true);
    }
    else
        return false;

    return true;
}
void	ArtisanManager::finishSampling(PlayerObject* player, CurrentResource* resource, SurveyTool* tool, uint32 sampleAmount)
{
    bool foundSameType = false;

    //grants 20xp -> 40xp inclusive -- Feature suggestion:  Grant less XP for smaller samples, more xp for greater samples.  IE:  20 + X*sampleSize
    gSkillManager->addExperience(XpType_resource_harvesting_inorganic,(int32)((gRandom->getRand()%20) + 20),player);

    // see if we can add it to an existing container

    Inventory*	inventory	= player->getInventory();
    ObjectIDList*			invObjects	= inventory->getObjects();
    ObjectIDList::iterator	listIt		= invObjects->begin();

    while(listIt != invObjects->end())
    {
        // we are looking for resource containers
        ResourceContainer* resCont = dynamic_cast<ResourceContainer*>(gWorldManager->getObjectById((*listIt)));
        if(resCont)
        {
            uint32 targetAmount	= resCont->getAmount();
            uint32 maxAmount	= resCont->getMaxAmount();
            uint32 newAmount;

            if(resCont->getResourceId() == resource->getId() && targetAmount < maxAmount)
            {
                foundSameType = true;

                if((newAmount = targetAmount + sampleAmount) <= maxAmount)
                {
                    // update target container
                    resCont->setAmount(newAmount);

                    gMessageLib->sendResourceContainerUpdateAmount(resCont,player);

                    stringstream query_stream;
                    query_stream << "UPDATE "<<gWorldManager->getDatabase()->galaxy()<<".resource_containers SET amount=" << newAmount
                                 << " WHERE id=" << resCont->getId();
                    gWorldManager->getDatabase()->executeAsyncSql(query_stream);
                }
                // target container full, put in what fits, create a new one
                else if(newAmount > maxAmount)
                {
                    uint32 selectedNewAmount = newAmount - maxAmount;

                    resCont->setAmount(maxAmount);

                    gMessageLib->sendResourceContainerUpdateAmount(resCont,player);
                    stringstream query_stream;
                    query_stream << "UPDATE "<<gWorldManager->getDatabase()->galaxy()<<".resource_containers SET amount=" << newAmount
                                 << " WHERE id=" << resCont->getId();
                    gWorldManager->getDatabase()->executeAsyncSql(query_stream);
                    gObjectFactory->requestNewResourceContainer(inventory,resource->getId(),inventory->getId(),99,selectedNewAmount);
                }

                break;
            }
        }

        ++listIt;
    }
    // or need to create a new one
    if(!foundSameType)
    {
        gObjectFactory->requestNewResourceContainer(inventory,resource->getId(),inventory->getId(),99,sampleAmount);
    }

    // deplete resource
    gResourceManager->setResourceDepletion(resource, sampleAmount);
    return;
}
bool	ArtisanManager::stopSampling(PlayerObject* player, CurrentResource* resource, SurveyTool* tool)
{
    Ham* ham = player->getHam();
    bool stop = false;

    if(!resource || !tool || !player->isConnected() || !player->getSamplingState()||player->getSurveyState())
    {
        stop = true;
    }
    // you can't take sample while under attack!
    if(player->states.checkState(CreatureState_Combat))
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("survey", "sample_cancel_attack"), player);
        return false;
    }
    // you can't take samples while standing
    if (player->states.checkPosture(CreaturePosture_Upright))
    {
        stop = true;
    }
    Inventory* inventory = dynamic_cast<Inventory*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
    if(!inventory)
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("error_message", "sample_gone"), player);
        stop = true;
    }

    tool		= dynamic_cast<SurveyTool*>(inventory->getObjectById(tool->getId()));
    if(!tool)
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("error_message", "sample_gone"), player);
        stop = true;
    }

    uint32 actionCost = mSampleActionCost;

    if(!ham->checkMainPools(0,actionCost,0))
    {
        //return message for sampling cancel based on HAM
        if(ham->mAction.getCurrentHitPoints() < (int32)actionCost)
        {
            gMessageLib->SendSystemMessage(::common::OutOfBand("error_message", "sample_mind"), player);
            stop = true;
        }
    }

    if (stop)
    {
        player->setSamplingState(false);
        gStateManager.setCurrentPostureState(player, CreaturePosture_Upright);
    }
    return stop;
}

////=============================================================================
////
//// survey event
////
//
void ArtisanManager::surveyEvent(PlayerObject* player, CurrentResource* resource, SurveyTool* tool)
{
    if(tool && resource && player->isConnected())
    {
        Datapad* datapad					= player->getDataPad();
        ResourceLocation	highestDist		= gMessageLib->sendSurveyMessage(tool->getInternalAttribute<uint16>("survey_range"),tool->getInternalAttribute<uint16>("survey_points"),resource,player);

        // this is 0, if resource is not located
        if(highestDist.position.y == 5.0)
        {
            WaypointObject*	waypoint = datapad->getWaypointByName("Resource Survey");

            // remove the old one
            if(waypoint)
            {
                datapad->updateWaypoint(waypoint->getId(), waypoint->getName(), glm::vec3(highestDist.position.x,0.0f,highestDist.position.z),
                                        static_cast<uint16>(gWorldManager->getZoneId()), player->getId(), WAYPOINT_ACTIVE);
                gMessageLib->sendUpdateWaypoint(waypoint,ObjectUpdateChange,player);
            }
            else
            {
                // create a new one
                if(datapad->getCapacity())
                {
                    gMessageLib->SendSystemMessage(::common::OutOfBand("survey", "survey_waypoint"), player);
                    //gMessageLib->sendSystemMessage(this,L"","survey","survey_waypoint");
                }
                //the datapad automatically checks if there is room and gives the relevant error message
                datapad->requestNewWaypoint("Resource Survey", glm::vec3(highestDist.position.x,0.0f,highestDist.position.z),static_cast<uint16>(gWorldManager->getZoneId()),Waypoint_blue);
            }

            gMissionManager->checkSurveyMission(player,resource,highestDist);
        }
    }

    player->getSampleData()->mPendingSurvey = false;
}
//=============================================================================
//
// handles any UIWindow callbacks for sampling events
//

void ArtisanManager::handleUIEvent(uint32 action,int32 element,BString inputStr,UIWindow* window)
{
    PlayerObject* player = window->getOwner();
    std::shared_ptr<SimpleEvent> sample_UI_event = nullptr;
    if(!player)
    {
        return;
    }

    Inventory* inventory = dynamic_cast<Inventory*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
    if(!inventory)
    {
        return;
    }

    WindowAsyncContainerCommand* asyncContainer = (WindowAsyncContainerCommand*)window->getAsyncContainer();
    if(!asyncContainer)
        return;

    Ham* ham = player->getHam();

    switch(window->getWindowType())
    {
        // Sampling Radioactive Msg Box
    case SUI_Window_SmplRadioactive_MsgBox:
    {
        //we stopped the sampling
        if(action == 1)
        {
            player->getSampleData()->mPassRadioactive = false;
            player->getSampleData()->mPendingSample = false;
            gStateManager.setCurrentPostureState(player, CreaturePosture_Upright);
            return;
        }
        else
        {
            player->getSampleData()->mPassRadioactive = true;
            player->getSampleData()->mPendingSample = true;

            if(ham->checkMainPools(0,mSampleActionCost*2,0))
            {

                SurveyTool*			tool					= dynamic_cast<SurveyTool*>(inventory->getObjectById(asyncContainer->ToolId));
                CurrentResource*	resource				= (CurrentResource*)asyncContainer->CurrentResource;
                player->getSampleData()->mNextSampleTime	= Anh_Utils::Clock::getSingleton()->getLocalTime() + 4000;

                sample_UI_event = std::make_shared<SimpleEvent>(EventType("sample_radioactive"),0, 4000,
                                  std::bind(&ArtisanManager::sampleEvent,this, player, resource, tool));

            }
            else
            {
                gMessageLib->SendSystemMessage(::common::OutOfBand("survey", "gamble_no_action"), player);
                return;
            }
        }
    }
    break;

    case SUI_Window_SmplGamble_ListBox:
    {
        //action == 1 is cancel
        if(action == 1)
        {
            player->getSampleData()->mPendingSample = false;
            player->getSampleData()->mSampleGambleFlag = false;
            gStateManager.setCurrentPostureState(player, CreaturePosture_Upright);
            player->updateMovementProperties();
            gMessageLib->sendUpdateMovementProperties(player);
            gMessageLib->sendPostureAndStateUpdate(player);
            gMessageLib->sendSelfPostureUpdate(player);
            return;

        }
        else
        {
            if(element == 0)
            {
                player->getSampleData()->mPendingSample = true;
                player->getSampleData()->mSampleGambleFlag = false;

                SurveyTool*			tool		= dynamic_cast<SurveyTool*>(inventory->getObjectById(asyncContainer->ToolId));
                CurrentResource*	resource	= (CurrentResource*)asyncContainer->CurrentResource;
                player->getSampleData()->mNextSampleTime = Anh_Utils::Clock::getSingleton()->getLocalTime() + 1000;

                sample_UI_event = std::make_shared<SimpleEvent>(EventType("sample_gamble"),0, 1000,
                                  std::bind(&ArtisanManager::sampleEvent,this, player, resource, tool));

            }
            else
            {
                //action costs
                if(!ham->checkMainPools(0,mSampleActionCost*2,0))
                {
                    gStateManager.setCurrentPostureState(player, CreaturePosture_Upright);
                    player->getSampleData()->mSampleEventFlag = false;
                    player->getSampleData()->mSampleGambleFlag = false;
                    gMessageLib->SendSystemMessage(::common::OutOfBand("survey", "gamble_no_action"), player);
                    return;
                }
                player->getSampleData()->mPendingSample = true;

                //determine whether gamble is good or not
                int32 gambleRoll = int(gRandom->getRand()%2) + 1;

                if(gambleRoll == 1)
                {
                    player->getSampleData()->mSampleEventFlag = true;
                    player->getSampleData()->mSampleGambleFlag = true;
                }
                else
                {
                    player->getSampleData()->mSampleEventFlag = false;
                    player->getSampleData()->mSampleGambleFlag = false;
                    gMessageLib->SendSystemMessage(::common::OutOfBand("survey", "gamble_fail"), player);
                }

                SurveyTool*			tool		= dynamic_cast<SurveyTool*>(inventory->getObjectById(asyncContainer->ToolId));
                CurrentResource*	resource	= (CurrentResource*)asyncContainer->CurrentResource;
                player->getSampleData()->mNextSampleTime = Anh_Utils::Clock::getSingleton()->getLocalTime() + 1000;

                sample_UI_event = std::make_shared<SimpleEvent>(EventType("sample_gamble"),0, 1000,
                                  std::bind(&ArtisanManager::sampleEvent,this, player, resource, tool));

            }
        }
    }
    break;

    case SUI_Window_SmplWaypNode_ListBox:
    {
        if(action == 0)
        {
            //we hit ok and went for the wp
            if(element == 1)
            {
                player->getSampleData()->mPendingSample	= false;
                player->getSampleData()->mSampleNodeFlag = true;

                player->getSampleData()->Position.x = player->mPosition.x +(((gRandom->getRand()%50)+1));
                player->getSampleData()->Position.z = player->mPosition.z +(((gRandom->getRand()%50)+1));
                player->getSampleData()->zone		= gWorldManager->getZoneId();
                player->getSampleData()->resource	= (CurrentResource*)asyncContainer->CurrentResource;


                Datapad* datapad			= player->getDataPad();
                datapad->requestNewWaypoint("Resource Node", player->getSampleData()->Position ,static_cast<uint16>(gWorldManager->getZoneId()),Waypoint_blue);
                gMessageLib->SendSystemMessage(::common::OutOfBand("survey", "node_waypoint"), player);

                gStateManager.setCurrentPostureState(player, CreaturePosture_Upright);
                return;
            }
            //we ignored the node - so continue sampling
            if(element == 0)
            {
                player->getSampleData()->mPendingSample = true;
                player->getSampleData()->mSampleGambleFlag = false;

                SurveyTool*			tool		= dynamic_cast<SurveyTool*>(inventory->getObjectById(asyncContainer->ToolId));
                CurrentResource*	resource	= (CurrentResource*)asyncContainer->CurrentResource;
                player->getSampleData()->mNextSampleTime = Anh_Utils::Clock::getSingleton()->getLocalTime() + 10000;

                sample_UI_event = std::make_shared<SimpleEvent>(EventType("sample_continue"),0, 10000,
                                  std::bind(&ArtisanManager::sampleEvent,this, player, resource, tool));

            }
        }
        else
        {
            player->getSampleData()->mPendingSample = false;
            player->getSampleData()->mSampleNodeFlag = false;
            player->getSampleData()->Position.x = 0;
            player->getSampleData()->Position.z = 0;
            player->getSampleData()->resource	= NULL;
            player->getSampleData()->zone		= 0;

            gStateManager.setCurrentPostureState(player, CreaturePosture_Upright);
            return;
        }
    }
    break;
    }
    //notify the listeners
    if (sample_UI_event)
        gEventDispatcher.Notify(sample_UI_event);
    SAFE_DELETE(asyncContainer);
}