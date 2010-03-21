/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/
#include "ConnectionDispatch.h"

#include "ConnectionClient.h"
#include "ConnectionDispatchCallback.h"

#include "LogManager/LogManager.h"

#include "NetworkManager/Session.h"

#include "Common/Message.h"

#include <stdio.h>

//======================================================================================================================

ConnectionDispatch::ConnectionDispatch(void)
{

}

//======================================================================================================================

ConnectionDispatch::~ConnectionDispatch(void)
{

}

//======================================================================================================================

void ConnectionDispatch::Process(void)
{

}

//======================================================================================================================

void ConnectionDispatch::RegisterMessageCallback(uint32 opcode, ConnectionDispatchCallback* callback)
{
	// Place our new callback in the map.
	mMessageCallbackMap.insert(std::make_pair(opcode,callback));
}

//======================================================================================================================

void ConnectionDispatch::UnregisterMessageCallback(uint32 opcode)
{
	// Remove our callback from the map.
	ConnectionMessageCallbackMap::iterator iter = mMessageCallbackMap.find(opcode);

	if(iter != mMessageCallbackMap.end())
	{
		mMessageCallbackMap.erase(iter);
	}
}

//======================================================================================================================

void ConnectionDispatch::handleIncomingMessage(ConnectionClient* client, Message* message)
{
	// Get our account id so we know who this is.
	message->setIndex(0);

	// What kind of message is it?
	uint32 opcode;
	message->getUint32(opcode);

	ConnectionMessageCallbackMap::iterator iter = mMessageCallbackMap.find(opcode);

	if(iter != mMessageCallbackMap.end())
	{
		// Reset our message index to just after the opcode.
		message->setIndex(4);

		// Call our handler
		(*iter).second->handleDispatchMessage(opcode, message, client);
	}
	else
	{
		gLogger->logMsgF("Unhandled opcode in ConnectionDispatch - 0x%x (%i)",MSG_NORMAL,opcode,opcode);
	}

	// Delete our message
	client->getSession()->DestroyIncomingMessage(message);
}

//======================================================================================================================



