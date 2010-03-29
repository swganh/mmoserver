/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_NETWORKMANAGER_SESSIONFACTORY_H
#define ANH_NETWORKMANAGER_SESSIONFACTORY_H

#include "Utils/typedefs.h"
#include "Session.h"
#include <boost/pool/singleton_pool.hpp>


//======================================================================================================================

class PacketFactory;
class MessageFactory;
class GameService;

//======================================================================================================================

typedef boost::singleton_pool<Session,sizeof(Session),boost::default_user_allocator_malloc_free>	SessionPool;

//======================================================================================================================

class SessionFactory
{
	public:
									SessionFactory(void);
									~SessionFactory(void);

	  void                          Startup(GameService* service, PacketFactory* packetFactory, MessageFactory* messageFactory);
	  void                          Shutdown(void);
	  void                          Process(void);

	  Session*                      CreateSession(void);
	  void                          DestroySession(Session* packet);

	  GameService*                     getService() { return mService; }

	private:

	  GameService*                  mService;
	  PacketFactory*                mPacketFactory;
	  MessageFactory*               mMessageFactory;
	  uint32                        mSessionIdNext;
};




#endif //ANH_NETWORKMANAGER_SESSIONFACTORY_H


