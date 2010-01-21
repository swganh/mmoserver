/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_TANGIBLE_WEARABLE_H
#define ANH_ZONESERVER_TANGIBLE_WEARABLE_H

#include "Item.h"

//=============================================================================

class Wearable : public Item
{
	friend class ItemFactory;

	public:

		Wearable();
		virtual ~Wearable();
		string	getAuctionDetails();

		void	sendAttributes(PlayerObject* playerObject);

	private:
};

//=============================================================================

#endif

