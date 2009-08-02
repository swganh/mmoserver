/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "EventHandler.h"


//======================================================================================================================

Anh_Utils::EventHandler::~EventHandler()
{
	mHandlers.clear();
}

//======================================================================================================================

void Anh_Utils::EventHandler::handleEvent(const Anh_Utils::Event* event)
{
	Handlers::iterator it = mHandlers.find(Anh_Utils::TypeInfo(typeid(*event)));

	if(it != mHandlers.end())
	{
		it->second->execute(event);
	}
}

//======================================================================================================================


