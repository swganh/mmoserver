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

#ifndef ANH_NETWORKMANAGER_SESSION_H
#define ANH_NETWORKMANAGER_SESSION_H

#include <list>
#include <queue>

#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/thread.hpp>

#include "Utils/clock.h"
#include "Utils/typedefs.h"

#include "NetworkManager/Message.h"

#include "NetworkManager/NetConfig.h"
#include "NetworkManager/declspec.h"

//======================================================================================================================

class NetworkClient;
class Service;
class SocketReadThread;
class SocketWriteThread;
class PacketFactory;
class MessageFactory;
class Packet;
class SessionPacket;

//======================================================================================================================

typedef std::list<Packet*,std::allocator<Packet*> >		PacketWindowList;
typedef std::queue<Packet*>								PacketQueue;
//typedef std::priority_queue<Message*,std::vector<Message*>,CompareMsg>  MessageQueue;
typedef std::queue<Message*>							MessageQueue;

//======================================================================================================================

enum SessionStatus
{
    SSTAT_Initialize = 0,
    SSTAT_Connecting,
    SSTAT_Connected,
    SSTAT_Disconnecting,
    SSTAT_Disconnected,
    SSTAT_Destroy,
    SSTAT_Timeout,
    SSTAT_Error
};

enum SessionCommand
{
    SCOM_None = 0,
    SCOM_Connect,
    SCOM_Disconnect
};

//======================================================================================================================

class Session
{
    public:
      NET_API                             Session(void);
      NET_API                             ~Session(void);

      NET_API void                        ProcessReadThread(void);
      NET_API void                        ProcessWriteThread(void);

      NET_API void                        HandleSessionPacket(Packet* packet);
      NET_API void                        SortSessionPacket(Packet* packet, uint16 type);
      NET_API void                        HandleFastpathPacket(Packet* packet);
      
      NET_API void                        SendChannelA(Message* message);
    
      NET_API void						  SendChannelAUnreliable(Message* message);
      NET_API void                        DestroyIncomingMessage(Message* message);
      NET_API void                        DestroyPacket(Packet* packet);
      
      
      // Accessor methods
      NET_API NetworkClient*              getClient(void)                                 { return mClient; }
      NET_API Service*                    getService(void)                                { return mService; }
      NET_API uint32                      getId(void)                                     { return mId; }
      NET_API uint32                      getAddress(void)                                { return mAddress; }
      NET_API int8*                       getAddressString(void);
      NET_API uint16                      getPort(void)                                   { return mPort; }
      NET_API uint16                      getPortHost(void);
      NET_API uint32                      getOutgoingReliablePacketCount(void)            { return mOutgoingReliablePacketQueue.size(); }
      NET_API Packet*                     getOutgoingReliablePacket(void);
      NET_API uint32                      getOutgoingUnreliablePacketCount(void)          { return mOutgoingUnreliablePacketQueue.size(); }
      NET_API Packet*                     getOutgoingUnreliablePacket(void);
      NET_API uint32                      getIncomingQueueMessageCount()    { return mIncomingMessageQueue.size(); }
      NET_API Message*                    getIncomingQueueMessage();
      NET_API uint32                      getEncryptKey(void)                             { return mEncryptKey; }
      NET_API SessionStatus               getStatus(void)                                 { return mStatus; }
      NET_API SessionCommand              getCommand(void)                                { return mCommand; }
      NET_API bool                        getInOutgoingQueue(void)                        { return mInOutgoingQueue; }
      NET_API bool                        getInIncomingQueue(void)                        { return mInIncomingQueue; }
      NET_API uint32					  getResendWindowSize()							  { return mWindowResendSize; }


      NET_API void						  setResendWindowSize(uint32 resendWindowSize)	  { mWindowResendSize = resendWindowSize;  mWindowSizeCurrent = resendWindowSize; }
      NET_API void                        setClient(NetworkClient* client)                { mClient = client; }
      NET_API void                        setService(Service* service)                    { mService = service; }
      NET_API void                        setSocketReadThread(SocketReadThread* thread)   { mSocketReadThread = thread; }
      NET_API void                        setSocketWriteThread(SocketWriteThread* thread) { mSocketWriteThread = thread; }
      NET_API void                        setPacketFactory(PacketFactory* factory)        { mPacketFactory = factory; }
      NET_API void                        setMessageFactory(MessageFactory* factory)      { mMessageFactory = factory; }
      NET_API void                        setId(uint32 id)                                { mId = id; }
      NET_API void                        setAddress(uint32 address)					  { mAddress = address; }
      NET_API void                        setPort(uint16 port)                            { mPort = port; }
      NET_API void                        setEncryptKey(uint32 key)                       { mEncryptKey = key; }
      NET_API void                        setStatus(SessionStatus status)                 { mStatus = status; }
      NET_API void                        setCommand(SessionCommand command)              { mCommand = command; }
      NET_API void                        setInOutgoingQueue(bool in)                     { mInOutgoingQueue = in; }
      NET_API void                        setInIncomingQueue(bool in)                     { mInIncomingQueue = in; }
      NET_API void                        setPacketSize(uint16 size)					  { mMaxPacketSize = size;}
      NET_API void                        setUnreliableSize(uint16 size)				  { mMaxUnreliableSize = size;}

      NET_API void						  setServerService(bool yes){mServerService = yes;}
      NET_API bool						  getServerService(){return mServerService;}

      uint64					  mLastPacketDestroyed;
      uint64					  mHash;

private:
      void                        _processSessionRequestPacket(Packet* packet);
      void                        _processDisconnectPacket(Packet* packet);
      void                        _processMultiPacket(Packet* packet);
      void                        _processNetStatRequestPacket(Packet* packet);
      
      void                        _processDataChannelPacket(Packet* packet, bool fastPath);
      void                        _processDataChannelB(Packet* packet);
      
      void						  _resendData();

      void                        _processDataOrderPacket(Packet* packet);
      void                        _processDataOrderChannelB(Packet* packet);
      void                        _processDataChannelAck(Packet* packet);
      void                        _processFragmentedPacket(Packet* packet);
      void						  _processRoutedFragmentedPacket(Packet* packet);
      void                        _processPingPacket(Packet* packet);

      void                        _processConnectCommand(void);
      void                        _processDisconnectCommand(void);

      void                        _addOutgoingMessage(Message* message, uint8 priority, bool fastpath);
      void                        _addIncomingMessage(Message* message, uint8 priority);

      uint32					  _buildPackets();
      uint32					  _buildPacketsUnreliable();


      void						  _buildMultiDataPacket();//fastpath
      void						  _buildRoutedMultiDataPacket();
      void						  _buildUnreliableMultiDataPacket();

      void                        _buildOutgoingReliablePackets(Message* message);
      void						  _buildOutgoingReliableRoutedPackets(Message* message);
      void                        _buildOutgoingUnreliablePackets(Message* message);
      void                        _addOutgoingReliablePacket(Packet* packet);
      void                        _addOutgoingUnreliablePacket(Packet* packet);
      void                        _resendOutgoingPackets(void);
      void                        _sendPingPacket(void);

      void						  _handleOutSequenceRollover();
      

      //we want to use bigger packets in the zone connection server communication!
      uint16					  mMaxPacketSize;
      uint16					  mMaxUnreliableSize;

      //we want to only encrypt / pack when we communicate with the client
      bool						  mServerService;

      Service*                    mService;
      NetworkClient*              mClient;
      SocketReadThread*           mSocketReadThread;
      SocketWriteThread*          mSocketWriteThread;
      PacketFactory*              mPacketFactory;
      MessageFactory*             mMessageFactory;
      // Anh_Utils::Clock*           mClock;
      

      uint32                      mId;
      uint32                      mAddress;                 // stored in network order
      uint16                      mPort;                    // stored in network order
      uint32                      mEncryptKey;
      uint32                      mRequestId;
      uint32                      mOutgoingPingSequence;

      // Incoming fragmented packet processing.
      uint32                      mFragmentedPacketTotalSize;
      uint32                      mFragmentedPacketCurrentSize;
      uint16					  mFragmentedPacketStartSequence;
      uint16					  mFragmentedPacketCurrentSequence;

      uint32                      mRoutedFragmentedPacketTotalSize;
      uint32                      mRoutedFragmentedPacketCurrentSize;
      uint16					  mRoutedFragmentedPacketStartSequence;
      uint16					  mRoutedFragmentedPacketCurrentSequence;

      uint64                      mConnectStartEvent;       // For SCOM_Connect commands
      uint64                      mLastConnectRequestSent;  

      uint64                      mLastPacketReceived;      // General session timeout
      uint64                      mLastPacketSent;          // General session timeout


      uint64                      mLastPingPacketSent;          // General session timeout

      // Netstats
      uint32				      mServerTickCount;
      uint32                      mLastRoundtripTime;
      uint32                      mAverageRoundtripTime;
      uint32                      mShortestRoundtripTime;
      uint32                      mLongestRoundtripTime;
      uint64                      mClientPacketsSent;
      uint64                      mClientPacketsReceived;
      uint64                      mServerPacketsSent;
      uint64                      mServerPacketsReceived;

      // Reliability
      uint16                      mOutSequenceNext;
      uint16                      mInSequenceNext;
      
      bool						  mOutSequenceRollover;
      uint16                      mNextPacketSequenceSent;
      uint64                      mLastRemotePacketAckReceived;
      uint32                      mWindowSizeCurrent;		//amount of packets we want to send in one round
      uint32                      mWindowResendSize;	    //

      bool                        mSendDelayedAck;        // We processed some incoming packets, send an ack
      bool                        mInOutgoingQueue;       // Are we already in the queue?
      bool                        mInIncomingQueue;       // Are we already in the queue?

      uint16                      mLastSequenceAcked;

      SessionStatus               mStatus;
      SessionCommand              mCommand;

      // Message queues.
      MessageQueue                mOutgoingMessageQueue;		//here we store the messages given to us by the messagelib
      MessageQueue                mUnreliableMessageQueue;

      MessageQueue                mIncomingMessageQueue;
      MessageQueue				  mMultiMessageQueue;
      MessageQueue				  mRoutedMultiMessageQueue;
      MessageQueue				  mMultiUnreliableQueue;

      // Packet queues.
      PacketQueue                 mOutgoingReliablePacketQueue;		//these are packets put on by the sessionwrite thread to send
      PacketQueue                 mOutgoingUnreliablePacketQueue;   //build unreliables they will get send directly by the socket write thread  without storing for possible r esends
      PacketWindowList            mWindowPacketList;				//our build packets - ready to get send
      PacketWindowList			  mRolloverWindowPacketList;		//send packets after a rollover they await sending and / or acknowledgement by the client
      PacketWindowList			  mNewRolloverWindowPacketList;
      PacketWindowList            mNewWindowPacketList;	
      PacketWindowList			  mOutOfOrderPackets;

      PacketQueue                 mIncomingFragmentedPacketQueue;
      PacketQueue                 mIncomingRoutedFragmentedPacketQueue;
      PacketWindowList            mIncomingPacketList;				
      
      boost::recursive_mutex	  mSessionMutex;

      uint64					  lasttime;
      uint64					  avgTime;
      uint32                      avgPacketsbuild;
      uint32                      avgUnreliablesbuild;

      uint64					  mPacketBuildTimeLimit;
      uint64					  mLastWriteThreadTime;

      uint32					  endCount;
      uint16					  lowest;// the lowest packet requested from the server
      uint16					  lowestCount;// counts the requests up
      
};


//======================================================================================================================

#endif //ANH_NETWORKMANAGER_SESSION_H





