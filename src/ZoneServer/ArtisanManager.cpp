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
#include "Buff.h"
#include "Item.h"
#include "Datapad.h"
#include "Inventory.h"
#include "MissionManager.h"
#include "ObjectFactory.h"
#include "ObjectController.h"
#include "ObjectControllerOpcodes.h"
#include "ObjectControllerCommandMap.h"
#include "PlayerObject.h"
#include "ResourceManager.h"
#include "ResourceContainer.h"
#include "ResourceCollectionManager.h"
#include "ResourceType.h"
#include "SurveyTool.h"
#include "UIManager.h"
#include "Heightmap.h"
#include "WorldConfig.h"
#include "WorldManager.h"
#include "WaypointObject.h"

#include "SurveyEvent.h"
#include "SampleEvent.h"

#include "MessageLib/MessageLib.h"
#include "LogManager/LogManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/DatabaseResult.h"
#include "Common/atMacroString.h"
#include "Common/Message.h"
#include "Common/MessageFactory.h"
#include "ArtisanHeightmapAsyncContainer.h"
#include "Utils/EventHandler.h"
#include "Utils/rand.h"
#include "Utils/clock.h"

bool						ArtisanManager::mInsFlag    = false;
ArtisanManager*				ArtisanManager::mSingleton  = NULL;

ArtisanManager::ArtisanManager() : mObjectFactoryCallback(), mHeightMapCallback()
{
	//register event
	//registerEventFunction(this,&ArtisanManager::onSample);
	//registerEventFunction(this,&ArtisanManager::onSurvey);
};
ArtisanManager::~ArtisanManager()
{
};
//======================================================================================================================
//
// request survey
//

bool ArtisanManager::handleRequestSurvey(Object* player,Object* target,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*		playerObject = dynamic_cast<PlayerObject*>(player);

	// don't allow survey in buildings
	if(playerObject->getParentId())
	{
        gMessageLib->SendSystemMessage(::common::OutOfBand("error_message", "survey_in_structure"), playerObject);
		return false;
	}

	if(playerObject->getPerformingState() != PlayerPerformance_None)
	{
        gMessageLib->SendSystemMessage(::common::OutOfBand("error_message", "wrong_state"), playerObject);
		return false;
	}

	if(playerObject->getSurveyState())
	{
		playerObject->getSampleData()->mPendingSample = false;
        gMessageLib->SendSystemMessage(::common::OutOfBand("survey", "survey_sample"), playerObject);
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
        gMessageLib->SendSystemMessage(::common::OutOfBand("survey", "start_survey", L"", L"", resourceName.getUnicode16()), playerObject);

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

	if(playerObject->getPerformingState() != PlayerPerformance_None || playerObject->checkIfMounted() || playerObject->isDead())
	{
        gMessageLib->SendSystemMessage(::common::OutOfBand("error_message", "wrong_state"), playerObject);
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
		playerObject->setNextSampleTime(localTime + 30000);
        gMessageLib->SendSystemMessage(::common::OutOfBand("survey", "tool_recharge_time", 0, 0, 0, (int32)(playerObject->getNextSampleTime() - localTime) / 1000), playerObject);
		return false;
	}

	if(!playerObject->getNextSampleTime() || (int32)(playerObject->getNextSampleTime() - localTime) <= 0)
	{
		playerObject->getSampleData()->mPendingSample = false;
		playerObject->setNextSampleTime(localTime + 30000);
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
		return false;

	if((resource->getType()->getCategoryId() == 903)||(resource->getType()->getCategoryId() == 904))
	{
        gMessageLib->SendSystemMessage(::common::OutOfBand("survey", "must_have_harvester"), playerObject);
		return false;
	}
	playerObject->setSamplingState(true);
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
                gMessageLib->SendSystemMessage(::common::OutOfBand("error_message", "survey_swimming"), container->playerObject);
				return;
			}

			// put us into sampling mode
			container->playerObject->setSamplingState(true);

			container->resourceName.convert(BSTRType_Unicode16);
            gMessageLib->SendSystemMessage(::common::OutOfBand("survey", "start_sampling", L"", L"", container->resourceName.getUnicode16()), container->playerObject);

			// change posture
			container->playerObject->setCrouched();
			// play animation
			gWorldManager->getClientEffect(container->tool->getInternalAttribute<uint32>("sample_effect"));
			// schedule execution
			container->playerObject->getController()->addEvent(new SampleEvent(container->playerObject,container->tool,container->resource),2000);
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
// I have a feeling this is 'dosample' and should probably be used for every sample after the first, but I'll need to check
// into this...
//
bool ArtisanManager::handleSample(Object* player,Object* target,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	//PlayerObject*		playerObject = (PlayerObject*)mObject;

	//gLogger->hexDump(message->getData(),message->getSize());
	return true;
}

//======================================================================================================================

//=============================================================================
//
// sample event
//

void ArtisanManager::onSample(const SampleEvent* event)
{
	SurveyTool*			tool		= event->getTool();
	CurrentResource*	resource	= event->getResource();
	PlayerObject*		player		= event->getPlayer();

	Ham*				ham			= player->getHam();

	//====================================================
	//check whether we are able to sample in the first place
	//
	if (stopSampling(player, resource, tool))
		return;

	BString					effect			= gWorldManager->getClientEffect(tool->getInternalAttribute<uint32>("sample_effect"));
	bool					foundSameType	= false;
	float					ratio			= (resource->getDistribution((int)player->mPosition.x + 8192,(int)player->mPosition.z + 8192));
	int32					surveyMod		= player->getSkillModValue(SMod_surveying);
	uint32					sampleAmount	= 0;
	ObjectSet::iterator	it					= player->getKnownObjects()->begin();
	BString					resName			= resource->getName().getAnsi();
	uint32					resType			= resource->getType()->getCategoryId();
	uint16					resPE			= resource->getAttribute(ResAttr_PE);
	//bool					radioA			= false;
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
	uint32 actionCost		= gResourceCollectionManager->sampleActionCost;

	if(player->getSampleData()->mSampleNodeFlag)
	{
		if(setupForNodeSampleRecovery(player))
		{
			ratio	= 100.0;
			dieRoll = 200;
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
	if(player->getSampleData()->mSampleEventFlag == true)
	{
		player->getSampleData()->mSampleEventFlag = false;
		//set so a critical success happens -- A critical failure can also happen.  In either case, this took a significant amount of action (300 points???)
		// This appears to have been what the commented out gambling code below was supposed to do.
		dieRoll = 100;
	}

	if(ratio_100 >= minConcentration)
	{
		// Bug fix -- were saying we found something, then not saying we didn't if the die roll was a failure.
		// Now, say we found something only if we did in fact find something.
		//inside calculation section
		if(dieRoll <= failureChance)
		{
			// FAILED ATTEMPT
			sampleAmount = 0;
			successSample = false;
			gMessageLib->SendSystemMessage(::common::OutOfBand("survey", "sample_failed", L"", L"", resName.getUnicode16()), player);
		}
		// this probably needs to change right now == 98
		else if((dieRoll > 91)&&(dieRoll < 96))
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
				gUIManager->createNewListBox(gResourceCollectionManager,"gambleSample","@survey:gnode_t","@survey:gnode_d",items,player,SUI_Window_SmplGamble_ListBox,SUI_LB_OKCANCEL,0,0,asyncContainer);
			
				player->getSampleData()->mPendingSample = false;
				return;
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
				gUIManager->createNewListBox(gResourceCollectionManager,"waypNodeSample","@survey:cnode_t","@survey:cnode_d",items,player,SUI_Window_SmplWaypNode_ListBox,SUI_LB_OKCANCEL,0,0,asyncContainer);
		
				//Pause sampling
				player->getSampleData()->mPendingSample = false;
				player->getSampleData()->mSampleEventFlag = true;
			
				return;
			}
		}
		else
		{
			successSample = true;
			if(dieRoll == 200) 
			{
				sampleAmount = (static_cast<uint32>(3*maxSample));
                sampleAmount = std::max(sampleAmount,static_cast<uint>(1));
				gMessageLib->SendSystemMessage(::common::OutOfBand("survey", "node_recovery"), player);
				player->getSampleData()->mSampleEventFlag = false;
				player->getSampleData()->mSampleNodeFlag = false;
			}
			else
			// was set to == 100
			if(dieRoll >= 96) 
			{
				if(player->getSampleData()->mSampleGambleFlag)
				{
					gMessageLib->SendSystemMessage(::common::OutOfBand("survey", "gamble_success"), player);
					sampleAmount = (static_cast<uint32>(3*maxSample));
                    sampleAmount = std::max(sampleAmount, static_cast<uint>(1));
					gMessageLib->SendSystemMessage(::common::OutOfBand("survey", "sample_located", L"", L"", resName.getUnicode16(), sampleAmount), player);
					player->getSampleData()->mSampleGambleFlag = false;
					player->getSampleData()->mSampleEventFlag = false;
				}
				else
				{
				//CRITICAL SUCCESS
					sampleAmount = (static_cast<uint32>(2*maxSample));
                    sampleAmount = std::max(sampleAmount, static_cast<uint>(1));
                    gMessageLib->SendSystemMessage(::common::OutOfBand("survey", "critical_success", L"", L"", resName.getUnicode16()), player);
                    gMessageLib->SendSystemMessage(::common::OutOfBand("survey", "sample_located", L"", L"", resName.getUnicode16(), sampleAmount), player);

				}
			} 
			else 
			{
				//NORMAL SUCCESS
				sampleAmount = (static_cast<uint32>(floor(static_cast<float>((maxSample-minSample)*(dieRoll-failureChance)/(90-failureChance)+minSample))));         // floor == round down, so 9.9 == 9
                sampleAmount = std::max(sampleAmount, static_cast<uint>(1));
                gMessageLib->SendSystemMessage(::common::OutOfBand("survey", "sample_located", L"", L"", resName.getUnicode16(), sampleAmount), player);
			}
		}
	}
	else
	{
    gMessageLib->SendSystemMessage(::common::OutOfBand("survey", "density_below_threshold", L"", L"", resName.getUnicode16()), player);
		successSample = false;
		resAvailable = false;
	}
	// show the effects always
	gMessageLib->sendPlayClientEffectLocMessage(effect, player->mPosition, player);
		
	while(it != player->getKnownObjects()->end())
	{
		if(PlayerObject* targetPlayer = dynamic_cast<PlayerObject*>(*it))
		{
			gMessageLib->sendPlayClientEffectLocMessage(effect,player->mPosition,targetPlayer);
		}

		++it;
	}

	if (sampleAmount > 0)
	{
		// grant some xp
		gSkillManager->addExperience(XpType_resource_harvesting_inorganic,(int32)((gRandom->getRand()%20)+ sampleAmount),player); //grants 20xp -> 40xp inclusive -- Feature suggestion:  Grant less XP for smaller samples, more xp for greater samples.  IE:  20 + X*sampleSize

		// see if we can add it to an existing container
		Inventory*	inventory	= dynamic_cast<Inventory*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
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

						gWorldManager->getDatabase()->ExecuteSqlAsync(NULL,NULL,"UPDATE resource_containers SET amount=%u WHERE id=%"PRIu64"",newAmount,resCont->getId());
					}
					// target container full, put in what fits, create a new one
					else if(newAmount > maxAmount)
					{
						uint32 selectedNewAmount = newAmount - maxAmount;

						resCont->setAmount(maxAmount);

						gMessageLib->sendResourceContainerUpdateAmount(resCont,player);
						gWorldManager->getDatabase()->ExecuteSqlAsync(NULL,NULL,"UPDATE resource_containers SET amount=%u WHERE id=%"PRIu64"",maxAmount,resCont->getId());

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
	}

	// check our ham and keep sampling
	if(stopSampling(player, resource, tool))
	{
		actionCost = gResourceCollectionManager->sampleActionCost*2;
	}
	else
	{
		player->getSampleData()->mNextSampleTime = Anh_Utils::Clock::getSingleton()->getLocalTime() + 18000;
		player->getController()->addEvent(new SampleEvent(player,tool,resource),18000);
	}


	// update ham for standard sample action oc does this already - only the first time though???  !!!
	player->getHam()->updatePropertyValue(HamBar_Action,HamProperty_CurrentHitpoints,-(int32)actionCost, true);
}
bool	ArtisanManager::getNormalSample(PlayerObject* player, CurrentResource* resource, SurveyTool* tool)
{
	return true;
}
bool	ArtisanManager::setupForNodeSampleRecovery(PlayerObject* player)
{
		// need to be in a 5m radius of the node
        if(glm::distance(player->mPosition, player->getSampleData()->Position) <= 5.0)
		{
			player->getSampleData()->mSampleNodeRecovery = true;
		}
		else
		{
			gMessageLib->SendSystemMessage(::common::OutOfBand("survey", "node_not_close"), player);
			player->getSampleData()->mPendingSample		= false;
			player->getSampleData()->mSampleNodeFlag	= false;
			player->getSampleData()->mSampleNodeRecovery= false;
			return false;
		}


	return true;
}
bool	ArtisanManager::getGambleSample(PlayerObject* player, CurrentResource* resource, SurveyTool* tool)
{

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

			gUIManager->createNewMessageBox(gResourceCollectionManager,"radioactiveSample","@survey:radioactive_sample_t","@survey:radioactive_sample_d",player,SUI_Window_SmplRadioactive_MsgBox, SUI_MB_YESNO,asyncContainer);
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
		BuffAttribute* healthdebuffAttribute = new BuffAttribute(attr_health, -(int)hamReduc,0,hamReduc); 
		Buff* healthdebuff = Buff::SimpleBuff(player, player, 300000, 0, gWorldManager->GetCurrentGlobalTick());
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
bool	ArtisanManager::stopSampling(PlayerObject* player, CurrentResource* resource, SurveyTool* tool)
{
	Ham* ham = player->getHam();
	bool stop = false;

	if(!player->getSampleData()->mPendingSample || !resource || !tool || !player->isConnected())
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

	uint32 actionCost = gResourceCollectionManager->sampleActionCost;

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
		//message for stop sampling
		gMessageLib->SendSystemMessage(::common::OutOfBand("survey", "sample_cancel"), player);

		player->getSampleData()->mPendingSample	= false;
		player->setUpright();


		ham->updateRegenRates();
		player->updateMovementProperties();

		gMessageLib->sendUpdateMovementProperties(player);
		gMessageLib->sendPostureAndStateUpdate(player);
		gMessageLib->sendSelfPostureUpdate(player);
	}
	return stop;
}
////=============================================================================
////
//// survey event
////
//
//void ArtisanManager::onSurvey(const SurveyEvent* event)
//{
//	PlayerObject*		player		= event->getPlayer();
//	SurveyTool*			tool		= event->getTool();
//	CurrentResource*	resource	= event->getResource();
//
//	if(tool && resource && player->isConnected())
//	{
//		Datapad* datapad					= player->getDataPad();
//		ResourceLocation	highestDist		= gMessageLib->sendSurveyMessage(tool->getInternalAttribute<uint16>("survey_range"),tool->getInternalAttribute<uint16>("survey_points"),resource,player);
//
//		uint32 mindCost = gResourceCollectionManager->surveyMindCost;
//
//		//are we able to sample in the first place ??
//		if(!player->getHam()->checkMainPools(0,0,mindCost))
//		{
//			
//			int32 myMind = player->getHam()->mAction.getCurrentHitPoints();		
//			
//			//return message for sampling cancel based on HAM
//			if(myMind < (int32)mindCost)
//			{
//				gMessageLib->sendSystemMessage(player,L"","error_message","sample_mind");
//			}
//
//			//message for stop sampling
//			gMessageLib->sendSystemMessage(player,L"","survey","sample_cancel");
//
//			player->getSampleData()->mPendingSurvey = false;
//
//			player->getHam()->updateRegenRates();
//			player->updateMovementProperties();
//			return;
//		}
//
//		player->getHam()->updatePropertyValue(HamBar_Mind,HamProperty_CurrentHitpoints, -(int)mindCost);
//
//		// this is 0, if resource is not located
//		if(highestDist.position.y == 5.0)
//		{
//			WaypointObject*	waypoint = datapad->getWaypointByName("Resource Survey");
//
//			// remove the old one
//			if(waypoint)
//			{
//				gMessageLib->sendUpdateWaypoint(waypoint,ObjectUpdateDelete,player);
//				datapad->updateWaypoint(waypoint->getId(), waypoint->getName(), glm::vec3(highestDist.position.x,0.0f,highestDist.position.z),
//										static_cast<uint16>(gWorldManager->getZoneId()), player->getId(), WAYPOINT_ACTIVE);
//			}
//			else
//			{
//				// create a new one
//				if(datapad->getCapacity())
//				{
//					gMessageLib->sendSysMsg(player,"survey","survey_waypoint");
//					//gMessageLib->sendSystemMessage(this,L"","survey","survey_waypoint");
//				}
//				//the datapad automatically checks if there is room and gives the relevant error message
//				datapad->requestNewWaypoint("Resource Survey", glm::vec3(highestDist.position.x,0.0f,highestDist.position.z),static_cast<uint16>(gWorldManager->getZoneId()),Waypoint_blue);
//			}
//
//			gMissionManager->checkSurveyMission(player,resource,highestDist);
//		}
//	}
//
//	player->getSampleData()->mPendingSurvey = false;
//}