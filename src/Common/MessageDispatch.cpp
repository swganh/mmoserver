/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

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

MessageDispatch::MessageDispatch(void)
{

}

//======================================================================================================================

MessageDispatch::~MessageDispatch(void)
{

}

//======================================================================================================================

void MessageDispatch::Startup(Service* service)
{
	// Put ourselves on the service callback list.
	mRouterService = service;
	mRouterService->AddNetworkCallback(this);
}

//======================================================================================================================

void MessageDispatch::Shutdown(void)
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
	message->mSourceId = 60;

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

			gLogger->logMsgF("Destroying DispatchClient for account %u.", MSG_NORMAL, message->getAccountId());

			// Mark it for deletion
			deleteClient = true;
	
		}
		else
		{
			gLogger->logMsgF("*** Could not find DispatchClient for account %u to be deleted.", MSG_NORMAL, message->getAccountId());
			gLogger->hexDump(message->getData(),message->getSize());

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
			//gLogger->logMsgF("*** Could not find DispatchClient for account %u to route message 0x%.8x", MSG_NORMAL, message->getAccountId(), opcode);
			//gLogger->hexDump(message->getData(),message->getSize());
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
		message->mSourceId = 61;
		(*iter).second->handleDispatchMessage(opcode, message, dispatchClient);
	}
	else
	{
		gLogger->logMsgF("Unhandled opcode in MessageDispatch - 0x%x (%i)", MSG_NORMAL, opcode, opcode);
	}

	
	// Delete the client here if we got a disconnect.
	if(deleteClient)
	{
		// We will delete the client when we delete the player or reconnect again.
		// delete dispatchClient;
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



