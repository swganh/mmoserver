/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_TANGIBLE_FURNITURE_H
#define ANH_ZONESERVER_TANGIBLE_FURNITURE_H

#include "Item.h"

//=============================================================================

class Furniture : public Item
{
	friend class ItemFactory;

	public:

		Furniture();
		virtual ~Furniture();

		virtual void	prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount);
		virtual void	handleObjectMenuSelect(uint8 messageType,Object* srcObject);
		
	private:
};

//=============================================================================

#endif

