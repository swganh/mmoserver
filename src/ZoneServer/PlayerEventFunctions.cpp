/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "PlayerObject.h"
#include "Datapad.h"
#include "Inventory.h"
#include "MissionManager.h"
#include "ObjectFactory.h"
#include "ResourceContainer.h"
#include "ResourceCollectionManager.h"
#include "ResourceType.h"
#include "UIEnums.h"
#include "UIManager.h"
#include "WaypointObject.h"
#include "WorldManager.h"
#include "DatabaseManager/Database.h"
#include "Utils/clock.h"
#include "MessageLib/MessageLib.h"
#include "LogManager/LogManager.h"
#include "Common/Message.h"
#include "Common/MessageFactory.h"
#include "MathLib/Quaternion.h"


//=============================================================================
//
// survey event
//

void PlayerObject::onSurvey(const SurveyEvent* event)
{
	SurveyTool*			tool		= event->getTool();
	CurrentResource*	resource	= event->getResource();

	if(tool && resource && isConnected())
	{
		Datapad*			datapad			= dynamic_cast<Datapad*>(mEquipManager.getEquippedObject(CreatureEquipSlot_Datapad));
		ResourceLocation	highestDist		= gMessageLib->sendSurveyMessage(tool->getInternalAttribute<uint16>("survey_range"),tool->getInternalAttribute<uint16>("survey_points"),resource,this);

		// this is 0, if resource is not located
		if(highestDist.position.mY == 5.0)
		{
			WaypointObject*	waypoint = datapad->getWaypointByName("Resource Survey");

			// remove the old one
			if(waypoint)
			{
				gMessageLib->sendUpdateWaypoint(waypoint,ObjectUpdateDelete,this);

				datapad->removeWaypoint(waypoint);

				gObjectFactory->deleteObjectFromDB(waypoint);
			}

			// create a new one
			if(datapad->getCapacity())
			{
				datapad->requestNewWaypoint("Resource Survey",Anh_Math::Vector3(highestDist.position.mX,0.0f,highestDist.position.mZ),static_cast<uint16>(gWorldManager->getZoneId()),Waypoint_blue);
			}

			gMessageLib->sendSystemMessage(this,L"","survey","survey_waypoint");

			gMissionManager->checkSurveyMission(this,resource,highestDist);
		}
	}

	mPendingSurvey = false;
}

//=============================================================================
//
// sample event
//

void PlayerObject::onSample(const SampleEvent* event)
{
	SurveyTool*			tool		= event->getTool();
	CurrentResource*	resource	= event->getResource();

	if(!mPendingSample || !resource || !tool || !isConnected())
	{
		mPendingSample = false;
		return;
	}

	string					effect			= gWorldManager->getClientEffect(tool->getInternalAttribute<uint32>("sample_effect"));
	bool					foundSameType	= false;
	float					ratio			= (resource->getDistribution((int)mPosition.mX + 8192,(int)mPosition.mZ + 8192));
	int32					surveyMod		= getSkillModValue(SMod_surveying);
	uint32					sampleAmount	= 0;
	ObjectSet::iterator	it					= mKnownObjects.begin();
	string					resName			= resource->getName().getAnsi();
	uint32					resType			= resource->getType()->getCategoryId();
	uint16					resPE			= resource->getAttribute(ResAttr_PE);
	//bool					radioA			= false;
	bool					successSample	= false;
	bool					resAvailable	= true;
	resName.convert(BSTRType_Unicode16);
	// apply dmg/debuff if sampling rads
			if(resType == 477 || resType == 476 /* || resType == 475*/)
			{
				if(!mPassRadioactive)
				{
					//UI Integration
					//gUIManager->createNewMessageBox(this,"radioactiveSample","Radioactive Sample","This is a radioactive resource. You will incur penalties if you choose to continue sampling a resource of this type.",this,SUI_Window_SmplRadioactive_MsgBox, SUI_MB_YESNO);

					gUIManager->createNewMessageBox(this,"radioactiveSample","@survey:radioactive_sample_t","@survey:radioactive_sample_d",this,SUI_Window_SmplRadioactive_MsgBox, SUI_MB_YESNO);
					//Pause Sampling
					mPendingSample = false;
					return;
				}
				uint32 playerBF = mHam.getBattleFatigue();
				uint32 woundDmg = 50*(1 + (playerBF/100)) + (50*(1 + (resPE/1000)));
				uint32 bfDmg    = static_cast<uint32>(0.075*resPE);
				uint32 hamReduc = 100*(2+ (resPE/1000));

				gLogger->logMsgF("woundDmg is: %u\n", MSG_NORMAL, woundDmg);
				gLogger->logMsgF("bfDmg is: %u\n", MSG_NORMAL, bfDmg);
				gLogger->logMsgF("hamReduc is: %u\n", MSG_NORMAL, hamReduc);

				if(resPE >= 500)
				{
					//wound and BF dmg
					gLogger->logMsg("PE > 500: ready to apply new BF/wound dmg\n");
					mHam.updateBattleFatigue(bfDmg);
					mHam.updatePropertyValue(HamBar_Health,HamProperty_Wounds, woundDmg); //does not function
					mHam.updatePropertyValue(HamBar_Action,HamProperty_Wounds, woundDmg);
					mHam.updatePropertyValue(HamBar_Mind,HamProperty_Wounds, woundDmg);
					//normal ham
					mHam.updatePropertyValue(HamBar_Action,HamProperty_CurrentHitpoints,hamReduc);
					mHam.updatePropertyValue(HamBar_Health,HamProperty_CurrentHitpoints,hamReduc); //does not function
				}
				else
				{
					//this should be a timed debuff per instance -- Do not cause wounds unless potential energy >= 500?
					mHam.updatePropertyValue(HamBar_Action,HamProperty_CurrentHitpoints,hamReduc);
					mHam.updatePropertyValue(HamBar_Health,HamProperty_CurrentHitpoints,hamReduc); //does not function
					gLogger->logMsgF("applied ham costs H/A w/ reduc: %u", MSG_NORMAL, hamReduc);
				}
			}

	// update ham for standard sample action oc does this already
	//mHam.updatePropertyValue(HamBar_Action,HamProperty_CurrentHitpoints,-(int32)gResourceCollectionManager->sampleActionCost);

	// calculate sample size if concentration >0 and surveyMod exists for player
	if(ratio > 0.0f && surveyMod)
	{
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

		//for checking abs minimum conc
		//float minConc100sk		= static_cast<float>(-1.12*maxSuccessChance+88);

		// ////////////////////////////////////////
		gLogger->logMsgF("here is the dieRoll: %f", MSG_NORMAL, dieRoll);

		//////////////////////////////////////////
		//set dieRoll for debugging
		//dieRoll = 92;
		//gLogger->logMsgF("here is the dieRoll CHANGED: %f", MSG_NORMAL, dieRoll);

		//If previous call triggered a sample event, set the roll to ensure critical success
		if(mSampleEventFlag == true)
		{
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
				gMessageLib->sendSystemMessage(this,L"","survey","sample_failed","","",resName);
			}

			//TODO: removed this code for now, as you would have to dcon and rcon to work with the gambles
			//else if(dieRoll > 91)
			//{
			//	//EVENT WINDOW CASE
			//	int32 eventRoll = int(gRandom->getRand()%2)+1;
			//	//do event 1 or event 2 based on roll
			//	if(eventRoll == 1)
			//	{
			//		//GAMBLE Event
			//		gLogger->logMsg("sampling:gamble event.");
			//		//TODO: Change UI integration
			//		gUIManager->createNewMessageBox(this,"gambleSample","@survey:gnode_t","@survey:gnode_d",this,SUI_Window_SmplGamble_ListBox);
			//		//Pause sampling
			//		mPendingSample = true;
			//		sampleAmount = 50; //for testing purposes only, will be revamped when ui is integrated
			//		//return;
			//	}
			//	else
			//	{
			//		//string waypConcentrationChoices = "Tune device to concentration.";
			//		//waypConcentrationChoices.convert(BSTRType_Unicode16);
			//		//WAYP CONCENTRATION
			//		gLogger->logMsg("sampling: waypt concentration");
			//		//TODO: Change UI integration
			//		gUIManager->createNewMessageBox(this,"waypNodeSample","@survey:cnode_t","@survey:cnode_d",this,SUI_Window_SmplWaypNode_ListBox);
			//
			//		//Pause sampling
			//		mPendingSample = false;
			//		mSampleEventFlag = true;
			//		gLogger->logMsg("Event flag SET");
			//		sampleAmount = 50; //for testing purposes only, will be revamped when ui is integrated
			//		//return;
			//	}
			//}
			else
			{
				successSample = true;
				if(dieRoll >= 96) {
				//CRITICAL SUCCESS
					sampleAmount = (static_cast<uint32>(2*maxSample));
					gMessageLib->sendSystemMessage(this,L"","survey","critical_success","","",resName);
				} else {
					//NORMAL SUCCESS
					sampleAmount = (static_cast<uint32>(floor(static_cast<float>((maxSample-minSample)*(dieRoll-failureChance)/(90-failureChance)+minSample))));         // floor == round down, so 9.9 == 9
					gMessageLib->sendSystemMessage(this,L"","survey","sample_located","","",resName,sampleAmount);
				}
			}
		}
		else
		{
			gMessageLib->sendSystemMessage(this,L"","survey","density_below_threshold","","",resName);
			successSample = false;
			resAvailable = false;
		}
	}

	//Fix for bug#870 - A very small success which rounds to 0 should actually give 1 unit
	if(successSample && sampleAmount == 0)
	{
		sampleAmount = 1;
	}

	// show the effects
	if (successSample) {
		gMessageLib->sendPlayClientEffectLocMessage(effect,mPosition,this);

		while(it != mKnownObjects.end())
		{
			if(PlayerObject* targetPlayer = dynamic_cast<PlayerObject*>(*it))
			{
				gMessageLib->sendPlayClientEffectLocMessage(effect,mPosition,targetPlayer);
			}

			++it;
		}
	}

	if (sampleAmount > 0)
	{
		// grant some xp
		gSkillManager->addExperience(XpType_resource_harvesting_inorganic,(int32)((gRandom->getRand()%20)+ 20),this); //grants 20xp -> 40xp inclusive -- Feature suggestion:  Grant less XP for smaller samples, more xp for greater samples.  IE:  20 + X*sampleSize

		// see if we can add it to an existing container
		Inventory*	inventory	= dynamic_cast<Inventory*>(mEquipManager.getEquippedObject(CreatureEquipSlot_Inventory));
		ObjectList*	invObjects	= inventory->getObjects();
		ObjectList::iterator listIt = invObjects->begin();

		while(listIt != invObjects->end())
		{
			// we are looking for resource containers
			if(ResourceContainer* resCont = dynamic_cast<ResourceContainer*>(*listIt))
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

						gMessageLib->sendResourceContainerUpdateAmount(resCont,this);

						gWorldManager->getDatabase()->ExecuteSqlAsync(NULL,NULL,"UPDATE resource_containers SET amount=%u WHERE id=%"PRId64"",newAmount,resCont->getId());
					}
					// target container full, put in what fits, create a new one
					else if(newAmount > maxAmount)
					{
						uint32 selectedNewAmount = newAmount - maxAmount;

						resCont->setAmount(maxAmount);

						gMessageLib->sendResourceContainerUpdateAmount(resCont,this);
						gWorldManager->getDatabase()->ExecuteSqlAsync(NULL,NULL,"UPDATE resource_containers SET amount=%u WHERE id=%"PRId64"",maxAmount,resCont->getId());

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
	if(mHam.checkMainPools(0,gResourceCollectionManager->sampleActionCost,0) && (resAvailable))
	{
		mNextSampleTime = Anh_Utils::Clock::getSingleton()->getLocalTime() + 3000; //change back to 30000 after testing is finished
		mObjectController.addEvent(new SampleEvent(tool,resource),10000);
	}
	// out of ham or not enough skill, or resource not spawned in current location, stop sampling
	else
	{
		int32 myHealth = mHam.mHealth.getCurrentHitPoints();
		int32 myAction = mHam.mAction.getCurrentHitPoints();
		int32 myMind = mHam.mMind.getCurrentHitPoints();
		gLogger->logMsgF("here is myHealth: %i\n", MSG_NORMAL, myHealth);
		gLogger->logMsgF("here is myAction: %i\n", MSG_NORMAL, myAction);
		gLogger->logMsgF("here is myMind: %i\n", MSG_NORMAL, myMind);
		//return message for sampling cancel based on HAM
		if(myAction < 150)
		{
			gMessageLib->sendSystemMessage(this,L"","survey","sample_action");
		}

		//message for stop sampling
		gMessageLib->sendSystemMessage(this,L"","survey","sample_cancel");


		mPendingSample	= false;
		mPosture		= CreaturePosture_Upright;

		mHam.updateRegenRates();
		updateMovementProperties();

		gMessageLib->sendUpdateMovementProperties(this);
		gMessageLib->sendPostureAndStateUpdate(this);
		gMessageLib->sendSelfPostureUpdate(this);
	}
}

//=============================================================================

