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

#include "PacketFactory.h"
#include "Packet.h"


//======================================================================================================================

PacketFactory::PacketFactory(bool serverservice, NetworkConfig& network_configuration)
    : mPacketPool(sizeof(Packet))
{
    mPacketCount = 0;

    if(serverservice)
        mMaxPayLoad = network_configuration.getServerToServerReliableSize();
    else
        mMaxPayLoad = network_configuration.getServerToClientReliableSize();
}


//======================================================================================================================

PacketFactory::~PacketFactory(void)
{
    // Destory our clock
    // delete mClock;

    mPacketPool.purge_memory();
}

//======================================================================================================================

void PacketFactory::Process(void)
{
    // Update our clock
    // mClock->Update();
}


//======================================================================================================================

Packet* PacketFactory::CreatePacket(void)
{

    boost::recursive_mutex::scoped_lock lk(mPacketFactoryMutex);
    Packet* newPacket = new(mPacketPool.malloc()) Packet();

    newPacket->setTimeCreated(Anh_Utils::Clock::getSingleton()->getStoredTime());
    newPacket->setMaxPayload(mMaxPayLoad);

    mPacketCount++;

    return newPacket;
}

//======================================================================================================================

void PacketFactory::DestroyPacket(Packet* packet)
{
    boost::recursive_mutex::scoped_lock lk(mPacketFactoryMutex);

    mPacketPool.free(packet);
    mPacketCount--;
}

//======================================================================================================================

