/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

---------------------------------------------------------------------------------------
*/
#include "BazaarTerminal.h"
#include "PlayerObject.h"
#include "LogManager/LogManager.h"
#include "MathLib/Quaternion.h"


//=============================================================================

BazaarTerminal::BazaarTerminal() : Terminal()
{
	mRadialMenu = RadialMenuPtr(new RadialMenu());
	
	// any object with callbacks needs to handle those (received with menuselect messages) !
	mRadialMenu->addItem(1,0,radId_terminalAuctionUse,radAction_ObjCallback);
	mRadialMenu->addItem(2,0,radId_examine,radAction_Default);
}


//=============================================================================

void BazaarTerminal::handleObjectMenuSelect(uint8 messageType,Object* srcObject)
{
	if(messageType == radId_itemUse)
	{
		
	}
	else
		gLogger->logMsgF("BazaarTerminal: Unhandled MenuSelect: %u",MSG_HIGH,messageType);
}


//=============================================================================

BazaarTerminal::~BazaarTerminal()
{
}

//=============================================================================


