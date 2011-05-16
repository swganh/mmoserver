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

#ifndef ANH_NETWORKMANAGER_SOCKETREADTHREAD_H
#define ANH_NETWORKMANAGER_SOCKETREADTHREAD_H

#include "Utils/typedefs.h"
#include "NetworkConfig.h"
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <list>
#include <map>

//======================================================================================================================

class SocketWriteThread;
class PacketFactory;
class SessionFactory;
class MessageFactory;
class CompCryptor;
class Session;
class Service;
class Packet;

//======================================================================================================================

typedef std::list<Session*>			SessionList;
typedef std::map<uint64,Session*>	AddressSessionMap;


//======================================================================================================================

class NewConnection
{
public:

    int8              mAddress[256];
    uint16            mPort;
    Session*          mSession;
};

//======================================================================================================================

class SocketReadThread
{
public:
    SocketReadThread(SOCKET socket, SocketWriteThread* writeThread, Service* service,uint32 mfHeapSize, bool serverservice, NetworkConfig& network_configuration_);
    ~SocketReadThread();

    virtual void					run();

    void                          NewOutgoingConnection(const int8* address, uint16 port);
    void                          RemoveAndDestroySession(Session* session);

    NewConnection*                getNewConnectionInfo(void)  {
        return &mNewConnection;
    };
    bool                          getIsRunning(void)          {
        return mIsRunning;
    }
    void							requestExit()				{
        mExit = true;
    }

protected:

    void                          _startup(void);
    void                          _shutdown(void);

    Packet*                       mReceivePacket;
    Packet*                       mDecompressPacket;

    uint16						mMessageMaxSize;
    SocketWriteThread*            mSocketWriteThread;
    SessionFactory*               mSessionFactory;
    PacketFactory*                mPacketFactory;
    MessageFactory*               mMessageFactory;
    CompCryptor*                  mCompCryptor;
    NewConnection                 mNewConnection;

    SOCKET                        mSocket;

    bool							mIsRunning;

    uint32						mSessionResendWindowSize;

    boost::thread 				mThread;
    boost::mutex					mSocketReadMutex;
    AddressSessionMap             mAddressSessionMap;

    bool							mExit;
};

//======================================================================================================================

#endif //ANH_NETWORKMANAGER_SOCKETREADTHREAD_H



