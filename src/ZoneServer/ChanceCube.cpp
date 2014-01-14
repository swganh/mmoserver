/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "ChanceCube.h"
#include "PlayerObject.h"
#include "MessageLib/MessageLib.h"

//=============================================================================

ChanceCube::ChanceCube() : Item()
{
	mRadialMenu = RadialMenuPtr(new RadialMenu());

	mRadialMenu->addItem(1,0,radId_itemUse,radAction_ObjCallback);
	mRadialMenu->addItem(2,0,radId_examine,radAction_Default);
}


//=============================================================================

ChanceCube::~ChanceCube(void)
{
}

//=============================================================================

void ChanceCube::handleObjectMenuSelect(uint8 messageType,Object* srcObject)
{
	if(PlayerObject* playerObject = dynamic_cast<PlayerObject*>(srcObject))
	{	
		if(messageType == radId_itemUse)
		{
			gMessageLib->sendSystemMessage(playerObject,L"","dice","red");
		}
	}
}