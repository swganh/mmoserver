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
#include "TravelTerminal.h"
#include "PlayerObject.h"
#include "Tutorial.h"
#include "WorldConfig.h"
#include "MessageLib/MessageLib.h"

#ifdef WIN32
#undef ERROR
#endif

#include "Utils/logger.h"

//=============================================================================

TravelTerminal::TravelTerminal() : Terminal()
{

}

//=============================================================================

TravelTerminal::~TravelTerminal()
{
}
//=============================================================================

void TravelTerminal::handleObjectMenuSelect(uint8 messageType,Object* srcObject)
{
	if(messageType == radId_itemUse)
	{
		PlayerObject* playerObject = dynamic_cast<PlayerObject*>(srcObject);

		if(!playerObject || !playerObject->isConnected() || playerObject->getSamplingState() || playerObject->isIncapacitated() || playerObject->isDead() || playerObject->states.checkState(CreatureState_Combat))
		{
			return;
		}

		if (gWorldConfig->isTutorial())
		{
			#if defined(_MSC_VER)
			if (this->getId() == 4294968329)
			#else
			if (this->getId() == 4294968329LLU)
			#endif
			{
				// This is the Travel Terminal in the tutorial.
				Tutorial* tutorial = playerObject->getTutorial();

				// We will not display this when tutorial is in state 26 or above.
				if (tutorial->getSubState() < 26)
				{
					// The quatermaster speaks...
					#if defined(_MSC_VER)
					tutorial->spatialChat(47513085685, "Wait a minute there!  You can't use that travel terminal without the proper authorization.  Do you have your release documents on you?");
					#else
					tutorial->spatialChat(47513085685LLU, "Wait a minute there!  You can't use that travel terminal without the proper authorization.  Do you have your release documents on you?");
					#endif
				}
				else
				{
					tutorial->sendStartingLocationList();
				}
			}
			#if defined(_MSC_VER)
			else if (this->getId() == 4294968331)
			#else
			else if (this->getId() == 4294968331LLU)
			#endif
			{
				Tutorial* tutorial = playerObject->getTutorial();
				tutorial->sendStartingLocationList();
			}
		}
		else
		{
			// set the players current position, needed for handling the travelpointlistrequest
			playerObject->setTravelPoint(this);

			// bring up the terminal window
			//gMessageLib->sendHeartBeat(playerObject->getClient());
			gMessageLib->sendEnterTicketPurchaseModeMessage(this,playerObject);
		}
	}
	else
	{
		DLOG(INFO) << "TravelTerminal: Unhandled MenuSelect: " << messageType;
	}

}

//=============================================================================


void TravelTerminal::prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount)
{
    mRadialMenu = RadialMenuPtr(new RadialMenu());

    mRadialMenu->addItem(1,0,radId_itemUse,radAction_ObjCallback);
    mRadialMenu->addItem(2,0,radId_examine,radAction_Default);

}

