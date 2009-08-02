/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "RadialMenuItem.h"


RadialMenuItem::RadialMenuItem(uint8 index,uint8 parentItem,RadialIdentifier identifier,uint8 action,const int8* description)
:mIndex(index),mParentItem(parentItem),mIdentifier(identifier),mAction(action)
{
	mExtendedDescription = description;
}

//=============================================================================

RadialMenuItem::~RadialMenuItem()
{
}

//=============================================================================

