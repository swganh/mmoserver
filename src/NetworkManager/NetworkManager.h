/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_NETWORKMANAGER_NETWORKMANAGER_H
#define ANH_NETWORKMANAGER_NETWORKMANAGER_H

#include <queue>
#include <boost/asio.hpp>
#include "Utils/concurrent_queue.h"
#include "Utils/typedefs.h"
#include "Service.h"

//======================================================================================================================

class Client;
class LogManager;
class NetworkCallback;
class Session;

//======================================================================================================================

typedef Anh_Utils::concurrent_queue<Service*>	ServiceQueue;

//======================================================================================================================

class NetworkManager
{
	public:

		NetworkManager(void);
		~NetworkManager(void);

		void		Startup(void);
		void		Shutdown(void);
		void		Process(void);

		Service*	GenerateService(int8* address, uint16 port,uint32 mfHeapSize, bool serverservice);
		void		DestroyService(Service* service);

		void		AddServiceToProcessQueue(Service* service);

	private:

	  ServiceQueue					mServiceProcessQueue;
	  uint32						mServiceIdIndex;
	  boost::asio::io_service		mIOService;
};


//=====================================================================================================================

inline void NetworkManager::AddServiceToProcessQueue(Service* service)
{
	if(!service->isQueued())
	{
		service->setQueued(true);

		mServiceProcessQueue.push(service);
	}
}

//======================================================================================================================

#endif // ANH_NETWORKMANAGER_NETWORKMANAGER_H





