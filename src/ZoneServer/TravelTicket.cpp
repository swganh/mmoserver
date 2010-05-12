/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "TravelTicket.h"
#include "PlayerObject.h"
#include "Shuttle.h"
#include "TicketCollector.h"
#include "TravelMapHandler.h"
#include "WorldManager.h"
#include "ZoneTree.h"
#include "ZoneServer/ZoneOpcodes.h"
#include "MessageLib/MessageLib.h"
#include "LogManager/LogManager.h"



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

string TravelTicket::getBazaarName()
{
	int8	ticketStr[256];

	sprintf(ticketStr,"Travel Ticket %s : %s"
		,((getAttribute<std::string>("travel_departure_planet")).c_str())
		,((getAttribute<std::string>("travel_arrival_planet")).c_str()));

	string value = ticketStr;
	
	return value;
}

//=============================================================================

void TravelTicket::handleObjectMenuSelect(uint8 messageType,Object* srcObject)
{
	if(messageType == radId_itemUse)
	{
		PlayerObject*	player	= dynamic_cast<PlayerObject*>(srcObject);

		if(player->getPosture() == CreaturePosture_SkillAnimating)
		{
			gMessageLib->sendSystemMessage(player,L"","error_message","wrong_state");
			return;
		}

		ObjectSet objects;

		// see if a shuttle is in range
		gWorldManager->getSI()->getObjectsInRange(player,&objects,ObjType_NPC | ObjType_Creature,25.0f);

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
	
		gMessageLib->sendSystemMessage(player,L"There is no shuttle nearby");
	}
}

//=============================================================================


