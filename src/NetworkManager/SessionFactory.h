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

#ifndef ANH_NETWORKMANAGER_SESSIONFACTORY_H
#define ANH_NETWORKMANAGER_SESSIONFACTORY_H

//#ifndef _WINSOCK2API_
//#include <WINSOCK2.h>
//#endif
#include "Utils/typedefs.h"
#include "Session.h"
#include "NetworkConfig.h"
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
    SessionFactory(SocketWriteThread* writeThread, Service* service, PacketFactory* packetFactory, MessageFactory* messageFactory, bool serverservice, NetworkConfig& network_configuration);
    ~SessionFactory(void);

    void                          Process(void);

    Session*                      CreateSession(void);
    void                          DestroySession(Session* packet);

    Service*                      getService() {
        return mService;
    }

private:

    bool                          mServerService; //marks the service as server / client important to determine packetsize
    Service*                      mService;
    SocketWriteThread*            mSocketWriteThread;
    PacketFactory*                mPacketFactory;
    MessageFactory*               mMessageFactory;
    uint32                        mSessionIdNext;
	NetworkConfig				  network_configuration_;
};




#endif //ANH_NETWORKMANAGER_SESSIONFACTORY_H


