/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_TANGIBLE_CHANCECUBE_H
#define ANH_ZONESERVER_TANGIBLE_CHANCECUBE_H

#include "Item.h"

//=============================================================================

class ChanceCube : public Item
{
	friend class ItemFactory;

	public:

		ChanceCube();
		virtual ~ChanceCube(void);

		void	handleObjectMenuSelect(uint8 messageType,Object* srcObject);
};

//=============================================================================

#endif;