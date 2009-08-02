/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_TANGIBLE_MISSIONTERMINAL_H
#define ANH_ZONESERVER_TANGIBLE_MISSIONTERMINAL_H

#include "Terminal.h"

//=============================================================================

class MissionTerminal : public Terminal
{
	friend class TerminalFactory;

	public:

		MissionTerminal();
		~MissionTerminal();
		virtual void prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount);

	private:
};

//=============================================================================

#endif

