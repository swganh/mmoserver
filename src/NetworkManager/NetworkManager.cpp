/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "NetworkManager.h"
#include "NetConfig.h"
#include "Service.h"
#include "LogManager/LogManager.h"

#include "Utils/typedefs.h"


//======================================================================================================================

NetworkManager::NetworkManager(void) :
mServiceIdIndex(1)
{
	// for safety, in case someone forgot to init previously
	LogManager::Init();
}

//======================================================================================================================

NetworkManager::~NetworkManager(void)
{

}

//======================================================================================================================

void NetworkManager::Startup(void)
{
	NetConfig::Init();
}

//======================================================================================================================

void NetworkManager::Shutdown(void)
{
}

//======================================================================================================================

void NetworkManager::Process(void)
{
	// Get the current count of Services to be processed.  We can't just check to see if the queue is empty, since
	// the other threads could keep placing more Service objects in the queue, and this could cause a stall in the
	// main thread.

	mIOService.poll();
   
	Service*	service = 0;
	uint32		serviceCount = mServiceProcessQueue.size();

	for(uint32 i = 0; i < serviceCount; i++)
	{
		// Grab our next Service to process
		service = mServiceProcessQueue.pop();
	
		if(service)
		{
			service->setQueued(false);
			service->Process();
		}
	}	
}


//======================================================================================================================

Service* NetworkManager::GenerateService(int8* address, uint16 port,uint32 mfHeapSize,  bool serverservice)
{
	Service* newService = 0;

	newService = new Service(this, &mIOService, serverservice);
	newService->setId(mServiceIdIndex++);
	newService->Startup(address, port,mfHeapSize);
	
	return newService;
}

//======================================================================================================================

void NetworkManager::DestroyService(Service* service)
{
	service->Shutdown();
	delete(service);
}

//======================================================================================================================