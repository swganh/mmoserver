/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/
#include "TravelTerminal.h"
#include "PlayerObject.h"
#include "Tutorial.h"
#include "WorldConfig.h"
#include "MessageLib/MessageLib.h"
#include "LogManager/LogManager.h"
#include "MathLib/Quaternion.h"

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

		if(!playerObject || !playerObject->isConnected() || playerObject->getSamplingState() || playerObject->isIncapacitated() || playerObject->isDead() || playerObject->checkState(CreatureState_Combat))
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


void TravelTerminal::prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount)
{
	mRadialMenu = RadialMenuPtr(new RadialMenu());
	
	mRadialMenu->addItem(1,0,radId_itemUse,radAction_ObjCallback);
	mRadialMenu->addItem(2,0,radId_examine,radAction_Default);
	
}

