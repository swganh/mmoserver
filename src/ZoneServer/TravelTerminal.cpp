/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "MathLib/Quaternion.h"
#include "PlayerObject.h"
#include "MessageLib/MessageLib.h"
#include "TravelTerminal.h"
#include "LogManager/LogManager.h"
#include "WorldConfig.h"
#include "Tutorial.h"

//=============================================================================

TravelTerminal::TravelTerminal() : Terminal()
{
	mRadialMenu = RadialMenuPtr(new RadialMenu());
	
	// any object with callbacks needs to handle those (received with menuselect messages) !
	mRadialMenu->addItem(1,0,radId_itemUse,radAction_ObjCallback);
	mRadialMenu->addItem(2,0,radId_examine,radAction_Default);
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

		if(!playerObject || !playerObject->isConnected() || playerObject->getSamplingState() || playerObject->isIncapacitated() || playerObject->isDead())
		{
			return;
		}

		if (gWorldConfig->isTutorial())
		{
			if (this->getId() == 4294968329)
			{
				// This is the Travel Terminal in the tutorial.
				Tutorial* tutorial = playerObject->getTutorial();

				// We will not display this when tutorial is in state 26 or above.
				if (tutorial->getSubState() < 26)
				{
					// The quatermaster speaks...
					tutorial->spatialChat(47513085685, "Wait a minute there!  You can't use that travel terminal without the proper authorization.  Do you have your release documents on you?");
				}
				else
				{
					tutorial->sendStartingLocationList();
				}
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
		gLogger->logMsgF("TravelTerminal: Unhandled MenuSelect: %u",MSG_HIGH,messageType);
	}
}

//=============================================================================

