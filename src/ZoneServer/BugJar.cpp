/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "BugJar.h"
#include "PlayerObject.h"
#include "MessageLib/MessageLib.h"

//=============================================================================

BugJar::BugJar() : Item()
{
}


//=============================================================================

BugJar::~BugJar(void)
{
}

//=============================================================================

void BugJar::handleObjectMenuSelect(uint8 messageType,Object* srcObject)
{
}

void BugJar::prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount)
{
	mRadialMenu = RadialMenuPtr(new RadialMenu());
	mRadialMenu->addItem(1,0,radId_examine,radAction_ObjCallback,"");
	mRadialMenu->addItem(2,0,radId_itemDestroy,radAction_ObjCallback,"");
}