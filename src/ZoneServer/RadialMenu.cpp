/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "RadialMenu.h"

//=======================================================================

RadialMenu::RadialMenu()
{
}

//=======================================================================

RadialMenu::~RadialMenu()
{
	RadialItemList::iterator it = mItemList.begin();
	while(it != mItemList.end())
	{
		delete(*it);
		mItemList.erase(it);
		it = mItemList.begin();
	}
}

//=======================================================================

void RadialMenu::addItem(uint8 index,uint8 parentItem,RadialIdentifier identifier,uint8 action,const int8* description)
{
	mItemList.push_back(new RadialMenuItem(index,parentItem,identifier,action,description));
}

//=======================================================================
