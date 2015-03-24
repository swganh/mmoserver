/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2014 The SWG:ANH Team
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
#include "TicketCollector.h"
#include "Zoneserver/Objects/Inventory.h"
#include "ZoneServer/Objects/Player Object/PlayerObject.h"
#include "ZoneServer/Objects/Object/ObjectFactory.h"
#include "ZoneServer/Objects/Shuttle.h"
#include "ZoneServer/GameSystemManagers/Travel Manager/TravelMapHandler.h"
#include "TravelTicket.h"
#include "ZoneServer/GameSystemManagers/UI Manager/UIListBox.h"
#include "ZoneServer/GameSystemManagers/UI Manager/UIManager.h"
#include "ZoneServer/WorldManager.h"
#include "ZoneServer/GameSystemManagers/Container Manager/ContainerManager.h"
#include "ZoneServer\Services\equipment\equipment_service.h"
#include "MessageLib/MessageLib.h"

#include <time.h>
#include "anh/Utils/rand.h"

//=============================================================================

TicketCollector::TicketCollector() : TangibleObject(),
    mShuttle(NULL)
{
    mTanGroup	= TanGroup_TicketCollector;
    mTanType	= TanType_TicketCollector;

    mPortDescriptor.setLength(64);

}

//=============================================================================

TicketCollector::~TicketCollector()
{
}

//=============================================================================

void TicketCollector::handleObjectMenuSelect(uint8 messageType,Object* srcObject)
{
	if(messageType == radId_itemUse)
	{
		PlayerObject* playerObject = dynamic_cast<PlayerObject*>(srcObject);
		
		// don't use while incapped or dead or in combat
		if(playerObject->GetCreature()->isIncapacitated() || playerObject->GetCreature()->isDead() || playerObject->GetCreature()->states.checkState(CreatureState_Combat))
		{
			return;
		}

		// in range check for shuttle not for the droid
		if(playerObject->GetCreature()->getParentId() != mParentId || (glm::distance(playerObject->GetCreature()->mPosition, mShuttle->mPosition) > 25.0f))
        {
            gMessageLib->SendSystemMessage(::common::OutOfBand("travel", "boarding_too_far"), playerObject);
            return;
        }

        if(playerObject->checkUIWindow("handleTicketSelect"))
        {
            gMessageLib->SendSystemMessage(::common::OutOfBand("travel", "boarding_ticket_selection"), playerObject);
            return;
        }

        if(mShuttle)
        {
            mShuttle->useShuttle(playerObject);
        }
    }
    else
        DLOG(INFO) << "TravelTerminal: Unhandled MenuSelect: " << messageType;
}

//=============================================================================

void TicketCollector::prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount)
{
    mRadialMenu = RadialMenuPtr(new RadialMenu());

    mRadialMenu->addItem(1,0,radId_itemUse,radAction_ObjCallback);
    mRadialMenu->addItem(2,0,radId_examine,radAction_Default);
}


//=============================================================================

void TicketCollector::_createTicketSelectMenu(PlayerObject* playerObject)
{
    StringVector	availableTickets;
    uint32			zoneId = gWorldManager->getZoneId();

	auto inventory = gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::equipment::EquipmentService>("EquipmentService")->GetEquippedObject(playerObject, "inventory");

	inventory->ViewObjects(playerObject, 0, true, [&] (Object* object) {
    
		if(TravelTicket* ticket = dynamic_cast<TravelTicket*>(object))
        {
            BString srcPoint		= (int8*)((ticket->getAttribute<std::string>("travel_departure_point")).c_str());
            uint16 srcPlanetId	= static_cast<uint16>(gWorldManager->getPlanetIdByName(ticket->getAttribute<std::string>("travel_departure_planet")));

            if(srcPlanetId == zoneId && strcmp(srcPoint.getAnsi(),mPortDescriptor.getAnsi()) == 0)
            {
                BString dstPoint = (int8*)((ticket->getAttribute<std::string>("travel_arrival_point")).c_str());

                availableTickets.push_back(dstPoint.getAnsi());
            }
        }
    });

    gUIManager->createNewListBox(this,"handleticketselect","select destination","Select destination",availableTickets,playerObject);
}

//=============================================================================

void TicketCollector::handleUIEvent(uint32 action,int32 element,std::u16string inputStr,UIWindow* window, std::shared_ptr<WindowAsyncContainerCommand> AsyncContainer)
{
    if (!action && element != -1 && mShuttle != NULL && mShuttle->availableInPort())
    {
        uint32			zoneId			= gWorldManager->getZoneId();
        PlayerObject*	playerObject	= window->getOwner();

        if(playerObject->getSurveyState() || playerObject->getSamplingState() || playerObject->GetCreature()->isIncapacitated() || playerObject->GetCreature()->isDead())
        {
            return;
        }

        // in range check
		if(playerObject->getParentId() != mParentId || (glm::distance(playerObject->GetCreature()->mPosition, this->mPosition) > 10.0f))
        {
            gMessageLib->SendSystemMessage(::common::OutOfBand("travel", "boarding_too_far"), playerObject);
            return;
        }

        auto inventory = gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::equipment::EquipmentService>("EquipmentService")->GetEquippedObject(playerObject, "inventory");

		inventory->ViewObjects(playerObject, 0, true, [&] (Object* object) {
            if(TravelTicket* ticket = dynamic_cast<TravelTicket*>(object))
            {
                std::string srcPoint		= ticket->getAttribute<std::string>("travel_departure_point");
                std::string dstPointStr	= ticket->getAttribute<std::string>("travel_arrival_point");
                uint16 srcPlanetId	= static_cast<uint16>(gWorldManager->getPlanetIdByName(ticket->getAttribute<std::string>("travel_departure_planet")));
                uint16 dstPlanetId	= static_cast<uint16>(gWorldManager->getPlanetIdByName(ticket->getAttribute<std::string>("travel_arrival_planet")));

                StringVector* items = (dynamic_cast<UIListBox*>(window))->getDataItems();
                std::string selectedDst = items->at(element);

                if(srcPlanetId == zoneId && (strcmp(srcPoint.c_str(),mPortDescriptor.getAnsi()) == 0)
					&& (strcmp(dstPointStr.c_str(),selectedDst.c_str()) == 0))
                {
                    if(TravelPoint* dstPoint = gTravelMapHandler->getTravelPoint(dstPlanetId,dstPointStr))
                    {
                        glm::vec3 destination;
                        // getRand(5) return 0-4, then sub 2, and you get equal of random values at both sides of zero. (-2, -1, 0, 1, 2)
                        destination.x = dstPoint->spawnX + (gRandom->getRand()%5 - 2);
                        destination.y = dstPoint->spawnY;
                        destination.z = dstPoint->spawnZ + (gRandom->getRand()%5 - 2);

                        // If it's on this planet, then just warp, otherwize zone
                        if(dstPlanetId == zoneId)
                        {
                            // only delete the ticket if we are warping on this planet.
							TangibleObject* tO = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById(ticket->getParentId()));
							gContainerManager->deleteObject(ticket, tO);
                            
                            gWorldManager->warpPlanet(playerObject,destination,0);
                        }
                        else
                        {
                            gMessageLib->sendClusterZoneTransferRequestByTicket(playerObject, ticket->getId(), dstPoint->planetId);

                        }
                    }
                    else
                    {
                        DLOG(INFO) << "TicketCollector: Error getting TravelPoint";
                    }
                    return;
                }
            }
        });
    }
}

//=============================================================================

void TicketCollector::travelRequest(TravelTicket* ticket,PlayerObject* playerObject)
{
    if(playerObject->getSurveyState() || playerObject->getSamplingState() || playerObject->GetCreature()->isIncapacitated() || playerObject->GetCreature()->isDead())
    {
        return;
    }

    uint32 zoneId		= gWorldManager->getZoneId();
    BString srcPoint		= (int8*)((ticket->getAttribute<std::string>("travel_departure_point")).c_str());
    std::string dstPointStr	= ticket->getAttribute<std::string>("travel_arrival_point");
    uint16 srcPlanetId	= static_cast<uint16>(gWorldManager->getPlanetIdByName(ticket->getAttribute<std::string>("travel_departure_planet")));
    uint16 dstPlanetId	= static_cast<uint16>(gWorldManager->getPlanetIdByName(ticket->getAttribute<std::string>("travel_arrival_planet")));

    if (mShuttle && mShuttle->availableInPort())
    {
        if(srcPlanetId == zoneId && (strcmp(srcPoint.getAnsi(),mPortDescriptor.getAnsi()) == 0))
        {
            TravelPoint* dstPoint = gTravelMapHandler->getTravelPoint(dstPlanetId,dstPointStr);

            if(dstPoint)
            {
                glm::vec3 destination;
                destination.x = dstPoint->spawnX + (gRandom->getRand()%5 - 2);
                destination.y = dstPoint->spawnY;
                destination.z = dstPoint->spawnZ + (gRandom->getRand()%5 - 2);

                // If it's on this planet, then just warp, otherwize zone
                if(dstPlanetId == zoneId)
                {
                    // only delete the ticket if we are warping on this planet.
					TangibleObject* tO = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById(ticket->getParentId()));
					gContainerManager->deleteObject(ticket, tO);
                    
                    gWorldManager->warpPlanet(playerObject,destination,0);
                }
                else
                {
                    gMessageLib->sendClusterZoneTransferRequestByTicket(playerObject,ticket->getId(),dstPoint->planetId);
                }
            }
            else
            {
                DLOG(INFO) << "TicketCollector: Error getting TravelPoint";
            }
        }
    }
}

//=============================================================================


