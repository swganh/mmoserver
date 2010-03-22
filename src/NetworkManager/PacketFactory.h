/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_NETWORKMANAGER_PACKETFACTORY_H
#define ANH_NETWORKMANAGER_PACKETFACTORY_H

#include "Utils/typedefs.h"
#include "Utils/clock.h"
#include "Packet.h"
#include <boost/pool/pool.hpp>


//======================================================================================================================

typedef boost::pool<boost::default_user_allocator_malloc_free> PacketPool;

//======================================================================================================================

class PacketFactory
{
	public:

		PacketFactory(bool serverservice);
		~PacketFactory(void);

		void		Process(void);

		Packet*		CreatePacket(void);
		void		DestroyPacket(Packet* packet);

		uint16		mMaxPayLoad;

	private:
        PacketPool mPacketPool;
		boost::recursive_mutex        mPacketFactoryMutex;
	  // Anh_Utils::Clock*	mClock;
};

//======================================================================================================================

#endif //ANH_NETWORKMANAGER_PACKETFACTORY_H





