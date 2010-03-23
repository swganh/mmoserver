/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_BURSTRUN_EVENT_H
#define ANH_ZONESERVER_BURSTRUN_EVENT_H

#include "Utils/EventHandler.h"




//======================================================================================================================

class BurstRunEvent : public Anh_Utils::Event
{
	public:

		BurstRunEvent(uint64 time, uint64 cooldown, bool coolDown = false) : mEndTime(time), mCoolDown(cooldown), mCDExpire(coolDown){}
		
		uint64	getEndTime()	const {return  mEndTime;}
		uint64	getCoolDown()	const {return  mCoolDown;}
		bool	getCDExpired()	const {return  mCDExpire;}


	private:

		uint64				mEndTime;
		uint64				mCoolDown;
		bool				mCDExpire;
};


//======================================================================================================================

#endif



