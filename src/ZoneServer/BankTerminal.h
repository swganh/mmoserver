/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_TANGIBLE_BANKTERMINAL_H
#define ANH_ZONESERVER_TANGIBLE_BANKTERMINAL_H

#include "Terminal.h"

//=============================================================================

class BankTerminal : public Terminal
{
	friend class TerminalFactory;

	public:

		BankTerminal();
		~BankTerminal();

		void			prepareRadialMenu();
		virtual void	prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount);
		void			handleObjectMenuSelect(uint8 messageType,Object* srcObject);
		void			handleUIEvent(string inventoryCash, string bankCash, UIWindow* window);

	private:




};

//=============================================================================

#endif

