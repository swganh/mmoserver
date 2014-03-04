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

#include "ZoneServer/GameSystemManagers/Travel Manager/TravelTicket.h"
#include "ZoneServer/Objects/Player Object/PlayerObject.h"
#include "ZoneServer/Objects/Shuttle.h"
#include "ZoneServer/GameSystemManagers/Travel Manager/TicketCollector.h"
#include "ZoneServer/GameSystemManagers/Travel Manager/TravelMapHandler.h"
#include "ZoneServer/GameSystemManagers/Spatial Index Manager/SpatialIndexManager.h"
#include "ZoneServer/WorldManager.h"
#include "ZoneServer/ZoneOpcodes.h"
#include "MessageLib/MessageLib.h"




//=============================================================================

TravelTicket::TravelTicket() : Item()
{

}

void TravelTicket::prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount)
{
    mRadialMenu = RadialMenuPtr(new RadialMenu());

    mRadialMenu->addItem(1,0,radId_itemUse,radAction_ObjCallback);
    mRadialMenu->addItem(2,0,radId_examine,radAction_Default);
}


//=============================================================================

TravelTicket::~TravelTicket()
{
}

//=============================================================================

BString TravelTicket::getBazaarName()
{
    int8	ticketStr[256];

    sprintf(ticketStr,"Travel Ticket %s : %s"
            ,((getAttribute<std::string>("travel_departure_planet")).c_str())
            ,((getAttribute<std::string>("travel_arrival_planet")).c_str()));

    BString value = ticketStr;

    return value;
}

//=============================================================================

void TravelTicket::handleObjectMenuSelect(uint8 messageType,Object* srcObject)
{
    if(messageType == radId_itemUse)
    {
        PlayerObject*	player	= dynamic_cast<PlayerObject*>(srcObject);

		if(player->GetCreature()->GetPosture() == CreaturePosture_SkillAnimating)
		{
            gMessageLib->SendSystemMessage(::common::OutOfBand("error_message", "wrong_state"), player);
            return;
        }

        ObjectSet objects;

		// see if a shuttle is in range
		gSpatialIndexManager->getObjectsInRange(player,&objects,ObjType_NPC | ObjType_Creature,25.0f,true);

        ObjectSet::iterator objIt = objects.begin();

        while(objIt != objects.end())
        {
            if(Shuttle* shuttle = dynamic_cast<Shuttle*> (*objIt))
            {
                if(player->getParentId() == shuttle->getParentId())
                {
                    gTravelMapHandler->useTicket(player, (TravelTicket*) this,shuttle);
                    return;
                }
            }

            ++objIt;
        }
		std::string message_ansi("There is no shuttle nearby");
        gMessageLib->SendSystemMessage(std::u16string(message_ansi.begin(), message_ansi.end()), player);
    }
}

//=============================================================================


