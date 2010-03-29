/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "NetworkManager.h"
#include "NetConfig.h"
#include "GameService.h"
#include "LogManager/LogManager.h"

#include "Utils/typedefs.h"


//======================================================================================================================

NetworkManager::NetworkManager(void) :
mServiceIdIndex(1),
mIOService()
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
	mIOService.poll();

	for( ServiceList::iterator iter = mServiceList.begin(); iter != mServiceList.end(); iter++ )
		(*iter)->Process();
}


//======================================================================================================================

IService* NetworkManager::GenerateService(std::string address, uint16 port,uint32 mfHeapSize,  ServiceType type)
{
	IService* newService = new GameService(this, &mIOService);
	newService->setId(mServiceIdIndex++);
	newService->Startup(address, port,mfHeapSize);
	mServiceList.push_back(newService);
	return newService;
}

//======================================================================================================================

void NetworkManager::DestroyService(IService* service)
{
	for( ServiceList::iterator iter = mServiceList.begin(); iter != mServiceList.end(); iter++ )
	{
		if( (*iter)->getId() == service->getId() )
		{
			mServiceList.erase(iter);
			break;
		}
	}

	service->Shutdown();
	delete(service);
}

//======================================================================================================================