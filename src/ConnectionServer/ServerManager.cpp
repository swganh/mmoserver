/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

---------------------------------------------------------------------------------------
*/
#include "ServerManager.h"

#include "ClientManager.h"
#include "ConnectionClient.h"
#include "ConnectionDispatch.h"
#include "ConnectionDispatchClient.h"
#include "MessageRouter.h"

#include "NetworkManager/Session.h"
#include "NetworkManager/Service.h"

#include "LogManager/LogManager.h"

#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"

#include "Common/Message.h"
#include "Common/MessageOpcodes.h"
#include "Common/MessageFactory.h"
#include "ConfigManager/ConfigManager.h"

#include <cstring>

//======================================================================================================================

ServerManager::ServerManager(void) :
mMessageRouter(0),
mServerService(0),
mDatabase(0),
mConnectionDispatch(0),
mTotalActiveServers(0),
mTotalConnectedServers(0)
{
	memset(&mServerAddressMap, 0, sizeof(mServerAddressMap));
}

//======================================================================================================================

ServerManager::~ServerManager(void)
{

}

//======================================================================================================================

void ServerManager::Startup(Service* service, Database* database, MessageRouter* router, ConnectionDispatch* dispatch,ClientManager* clientManager)
{
	// Set our member variables
	mServerService        = service;
	mDatabase             = database;
	mMessageRouter        = router;
	mConnectionDispatch   = dispatch;
	mClientManager		  = clientManager;
	mMessageRouter->setServerManager(this);

	// Put ourselves on the callback list for this service.
	mServerService->AddNetworkCallback(this);

	// Register any messages we need to handle from tha backend servers
	mConnectionDispatch->RegisterMessageCallback(opClusterRegisterServer, this);
	mConnectionDispatch->RegisterMessageCallback(opClusterZoneTransferRequestByTicket, this);
	mConnectionDispatch->RegisterMessageCallback(opClusterZoneTransferRequestByPosition, this);

	// Update our id
	mClusterId = gConfig->read<uint32>("ClusterId");

	// setup data bindings
	_setupDataBindings();

	// load process address map
	_loadProcessAddressMap();
}

//======================================================================================================================

void ServerManager::Shutdown(void)
{
	mConnectionDispatch->UnregisterMessageCallback(opClusterRegisterServer);
	mConnectionDispatch->UnregisterMessageCallback(opClusterZoneTransferRequestByTicket);
	mConnectionDispatch->UnregisterMessageCallback(opClusterZoneTransferRequestByPosition);

	_destroyDataBindings();
}

//======================================================================================================================

void ServerManager::Process(void)
{

}

//======================================================================================================================

void ServerManager::SendMessageToServer(Message* message)
{
	message->setRouted(true);

	if(mServerAddressMap[message->getDestinationId()].mConnectionClient)
	{
		mServerAddressMap[message->getDestinationId()].mConnectionClient->SendChannelA(message, message->getPriority(), message->getFastpath());
	}
	else
	{
		gLogger->logMsgF("ServerManager: failed routing message to server %u",MSG_NORMAL,message->getDestinationId());
		gMessageFactory->DestroyMessage(message);
	}
}

//======================================================================================================================

NetworkClient* ServerManager::handleSessionConnect(Session* session, Service* service)
{
	NetworkClient*	newClient = 0;
	ServerAddress	serverAddress;

//	int8 address[32] = boost::asio::ip::address::to_string()(endpoint.address();//(endpoint.address);
	// Execute our statement
	int8 sql[500];
	sprintf(sql,"SELECT id, address, port, status, active FROM config_process_list WHERE address='%s' AND port=%u;", session->getAddressString(), session->getPortHost());
	DatabaseResult* result = mDatabase->ExecuteSynchSql(sql);
	gLogger->logMsgF(sql,MSG_HIGH);
	gLogger->logMsg("\n");
	// If we found them
	if(result->getRowCount() == 1)
	{
		// Retrieve our routes and add them to the map.
		result->GetNextRow(mServerBinding,&serverAddress);

		// put this fresh data in our list.
		newClient = new ConnectionClient();
		ConnectionClient* connClient = reinterpret_cast<ConnectionClient*>(newClient);

		connClient->setServerId(serverAddress.mId);

		memcpy(&mServerAddressMap[serverAddress.mId], &serverAddress, sizeof(ServerAddress));
		mServerAddressMap[serverAddress.mId].mConnectionClient = connClient;

		// If this is one of the servers we're waiting for, then update our count
		if(mServerAddressMap[serverAddress.mId].mActive)
		{
			++mTotalConnectedServers;

			if(mTotalConnectedServers == mTotalActiveServers)
			{
				mDatabase->ExecuteSqlAsync(0,0,"UPDATE galaxy SET status=2, last_update=NOW() WHERE galaxy_id=%u;", mClusterId);
			}
		}
	}
	else
	{
		gLogger->logMsg("*** Backend server connect error - Server not found in DB\n");
	}

	// Delete our DB objects.
	mDatabase->DestroyResult(result);

	return(newClient);
}


//======================================================================================================================
//
// TODO: update the server stats, get rid of clients that were connected to the dropped server
//

void ServerManager::handleSessionDisconnect(NetworkClient* client)
{
	ConnectionClient* connClient = reinterpret_cast<ConnectionClient*>(client);

	// Server disconnected.  But don't remove the mapping if it's not the same one.
	if(mServerAddressMap[connClient->getServerId()].mConnectionClient == connClient)
	{
		mServerAddressMap[connClient->getServerId()].mConnectionClient = 0;
	}

	// update the galaxy state
	if(mServerAddressMap[connClient->getServerId()].mActive)
	{
		--mTotalConnectedServers;

		mDatabase->ExecuteSqlAsync(0,0,"UPDATE galaxy SET status=1,last_update=NOW() WHERE galaxy_id=%u;", mClusterId);
	}

	gLogger->logMsgF("Servermanager handle server down\n", MSG_HIGH);
	mClientManager->handleServerDown(connClient->getServerId());

	delete(client);
}


//======================================================================================================================
void ServerManager::handleSessionMessage(NetworkClient* client, Message* message)
{
  ConnectionClient* connClient = reinterpret_cast<ConnectionClient*>(client);
  message->setSourceId(static_cast<uint8>(connClient->getServerId()));

  // Send the message off to the router.
  mMessageRouter->RouteMessage(message,connClient);
}

//======================================================================================================================

void ServerManager::handleDispatchMessage(uint32 opcode,Message* message,ConnectionClient* client)
{
	switch(opcode)
	{
		case opClusterRegisterServer:
		{
			_processClusterRegisterServer(client,message);
		}
		break;

		case opClusterZoneTransferRequestByTicket:
		{
			_processClusterZoneTransferRequestByTicket(client,message);
		}
		break;

		case opClusterZoneTransferRequestByPosition:
		{
			_processClusterZoneTransferRequestByPosition(client,message);
		}
		break;

		default: break;
	}
}

//======================================================================================================================

void ServerManager::handleDatabaseJobComplete(void* ref, DatabaseResult* result)
{

}

//======================================================================================================================

void ServerManager::_loadProcessAddressMap(void)
{
	//bool            serversOnline = false;
	ServerAddress   serverAddress;

	// retrieve our list of process addresses.
	DatabaseResult* result = mDatabase->ExecuteSynchSql("SELECT id, address, port, status, active FROM config_process_list WHERE active=1 ORDER BY id;");

	mTotalActiveServers = static_cast<uint32>(result->getRowCount());

	for(uint32 i = 0; i < mTotalActiveServers; i++)
	{
		// Retrieve our server data
		result->GetNextRow(mServerBinding,&serverAddress);
		memcpy(&mServerAddressMap[serverAddress.mId], &serverAddress, sizeof(ServerAddress));
	}

	// Delete our DB objects.
	mDatabase->DestroyResult(result);
}

//======================================================================================================================

void ServerManager::_processClusterRegisterServer(ConnectionClient* client, Message* message)
{

}

//======================================================================================================================
//
// transfer through travel ticket
//

void ServerManager::_processClusterZoneTransferRequestByTicket(ConnectionClient* client, Message* message)
{
  // get our destination zone, planetId + 8
  uint32 destinationZone = message->getUint32() + 8;
  uint64 ticketId = message->getUint64();

  // see if that zone is available or not.
  if (mServerAddressMap[destinationZone].mConnectionClient)
  {
    // Send back a opClusterZoneTransferApproved
    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opClusterZoneTransferApprovedByTicket);
    gMessageFactory->addUint64(ticketId);
    Message* newMessage = gMessageFactory->EndMessage();

    // This one goes to the originating zone
    newMessage->setAccountId(message->getAccountId());
    newMessage->setDestinationId(static_cast<uint8>(client->getServerId()));
    newMessage->setRouted(true);
    mMessageRouter->RouteMessage(newMessage, client);
  }
  else
  {
    // Send back a opClusterZoneTransferDenied
    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opClusterZoneTransferDenied);
    gMessageFactory->addUint32(1);                    // Reason: Server not available
    Message* newMessage = gMessageFactory->EndMessage();

    // This one goes to
    newMessage->setAccountId(message->getAccountId());
    newMessage->setDestinationId(static_cast<uint8>(client->getServerId()));
    newMessage->setRouted(true);
    mMessageRouter->RouteMessage(newMessage, client);
  }
}

//======================================================================================================================
//
// transfer through admin command
//

void ServerManager::_processClusterZoneTransferRequestByPosition(ConnectionClient* client, Message* message)
{
	// get our destination zone, planetId + 8
	uint32	destinationZone = message->getUint32() + 8;
	float	x				= message->getFloat();
	float	z				= message->getFloat();

	// see if that zone is available or not.
	if(mServerAddressMap[destinationZone].mConnectionClient)
	{
		// Send back a opClusterZoneTransferApproved
		gMessageFactory->StartMessage();
		gMessageFactory->addUint32(opClusterZoneTransferApprovedByPosition);
		gMessageFactory->addUint32(destinationZone - 8);
		gMessageFactory->addFloat(x);
		gMessageFactory->addFloat(z);
		Message* newMessage = gMessageFactory->EndMessage();

		// This one goes to
		newMessage->setAccountId(message->getAccountId());
		newMessage->setDestinationId(static_cast<uint8>(client->getServerId()));
		newMessage->setRouted(true);
		mMessageRouter->RouteMessage(newMessage, client);
	}
	else
	{
		// Send back a opClusterZoneTransferDenied
		gMessageFactory->StartMessage();
		gMessageFactory->addUint32(opClusterZoneTransferDenied);
		gMessageFactory->addUint32(1);                    // Reason: Server not available
		Message* newMessage = gMessageFactory->EndMessage();

		// This one goes to
		newMessage->setAccountId(message->getAccountId());
		newMessage->setDestinationId(static_cast<uint8>(client->getServerId()));
		newMessage->setRouted(true);
		mMessageRouter->RouteMessage(newMessage, client);
	}
}

//======================================================================================================================
//
// setup database bindings
//

void ServerManager::_setupDataBindings()
{
	mServerBinding = mDatabase->CreateDataBinding(5);
	mServerBinding->addField(DFT_uint32, offsetof(ServerAddress,mId),4);
	mServerBinding->addField(DFT_string, offsetof(ServerAddress,mAddress),16);
	mServerBinding->addField(DFT_uint16, offsetof(ServerAddress,mPort),2);
	mServerBinding->addField(DFT_uint32, offsetof(ServerAddress,mStatus),4);
	mServerBinding->addField(DFT_uint32, offsetof(ServerAddress,mActive),4);
}

//======================================================================================================================
//
// destroy database bindings
//

void ServerManager::_destroyDataBindings()
{
	mDatabase->DestroyDataBinding(mServerBinding);
}

//======================================================================================================================


