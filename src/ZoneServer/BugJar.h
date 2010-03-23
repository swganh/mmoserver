/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_TANGIBLE_BUGJAR_H
#define ANH_ZONESERVER_TANGIBLE_BUGJAR_H

#include "Item.h"

//=============================================================================

class BugJar : public Item
{
	friend class ItemFactory;

	public:

		BugJar();
		virtual ~BugJar(void);

		void	handleObjectMenuSelect(uint8 messageType,Object* srcObject);
		virtual void	ResetRadialMenu(){;}//RadialMenu* radial	= NULL;RadialMenuPtr radialPtr(radial);	mRadialMenu = radialPtr;}
		void	prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount);
};

//=============================================================================

#endif;