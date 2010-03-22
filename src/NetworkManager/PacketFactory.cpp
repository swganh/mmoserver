/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "PacketFactory.h"
#include "Packet.h"
#include "NetConfig.h"


//======================================================================================================================

PacketFactory::PacketFactory(bool serverservice)
: mPacketPool(sizeof(Packet))
{
	if(serverservice)
		mMaxPayLoad = gNetConfig->getServerServerReliableSize();
	else
		mMaxPayLoad = gNetConfig->getServerClientReliableSize();
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

	newPacket->setTimeCreated(Anh_Utils::Clock::getSingleton()->getLocalTime());
	newPacket->setMaxPayload(mMaxPayLoad);

	return newPacket;
}

//======================================================================================================================

void PacketFactory::DestroyPacket(Packet* packet)
{
	boost::recursive_mutex::scoped_lock lk(mPacketFactoryMutex);
	
	mPacketPool.free(packet);
}

//======================================================================================================================

