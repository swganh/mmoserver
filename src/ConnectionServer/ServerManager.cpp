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

ServerManager::ServerManager(Service* service, Database* database, MessageRouter* router, ConnectionDispatch* dispatch,ClientManager* clientManager) :
mMessageRouter(router),
mServerService(service),
mDatabase(database),
mConnectionDispatch(dispatch),
mTotalActiveServers(0),
mTotalConnectedServers(0),
mClientManager(clientManager)
{
	memset(&mServerAddressMap, 0, sizeof(mServerAddressMap));

	// Set our member variables
	mMessageRouter->setServerManager(this);

	// Put ourselves on the callback list for this service.
	mServerService->AddNetworkCallback(this);

	// Register any messages we need to handle from tha backend servers
	mConnectionDispatch->RegisterMessageCallback(opClusterRegisterServer, this);
	mConnectionDispatch->RegisterMessageCallback(opClusterZoneTransferRequestByTicket, this);
	mConnectionDispatch->RegisterMessageCallback(opClusterZoneTransferRequestByPosition, this);
	mConnectionDispatch->RegisterMessageCallback(opTutorialServerStatusRequest, this);

	// Update our id
	mClusterId = gConfig->read<uint32>("ClusterId");

	// setup data bindings
	_setupDataBindings();

	// load process address map
	_loadProcessAddressMap();
}

//======================================================================================================================

ServerManager::~ServerManager(void)
{
	mConnectionDispatch->UnregisterMessageCallback(opClusterRegisterServer);
	mConnectionDispatch->UnregisterMessageCallback(opClusterZoneTransferRequestByTicket);
	mConnectionDispatch->UnregisterMessageCallback(opClusterZoneTransferRequestByPosition);
	mConnectionDispatch->UnregisterMessageCallback(opTutorialServerStatusRequest);

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
		gLogger->log(LogManager::INFORMATION,"ServerManager: failed routing message to server %u",message->getDestinationId());
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
	gLogger->log(LogManager::DEBUG, sql);
	gLogger->logCont(LogManager::DEBUG,"\n");
							
	// If we found them
	if(result->getRowCount() == 1)
	{
		// Retrieve our routes and add them to the map.
		result->GetNextRow(mServerBinding,&serverAddress);

		// put this fresh data in our list.
		newClient = new ConnectionClient();
		ConnectionClient* connClient = reinterpret_cast<ConnectionClient*>(newClient);

		ConnectionClient* oldClient = mServerAddressMap[serverAddress.mId].mConnectionClient;
		if(oldClient)
		{
			delete(oldClient);
			--mTotalConnectedServers;
		}

		connClient->setServerId(serverAddress.mId);

		memcpy(&mServerAddressMap[serverAddress.mId], &serverAddress, sizeof(ServerAddress));
		mServerAddressMap[serverAddress.mId].mConnectionClient = connClient;
		
		gLogger->log(LogManager::DEBUG,"*** Backend server connected id: %u\n",mServerAddressMap[serverAddress.mId].mId);
		
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
		gLogger->log(LogManager::CRITICAL,"*** Backend server connect error - Server not found in DB\n");
		gLogger->log(LogManager::DEBUG,sql);
		gLogger->log(LogManager::DEBUG,"\n");
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
		//its undefined later
		uint32 id = connClient->getServerId();
		//we actually delete the client in line 186 with delete(client)
		//deleting it here just crashes us everytime a server disconnects :)

		//delete mServerAddressMap[id].mConnectionClient;
		mServerAddressMap[id].mConnectionClient = 0;
	}

	// update the galaxy state
	if(mServerAddressMap[connClient->getServerId()].mActive)
	{
		--mTotalConnectedServers;

		mDatabase->ExecuteSqlAsync(0,0,"UPDATE galaxy SET status=1,last_update=NOW() WHERE galaxy_id=%u;", mClusterId);
	}

	gLogger->log(LogManager::DEBUG,"Servermanager handle server down\n");
	mClientManager->handleServerDown(connClient->getServerId());

	connClient->getSession()->setStatus(SSTAT_Destroy);
	connClient->getSession()->getService()->AddSessionToProcessQueue(connClient->getSession());
	

	delete(client);
}


//======================================================================================================================
void ServerManager::handleSessionMessage(NetworkClient* client, Message* message)
{
  ConnectionClient* connClient = reinterpret_cast<ConnectionClient*>(client);
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

		case opTutorialServerStatusRequest:
		{
			_processClusterZoneTutorialTerminal(client,message);
		}

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
		mServerAddressMap[serverAddress.mId].mConnectionClient = NULL;
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

void ServerManager::_processClusterZoneTutorialTerminal(ConnectionClient* client, Message* message)
{
	gLogger->log(LogManager::DEBUG,"Sending Tutorial Status Reply\n");

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opTutorialServerStatusReply);
	gMessageFactory->addUint64(message->getUint64());

	if(mServerAddressMap[16].mStatus == 2)
		gMessageFactory->addUint8(1);
	else
		gMessageFactory->addUint8(0);

	if(mServerAddressMap[8].mStatus == 2)
		gMessageFactory->addUint8(1);
	else
		gMessageFactory->addUint8(0);

	if(mServerAddressMap[15].mStatus == 2)
		gMessageFactory->addUint8(1);
	else
		gMessageFactory->addUint8(0);

	if(mServerAddressMap[14].mStatus == 2)
		gMessageFactory->addUint8(1);
	else
		gMessageFactory->addUint8(0);

	if(mServerAddressMap[13].mStatus == 2)
		gMessageFactory->addUint8(1);
	else
		gMessageFactory->addUint8(0);

	Message* newMessage = gMessageFactory->EndMessage();

	// This one goes back from whence it came
    newMessage->setAccountId(message->getAccountId());
    newMessage->setDestinationId(static_cast<uint8>(client->getServerId()));
    newMessage->setRouted(true);
    mMessageRouter->RouteMessage(newMessage, client);
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


