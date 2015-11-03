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
#include "ClientManager.h"
#include "ConnectionClient.h"
#include "ConnectionDispatch.h"
#include "MessageRouter.h"

#include "NetworkManager/Service.h"




#include "Utils/logger.h"

#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"

#include "NetworkManager/Message.h"
#include "NetworkManager/MessageFactory.h"
#include "NetworkManager/MessageOpcodes.h"

//======================================================================================================================

ClientManager::ClientManager(Service* service, Database* database, MessageRouter* router, ConnectionDispatch* dispatch, uint32_t cluster_id) :
    mClientService(service),
    mDatabase(database),
    mMessageRouter(router),
    mConnectionDispatch(dispatch),
	mClusterId(cluster_id)
{
    // Set our member variables
    mMessageRouter->setClientManager(this);

    // Put ourselves on the callback list for this service.
    mClientService->AddNetworkCallback(this);

    // Register our opcodes
    mConnectionDispatch->RegisterMessageCallback(opClientIdMsg, this);
    mConnectionDispatch->RegisterMessageCallback(opSelectCharacter, this);
    mConnectionDispatch->RegisterMessageCallback(opClusterZoneTransferCharacter, this);
}

//======================================================================================================================

ClientManager::~ClientManager(void)
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

	// We're headed to the client, don't use the routing header.
    message->setRouted(false);

    // Find our client from the accountId.
    boost::recursive_mutex::scoped_lock lk(mServiceMutex);

    PlayerClientMap::iterator iter = mPlayerClientMap.find(message->getAccountId());
	
    // If we found the client, send the data.
    if (iter != mPlayerClientMap.end())
    {
        ConnectionClient* client = (*iter).second;
		
		//unlock here sendchannel is getting the mSessionMutex we dont want to spend time waiting to synchronize mutexes
		lk.unlock();

        client->SendChannelA(message, message->getPriority(), message->getFastpath());
    }
    else
    {
        //happens when the client logs out
        gMessageFactory->DestroyMessage(message);
    }
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
        boost::recursive_mutex::scoped_lock lk(mServiceMutex);

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
    mDatabase->executeProcedureAsync(0, 0, "CALL %s.sp_AccountStatusUpdate(%u, %u);",mDatabase->galaxy(), 0, connClient->getAccountId());

    // Client has disconnected.

    boost::recursive_mutex::scoped_lock lk(mServiceMutex);
    PlayerClientMap::iterator iter = mPlayerClientMap.find(connClient->getAccountId());

    if(iter != mPlayerClientMap.end())
    {
        delete ((*iter).second);
        mPlayerClientMap.erase(iter);
    }
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
    case CCSTATE_AllowedChars:
    {
        struct charsCurrentAllowed {
            uint32  currentChars;
            uint32	charsAllowed;
        } charsStruct;

        DataBinding* binding = mDatabase->createDataBinding(2);
        binding->addField(DFT_int32,offsetof(charsCurrentAllowed, currentChars), 4, 0);
        binding->addField(DFT_int32,offsetof(charsCurrentAllowed, charsAllowed), 4, 1);

        result->getNextRow(binding,&charsStruct);
        client->setCharsAllowed(charsStruct.charsAllowed);
        client->setCurrentChars(charsStruct.currentChars);

        client->setState(CCSTATE_QueryAuth);
        mDatabase->destroyDataBinding(binding);
        break;
    }
    default:
        break;
    }
}


//======================================================================================================================
void ClientManager::_processClientIdMsg(ConnectionClient* client, Message* message)
{
    // We only need the account data that is at the end of the message.
    message->getUint32();  // unknown.
    uint32 dataSize = message->getUint32();
    message->setIndex(message->getIndex() + (uint16)dataSize - 4);
    client->setAccountId(message->getUint32());

    _processAllowedChars(this, client);
}

//======================================================================================================================
void ClientManager::_processSelectCharacter(ConnectionClient* client, Message* message)
{
    uint64 characterId = message->getUint64();

    DatabaseResult* result = mDatabase->executeSynchSql("SELECT planet_id FROM %s.characters WHERE id=%" PRIu64 ";",mDatabase->galaxy(), characterId);

    uint32 serverId;
    DataBinding* binding = mDatabase->createDataBinding(1);
    binding->addField(DFT_uint32, 0, 4);
    result->getNextRow(binding, &serverId);

    client->setServerId(serverId + 8);  // server ids for zones are planetId + 8;

    mDatabase->destroyDataBinding(binding);
    mDatabase->destroyResult(result);

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
    boost::recursive_mutex::scoped_lock lk(mServiceMutex);
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
        LOG(WARNING) << "Client not found during zone transfer.\n";
    }
}


//======================================================================================================================
void ClientManager::_handleQueryAuth(ConnectionClient* client, DatabaseResult* result)
{
    // If there are any results, this account is properly authenticated.
    if (result->getRowCount())
    {
        // Update the account record that it is now logged in and last login date.
        mDatabase->executeProcedureAsync(0, 0, "CALL %s.sp_AccountStatusUpdate(%u, %u);", mDatabase->galaxy(), mClusterId, client->getAccountId());

        // finally add them to our accountId map.
        boost::recursive_mutex::scoped_lock lk(mServiceMutex);
        mPlayerClientMap.insert(std::make_pair(client->getAccountId(), client));
        lk.unlock();

        // send an opClusterClientConnect message to admin server.
        gMessageFactory->StartMessage();
        gMessageFactory->addUint32(opClusterClientConnect);
        gMessageFactory->addUint64(0);        //We don't have a characterId yet.
        Message* adminMessage = gMessageFactory->EndMessage();

        // This one goes to the AdminServer
        adminMessage->setAccountId(client->getAccountId());
        adminMessage->setDestinationId(CR_Chat);
        adminMessage->setRouted(true);
        mMessageRouter->RouteMessage(adminMessage, client);

        gMessageFactory->StartMessage();
        gMessageFactory->addUint32(opClientPermissionsMessage);
        gMessageFactory->addUint8(1);             // Galaxy Available

        // Checks the Clients Characters allowed against how many they have and sends the flag accordingly for char creation Also checks Unlimited Char Creation
        if (client->getCharsAllowed() > client->getCurrentChars() && client->getCharsAllowed() != 0) {
            gMessageFactory->addUint8(1);             // Character creation allowed
            gMessageFactory->addUint8(0);             // Unlimited Character Creation Flag DISABLED
        }
        else if(client->getCharsAllowed() < client->getCurrentChars() && client->getCharsAllowed() != 0) {
            gMessageFactory->addUint8(0);             // Character creation disabled
            gMessageFactory->addUint8(0);             // Unlimited Character Creation Flag DISABLED
        }
        else if(client->getCharsAllowed() == 0) {
            gMessageFactory->addUint8(1);             // Character creation allowed
            gMessageFactory->addUint8(1);             // Unlimited Character Creation Flag DISABLED
        }
        else {
            gMessageFactory->addUint8(0);             // Character creation disabled
            gMessageFactory->addUint8(0);             // Unlimited Character Creation Flag DISABLED
        }
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
void ClientManager::_processAllowedChars(DatabaseCallback* callback,ConnectionClient* client)
{
    client->setState(CCSTATE_AllowedChars);
    mDatabase->executeSqlAsync(this, client,"SELECT COUNT(characters.id) AS account_current_characters, account_characters_allowed FROM %s.account INNER JOIN %s.characters ON characters.account_id = account.account_id where characters.archived = '0' AND account.account_id = '%u'",mDatabase->galaxy(),mDatabase->galaxy(),client->getAccountId());
    

    mDatabase->executeSqlAsync(this,client, "SELECT * FROM %s.account WHERE account_id=%u AND account_authenticated=1 AND account_loggedin=0;",mDatabase->galaxy(), client->getAccountId());
    
}


