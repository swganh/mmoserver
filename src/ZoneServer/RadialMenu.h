/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_RADIALMENU_H
#define ANH_ZONESERVER_RADIALMENU_H

#include "RadialMenuItem.h"
#include <vector>

typedef std::vector<RadialMenuItem*>	RadialItemList;

class RadialMenu
{
	public:
		RadialMenu();
		~RadialMenu();

		void			addItem(uint8 index,uint8 parentItem,RadialIdentifier identifier,uint8 action,const int8* description = "");
		RadialItemList*	getItemList(){ return &mItemList; }

	private:

		RadialItemList	mItemList;
};

#endif


