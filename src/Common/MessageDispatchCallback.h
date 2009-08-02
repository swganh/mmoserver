/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_COMMON_MESSAGEDISPATCHCALLBACK_H
#define ANH_COMMON_MESSAGEDISPATCHCALLBACK_H

#include "Utils/typedefs.h"


//======================================================================================================================

class Message;
class DispatchClient;

//======================================================================================================================

class MessageDispatchCallback
{
	public:
		virtual void	handleDispatchMessage(uint32 opcode, Message* message, DispatchClient* client) = 0;

	private:

};

//======================================================================================================================

#endif //MMOSERVER_COMMON_MESSAGEDISPATCHCALLBACK_H


