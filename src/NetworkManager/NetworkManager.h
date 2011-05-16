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

#ifndef ANH_NETWORKMANAGER_NETWORKMANAGER_H
#define ANH_NETWORKMANAGER_NETWORKMANAGER_H

#include <queue>
#include "Utils/concurrent_queue.h"
#include "Utils/typedefs.h"
#include "NetworkConfig.h"
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

    NetworkManager(const NetworkConfig& network_configuration);
    ~NetworkManager(void);

    void		Process(void);

    Service*	GenerateService(int8* address, uint16 port,uint32 mfHeapSize, bool serverservice);
    void		DestroyService(Service* service);
    Client*		Connect(void);

    void		RegisterCallback(NetworkCallback* callback);
    void		UnregisterCallback(NetworkCallback* callback);

    void		AddServiceToProcessQueue(Service* service);

private:

    ServiceQueue		mServiceProcessQueue;
	NetworkConfig		network_configuration_;

    uint32			mServiceIdIndex;
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





