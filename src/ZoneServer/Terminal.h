/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_TANGIBLE_TERMINAL_H
#define ANH_ZONESERVER_TANGIBLE_TERMINAL_H

#include "TangibleObject.h"

//=============================================================================

class Terminal : public TangibleObject
{
	friend class TerminalFactory;

	public:

		Terminal();
		~Terminal();

		uint32			getTerminalType(){return mTerminalType;}
		void			setTerminalType(uint32 type ){mTerminalType = type;}
	

	private:
		uint32			mTerminalType;
	
};

//=============================================================================

#endif

