/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_TANGIBLE_SCOUT_H
#define ANH_ZONESERVER_TANGIBLE_SCOUT_H

#include "Item.h"

//=============================================================================

class Scout : public Item
{
	friend class ItemFactory;

	public:

		Scout();
		virtual ~Scout();
		

		virtual void	prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount);
		virtual void	handleObjectMenuSelect(uint8 messageType,Object* srcObject);
		void			sendAttributes(PlayerObject* playerObject);

	private:
};

//=============================================================================

#endif