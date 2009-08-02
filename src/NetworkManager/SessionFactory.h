/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_NETWORKMANAGER_SESSIONFACTORY_H
#define ANH_NETWORKMANAGER_SESSIONFACTORY_H

//#ifndef _WINSOCK2API_
//#include <WINSOCK2.h>
//#endif
#include "Utils/typedefs.h"
#include "Session.h"
#include <boost/pool/singleton_pool.hpp>


//======================================================================================================================

class SocketWriteThread;
class PacketFactory;
class MessageFactory;
class Service;

//======================================================================================================================

typedef boost::singleton_pool<Session,sizeof(Session),boost::default_user_allocator_malloc_free>	SessionPool;

//======================================================================================================================

class SessionFactory
{
	public:
									SessionFactory(void);
									~SessionFactory(void);

	  void                          Startup(SocketWriteThread* writeThread, Service* service, PacketFactory* packetFactory, MessageFactory* messageFactory, bool serverservice);
	  void                          Shutdown(void);
	  void                          Process(void);

	  Session*                      CreateSession(void);
	  void                          DestroySession(Session* packet);

	  Service*                      getService()                { return mService; }

	private:

	  bool							mServerService;//marks the service as server / client important to determine packetsize
	  Service*                      mService;
	  SocketWriteThread*            mSocketWriteThread;
	  PacketFactory*                mPacketFactory;
	  MessageFactory*               mMessageFactory;
	  uint32                        mSessionIdNext;
};



#endif //ANH_NETWORKMANAGER_SESSIONFACTORY_H


