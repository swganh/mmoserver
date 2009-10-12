/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_TANGIBLE_CLONINGTERMINAL_H
#define ANH_ZONESERVER_TANGIBLE_CLONINGTERMINAL_H

#include "Terminal.h"

//=============================================================================

class CloningTerminal : public Terminal
{
	friend class TerminalFactory;

	public:

		CloningTerminal();
		~CloningTerminal();

		void		handleObjectMenuSelect(uint8 messageType,Object* srcObject);
		void		handleUIEvent(uint32 action,int32 element,string inputStr,UIWindow* window);

	private:

	
		

};

//=============================================================================

#endif

