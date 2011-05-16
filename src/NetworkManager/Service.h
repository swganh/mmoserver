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

#ifndef ANH_NETWORKMANAGER_SERVICE_H
#define ANH_NETWORKMANAGER_SERVICE_H

#include "Utils/typedefs.h"
#include "Utils/ConcurrentQueue.h"

#include "NetworkConfig.h"
#include <list>


//======================================================================================================================

class NetworkClient;
class Session;
class SocketReadThread;
class SocketWriteThread;
class NetworkManager;
class NetworkCallback;

//======================================================================================================================

typedef utils::ConcurrentQueue<Session*>	SessionQueue;
typedef std::list<NetworkCallback*>				NetworkCallbackList;

//======================================================================================================================

class Service
{
public:

    Service(NetworkManager* networkManager, bool serverservice, uint32 id, int8* localAddress, uint16 localPort,uint32 mfHeapSize, NetworkConfig& network_configuration);
    ~Service(void);

    void	Process();

    void	Connect(NetworkClient* client, const int8* address, uint16 port);

    void	AddSessionToProcessQueue(Session* session);
    //void	AddNetworkCallback(NetworkCallback* callback){ mNetworkCallbackList.push_back(callback); }
    void	AddNetworkCallback(NetworkCallback* callback) {

        mCallBack = callback;
    }


    int8*	getLocalAddress(void);
    uint16	getLocalPort(void);
    uint32	getId(void) {
        return mId;
    };

    void	setId(uint32 id) {
        mId = id;
    };
    void	setQueued(bool b) {
        mQueued = b;
    }
    bool	isQueued() {
        return mQueued;
    }

private:

    NetworkCallback*		mCallBack;
    //NetworkCallbackList		mNetworkCallbackList;

    SessionQueue			mSessionProcessQueue;

    int8					mLocalAddressName[256];
    NetworkManager*			mNetworkManager;
    SocketReadThread*		mSocketReadThread;
    SocketWriteThread*		mSocketWriteThread;
    SOCKET					mLocalSocket;
    uint64					avgTime;
    uint64					lasttime;
    uint32					avgPacketsbuild;
    uint32					mId;
    uint32					mLocalAddress;
    uint32					mSessionResendWindowSize;
    uint16					mLocalPort;
    bool volatile			mQueued;
    bool					mServerService;	//marks us as the serverservice / clientservice

    static bool				mSocketsSubsystemInitComplete;
};



//======================================================================================================================

#endif //ANH_NETWORKMANAGER_SERVICE_



