/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_ITEMDELETE_EVENT_H
#define ANH_ZONESERVER_ITEMDELETE_EVENT_H

#include "Utils/EventHandler.h"




//======================================================================================================================

class ItemDeleteEvent : public Anh_Utils::Event
{
	public:

		ItemDeleteEvent(uint64 time, uint64 item, bool db = true) : mExecuteTime(time), mItem(item), mDB(db){}
		
		uint64	getExecuteTime()	const {return  mExecuteTime;}
		uint64	getItem()			const {return  mItem;}
		bool	geDB()				const {return  mDB;}


	private:

		uint64				mExecuteTime;
		uint64				mItem;
		bool				mDB;
};


//======================================================================================================================

#endif



