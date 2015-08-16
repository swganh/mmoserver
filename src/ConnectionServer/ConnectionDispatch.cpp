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
#include "ConnectionDispatch.h"
#include "ConnectionClient.h"
#include "ConnectionDispatchCallback.h"
#include "NetworkManager/Session.h"
#include "NetworkManager/Message.h"




#include "Utils/logger.h"

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
        LOG(INFO) << "Unhandled opcode in ConnectionDispatch - " << opcode;
    }

    // Delete our message
    client->getSession()->DestroyIncomingMessage(message);
}

//======================================================================================================================



