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

#include "NetworkManager.h"
#include "Service.h"


#include "Utils/typedefs.h"

#if defined(_MSC_VER)
#ifndef _WINSOCK2API_
#include <WINSOCK2.h>
#endif
#endif

//======================================================================================================================

NetworkManager::NetworkManager(const NetworkConfig& network_configuration) 
	: mServiceIdIndex(1)
	, network_configuration_(network_configuration)
{
    
}

//======================================================================================================================

NetworkManager::~NetworkManager(void)
{
}

//======================================================================================================================

void NetworkManager::Process(void)
{
    // Get the current count of Services to be processed.  We can't just check to see if the queue is empty, since
    // the other threads could keep placing more Service objects in the queue, and this could cause a stall in the
    // main thread.

    Service*	service = 0;
    uint32		serviceCount = mServiceProcessQueue.size();

    for(uint32 i = 0; i < serviceCount; i++)
    {
        // Grab our next Service to process
        service = mServiceProcessQueue.pop();

        if(service)
        {
            service->Process();
            service->setQueued(false);
        }
    }

}


//======================================================================================================================

Service* NetworkManager::GenerateService(int8* address, uint16 port,uint32 mfHeapSize,  bool serverservice)
{
    Service* newService = 0;

    newService = new Service(this, serverservice, mServiceIdIndex++, address, port,mfHeapSize, network_configuration_);

    return newService;
}

//======================================================================================================================

void NetworkManager::DestroyService(Service* service)
{
    delete(service);
}

//======================================================================================================================

Client* NetworkManager::Connect(void)
{
    Client* newClient = 0;

    return newClient;
}

//======================================================================================================================

void NetworkManager::RegisterCallback(NetworkCallback* callback)
{
}

//======================================================================================================================

void NetworkManager::UnregisterCallback(NetworkCallback* callback)
{
}

//======================================================================================================================




