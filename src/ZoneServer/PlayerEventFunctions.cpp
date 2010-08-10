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

#include "PlayerObject.h"
#include "Datapad.h"
#include "Inventory.h"
#include "MissionManager.h"
#include "ObjectFactory.h"
#include "ResourceManager.h"
#include "ResourceContainer.h"
#include "ResourceCollectionManager.h"
#include "ResourceType.h"
#include "Buff.h"
#include "UIEnums.h"
#include "UIManager.h"
#include "Heightmap.h"
#include "WaypointObject.h"
#include "WorldManager.h"
#include "DatabaseManager/Database.h"
#include "Utils/clock.h"
#include "MessageLib/MessageLib.h"
#include "Common/LogManager.h"
#include "Common/OutOfBand.h"
#include "NetworkManager/Message.h"
#include "NetworkManager/MessageFactory.h"

#include "utils/rand.h"

#include <algorithm>

using ::common::OutOfBand;

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
        Datapad* datapad					= getDataPad();
        ResourceLocation	highestDist		= gMessageLib->sendSurveyMessage(tool->getInternalAttribute<uint16>("survey_range"),tool->getInternalAttribute<uint16>("survey_points"),resource,this);

        uint32 mindCost = gResourceCollectionManager->surveyMindCost;

        //are we able to sample in the first place ??
        if(!mHam.checkMainPools(0,0,mindCost))
        {
            
            int32 myMind = mHam.mAction.getCurrentHitPoints();		
            
            //return message for sampling cancel based on HAM
            if(myMind < (int32)mindCost)
            {
                gMessageLib->SendSystemMessage(::common::OutOfBand("error_message", "sample_mind"), this);
            }

            //message for stop sampling
            gMessageLib->SendSystemMessage(::common::OutOfBand("survey", "sample_cancel"), this);

            getSampleData()->mPendingSurvey = false;

            mHam.updateRegenRates();
            updateMovementProperties();
            return;
        }

        mHam.performSpecialAction(0,0,(float)mindCost,HamProperty_CurrentHitpoints);

        // this is 0, if resource is not located
        if(highestDist.position.y == 5.0)
        {
            WaypointObject*	waypoint = datapad->getWaypointByName("Resource Survey");

            // remove the old one
            if(waypoint)
            {
                gMessageLib->sendUpdateWaypoint(waypoint,ObjectUpdateDelete,this);
                datapad->updateWaypoint(waypoint->getId(), waypoint->getName(), glm::vec3(highestDist.position.x,0.0f,highestDist.position.z),
                                        static_cast<uint16>(gWorldManager->getZoneId()), this->getId(), WAYPOINT_ACTIVE);
            }
            else
            {
                // create a new one
                if(datapad->getCapacity())
                {
                    gMessageLib->SendSystemMessage(::common::OutOfBand("survey", "survey_waypoint"), this);
                    //gMessageLib->sendSystemMessage(this,L"","survey","survey_waypoint");
                }
                //the datapad automatically checks if there is room and gives the relevant error message
                datapad->requestNewWaypoint("Resource Survey", glm::vec3(highestDist.position.x,0.0f,highestDist.position.z),static_cast<uint16>(gWorldManager->getZoneId()),Waypoint_blue);
            }

            gMissionManager->checkSurveyMission(this,resource,highestDist);
        }
    }

    getSampleData()->mPendingSurvey = false;
}

//=============================================================================
//
// sample event
//

void PlayerObject::onSample(const SampleEvent* event)
{
    // this will be replaced as soon as events are torn out of player object
    gArtisanManager->onSample(event);
    return;
}

//=============================================================================
// this event manages the logout through the /logout command
//

void PlayerObject::onLogout(const LogOutEvent* event)
{
    
    if(!this->checkPlayerCustomFlag(PlayerCustomFlag_LogOut))
    {
        return;
    }
    //is it time for logout yet ?
    if(Anh_Utils::Clock::getSingleton()->getLocalTime() <  event->getLogOutTime())
    {
        //tell the time and dust off
        mObjectController.addEvent(new LogOutEvent(event->getLogOutTime(),event->getLogOutSpacer()),event->getLogOutSpacer());
        uint32 timeLeft = (uint32)(event->getLogOutTime()- Anh_Utils::Clock::getSingleton()->getLocalTime())/1000;
        gMessageLib->SendSystemMessage(OutOfBand("logout", "time_left", 0, 0, 0, timeLeft), this);
        return;
    }
            
    gMessageLib->SendSystemMessage(OutOfBand("logout", "safe_to_log_out"), this);
    
    gMessageLib->sendLogout(this);
    this->togglePlayerCustomFlagOff(PlayerCustomFlag_LogOut);	
    gWorldManager->addDisconnectedPlayer(this);
    //Initiate Logout
    
}


//=============================================================================
// this event manages the removeal of consumeables - so an object doesnt have to delete itself
// CAVE we only remove it out of the inventory / objectmap
void PlayerObject::onItemDeleteEvent(const ItemDeleteEvent* event)
{

    uint64 now = Anh_Utils::Clock::getSingleton()->getLocalTime();

    //do we have to remove the cooldown?
    
    Item* item = dynamic_cast<Item*>(gWorldManager->getObjectById(event->getItem()));
    if(!item)
    {
        gLogger->log(LogManager::DEBUG,"PlayerObject::onItemDeleteEvent: Item %I64u not found",event->getItem());
        return;
    }
    
    TangibleObject* tO = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById(item->getParentId()));
    tO->deleteObject(item);
        
}

//=============================================================================
// this event manages injury treatment cooldowns.
//
void PlayerObject::onInjuryTreatment(const InjuryTreatmentEvent* event)
{
    uint64 now = gWorldManager->GetCurrentGlobalTick();
    uint64 t = event->getInjuryTreatmentTime();

    if(now > t)
    {
        this->togglePlayerCustomFlagOff(PlayerCustomFlag_InjuryTreatment);
        gMessageLib->SendSystemMessage(::common::OutOfBand("healing_response", "healing_response_58"), this);
    }
    
    //have to call once more so we can get back here...
    else
    {
        mObjectController.addEvent(new InjuryTreatmentEvent(t), t-now);
    }
}
//=============================================================================
// this event manages quickheal injury treatment cooldowns.
//
void PlayerObject::onQuickHealInjuryTreatment(const QuickHealInjuryTreatmentEvent* event)
{
    uint64 now = gWorldManager->GetCurrentGlobalTick();
    uint64 t = event->getQuickHealInjuryTreatmentTime();

    if(now > t)
    {
        this->togglePlayerCustomFlagOff(PlayerCustomFlag_QuickHealInjuryTreatment);
        gMessageLib->SendSystemMessage(::common::OutOfBand("healing_response", "healing_response_58"), this);
    }
    
    //have to call once more so we can get back here...
    else
    {
        mObjectController.addEvent(new QuickHealInjuryTreatmentEvent(t), t-now);
    }
}

//=============================================================================
// this event manages wound treatment cooldowns.
//
void PlayerObject::onWoundTreatment(const WoundTreatmentEvent* event)
{
    uint64 now = gWorldManager->GetCurrentGlobalTick();
    uint64 t = event->getWoundTreatmentTime();

    if(now >  t)
    {
        this->togglePlayerCustomFlagOff(PlayerCustomFlag_WoundTreatment);
        
        gMessageLib->SendSystemMessage(::common::OutOfBand("healing_response", "healing_response_59"), this);
    }
    //have to call once more so we can get back here...
    else
    {
        mObjectController.addEvent(new WoundTreatmentEvent(t), t-now);
    }
}
