/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_LOGOUT_EVENT_H
#define ANH_ZONESERVER_LOGOUT_EVENT_H

#include "Utils/EventHandler.h"




//======================================================================================================================

class LogOutEvent : public Anh_Utils::Event
{
	public:

		LogOutEvent(uint64 time, uint32 spacer) : mLogOutTime(time), mLogOutSpacer(spacer){}
		
		uint64 getLogOutTime() const {return  mLogOutTime;}
		uint32 getLogOutSpacer() const {return  mLogOutSpacer;}


	private:

		uint64				mLogOutTime;
		uint32				mLogOutSpacer;
};


//======================================================================================================================

#endif



