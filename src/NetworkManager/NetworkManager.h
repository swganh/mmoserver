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
#include "IService.h"

//======================================================================================================================

class Client;
class LogManager;
class NetworkCallback;
class Session;

//======================================================================================================================

typedef Anh_Utils::concurrent_queue<IService*>	ServiceQueue;

//======================================================================================================================

class NetworkManager
{
	public:

		NetworkManager(void);
		~NetworkManager(void);

		void		Startup(void);
		void		Shutdown(void);
		void		Process(void);

		IService*	GenerateService(std::string address, uint16 port,uint32 mfHeapSize, ServiceType type);
		void		DestroyService(IService* service);

		void		AddServiceToProcessQueue(IService* service);

	private:

	  ServiceQueue					mServiceProcessQueue;
	  uint32						mServiceIdIndex;
	  boost::asio::io_service		mIOService;
};


//=====================================================================================================================

inline void NetworkManager::AddServiceToProcessQueue(IService* service)
{
	if(!service->isQueued())
	{
		service->setQueued(true);

		mServiceProcessQueue.push(service);
	}
}

//======================================================================================================================

#endif // ANH_NETWORKMANAGER_NETWORKMANAGER_H





