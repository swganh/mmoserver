/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_TANGIBLE_CRAFTINGTOOL_H
#define ANH_ZONESERVER_TANGIBLE_CRAFTINGTOOL_H

#include "Item.h"

//=============================================================================

class CraftingTool : public Item
{
	friend class ItemFactory;

	public:

		CraftingTool();
		virtual ~CraftingTool();
	
		Item*	getCurrentItem(){ return mCurrentItem; }
		void	setCurrentItem(Item* item){ mCurrentItem = item; }
		virtual void	prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount);

	private:

		Item*	mCurrentItem;

};

//=============================================================================

#endif

