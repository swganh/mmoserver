/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2010 The SWG:ANH Team
---------------------------------------------------------------------------------------
Use of this source code is governed by the GPL v3 license that can be found
in the COPYING file or at http://www.gnu.org/licenses/gpl-3.0.html

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
---------------------------------------------------------------------------------------
*/


#include "MessageDispatch.h"
#include "Message.h"
#include "MessageOpcodes.h"
#include "MessageFactory.h"
#include "DispatchClient.h"
#include "MessageDispatchCallback.h"
#include "NetworkManager/Service.h"
#include "NetworkManager/Session.h"
#include "NetworkManager/NetworkClient.h"
#include "LogManager/LogManager.h"


//#include <stdio.h>


//======================================================================================================================

MessageDispatch::MessageDispatch(Service* service) :
mRouterService(service)
{
	// Put ourselves on the service callback list.
	mRouterService->AddNetworkCallback(this);
}

//======================================================================================================================

MessageDispatch::~MessageDispatch(void)
{

}

//======================================================================================================================

void MessageDispatch::Process(void)
{

}

//======================================================================================================================

void MessageDispatch::RegisterMessageCallback(uint32 opcode, MessageDispatchCallback* callback)
{
	// Place our new callback in the map.
	mMessageCallbackMap.insert(std::make_pair(opcode,callback));
}

//======================================================================================================================


void MessageDispatch::UnregisterMessageCallback(uint32 opcode)
{
	// Remove our callback from the map.
	MessageCallbackMap::iterator iter;

	iter = mMessageCallbackMap.find(opcode);

	if(iter != mMessageCallbackMap.end())
	{
		mMessageCallbackMap.erase(iter);
	}
}

//======================================================================================================================

NetworkClient* MessageDispatch::handleSessionConnect(Session* session, Service* service)
{
	// MessageDispatch should never recieve new connections.
	return 0;
}


//======================================================================================================================
void MessageDispatch::handleSessionDisconnect(NetworkClient* client)
{

}

//======================================================================================================================

// Why is the session mutex used?
// Can we get called more than once at the same time?

void MessageDispatch::handleSessionMessage(NetworkClient* client, Message* message)
{

	DispatchClient* dispatchClient = 0;
	bool deleteClient = false;

    boost::recursive_mutex::scoped_lock lk(mSessionMutex);

	message->ResetIndex();

	// What kind of message is it?
	uint32 opcode;
	message->getUint32(opcode);

	// We want to intercept the opClusterClientConnect and opClusterClientDisconnect messages
	// so we can create account specific clients for use in async calls.
	if (opcode == opClusterClientConnect)
	{
		dispatchClient = new DispatchClient();
		dispatchClient->setAccountId(message->getAccountId());
		dispatchClient->setSession(client->getSession());
	
		mAccountClientMap.insert(std::make_pair(message->getAccountId(),dispatchClient));
	}
	else if (opcode == opClusterClientDisconnect)
	{
		// First find our DispatchClient.
		AccountClientMap::iterator iter = mAccountClientMap.find(message->getAccountId());

		if(iter != mAccountClientMap.end())
		{
			dispatchClient = (*iter).second;
			mAccountClientMap.erase(iter);

			gLogger->log(LogManager::DEBUG, "Destroying Dispatch Client for account %u.", message->getAccountId());

			// Mark it for deletion
			deleteClient = true;
	
		}
		else
		{
			gLogger->log(LogManager::NOTICE, "Could not find DispatchClient for account %u to be deleted.", message->getAccountId());

			client->getSession()->DestroyIncomingMessage(message);
            lk.unlock();

			return;
		}
	}
	else
	{
		AccountClientMap::iterator iter = mAccountClientMap.find(message->getAccountId());

		if(iter != mAccountClientMap.end())
		{
			dispatchClient = (*iter).second;
		}
		else
		{
			client->getSession()->DestroyIncomingMessage(message);

			lk.unlock();
			return;
		}
		/*
		else
		{
		  dispatchClient = new DispatchClient();
		  dispatchClient->setAccountId(message->getAccountId());
		  dispatchClient->setSession(client->getSession());
		  mAccountClientMap.insert(message->getAccountId(), dispatchClient);
		}
		*/
	}
	lk.unlock();

	MessageCallbackMap::iterator iter = mMessageCallbackMap.find(opcode);

	if(iter != mMessageCallbackMap.end())
	{
		// Reset our message index to just after the opcode.
		message->setIndex(4);

		// Call our handler
		(*iter).second->handleDispatchMessage(opcode, message, dispatchClient);
	}
	else
	{
		gLogger->log(LogManager::INFORMATION, "Unhandled opcode in MessageDispatch - 0x%x (%i)", opcode, opcode);
	}

	
	// Delete the client here if we got a disconnect.
	if(deleteClient)
	{
		// We will delete the client when we delete the player or reconnect again.
		//delete dispatchClient;
		dispatchClient = NULL;
	}

	// We need to destroy the incoming message for the session here
	// We want the application to decide whether the message is needed further or not.
	// This is mainly used in the ConnectionServer since routing messages need a longer life than normal
	message->setPendingDelete(true);
}

//======================================================================================================================
//
//	Create a sessionless dispatch client.
//
//	Clients created here may only receive data. Do NOT use when sending (the session is missing, you know).
// 
//======================================================================================================================

void MessageDispatch::registerSessionlessDispatchClient(uint32 accountId)
{
	// Verify not allready registred.
	AccountClientMap::iterator iter = mAccountClientMap.find(accountId);

	if (iter == mAccountClientMap.end())
	{
		DispatchClient* dispatchClient = new DispatchClient();
		dispatchClient->setAccountId(accountId);
		mAccountClientMap.insert(std::make_pair(accountId,dispatchClient));
	}
}


//======================================================================================================================
//
//	Remove a sessionless dispatch client.
//
// 
//======================================================================================================================
void MessageDispatch::unregisterSessionlessDispatchClient(uint32 accountId)
{
	// Verify not allready registred.
	AccountClientMap::iterator iter = mAccountClientMap.find(accountId);

	if (iter != mAccountClientMap.end())
	{
		// Delete the object referenced (pointed at).
		delete (*iter).second;

		// Delete actual storage and the items contained in the map.
		mAccountClientMap.erase(iter);
	}
}



