/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_CONNECTIONSERVER_CONNECTIONDISPATCHCALLBACK_H
#define ANH_CONNECTIONSERVER_CONNECTIONDISPATCHCALLBACK_H

#include "Utils/typedefs.h"


//======================================================================================================================

class Message;
class ConnectionClient;

//======================================================================================================================

class ConnectionDispatchCallback
{
	public:

	  virtual void	handleDispatchMessage(uint32 opcode, Message* message, ConnectionClient* client) = 0;
};

//======================================================================================================================

#endif // ANH_CONNECTIONSERVER_CONNECTIONDISPATCHCALLBACK_H



