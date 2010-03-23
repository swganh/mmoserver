/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_RADIALMENU_ITEM_H
#define ANH_ZONESERVER_RADIALMENU_ITEM_H

#include "RadialEnums.h"
#include "Utils/typedefs.h"

class RadialMenuItem
{
	public:
		RadialMenuItem(uint8 mIndex,uint8 parentItem,RadialIdentifier identifier,uint8 action,const int8* description);
		~RadialMenuItem();

		uint8				mIndex;
		uint8				mParentItem;
		RadialIdentifier	mIdentifier;
		uint8				mAction;
		string				mExtendedDescription;
};

#endif


