/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_CONNECTIONSERVER_CONNECTIONDISPATCH_H
#define ANH_CONNECTIONSERVER_CONNECTIONDISPATCH_H

//#include <WINSOCK2.h>
#include "Utils/typedefs.h"
#include <map>


//======================================================================================================================

class ConnectionDispatchCallback;
class ConnectionClient;
class Message;

typedef std::map<uint32,ConnectionDispatchCallback*>   MessageCallbackMap;

//======================================================================================================================

class ConnectionDispatch
{
	public:

		ConnectionDispatch(void);
		~ConnectionDispatch(void);

		void	Startup(void);
		void	Shutdown(void);
		void	Process(void);

		void	RegisterMessageCallback(uint32 opcode, ConnectionDispatchCallback* callback);
		void	UnregisterMessageCallback(uint32 opcode);

		void	handleIncomingMessage(ConnectionClient* client, Message* message);

	private:

		MessageCallbackMap              mMessageCallbackMap;
};


#endif //MMOSERVER_CONNECTIONSERVER_CONNECTIONDISPATCH_H



