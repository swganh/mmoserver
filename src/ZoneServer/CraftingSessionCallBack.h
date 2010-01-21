/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_CRAFTINGSESSION_CALLBACK_H
#define ANH_ZONESERVER_CRAFTINGSESSION_CALLBACK_H

#include "Utils/typedefs.h"


class CraftingSession;

//======================================================================================================================

class CraftingSessionCallback
{
	public:

		virtual void	handleCraftingSessionReady(CraftingSession* session) {}
};

//======================================================================================================================

#endif 

