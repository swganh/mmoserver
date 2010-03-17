/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "CraftingTool.h"
#include "MathLib/Quaternion.h"


//=============================================================================

CraftingTool::CraftingTool() : Item(),
mCurrentItem(NULL)
{
	
}

void CraftingTool::prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount)
{
	mRadialMenu	= RadialMenuPtr(new RadialMenu());

	mRadialMenu->addItem(1,0,radId_craftStart,radAction_Default);
	mRadialMenu->addItem(2,0,radId_examine,radAction_Default);
	mRadialMenu->addItem(3,0,radId_itemDestroy,radAction_Default);
}

//=============================================================================

CraftingTool::~CraftingTool()
{
}

//=============================================================================


