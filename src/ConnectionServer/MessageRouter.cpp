/*
 This file is part of MMOServer. For more information, visit http://swganh.com
 
 Copyright (c) 2006 - 2011 The SWG:ANH Team

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "MessageRouter.h"

#include "ClientManager.h"
#include "ConnectionClient.h"
#include "ConnectionDispatch.h"
#include "ServerManager.h"



#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"

#include "NetworkManager/Message.h"
#include "NetworkManager/MessageFactory.h"
#include "NetworkManager/MessageOpcodes.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>

//======================================================================================================================

MessageRouter::MessageRouter(Database* database, ConnectionDispatch* connectionDispatch) :
    mConnectionDispatch(connectionDispatch),
    mDatabase(database)
{
    // Load our message and process lists from the DB.
    _loadMessageProcessMap();
}

//======================================================================================================================

MessageRouter::~MessageRouter(void)
{

}

//======================================================================================================================

void MessageRouter::Process(void)
{

}

//======================================================================================================================

void MessageRouter::RouteMessage(Message* message, ConnectionClient* client)
{
    // We need to parse the opcode and route the message
	message->ResetIndex();
   
    uint8 routed      = message->getRouted();    // source
    
    // If the message is from a client (routed == 0) lookup the opcode and route to the default server.
    if(routed == 0)
    {
        // Get our opcode so we can lookup the default route
        uint32 opcode = message->getUint32();

        MessageRouteMap::iterator iter = mMessageRouteMap.find(opcode);

        if(iter != mMessageRouteMap.end())
        {
			uint8 dest = static_cast<uint8>((*iter).second);

            // Set our destination server
            message->setDestinationId(dest);

            // If it's for the connection server, route it locally.
            if(dest == 1)
            {
                mConnectionDispatch->handleIncomingMessage(client, message);
            }
            else
            {
                // Send it off to the destination server
                mServerManager->SendMessageToServer(message);
            }
        }
        else
        {
            // No route found so send it to the ZoneServer the client is on.
            message->setDestinationId(static_cast<uint8>(client->getServerId()));
            mServerManager->SendMessageToServer(message);
        }
    }
    else  // This is from a server and already has a routing header
    {
        // If this is meant for a client, send it to the ClientManager
        if(message->getDestinationId() == 0)
        {
            mClientManager->SendMessageToClient(message);
        }
        else // Send it to the ServerManager
        {
            if(message->getDestinationId() == 1)  // This is for us, route it locally.
            {
                mConnectionDispatch->handleIncomingMessage(client, message);
            }
            else
            {
                mServerManager->SendMessageToServer(message);
            }
        }
    }
}

//======================================================================================================================

void MessageRouter::_loadMessageProcessMap(void)
{
    MessageRoute route;

    // We need to populate our message map.
    // setup our databinding parameters.
    DataBinding* binding = mDatabase->createDataBinding(2);
    binding->addField(DFT_uint32, offsetof(MessageRoute, mMessageId), 4);
    binding->addField(DFT_uint32, offsetof(MessageRoute, mProcessId), 4);

    // Execute our statement
    DatabaseResult* result = mDatabase->executeSynchSql("SELECT messageId, processId FROM %s.config_message_routes;",mDatabase->galaxy());
    
    uint32 count = static_cast<uint32>(result->getRowCount());

    // Retrieve our routes and add them to the map.
    for(uint32 i = 0; i < count; i++)
    {
        result->getNextRow(binding, &route);
        mMessageRouteMap.insert(std::make_pair(route.mMessageId, route.mProcessId));
    }

    // Delete our DB objects.
    mDatabase->destroyDataBinding(binding);
    mDatabase->destroyResult(result);
}

//======================================================================================================================




