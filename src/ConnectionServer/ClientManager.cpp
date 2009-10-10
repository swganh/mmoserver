/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

---------------------------------------------------------------------------------------
*/
#include "ClientManager.h"
#include "ConnectionClient.h"
#include "ConnectionDispatch.h"
#include "MessageRouter.h"

#include "NetworkManager/Service.h"

#include "LogManager/LogManager.h"

#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"

#include "Common/Message.h"
#include "Common/MessageFactory.h"
#include "Common/MessageOpcodes.h"

#include "ConfigManager/ConfigManager.h"

//======================================================================================================================

ClientManager::ClientManager(void)
{

}

//======================================================================================================================

ClientManager::~ClientManager(void)
{

}

//======================================================================================================================

void ClientManager::Startup(Service* service, Database* database, MessageRouter* router, ConnectionDispatch* dispatch)
{
	// Set our member variables
	mClientService          = service;
	mDatabase               = database;
	mMessageRouter          = router;
	mConnectionDispatch     = dispatch;
	mMessageRouter->setClientManager(this);

	// Put ourselves on the callback list for this service.
	mClientService->AddNetworkCallback(this);

	// Register our opcodes
	mConnectionDispatch->RegisterMessageCallback(opClientIdMsg, this);
	mConnectionDispatch->RegisterMessageCallback(opSelectCharacter, this);
	mConnectionDispatch->RegisterMessageCallback(opClusterZoneTransferCharacter, this);
}

//======================================================================================================================

void ClientManager::Shutdown(void)
{
	// Unregister our opcodes
	mConnectionDispatch->UnregisterMessageCallback(opClientIdMsg);
	mConnectionDispatch->UnregisterMessageCallback(opSelectCharacter);
	mConnectionDispatch->UnregisterMessageCallback(opClusterZoneTransferCharacter);
}

//======================================================================================================================

void ClientManager::Process(void)
{

}

//======================================================================================================================

void ClientManager::SendMessageToClient(Message* message)
{
	// Find our client from the accountId.
	
	mServiceMutex.acquire();
	PlayerClientMap::iterator iter = mPlayerClientMap.find(message->getAccountId());

	// We're headed to the client, don't use the routing header.
	message->setRouted(false);

	// If we found the client, send the data.
	if (iter != mPlayerClientMap.end())
	{
		ConnectionClient* client = (*iter).second;
		client->SendChannelA(message, message->getPriority(), message->getFastpath());
	}
	else
	{
		//gLogger->logMsgF("ClientManager: failed routing message to client %u",MSG_NORMAL,message->getAccountId());
		//happens when the client logs out 

		//if(!message->getAccountId())
			//gLogger->hexDump(message->getData(),message->getSize());

		gMessageFactory->DestroyMessage(message);
	}
	mServiceMutex.release();
}

//======================================================================================================================
//
// handleserverdown
//

void ClientManager::handleServerDown(uint32 serverId)
{
	// disconnect all clients that were on this server, if its a zone
	if(serverId >= 8)
	{
		mServiceMutex.acquire();
		PlayerClientMap::iterator it = mPlayerClientMap.begin();

		while(it != mPlayerClientMap.end())
		{
			ConnectionClient* client = (*it).second;

			if(client->getServerId() == serverId)
			{
				client->Disconnect(0);
			}

			++it;
		}
		mServiceMutex.release();
	}
	
}

//======================================================================================================================

NetworkClient* ClientManager::handleSessionConnect(Session* session, Service* service)
{
	// Create a new client for the network.
	ConnectionClient* newClient = new ConnectionClient();

	return reinterpret_cast<NetworkClient*>(newClient);
}

//======================================================================================================================

void ClientManager::handleSessionDisconnect(NetworkClient* client)
{
	Message* message;
	ConnectionClient* connClient = reinterpret_cast<ConnectionClient*>(client);

	// Create a ClusterClientDisconnect message and send it to the servers
	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opClusterClientDisconnect);
	gMessageFactory->addUint32(0);                        // Reason: Disconnected
	message = gMessageFactory->EndMessage();

	message->setAccountId(connClient->getAccountId());
	message->setDestinationId(CR_Chat);  // chat server
	message->setRouted(true);
	mMessageRouter->RouteMessage(message, connClient);

	// Create a ClusterClientDisconnect message and send it to the servers
	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opClusterClientDisconnect);
	gMessageFactory->addUint32(0);                        // Reason: Disconnected
	message = gMessageFactory->EndMessage();

	message->setAccountId(connClient->getAccountId());
	message->setDestinationId(static_cast<uint8>(connClient->getServerId()));  // zone server
	message->setRouted(true);
	mMessageRouter->RouteMessage(message, connClient);

	// Update the account record that the account is logged out.
	mDatabase->ExecuteSqlAsync(0, 0, "UPDATE account SET loggedin=0 WHERE account_id=%u;", connClient->getAccountId());

	// Client has disconnected.
	mServiceMutex.acquire();
	PlayerClientMap::iterator iter = mPlayerClientMap.find(connClient->getAccountId());

	if(iter != mPlayerClientMap.end())
	{
		mPlayerClientMap.erase(iter);
	}
	mServiceMutex.release();
}


//======================================================================================================================
void ClientManager::handleSessionMessage(NetworkClient* client, Message* message)
{ 
  ConnectionClient* connClient = reinterpret_cast<ConnectionClient*>(client);

  // Assign our account info to this message, then route it.
  message->setAccountId(connClient->getAccountId());

  // Dispatch this message to the router
  mMessageRouter->RouteMessage(message, connClient);
}


//======================================================================================================================
void ClientManager::handleDispatchMessage(uint32 opcode, Message* message, ConnectionClient* client)
{
  switch(opcode)         
  {
  case opClientIdMsg:
    {
      _processClientIdMsg(client, message);
      break;
    }
  case opSelectCharacter:
    {
      _processSelectCharacter(client, message);
      break;
    }
  case opClusterZoneTransferCharacter:
    {
      _processClusterZoneTransferCharacter(client, message);
      break;
    }
  }
}


//======================================================================================================================
void ClientManager::handleDatabaseJobComplete(void* ref, DatabaseResult* result)
{
  // This assumes only authentication calls are async right now.  Will change as needed.
  ConnectionClient* client = reinterpret_cast<ConnectionClient*>(ref);

  switch (client->getState())
  {
  case CCSTATE_QueryAuth:
    {
      _handleQueryAuth(client, result);
      break;
    }
  }
}


//======================================================================================================================
void ClientManager::_processClientIdMsg(ConnectionClient* client, Message* message)
{ 
  // We only need the account data that is at the end of the message.
  message->getUint32();  // unkown.
  uint32 dataSize = message->getUint32();
  message->setIndex(message->getIndex() + (uint16)dataSize - 4);
  client->setAccountId(message->getUint32());
  
  // Start our auth query
  client->setState(CCSTATE_QueryAuth);
  mDatabase->ExecuteSqlAsync(this, (void*)client, "SELECT * FROM account WHERE account_id=%u AND authenticated=1 AND loggedin=0;", client->getAccountId());  
}


//======================================================================================================================
void ClientManager::_processSelectCharacter(ConnectionClient* client, Message* message)
{
  uint64 characterId = message->getUint64();
  
  DatabaseResult* result = mDatabase->ExecuteSynchSql("SELECT planet_id FROM characters WHERE id=%I64u;", characterId);

  uint32 serverId;
  DataBinding* binding = mDatabase->CreateDataBinding(1);
  binding->addField(DFT_uint32, 0, 4);
  result->GetNextRow(binding, &serverId);

  client->setServerId(serverId + 8);  // server ids for zones are planetId + 8;

  mDatabase->DestroyDataBinding(binding);
  mDatabase->DestroyResult(result);

  // send an opClusterClientConnect message to zone server.
  gMessageFactory->StartMessage();
  gMessageFactory->addUint32(opClusterClientConnect);
  gMessageFactory->addUint64(characterId);
  Message* zoneMessage = gMessageFactory->EndMessage();

  // This one goes to the ZoneServer the client is currently on.
  zoneMessage->setAccountId(client->getAccountId());
  zoneMessage->setDestinationId(static_cast<uint8>(serverId + 8));
  zoneMessage->setRouted(true);
  mMessageRouter->RouteMessage(zoneMessage, client);

  // send an opClusterClientConnect message to chat server.
  gMessageFactory->StartMessage();
  gMessageFactory->addUint32(opClusterClientConnect);
  gMessageFactory->addUint64(characterId);
  gMessageFactory->addUint32(serverId);
  Message* chatMessage = gMessageFactory->EndMessage();

  // This one goes to the ChatServer
  chatMessage->setAccountId(client->getAccountId());
  chatMessage->setDestinationId(CR_Chat);
  chatMessage->setRouted(true);
  mMessageRouter->RouteMessage(chatMessage, client);

  // Now send the SelectCharacter message off to the zone server.
  gMessageFactory->StartMessage();
  gMessageFactory->addData(message->getData(), message->getSize());
  Message* selectMessage = gMessageFactory->EndMessage();

  selectMessage->setAccountId(client->getAccountId());
  selectMessage->setDestinationId(static_cast<uint8>(serverId + 8));
  selectMessage->setRouted(true);
  mMessageRouter->RouteMessage(selectMessage, client);
}


//======================================================================================================================
void ClientManager::_processClusterZoneTransferCharacter(ConnectionClient* client, Message* message)
{
  uint64 characterId = message->getUint64();
  uint32 newPlanetId = message->getUint32(); 
  uint32 oldServerId = 0;

  // Update our client
  mServiceMutex.acquire();
  PlayerClientMap::iterator iter;
  iter = mPlayerClientMap.find(message->getAccountId());
  if (iter != mPlayerClientMap.end())
  {
    ConnectionClient* connClient = (*iter).second;

    oldServerId = connClient->getServerId(); 
    connClient->setServerId(newPlanetId + 8);

    // send an opClusterClientDisconnnect message to the old zone server.
    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opClusterClientDisconnect);
    gMessageFactory->addUint32(1);                        // Reason: Zone transfer
    Message* oldZoneMessage = gMessageFactory->EndMessage();

    // This one goes to
    oldZoneMessage->setAccountId(connClient->getAccountId());
    oldZoneMessage->setDestinationId(static_cast<uint8>(oldServerId));        // zoneIds are planetIds + 8
    oldZoneMessage->setRouted(true);
    mMessageRouter->RouteMessage(oldZoneMessage, client);


    // send an opClusterClientConnect message to the new zone server.
    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opClusterClientConnect);
    Message* newZoneMessage = gMessageFactory->EndMessage();

    // This one goes to
    newZoneMessage->setAccountId(connClient->getAccountId());
    newZoneMessage->setDestinationId(static_cast<uint8>(newPlanetId + 8));   // zoneIds are planetIds + 8
    newZoneMessage->setRouted(true);
    mMessageRouter->RouteMessage(newZoneMessage, client);

    // send an opSelectCharacter message to the new zone server.
    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opSelectCharacter);
    gMessageFactory->addUint64(characterId);
    newZoneMessage = gMessageFactory->EndMessage();

    // This one goes to
    newZoneMessage->setAccountId(connClient->getAccountId());
    newZoneMessage->setDestinationId(static_cast<uint8>(newPlanetId + 8));   // zoneIds are planetIds + 8
    newZoneMessage->setRouted(true);
    mMessageRouter->RouteMessage(newZoneMessage, client);

	// notify the chatserver
	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opClusterZoneTransferCharacter);
	gMessageFactory->addUint32(newPlanetId);
	newZoneMessage = gMessageFactory->EndMessage();

	newZoneMessage->setAccountId(message->getAccountId());
	newZoneMessage->setDestinationId(CR_Chat);
	newZoneMessage->setRouted(true);
	mMessageRouter->RouteMessage(newZoneMessage,client);
  }
  else 
  {
    // client may have disconnected right in the middle of the transfer
    gLogger->logMsg("*** Client not found during zone transfer.\n");
  }
  mServiceMutex.release();
}


//======================================================================================================================
void ClientManager::_handleQueryAuth(ConnectionClient* client, DatabaseResult* result)
{
  // If there are any results, this account is properly authenticated.
  if (result->getRowCount())
  {
    // Update the account record that it is now logged in and last login date.
    mDatabase->ExecuteSqlAsync(0, 0, "UPDATE account SET lastlogin=NOW(), loggedin=%u WHERE account_id=%u;", gConfig->read<uint32>("ClusterId"), client->getAccountId());

    // finally add them to our accountId map.
	mServiceMutex.acquire();
	mPlayerClientMap.insert(std::make_pair(client->getAccountId(), client));
	mServiceMutex.release();

    // send an opClusterClientConnect message to admin server.
    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opClusterClientConnect);
    gMessageFactory->addUint64(0);        //We don't have a characterId yet.
    Message* adminMessage = gMessageFactory->EndMessage();

    // This one goes to the AdminServer
    adminMessage->setAccountId(client->getAccountId());
    adminMessage->setDestinationId(CR_Admin);
    adminMessage->setRouted(true);
    mMessageRouter->RouteMessage(adminMessage, client);

    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opClientPermissionsMessage);  
    gMessageFactory->addUint8(1);             // unknown
    gMessageFactory->addUint8(1);             // Character creation allowed?
    gMessageFactory->addUint8(1);
    Message* message = gMessageFactory->EndMessage();

    // Send our message to the client.
    client->SendChannelA(message, 4, false);
  }
  // They are not authenticated to the login server, so disconnect them.
  else
  {
    client->Disconnect(10);  // no idea if 10 is even a valid reason, just testing.
  }
}


