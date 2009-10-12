/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_TANGIBLE_CRAFTINGSTATION_H
#define ANH_ZONESERVER_TANGIBLE_CRAFTINGSTATION_H

#include "Item.h"

//=============================================================================

class CraftingStation : public Item
{
	friend class ItemFactory;

	public:

		CraftingStation();
		virtual ~CraftingStation();

	private:

};

//=============================================================================

#endif

