/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_TANGIBLE_BAZAARTERMINAL_H
#define ANH_ZONESERVER_TANGIBLE_BAZAARTERMINAL_H

#include "Terminal.h"

//=============================================================================

class BazaarTerminal : public Terminal
{
	friend class TerminalFactory;

	public:

		BazaarTerminal();
		~BazaarTerminal();

		void		handleObjectMenuSelect(uint8 messageType,Object* srcObject);

	private:
};

//=============================================================================

#endif

