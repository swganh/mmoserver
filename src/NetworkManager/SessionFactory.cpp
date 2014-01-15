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

#include "SessionFactory.h"
#include "Session.h"



//======================================================================================================================

SessionFactory::SessionFactory(SocketWriteThread* writeThread, Service* service, PacketFactory* packetFactory, MessageFactory* messageFactory, bool serverservice, NetworkConfig& network_configuration)
: mServerService(serverservice)
, mService(service)
, mSocketWriteThread(writeThread)
, mPacketFactory(packetFactory)
, mMessageFactory(messageFactory)
, mSessionIdNext(0)
, network_configuration_(network_configuration)
{

}

//======================================================================================================================

SessionFactory::~SessionFactory(void)
{
    SessionPool::purge_memory();
}

//======================================================================================================================

void SessionFactory::Process(void)
{

}

//======================================================================================================================

Session* SessionFactory::CreateSession(void)
{
    Session* session = new(SessionPool::malloc()) Session();

    session->setSocketWriteThread(mSocketWriteThread);
    session->setService(mService);
    session->setPacketFactory(mPacketFactory);
    session->setMessageFactory(mMessageFactory);
    session->setId(mSessionIdNext++);

    session->setServerService(mServerService);

    if(mServerService)
    {
        uint16 unreliable = network_configuration_.getServerToServerUnreliableSize();
        uint16 reliable = network_configuration_.getServerToServerReliableSize();

        if(reliable > MAX_SERVER_PACKET_SIZE)
            reliable = MAX_SERVER_PACKET_SIZE;
        if(unreliable > MAX_SERVER_PACKET_SIZE)
            unreliable = MAX_SERVER_PACKET_SIZE;

        session->setPacketSize(reliable);
        session->setUnreliableSize(unreliable);
    }
    else
    {
        uint16 unreliable = network_configuration_.getServerToClientUnreliableSize();
        uint16 reliable = network_configuration_.getServerToClientReliableSize();

        if(reliable > MAX_CLIENT_PACKET_SIZE)
            reliable = MAX_CLIENT_PACKET_SIZE;
        if(unreliable > MAX_CLIENT_PACKET_SIZE)
            unreliable = MAX_CLIENT_PACKET_SIZE;

        session->setPacketSize(reliable);
        session->setUnreliableSize(unreliable);
    }

    return session;
}

//======================================================================================================================

void SessionFactory::DestroySession(Session* session)
{
    session->~Session();
    SessionPool::free(session);
    session = nullptr;
}

//======================================================================================================================




