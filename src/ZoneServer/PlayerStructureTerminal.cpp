/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/
#include "PlayerStructureTerminal.h"
#include "Bank.h"

#include "PlayerObject.h"
#include "TangibleObject.h"
#include "TreasuryManager.h"
#include "Tutorial.h"
#include "UIManager.h"
#include "WorldConfig.h"
#include "WorldManager.h"
#include "MessageLib/MessageLib.h"
#include "LogManager/LogManager.h"
#include "DatabaseManager/Database.h"
#include "MathLib/Quaternion.h"


//=============================================================================

PlayerStructureTerminal::PlayerStructureTerminal() : Terminal()
{
	mRadialMenu = RadialMenuPtr(new RadialMenu());

	// any object with callbacks needs to handle those (received with menuselect messages) !
	mRadialMenu->addItem(1,0,radId_examine,radAction_ObjCallback,"@radial:examine");
	mRadialMenu->addItem(2,0,radId_serverTerminalManagement,radAction_Default,"@radial:management");
	mRadialMenu->addItem(3,0,radId_serverTerminalPermissions,radAction_ObjCallback, "@radial:permissions");

	
}

//=============================================================================

PlayerStructureTerminal::~PlayerStructureTerminal()
{
}

//=============================================================================

void PlayerStructureTerminal::handleObjectMenuSelect(uint8 messageType,Object* srcObject)
{
	PlayerObject* playerObject = (PlayerObject*)srcObject;

	if (playerObject && playerObject->isConnected())
	{
		// Fetch all items that can be insured.

	}
}


//=============================================================================

void PlayerStructureTerminal::handleUIEvent(uint32 action,int32 element,string inputStr,UIWindow* window)
{
	// gLogger->logMsgF("InsuranceTerminal::handleUIEvent You are here!",MSG_NORMAL);

	if(window == NULL)
	{
		return;
	}

	PlayerObject* playerObject = window->getOwner(); // window owner

	if(playerObject == NULL || !playerObject->isConnected() || playerObject->getSamplingState() || playerObject->isIncapacitated() || playerObject->isDead())
	{
		return;
	}
   /*
	switch(window->getWindowType())
	{
		
	

	

	

		default:
		{
		}
		break;
	}
	 */
	// gLogger->logMsgF("CloningTerminal::handleUIEvent You sure handled this UI-event!, Action = %d",MSG_NORMAL, action);
}
