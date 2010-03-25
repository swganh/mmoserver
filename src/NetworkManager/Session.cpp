/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

// this will send out of orders and just skip the packets missing
//#define SEND_OUT_OF_ORDERS 
#define WIN32_LEAN_AND_MEAN
#include "Session.h"

#include "NetworkClient.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Service.h"
#include "SocketReadThread.h"
#include "SocketWriteThread.h"

#include "LogManager/LogManager.h"

#include "Common/MessageFactory.h"

#include <boost/thread/thread.hpp>

#include "Utils/rand.h"
#include "Utils/utils.h"

#if !defined(_MSC_VER)
#include <arpa/inet.h>
#endif

#include <algorithm>
#include <stdio.h>


//======================================================================================================================

Session::Session(void) :
mService(0),
mClient(0),
mSocketReadThread(0),
mSocketWriteThread(0),
mPacketFactory(0),
mMessageFactory(0),
// mClock(0),
mId(0),
mEncryptKey(0),
mRequestId(0),
mOutgoingPingSequence(1),
mFragmentedPacketTotalSize(0),
mFragmentedPacketCurrentSize(0),
mFragmentedPacketStartSequence(0),
mFragmentedPacketCurrentSequence(0),
mRoutedFragmentedPacketTotalSize(0),
mRoutedFragmentedPacketCurrentSize(0),
mRoutedFragmentedPacketStartSequence(0),
mRoutedFragmentedPacketCurrentSequence(0),
mConnectStartEvent(0),   
mLastConnectRequestSent(0),
mLastPacketReceived(0),
mLastPacketSent(0),
mLastRoundtripTime(0),
mAverageRoundtripTime(0),
mShortestRoundtripTime(0),
mLongestRoundtripTime(0),
mClientPacketsSent(0),
mClientPacketsReceived(0),
mServerPacketsSent(0),
mServerPacketsReceived(0),
mOutSequenceNext(0),
mInSequenceNext(0),
mSendDelayedAck(false),
mInOutgoingQueue(false),
mInIncomingQueue(false),
mStatus(SSTAT_Initialize),
mCommand(SCOM_None),
mPacketBuildTimeLimit(15),
avgTime(0),
avgPacketsbuild(0),
avgUnreliablesbuild(0),
lowestCount(0),
lowest(0)
{
	mConnectStartEvent = lasttime = Anh_Utils::Clock::getSingleton()->getLocalTime();       // For SCOM_Connect commands
	mLastConnectRequestSent = mConnectStartEvent;  
	

	mServerService = false;
	mMaxPacketSize = MAX_PACKET_SIZE;
	mMaxUnreliableSize= (uint32) MAX_PACKET_SIZE/2;

	mLastPingPacketSent = 0;
	
	endCount = 0;
	mHash = 0;

	mCompCryptor.Startup();

}

//======================================================================================================================

Session::~Session(void)
{					  
	gLogger->logMsgF("Session::~Session ",MSG_HIGH,this->getId());
	Message* message = 0;
	
    boost::recursive_mutex::scoped_lock lk(mSessionMutex);

    while(!mOutgoingMessageQueue.empty())
    {
      message = mOutgoingMessageQueue.front();
      mOutgoingMessageQueue.pop();

      // We're done with this message.
	  message->setPendingDelete(true);
	  message->mSession = NULL;
    }

	while(!mIncomingMessageQueue.empty())
	{
		message = mIncomingMessageQueue.front();
		mIncomingMessageQueue.pop();//is this actually calling the messages destructor ?

		// We're done with this message.
		message->setPendingDelete(true);
		message->mSession = NULL;
	}

	while(!mUnreliableMessageQueue.empty())
	{
		message = mUnreliableMessageQueue.front();
		mUnreliableMessageQueue.pop();

		// We're done with this message.
		message->setPendingDelete(true);
		message->mSession = NULL;
	}

	while(!mMultiMessageQueue.empty())
	{
		message = mMultiMessageQueue.front();
		mMultiMessageQueue.pop();

		// We're done with this message.
		message->setPendingDelete(true);
		message->mSession = NULL;
	}


	while(!mRoutedMultiMessageQueue.empty())
	{
		message = mRoutedMultiMessageQueue.front();
		mRoutedMultiMessageQueue.pop();

		// We're done with this message.
		message->setPendingDelete(true);
		message->mSession = NULL;
	}

	while(!mMultiUnreliableQueue.empty())
	{
		message = mMultiUnreliableQueue.front();
		mMultiUnreliableQueue.pop();

		// We're done with this message.
		message->setPendingDelete(true);
		message->mSession = NULL;
	}

	mCompCryptor.Shutdown();

}

//======================================================================================================================

void Session::Update(void)
{
	boost::recursive_mutex::scoped_lock lk(mSessionMutex);

	//
	// Process incoming packets.
	//
	if( mIncomingPackets.size() > 0 )
	{
		for( unsigned int x = 0; x < mIncomingPackets.size(); x++ )
		{
			HandleSessionPacket( mIncomingPackets.front() );
			mIncomingPackets.pop();
		}
	}

	//
	// ....?
	//
	ProcessWriteThread();

	//
	// Send pending outgoing packets.
	//
	if( mOutgoingPackets.size() > 0 )
	{
		for( unsigned int x = 0; x < mOutgoingPackets.size(); x++ )
		{
			getService()->getSocket()->async_send_to( 
				boost::asio::buffer(mOutgoingPackets.front()->getData(), mOutgoingPackets.front()->getSize()),
				getRemoteEndpoint(),
				boost::bind(&Service::HandleSendTo, getService(), mOutgoingPackets.front())
				);
			mOutgoingPackets.pop();
		}
	}
}


//======================================================================================================================

void Session::ProcessReadThread(void)
{

}

//======================================================================================================================

void Session::ProcessWriteThread(void)
{
	bool say = false;
  uint64 packetBuildTimeStart;
  uint64 packetBuildTime = 0;
  
  uint64 wholeTime = packetBuildTime = packetBuildTimeStart = Anh_Utils::Clock::getSingleton()->getLocalTime();

  //only process when we are busy - we dont need to iterate through possible resends all the time
  if((!mUnreliableMessageQueue.size())&&(!mOutgoingMessageQueue.size()))
  {
	  if(!mSendDelayedAck)
	  {
		if(packetBuildTimeStart - mLastWriteThreadTime < 300)
		{
			endCount++;
			return;
		}
	  }
  }
  
  mLastWriteThreadTime = packetBuildTimeStart;

  uint32 outSize = mOutgoingMessageQueue.size();

  uint32 pBuild = 0;
  uint32 pUnreliableBuild = 0;

  //build reliable packets
  while(((packetBuildTime - packetBuildTimeStart) < mPacketBuildTimeLimit) && mOutgoingMessageQueue.size())
  {
	pBuild += _buildPackets();
	packetBuildTime = Anh_Utils::Clock::getSingleton()->getLocalTime();
	
  }
	
  uint32 resendPackets = 0;
 

  //build unreliable packets
  packetBuildTime = packetBuildTimeStart = Anh_Utils::Clock::getSingleton()->getLocalTime();
  while(((packetBuildTime - packetBuildTimeStart) < mPacketBuildTimeLimit) && mUnreliableMessageQueue.size())
  {
	  //unreliables are directly put on the wire without getting in the way of our window
	  //this way they get lost when we have lag but thats not exactly a hughe problem
	  //and we dont get problems with our window list filled with unreliables
	  //which never get acked
	  pUnreliableBuild += _buildPacketsUnreliable();
	  packetBuildTime = Anh_Utils::Clock::getSingleton()->getLocalTime();
 	
  }

    boost::recursive_mutex::scoped_lock lk(mSessionMutex);

	for( unsigned int x = 0; x < mOutgoingPackets.size(); x++ )
	{
		_addOutgoingReliablePacket( mOutgoingPackets.front() );
		mOutgoingPackets.pop();
	}

	lk.unlock();
  
  // Handle any specific commands
  switch (mCommand)
  {
    case SCOM_Connect:
    {
      _processConnectCommand();      
      break;
    }
    case SCOM_Disconnect:
    {
		gLogger->logMsgF("handle Session disconnect %u endcount %u", MSG_HIGH, this->getId(),endCount);   
      _processDisconnectCommand();      
      break;
    }

    default:
      break;
  } //end switch(mCommand)

  // Process our timeouts.
  // MUAHARHARHARHAR Welcome to the Netlayer /me laughs dirtily
  // please note several things!!!!!!
  // ZONESERVER DO NOT HAVE ( I repeat : zoneservers DONOTHAVE) ha clientsession!!!! 
  // this means that in a zoneserver mServerSession will always be true
  // the only server with a (several truth be told, as much as we have clients) clientsession is the connectionserver
  // mServerSession is set to true via the netlayer initialization in connectionserver/ zoneserver.cpp
  // if we are spamming pings to the zone ( they are actually filtered out by our toolset) it is a good idea to look at
  // the pingserver 
  // to send 50 pings per second with the keep alive code here we would need 250 servers pinging.
  // interisting is however the question on how we decide which pings to mirror, and which to ignore
  // so for startes I will limit the (back) pinging to once per second.
  // (_processPingPacket)
  // What happened with the current code, was that every ping, once started caused the server to mirror it
  // there never was a throttle - so pinging between servers once started - never stopped.
  // As fast as it could possibly happen. Given another reason to send a ping - the pinging just was doubled.


  if (mStatus == SSTAT_Connected)
  {

	  uint64 t = Anh_Utils::Clock::getSingleton()->getLocalTime() - mLastPacketReceived;
	  t = (uint64)t/1000;
      if ((Anh_Utils::Clock::getSingleton()->getLocalTime() - mLastPacketReceived) > 60000)
      {
		  if(this->mServerService)
		  {
				gLogger->logMsgF("Session disconnect last received packet > 60 (%I64u) seconds session Id :%u", MSG_HIGH, t, this->getId());   
				gLogger->logMsgF("Session lastpacket %I64u now %I64u diff : %I64u", MSG_HIGH, mLastPacketReceived, Anh_Utils::Clock::getSingleton()->getLocalTime(),(Anh_Utils::Clock::getSingleton()->getLocalTime() - mLastPacketReceived));   
				mCommand = SCOM_Disconnect;
		  }
		  else
		  {
			gLogger->logMsgF("Session disconnect last received packet > 60 (%I64u) seconds session Id :%u", MSG_HIGH, t, this->getId());   
 
			mCommand = SCOM_Disconnect;
		  }
      }
	  else	  
	  if (this->mServerService && ((Anh_Utils::Clock::getSingleton()->getLocalTime() - mLastPacketReceived) > 10000))
	  {
		  //gLogger->logMsgF("Session send server server ping", MSG_HIGH);   
		  if((Anh_Utils::Clock::getSingleton()->getLocalTime() - mLastPingPacketSent) > 1000)
				_sendPingPacket();
	  }
      
  }
 
}


//======================================================================================================================
void Session::SendChannelA(Message* message)
{
	// Do some boundschecking.
	if (message->getPriority() > 0x10)
	{
		gLogger->logMsgF("Session::SendChannelA priority messup!!!", MSG_HIGH );
		gLogger->hexDump(message->getData(), message->getSize());
		return;
	}

	message->mSession = this;
	//check whether we are disconnecting  this happens when a client or server crashes without sending a disconnect
	//however in these cases we get a lot of stuck messages on the heap which are orphaned
	if(mStatus != SSTAT_Connected)
	{
		//gLogger->logMsgF("Session::SendChannelA :: we are not connected -destroy message", MSG_HIGH);
		message->setPendingDelete(true);		
		return;
	}

  
  //gLogger->logMsgF("Sending message - Session:0x%x%.4x", MSG_LOW, mService->getId(), getId());

    boost::recursive_mutex::scoped_lock lk(mSessionMutex);

  //the connectionserver puts a lot of fastpaths here  - so just put them were they belong
  //this alone takes roughly 5% cpu off of the connectionserver

	message->mSourceId = 1;

  if(message->getFastpath()&& (message->getSize() < mMaxUnreliableSize))
	  mUnreliableMessageQueue.push(message);
  else
  {
	  message->setFastpath(false);	  //send it as reliable if its to big
	  mOutgoingMessageQueue.push(message);
  }
}

void Session::SendChannelAUnreliable(Message* message)
{
	// Do some boundschecking.
	if (message->getPriority() > 0x10)
	{
		gLogger->logMsgF("Session::SendChannelAUnreliablepriority messup!!!", MSG_HIGH );
		gLogger->hexDump(message->getData(), message->getSize());
		return;
	}

	message->mSession = this;

	//check whether we are disconnecting
	if(mStatus != SSTAT_Connected)
	{
		//gLogger->logMsgF("Session::SendChannelA :: we are not connected -destroy packet", MSG_HIGH);
		message->setPendingDelete(true);		
		return;
	}
  
  //gLogger->logMsgF("Sending message - Session:0x%x%.4x", MSG_LOW, mService->getId(), getId());

  boost::recursive_mutex::scoped_lock lk(mSessionMutex);
  if(message->getSize() > mMaxUnreliableSize)	//I send the attribute messages as unreliables	 but they can be to big!!
  {
	  message->setFastpath(false);	  //send it as reliable if its to big
	  mOutgoingMessageQueue.push(message);
  }
  else
	mUnreliableMessageQueue.push(message);
}


void Session::SortSessionPacket(Packet* packet, uint16 type)
{
	packet->setReadIndex(2);
	switch (type)
	  {
		case SESSIONOP_DataChannel1:
		{
		  // All data channels get handled here.
		  _processDataChannelPacket(packet, false);
		  break;
		}

		// This is part of a message fragment.  Just add it to the packet queue as it must be handled further up.
		case SESSIONOP_DataFrag1:
		{
		  // Handle our fragmented packets here.
		  _processFragmentedPacket(packet);
		  break;
		}
		default:
		{
		  // Unknown SESSIONOP code
		  gLogger->logMsgF("Unhandled Sequenced Message %u",MSG_HIGH, type);
		  mPacketFactory->DestroyPacket(packet);
		  break;
		}
	  }


}
//======================================================================================================================
void Session::HandleSessionPacket(Packet* packet)
{
	// Reset our packet read index and start parsing...
	packet->setReadIndex(0);
	uint16 packetType = packet->getUint16();

	gLogger->logMsgF("Session::HandleSessionPacket %X", MSG_NORMAL, packetType);

	// Set our last packet time index
	mLastPacketReceived = Anh_Utils::Clock::getSingleton()->getLocalTime();
	mClientPacketsReceived++;

	// If this is fastpath data, send it up.
	if (packetType < 0x0100)
	{
	HandleFastpathPacket(packet);
	return;
	}

	//get rid of all the unsequenced stuff
	switch (packetType)
	{
	// New connection request.
	case SESSIONOP_SessionRequest:
	{
	  // This is a new session request.
	  _processSessionRequestPacket(packet);
	  return;
	}
	// New connection request.
	case SESSIONOP_SessionResponse:
	{
	  // This is the session response
	  mCommand = SCOM_None;
	  mStatus = SSTAT_Connected;
	  packet->getUint32();                          // unkown
	  mEncryptKey = ntohl(packet->getUint32());     // Encryption key
	  return;
	}

	case SESSIONOP_DataOrder1:
	{
	  _processDataOrderPacket(packet);
	  return;
	}

	// Multiple session packets 
	case SESSIONOP_MultiPacket:
	{
		// itself has no sequence but its contained packets might   - we dont send reliables in multis
		// however they get handled through this function anyway
	  if(!ValidatePacketCrc(packet))
		  return;
	  DeCompCryptPacket(packet);
	  _processMultiPacket(packet);
	  return;
	}
	// Remote side disconnceted
	case SESSIONOP_Disconnect:
	{

			gLogger->logMsgF("Session::remote side disconnected", MSG_HIGH);
	  mStatus = SSTAT_Disconnecting;
	  _processDisconnectPacket(packet);
	  return;
	}

	// Data channel acks
	case SESSIONOP_DataAck1:
	{
	  _processDataChannelAck(packet);
	  return;
	}
	    

	// Ping, don't know what it's for yet.
	case SESSIONOP_Ping:
	{
		//_sendPingPacket();
	  _processPingPacket(packet);
	  return;
	}
	// Network Status.  Must return a proper packet or network layer stops working.
	case SESSIONOP_NetStatRequest:
	{
	  if(!ValidatePacketCrc(packet))
		  return;
	  DeCompCryptPacket(packet);
	  _processNetStatRequestPacket(packet);
	  return;
	}
	}

	if(!ValidatePacketCrc(packet))
	  return;
	DeCompCryptPacket(packet);

	//now we have a sequenced packet
	//check if we are in sequence

	uint16 sequence = ntohs(packet->getUint16());

	gLogger->logMsgF("Sequence: %u | Expected Sequence: %u", MSG_NORMAL, sequence, mInSequenceNext);
	if( mInSequenceNext == sequence )
	{
		Packet* ack = mPacketFactory->CreatePacket();
		ack->addUint16(SESSIONOP_DataAck1);
		ack->addUint16(htons(sequence));
		ack->setIsCompressed(false);
		ack->setIsEncrypted(false);
		_addOutgoingUnreliablePacket(ack);

		SortSessionPacket(packet, packetType);
	}
	else
	{
		_handleOutOfOrderPacket(sequence);
		mPacketFactory->DestroyPacket(packet);
	}

}

//======================================================================================================================

void Session::_handleOutOfOrderPacket(uint16 sequence)
{
	Packet* packet = mPacketFactory->CreatePacket();
	packet->addUint16(SESSIONOP_DataOrder1);
	packet->addUint16(htons(sequence));
	packet->setIsCompressed(false);
	packet->setIsEncrypted(true);
	_addOutgoingUnreliablePacket(packet);
}

//======================================================================================================================

void Session::HandleFastpathPacket(Packet* packet)
{

	uint8		priority		= 0;
	uint8		routed			= 0;
	uint8		dest			= 0;
	uint32	accountId		= 0;

	// Fast path is raw data.  Just send it up.
	mLastPacketReceived = Anh_Utils::Clock::getSingleton()->getLocalTime();
	
	packet->setReadIndex(0);

	priority = packet->getUint8();
	if (priority > 0x10)
	{
		gLogger->logMsgF("Session::HandleFastpathPacket priority messup!!!", MSG_HIGH );
		return;
	}

	routed = packet->getUint8();
	if (routed)
	{
	  dest = packet->getUint8();
	  accountId = packet->getUint32();
	}

	// Create our message and send it up.
	mMessageFactory->StartMessage();

	if (routed)
	{
	  mMessageFactory->addData(packet->getData() + 7, packet->getSize() - 7); // +2 priority/routed, +5 routing header
	}
	else
	{
	  mMessageFactory->addData(packet->getData() + 2, packet->getSize() - 2); // +2 priority/routed
	}

	Message* newMessage = mMessageFactory->EndMessage();

		
	newMessage->setPriority(priority);
	
	newMessage->setDestinationId(dest);
	newMessage->setAccountId(accountId);
	newMessage->setFastpath(true);
	
	if (routed)
	  newMessage->setRouted(true);

	_addIncomingMessage(newMessage, priority);
	
	mPacketFactory->DestroyPacket(packet);
	//_processDataChannelPacket(packet, true);
}

//======================================================================================================================

void Session::DestroyIncomingMessage(Message* message)      
{ 
	message->setPendingDelete(true);
}

//======================================================================================================================

void Session::DestroyPacket(Packet* packet)
{
	mPacketFactory->DestroyPacket(packet);
}

//======================================================================================================================
Message* Session::getIncomingQueueMessage()
{
  Message* message = 0;

  
  if (!mIncomingMessageQueue.size())
    return message;

  boost::recursive_mutex::scoped_lock lk(mSessionMutex);

  message = mIncomingMessageQueue.front();
  mIncomingMessageQueue.pop();

  return message;
}


//======================================================================================================================
void Session::_processSessionRequestPacket(Packet* packet)
{   
	// one problem of the bots is that we might get an IP port combination several times
	if(mStatus > SSTAT_Connected)
	{
		gLogger->logMsgF("IP Port given multiple times ??",MSG_HIGH);
		return;
	}

  // retrieve our request id.
  packet->getUint32();                      // Unknown
  mRequestId = packet->getUint32();         // Request id.

  // Get a new encryption key if we're not already initilized.
  // If we're not a new session, then we should drop the packet.
  if (mStatus == SSTAT_Initialize)
  {
    seed_rand_mwc1616(static_cast<uint32>(Anh_Utils::Clock::getSingleton()->getLocalTime()));
    mEncryptKey = rand_mwc1616();
  }

  // Build a SessionResponse packet here.
  Packet* newPacket = mPacketFactory->CreatePacket();             
  newPacket->addUint16(SESSIONOP_SessionResponse);      // Session packet type
  newPacket->addUint32(mRequestId);
  newPacket->addUint32(htonl(mEncryptKey));
  newPacket->addUint8(2);
  newPacket->addUint8(1);
  newPacket->addUint8(4);
  newPacket->addUint32(htonl(mMaxPacketSize));
  newPacket->setIsCompressed(false);
  newPacket->setIsEncrypted(false);

  // This session is in the process of connecting
  mStatus = SSTAT_Connecting;

  // Push the packet on our outgoing queue
  _addOutgoingUnreliablePacket(newPacket);
  mService->AddSessionToProcessQueue(this);

  // Destroy our incoming packet, it's not needed any longer.
  mPacketFactory->DestroyPacket(packet);
}


//======================================================================================================================
void Session::_processDisconnectPacket(Packet* packet)
{
  // Let the application know there was a disconnect.
  mService->AddSessionToProcessQueue(this);

  // Destroy our incoming packet, it's not needed any longer.
  mPacketFactory->DestroyPacket(packet);
}


//======================================================================================================================
void Session::_processMultiPacket(Packet* packet)
{
  Packet* newPacket = 0;
  uint16 packetIndex = 0, packetSize = 0;

  ValidatePacketCrc(packet);
  DeCompCryptPacket(packet);
  
  // Iterate through our multi-packet

  while (packet->getReadIndex() < packet->getSize())
  {

	 if (packet->getReadIndex() >= packet->getSize())
	  gLogger->logMsgF("bad Multi-03 index:%u size:%u",MSG_HIGH,packet->getReadIndex(),packet->getSize());
    
	 // Create a new packet
    newPacket = mPacketFactory->CreatePacket();

    // Get the size of our next packet and increment our index
    packetSize = packet->getUint8();

    if(packetSize > 0)
    {
      // insert the data.
      newPacket->Reset();
      newPacket->addData(&(packet->getData()[packet->getReadIndex()]), packetSize);
      packet->setReadIndex(packet->getReadIndex() + packetSize);

      // Process the new packet.
      this->HandleSessionPacket(newPacket);
    }
	else
	{
		gLogger->logMsgF("bad Multi-03 index:%u size:%u oacket size == 0",MSG_HIGH,packet->getReadIndex(),packet->getSize());
		gLogger->hexDump(&(packet->getData()[0]),packet->getSize());

	}
  }

  // Destroy our incoming packet, it's not needed any longer.
  mPacketFactory->DestroyPacket(packet);
}


//======================================================================================================================
/*
void Session::_processDataChannelPacket(Packet* packet, bool fastPath)
{
  uint8		priority		= 0;
  uint8		routed			= 0;
  uint8		dest			= 0;
  uint32	accountId		= 0;
  //bool		destroyPacket	= true;
  

  // If we're fastpath, just send it up.  
  if(fastPath)
  {
		gLogger->logMsgF("Session::_processDataChannel :: fastpath", MSG_HIGH);
		assert(false);

		
  }
  else
  {
		// Otherwise ack this packet then send it up
		packet->setReadIndex(0);
		uint16 packetType = packet->getUint16();
		uint16 sequence = ntohs(packet->getUint16());

		//if (mService->getId() == 1)
		{
		  //gLogger->logMsgF("Incoming data - seq: %i expect: %u Session:0x%x%.4x", MSG_LOW, sequence, mInSequenceNext, mService->getId(), getId());
		}

	  // check to see if this is a multi-message message
	  //uint16 len = packet->getSize() - 4;  // -2 header, -2 sequence
	  priority = packet->getUint8();
	  routed = packet->getUint8();
	  
	  // If we're from the server, strip off our routing header.
	  if(routed == 1)
		{
			gLogger->logMsgF("Session::_processDataChannelB :: thats not routed ... sob :(", MSG_HIGH);
			assert(false);
		}
  
	  if (routed == 0x19)
	  {
		// Next byte is size
		uint32 size = packet->getUint8();

		do 
		{
		  // More size bytes?
		  if (size == 0xff)
		  {
			  size = ntohs(packet->getUint16());
		  }
	      
		  // We need to get theses again.
		  priority = packet->getUint8();
		  packet->getUint8();  // Routing byte not used inside a multi-message, just dump it. (only for channel A!!!)

		  // Init a new message for this data.
		  mMessageFactory->StartMessage();
		  mMessageFactory->addData(packet->getData() + packet->getReadIndex(), static_cast<uint16>(size) - 2); // -1 priority, -1 routing
		  Message* newMessage = mMessageFactory->EndMessage();

		  // set our account and server id's
		  newMessage->setAccountId(accountId);
		  newMessage->setDestinationId(dest);
		  newMessage->setPriority(priority);
		  assert(newMessage->getPriority() < 0x10);

		  // Need to specify whether this is routed or not here, so we know in the app
		  newMessage->setRouted(false);

		  // Push the message on our incoming queue
		  _addIncomingMessage(newMessage, priority);

		  // Advance the message index
		  packet->setReadIndex(packet->getReadIndex() + static_cast<uint16>(size) - 2); // -1 priority, -1 routing

		  size = packet->getUint8();
		}
		while (packet->getReadIndex() < packet->getSize() && size != 0);
	  }
	  else
	  {
		// Create our new message and send it up.
		mMessageFactory->StartMessage();
		mMessageFactory->addData(packet->getData() + packet->getReadIndex(), packet->getSize() - packet->getReadIndex());
		Message* newMessage = mMessageFactory->EndMessage();

		// Need to specify whether this is routed or not here, so we know in the app
		newMessage->setRouted(false);

		// set our message parameters
		newMessage->setAccountId(accountId);
		newMessage->setDestinationId(dest);
		newMessage->setPriority(priority);
		assert(newMessage->getPriority() < 0x10);

		// Push the message on our incoming queue
		_addIncomingMessage(newMessage, priority);
	  }

   // Set our inSequence number so we know we recieved this packet.
   // in sequence is per packet not per message 
   // with the one exception of 03's
   
	//gLogger->logMsgF("_processDataChannelPacket::increasing mInSequence by 1 %u", MSG_HIGH, mInSequenceNext);  
	mInSequenceNext++;
	mSendDelayedAck = true;
	//gLogger->logMsgF("_processDataChannelPacket::send ack sequence %u", MSG_HIGH, mInSequenceNext-1);  


  }

  // Destroy our incoming packet, it's not needed any longer.
	mPacketFactory->DestroyPacket(packet);
}
*/
void Session::_processDataChannelPacket(Packet* packet, bool fastPath)
{
  uint8		priority		= 0;
  uint8		routed			= 0;
  uint8		dest			= 0;
  uint32	accountId		= 0;
  bool		destroyPacket	= true;
  

  // If we're fastpath, just send it up.  
  if(fastPath)
  {
		gLogger->logMsgF("Session::_processDataChannel :: fastpath", MSG_HIGH);
		return;
  }
  else
  {
		// Otherwise ack this packet then send it up
		packet->setReadIndex(0);
		uint16 packetType = packet->getUint16();
		uint16 sequence = ntohs(packet->getUint16());

		//if (mService->getId() == 1)
		{
		  //gLogger->logMsgF("Incoming data - seq: %i expect: %u Session:0x%x%.4x", MSG_LOW, sequence, mInSequenceNext, mService->getId(), getId());
		}

		// check to see if this is a multi-message message
		//uint16 len = packet->getSize() - 4;  // -2 header, -2 sequence
		priority = packet->getUint8();
		if (priority > 0x10)
		{
			gLogger->logMsgF("Session::_processDataChannelPacket priority messup!!!", MSG_HIGH );
			gLogger->hexDump(packet->getData(), packet->getSize());
			return;
		}

	  routed = packet->getUint8();
	  
	  // If we're from the server, strip off our routing header.
	  if (routed == 0x01)
	  {
		dest = packet->getUint8();
		accountId = packet->getUint32();
	  }
  
	  if (routed == 0x19)
	  {
		// Next byte is size
		uint32 size = packet->getUint8();

		do 
		{
		  // More size bytes?
		  if (size == 0xff)
		  {
			  size = ntohs(packet->getUint16());
		  }
	      
		  // We need to get theses again.
		  priority = packet->getUint8();
		  packet->getUint8();  // Routing byte not used inside a multi-message, just dump it. (only for channel A!!!)

		  // Init a new message for this data.
		  mMessageFactory->StartMessage();
		  mMessageFactory->addData(packet->getData() + packet->getReadIndex(), static_cast<uint16>(size) - 2); // -1 priority, -1 routing
		  Message* newMessage = mMessageFactory->EndMessage();

		  // set our account and server id's
		  newMessage->setAccountId(accountId);
		  newMessage->setDestinationId(dest);
		  newMessage->setPriority(priority);

		  // Need to specify whether this is routed or not here, so we know in the app
		  newMessage->setRouted(false);

		  // Push the message on our incoming queue
		  _addIncomingMessage(newMessage, priority);

		  // Advance the message index
		  packet->setReadIndex(packet->getReadIndex() + static_cast<uint16>(size) - 2); // -1 priority, -1 routing

		  size = packet->getUint8();
		}
		while (packet->getReadIndex() < packet->getSize() && size != 0);
	  }
	  else
	  {
		// Create our new message and send it up.
		mMessageFactory->StartMessage();
		mMessageFactory->addData(packet->getData() + packet->getReadIndex(), packet->getSize() - packet->getReadIndex());
		Message* newMessage = mMessageFactory->EndMessage();

		// Need to specify whether this is routed or not here, so we know in the app
		if (routed)
		  newMessage->setRouted(true);
		else
		  newMessage->setRouted(false);

		// set our message parameters
		newMessage->setAccountId(accountId);
		newMessage->setDestinationId(dest);
		newMessage->setPriority(priority);

		// Push the message on our incoming queue
		_addIncomingMessage(newMessage, priority);
	  }

   // Set our inSequence number so we know we recieved this packet.
   // in sequence is per packet not per message 
   // with the one exception of 03's
   
	//gLogger->logMsgF("_processDataChannelPacket::increasing mInSequence by 1 %u", MSG_HIGH, mInSequenceNext);  
	mInSequenceNext++;
	mSendDelayedAck = true;
	//gLogger->logMsgF("_processDataChannelPacket::send ack sequence %u", MSG_HIGH, mInSequenceNext-1);  


  }

  // Destroy our incoming packet, it's not needed any longer.
  if(destroyPacket)
	mPacketFactory->DestroyPacket(packet);
}

//======================================================================================================================
void Session::_processDataChannelAck(Packet* packet)
{
	packet->setReadIndex(2);
	boost::recursive_mutex::scoped_lock lk(mSessionMutex);

	SequencedPacketMap::iterator			i = mOutgoingPacketCache.find(ntohs(packet->getUint16()));
	SequencedPacketMap::reverse_iterator	ri( i );

	//
	// The packet we are looking for does not exist.
	// Destroy our incoming packet and return.
	//
	if( i == mOutgoingPacketCache.end() )
	{
		mPacketFactory->DestroyPacket(packet);
		return;
	}

	for( ri; ri != mOutgoingPacketCache.rend(); ri++ )
		mPacketFactory->DestroyPacket(ri->second);

	mOutgoingPacketCache.erase( mOutgoingPacketCache.begin(), i );
	mPacketFactory->DestroyPacket(packet);
}


//======================================================================================================================
void Session::_processDataOrderPacket(Packet* packet)
{
	packet->setReadIndex(2);
	boost::recursive_mutex::scoped_lock lk(mSessionMutex);
	uint16 outOfOrderSequence = (ntohs(packet->getUint16()));

	SequencedPacketMap::iterator i = mOutgoingPacketCache.find(outOfOrderSequence);
	SequencedPacketMap::iterator lowIt = mOutgoingPacketCache.lower_bound( 0 );

	gLogger->logMsgF("Session::_processDataOrderPacket() packet with sequence %u Out of Order.", MSG_NORMAL, outOfOrderSequence);

	//
	// Uh oh!
	//
	if( i == mOutgoingPacketCache.end() )
	{
		gLogger->logErrorF("Network Manager", "Session::_processDataOrderPacket() packet with sequence %u does not exist in the cache.", MSG_NORMAL, outOfOrderSequence);
		mPacketFactory->DestroyPacket(packet);
		return;
	}


	//
	// Iterate from the lowest sequence to the Out of Order Sequence.
	//
	for( ; lowIt != i; lowIt++ )
		_addOutgoingReliablePacket(i->second);

	mPacketFactory->DestroyPacket(packet);
}

//======================================================================================================================
void Session::_processFragmentedPacket(Packet* packet)
{

	packet->setReadIndex(2); //skip the header
	uint16 sequence = ntohs(packet->getUint16());
	uint8 priority = 0;
	uint8 routed = 0;
	uint8 dest = 0;
	uint32 accountId = 0;

	//if (mService->getId() == 1)
	{
	//gLogger->logMsgF("Incoming fragment - seq: %i expect: %u Session:0x%x%.4x", MSG_LOW, sequence, mInSequenceNext, mService->getId(), getId());
	}

	// check our sequence number.
	if (sequence < mInSequenceNext)
	{
		// This is a duplicate packet that we've already recieved.
		gLogger->logMsgF("*** Duplicate Fragged Packet Recieved.  seq: %u", MSG_HIGH, sequence);

		// Destroy our incoming packet, it's not needed any longer.
		mPacketFactory->DestroyPacket(packet);
		return;
	}
	else if (sequence > mInSequenceNext)
	{
		gLogger->logMsgF("*** Fragged packet received out of order - expect: %u, received: %u", MSG_HIGH, mInSequenceNext, sequence);

		mPacketFactory->DestroyPacket(packet);
		return;
	}

	// Inc our in seq
	//gLogger->logMsgF("_processFragmentedPacket::increasing mInSequence by 1 %u", MSG_HIGH, mInSequenceNext);  
	mInSequenceNext++;
  
	// Need to send out acks
	mSendDelayedAck = true;
	//gLogger->logMsgF("_processFragmentedPacket::send ack sequence %u", MSG_HIGH, mInSequenceNext-1);  

	// If we are not already processing a multi-packet message, start to.
	if (mFragmentedPacketTotalSize == 0)
	{
	    mFragmentedPacketTotalSize = ntohl(packet->getUint32());

		mFragmentedPacketCurrentSize = packet->getSize() - 8;  // -2 header, -2 sequence, -4 size - crc compflag have already been removed
		mFragmentedPacketCurrentSequence = mFragmentedPacketStartSequence = sequence;

		packet->setReadIndex(8);	//2opcode, 2 sequence and 4 size
		priority = packet->getUint8();

		if (priority > 0x10)
		{
			// the packet has had a proper sequence .. otherwise we wouldnt be here ...
			gLogger->logMsgF("Start incoming fragged packets - total: %u seq:%u", MSG_HIGH, mFragmentedPacketTotalSize, sequence);
			gLogger->logMsgF("Session::_processFragmentedPacket priority messup!!!", MSG_HIGH );
			gLogger->hexDump(packet->getData(), packet->getSize());

		}

		// Now push the packet into our fragmented queue
		mIncomingFragmentedPacketQueue.push(packet);
	}
	// This is the next packet in the multi-packet sequence.
	else
	{
		mFragmentedPacketCurrentSize += packet->getSize() - 4;  // -2 header, -2 sequence
		mFragmentedPacketCurrentSequence = sequence;

		//if (mService->getId() == 1)
		{
			//gLogger->logMsgF("Incoming fragged packet - size: %u, total: %u current: %u seq:%u", MSG_LOW, packet->getSize() - 4, mFragmentedPacketTotalSize, mFragmentedPacketCurrentSize, sequence);
		}

		// If this is our last packet, send them all up to the application
		if (mFragmentedPacketCurrentSize >= mFragmentedPacketTotalSize)
		{
			//gLogger->logMsgF("Finalize received fragged packet - size: %u, total: %u current: %u seq:%u", MSG_LOW, packet->getSize() - 4, mFragmentedPacketTotalSize, mFragmentedPacketCurrentSize, sequence);
			mIncomingFragmentedPacketQueue.push(packet);
			Packet* fragment = 0;

			// Build the message from the fragmented packet here and send it up
			mMessageFactory->StartMessage();
			uint32 fragmentCount = mIncomingFragmentedPacketQueue.size();
			for (uint32 i = 0; i < fragmentCount; i++)
			{
				fragment = mIncomingFragmentedPacketQueue.front();
				mIncomingFragmentedPacketQueue.pop();

				// if this is the first packet, make sure to skip the size.
				if (i == 0)
				{
					fragment->setReadIndex(8);	//2opcode, 2 sequence and 4 size
					priority = fragment->getUint8();
					routed = fragment->getUint8();

				    if (routed)
				    {
						dest = fragment->getUint8();
						accountId = fragment->getUint32();
						mMessageFactory->addData(fragment->getData() + 15, fragment->getSize() - 15); // -2 header, -2 sequence, -4 size, -2 priority/routing, -5 routing header
				    }
				    else
				    {
						mMessageFactory->addData(fragment->getData() + 10, fragment->getSize() - 10); // -2 header, -2 sequence, -4 size, -2 priority/routing
					}
				}
				// This is just additional data
				else
				{
					mMessageFactory->addData(fragment->getData() + 4, fragment->getSize() - 4); // -2 header, -2 sequence 
				}

				// delete our fragment
				mPacketFactory->DestroyPacket(fragment);
		  }
		  Message* newMessage = mMessageFactory->EndMessage();

		  // Set our message variables.
		  if (routed)
			newMessage->setRouted(true);
		  else
		    newMessage->setRouted(false);

		  newMessage->setPriority(priority);
		  newMessage->setDestinationId(dest);
		  newMessage->setAccountId(accountId);
		  
		  if (priority > 0x10)
		  {
			  gLogger->logMsgF("Session::_processFragmentedPacket priority messup!!!", MSG_HIGH );
			  gLogger->hexDump(newMessage->getData(), newMessage->getSize());
			  return;
		  }


		  // Push the message on our incoming queue
		  _addIncomingMessage(newMessage, priority);

			// Clear our size counters
			mFragmentedPacketTotalSize = 0;
			mFragmentedPacketCurrentSize = 0;
			mFragmentedPacketCurrentSequence = 0;
			mFragmentedPacketStartSequence = 0;
	  }
	  // This is just the next packet in sequence.  Throw it on the queue
	  else
	  { 
	    mIncomingFragmentedPacketQueue.push(packet);
	  }
    
  }
}

//======================================================================================================================

void Session::_processPingPacket(Packet* packet)
{

	//with the old code *every* ping caused a new ping
	//so once pinged we kept pinging.
	//when we then decided to add a ping we just doubled the pinging
	//as fast as the servers possibly could spam packets

   if((Anh_Utils::Clock::getSingleton()->getLocalTime() - mLastPingPacketSent) < 1000)
   {
	   return;
   }

  // Client sends a simple 5 byte ping.
  if (packet->getSize() == 5)
  {
    // Echo the ping packet back.
    Packet* newPacket = mPacketFactory->CreatePacket();
    newPacket->addUint16(SESSIONOP_Ping);

    newPacket->setIsCompressed(false);
    newPacket->setIsEncrypted(true);

    // Push the packet on our outgoing queue
    _addOutgoingUnreliablePacket(newPacket);
	mLastPingPacketSent = Anh_Utils::Clock::getSingleton()->getLocalTime();
  }
  // Backend servers are larger to incorporate more features, 9 bytes(packet size).
  else
  {
    uint32 pingType = packet->getUint32();

    if (pingType == 1) // ping request
    {
      // Echo the ping packet back.
      Packet* newPacket = mPacketFactory->CreatePacket();
      newPacket->addUint16(SESSIONOP_Ping);
      newPacket->addUint32(2);    // ping response

      newPacket->setIsCompressed(false);
      newPacket->setIsEncrypted(true);

      // Push the packet on our outgoing queue
      _addOutgoingUnreliablePacket(newPacket);
	  mLastPingPacketSent = Anh_Utils::Clock::getSingleton()->getLocalTime();
    }
  }
  
  // Destroy our incoming packet, it's not needed any longer.
  mPacketFactory->DestroyPacket(packet);
}

//======================================================================================================================

void Session::_processNetStatRequestPacket(Packet* packet)
{
  uint16 tick = packet->getUint16();


  mLastRoundtripTime        = packet->getUint32();
  mAverageRoundtripTime     = packet->getUint32();
  mShortestRoundtripTime    = packet->getUint32();
  mLongestRoundtripTime     = packet->getUint32();
  packet->getUint32();
  
  mClientPacketsSent        = packet->getUint64();
  mClientPacketsReceived    = packet->getUint64();

  uint64 clientSent         = mClientPacketsSent;
  uint64 clientReceived     = mClientPacketsReceived;
  uint64 serverSent         = mClientPacketsReceived;//mServerPacketsSent;
  uint64 serverReceived     = mClientPacketsSent;//mServerPacketsReceived;

 
  //clientSent = swap64(clientSent);
  //clientReceived = swap64(clientReceived);
  //serverSent = swap64(serverSent);
  //serverReceived = swap64(serverReceived);
  
 
  Packet* newPacket = mPacketFactory->CreatePacket();
  newPacket->addUint16(SESSIONOP_NetStatResponse);
  newPacket->addUint16(tick);
  newPacket->addUint32(htonl(static_cast<uint32>(Anh_Utils::Clock::getSingleton()->getLocalTime()) + tick));
  newPacket->addUint64(clientSent);
  newPacket->addUint64(clientReceived);
  newPacket->addUint64(serverSent);
  newPacket->addUint64(serverReceived);

  // Set our compression and encryption flags
  newPacket->setIsCompressed(false);
  newPacket->setIsEncrypted(true);
  
  // Push the packet on our outgoing queue
  _addOutgoingUnreliablePacket(newPacket);

  // Destroy our incoming packet, it's not needed any longer.
  mPacketFactory->DestroyPacket(packet);
}


//======================================================================================================================
void Session::_sendPingPacket(void)
{
  mLastPingPacketSent = Anh_Utils::Clock::getSingleton()->getLocalTime();

  // Create a new ping packet and send it on.
  Packet* packet = mPacketFactory->CreatePacket();
  packet->addUint16(SESSIONOP_Ping);
  packet->addUint32(1);       // ping request

  // Set our compression and encryption flags
  packet->setIsCompressed(false);
  packet->setIsEncrypted(true);
  
  // Push the packet on our outgoing queue
  _addOutgoingUnreliablePacket(packet);
}


//======================================================================================================================
void Session::_processConnectCommand(void)
{
  // If we havne't started connecting yet, then do so
  if (mStatus == SSTAT_Initialize)
  {
    // Update our status
    mStatus = SSTAT_Connecting;
    mConnectStartEvent = Anh_Utils::Clock::getSingleton()->getLocalTime();
    mLastConnectRequestSent = 0;
  }

  // Otherwise, see if we need to send another request packet, or if our timeout expired
  if (mStatus == SSTAT_Connecting && mCommand == SCOM_Connect)
  {
    if (Anh_Utils::Clock::getSingleton()->getLocalTime() - mLastConnectRequestSent > 5000)  // Send a request packet every 5 seconds
    {
      // If we hit our timeout, then cancel the connect
      if (Anh_Utils::Clock::getSingleton()->getLocalTime() - mConnectStartEvent > 30000)   // Timeout at 30 seconds
      {
        // Cancel our connect command and put us in a back in an uninit state
        mStatus = SSTAT_Initialize;
        mCommand = SCOM_None;
      }
      else
      {
        mLastConnectRequestSent = Anh_Utils::Clock::getSingleton()->getLocalTime();

        // Build a session request packet and send it.
        Packet* newPacket = mPacketFactory->CreatePacket();
        newPacket->addUint16(SESSIONOP_SessionRequest);
        newPacket->addUint16((uint16)htonl(2));
        newPacket->addUint32(htonl(37563635));
        newPacket->addUint32(htonl(mMaxPacketSize));

        newPacket->setIsCompressed(false);
        newPacket->setIsEncrypted(false);

        // Send out packet out.
        _addOutgoingUnreliablePacket(newPacket);
      }
    }
  }

}


//======================================================================================================================
void Session::_processDisconnectCommand(void)
{

	//gLogger->logMsgF("Session::_processDisconnectCommand",MSG_HIGH);
	// Set our status and clear the command
	mStatus = SSTAT_Disconnecting;
	mCommand = SCOM_None;

	// Send a disconnect packet
	Packet* newPacket = mPacketFactory->CreatePacket();
	newPacket->addUint16(SESSIONOP_Disconnect);
	newPacket->addUint32(mRequestId);
	newPacket->addUint16(0x0006);

	newPacket->setIsCompressed(false);
	newPacket->setIsEncrypted(true);

	mService->AddSessionToProcessQueue(this);
	gLogger->logMsgF("Session::_processDisconnectCommand added session to processqueue", MSG_HIGH);
	// Send out packet out.
	_addOutgoingUnreliablePacket(newPacket);
}


//======================================================================================================================
void Session::_addOutgoingMessage(Message* message, uint8 priority, bool fastpath)
{

}


//======================================================================================================================
void Session::_addIncomingMessage(Message* message, uint8 priority)
{
  // simple bounds checking
  //assert(priority < 0x10);

    boost::recursive_mutex::scoped_lock lk(mSessionMutex);

  mIncomingMessageQueue.push(message);

  lk.unlock();
  
  // Let the service know we need to be processed.
  mService->AddSessionToProcessQueue(this);
}



//======================================================================================================================
void Session::_buildOutgoingReliableRoutedPackets(Message* message)
{
  Packet*	newPacket = 0;
  uint16	messageIndex = 0;
  uint16	envelopeSize = 0;
  uint16	messageSize = message->getSize();

  message->mSourceId = 2;

  // fragments envelope sizes  
  envelopeSize = 18; // -2 header -2 seq -4 size -1 priority -1 routed flag -1 route destination -4 account id -3 comp/CRC 

  // If we're too large to fit in a single packet, split us up.  
  if(messageSize + envelopeSize > mMaxPacketSize)		   //why >= ??
  {
	  //gLogger->logMsgF("Session::_buildRoutedfragmentedPacket sequence :  %u", MSG_HIGH,mOutSequenceNext);

    // Build our first packet with the total size.
    newPacket = mPacketFactory->CreatePacket(); 

	newPacket->addUint16(SESSIONOP_DataFrag1);
	
    newPacket->addUint16(htons(mOutSequenceNext));

	 newPacket->addUint32(htonl(messageSize + 7));	 

	newPacket->addUint8(message->getPriority());

    newPacket->addUint8(1);                               // There is a routing header next
    newPacket->addUint8(message->getDestinationId());
    newPacket->addUint32(message->getAccountId());
    newPacket->addData(message->getData(), mMaxPacketSize- envelopeSize); // -2 header, -2 sequence, -4 size, -2 priority/routing, -5 routing, -2 crc
    messageIndex += mMaxPacketSize - envelopeSize;                         // -2 header, -2 sequence, -4 size, -2 priority/routing, -5 routing, -2 crc
    

    // Data channels need compression and encryption
	// no compression in server server communication!
    newPacket->setIsCompressed(false);
	newPacket->setIsEncrypted(true);
    
    // Push the packet on our outgoing queue
    boost::recursive_mutex::scoped_lock lk(mSessionMutex);

	mOutgoingPackets.push(newPacket);
    mOutgoingPacketCache[mOutSequenceNext] = newPacket;
	mOutSequenceNext++;

    lk.unlock();

    // Now build any remaining packets.
    while (messageSize > messageIndex)
    {
      newPacket = mPacketFactory->CreatePacket();

      // Build our remaining packets
	  newPacket->addUint16(SESSIONOP_DataFrag1);
	
      newPacket->addUint16(htons(mOutSequenceNext));
      newPacket->addData(message->getData() + messageIndex, std::min<uint16>(mMaxPacketSize - 7, messageSize - messageIndex));

	  //no new routing header necessary here
      messageIndex += mMaxPacketSize - 7;  // -2 header, -2 sequence, -3 comp/crc

      // Data channels need compression and encryption
	  // no compression in server server communication!
     newPacket->setIsCompressed(false);
	
     newPacket->setIsEncrypted(true);
      
      // Push the packet on our outgoing queue
      boost::recursive_mutex::scoped_lock lk(mSessionMutex);

	  mOutgoingPacketCache[mOutSequenceNext] = newPacket;
	  mOutSequenceNext++;
    }
  }
  else
  {
   
    // Create a new packet and push the data into it.
    newPacket = mPacketFactory->CreatePacket();
	
	newPacket->addUint16(SESSIONOP_DataChannel1);
	//newPacket->setSequence(mOutSequenceNext);
	newPacket->addUint16(htons(mOutSequenceNext));
	newPacket->addUint8(message->getPriority());
		
	newPacket->addUint8(message->getRouted());
	newPacket->addUint8(message->getDestinationId());
	newPacket->addUint32(message->getAccountId());
	newPacket->addData(message->getData(), message->getSize());  // -2 header, -2 sequence, -2 priority/routing, -5 routing, -3 comp/crc
	newPacket->setIsCompressed(false);
	
    newPacket->setIsEncrypted(true);
    
    // Push the packet on our outgoing queue
    boost::recursive_mutex::scoped_lock lk(mSessionMutex);

	mOutgoingPackets.push(newPacket);
	mOutgoingPacketCache[mOutSequenceNext] = newPacket;
	mOutSequenceNext++;
  }
  message->setPendingDelete(true);
}



//======================================================================================================================
// reliable NOT Routed Packets

void Session::_buildOutgoingReliablePackets(Message* message)
{

  Packet*	newPacket = 0;
  uint16	messageIndex = 0;
  uint16	envelopeSize = 13;// -2 header -2 seq -4 size -1 priority -1 routed flag -3 comp/CRC 
  uint16	messageSize = message->getSize();
  
  // If we're too large to fit in a single packet, split us up.
  if(messageSize + envelopeSize > mMaxPacketSize) //why >= ??
  {
    // Build our first packet with the total size.
    newPacket = mPacketFactory->CreatePacket(); 

	newPacket->addUint16(SESSIONOP_DataFrag1);	

    newPacket->addUint16(htons(mOutSequenceNext));

    newPacket->addUint32(htonl(messageSize + 2));

	newPacket->addUint8(message->getPriority());
    
    newPacket->addUint8(0);                                       // This byte is always 0 on the client
    newPacket->addData(message->getData(), mMaxPacketSize - envelopeSize); // -2 header, -2 sequence, -4 size, -2 priority/routing, -2 crc
    messageIndex += mMaxPacketSize - envelopeSize;                         // -2 header, -2 sequence, -4 size, -2 priority/routing, -2 crc
    
    // Data channels need compression and encryption
	// as this is server client communication
	newPacket->setIsCompressed(true);
    newPacket->setIsEncrypted(true);
    
    // Push the packet on our outgoing queue
    boost::recursive_mutex::scoped_lock lk(mSessionMutex);

	mOutgoingPackets.push(newPacket);
	mOutgoingPacketCache[mOutSequenceNext] = newPacket;
    mOutSequenceNext++;

    // Now build any remaining packets.
    while (messageSize > messageIndex)
    {
      newPacket = mPacketFactory->CreatePacket();

      // Build our remaining packets
	 newPacket->addUint16(SESSIONOP_DataFrag1);

      newPacket->addUint16(htons(mOutSequenceNext));
      newPacket->addData(message->getData() + messageIndex, std::min<uint16>(mMaxPacketSize - 7, messageSize - messageIndex));

      messageIndex += mMaxPacketSize - 7;  // -2 header, -2 sequence, -3 comp/crc

      // Data channels need compression and encryption
	  // no compression in server server communication!
   	 newPacket->setIsCompressed(true);

      newPacket->setIsEncrypted(true);
      
      // Push the packet on our outgoing queue
      boost::recursive_mutex::scoped_lock lk(mSessionMutex);

	  mOutgoingPackets.push(newPacket);
	  mOutgoingPacketCache[mOutSequenceNext] = newPacket;
	  mOutSequenceNext++;
    }
  }
  else
  {
   
    // Create a new packet and push the data into it.
    newPacket = mPacketFactory->CreatePacket();
	
	newPacket->addUint16(SESSIONOP_DataChannel1);
	//newPacket->setSequence(mOutSequenceNext);
	newPacket->addUint16(htons(mOutSequenceNext));
	newPacket->addUint8(message->getPriority());
	newPacket->addUint8(0);//NOT routed
	newPacket->addData(message->getData(), message->getSize());  // -2 header, -2 sequence, -2 priority/routing, -5 routing, -2 crc
	
	// Data channels need compression and encryption 
	// no compression in server server communication!
	newPacket->setIsCompressed(true);

    newPacket->setIsEncrypted(true);
    
    // Push the packet on our outgoing queue
    boost::recursive_mutex::scoped_lock lk(mSessionMutex);

	mOutgoingPackets.push(newPacket);
	mOutgoingPacketCache[mOutSequenceNext] = newPacket;
	mOutSequenceNext++;
  }
  message->setPendingDelete(true);
}


//======================================================================================================================
// Unreliable Packet Routed or server / client

void Session::_buildOutgoingUnreliablePackets(Message* message)
{
  Packet* newPacket = 0;
  uint16 messageIndex = 0;

  // Create a new packet and push the data into it.
  newPacket = mPacketFactory->CreatePacket();
  newPacket->addUint8(message->getPriority());
  newPacket->addUint8(message->getRouted());

  // If we're routed, prepend the routing header...
  if (message->getRouted())
  {
    newPacket->addUint8(message->getDestinationId());
    newPacket->addUint32(message->getAccountId());
  }
  newPacket->addData(message->getData(), message->getSize());

  // dont compress unreliables
  newPacket->setIsCompressed(false); 
  newPacket->setIsEncrypted(true);
  
  // Push the packet on our outgoing queue
  _addOutgoingUnreliablePacket(newPacket);
  
  message->setPendingDelete(true);
  
}


//======================================================================================================================
void Session::_addOutgoingReliablePacket(Packet* packet)
{
  // Push the packet on our outgoing queue
  boost::recursive_mutex::scoped_lock lk(mSessionMutex);

  // Set our last packet sent time index
  //packet->setTimeQueued(Anh_Utils::Clock::getSingleton()->getLocalTime());
  mOutgoingPackets.push(packet); 

  //gLogger->logMsgF("ReliableQueueAdd: Type:0x%.4x, Session:0x%x%.4x", MSG_LOW, packet->getPacketType(), mService->getId(), getId());
}


//======================================================================================================================
void Session::_addOutgoingUnreliablePacket(Packet* packet)
{
  // Push the packet on our outgoing queue
  boost::recursive_mutex::scoped_lock lk(mSessionMutex);

  // Set our last packet sent time index
  mOutgoingPackets.push(packet);

  //gLogger->logMsgF("UnreliableQueueAdd: Type:0x%.4x, Session:0x%x%.4x", MSG_LOW, packet->getPacketType(), mService->getId(), getId());
}

//======================================================================================================================

uint32 Session::_buildPackets()
{

	// 2 things
	// 1st try to make as few packets as possible !!!
	// thats a must for server server communication - the soe protocol is extremely expensive in cpu cycles

	// 2nd are there any ways a session can have to generate routed and not routed packets ????? - No its either or

	uint32 packetsbuild = 0;
	boost::recursive_mutex::scoped_lock lk(mSessionMutex);

	//get our message

	Message* message = mOutgoingMessageQueue.front();
	mOutgoingMessageQueue.pop();

	message->mPath = MP_buildPacketStarted;

	//are there still any fastpath packets around at this point ?
	assert(!message->getFastpath() && "No Fastpath messages should reach this point");

	//=================================
	// messages need to be of a certain size to make multimessages viable
	// so sort out the big ones or those which are alone in the queue and make a single packet if necessary

	if(!mOutgoingMessageQueue.size()
	//|| (message->getRouted() ^ mOutgoingMessageQueue.front()->getRouted())	 
	//|| message->getFastpath()
	|| message->getSize() + mOutgoingMessageQueue.front()->getSize() > mMaxPacketSize - 21)
	
	{
		//if (message->getFastpath()&&(message->getSize()<=mMaxUnreliableSize))
		//{
		//	packetsbuild++;
		//	_buildOutgoingUnreliablePackets(message);
		//}
		//else
		{
			packetsbuild++;
			
			if(message->getRouted())
				_buildOutgoingReliableRoutedPackets(message);
			else
				_buildOutgoingReliablePackets(message);
		}

		message->setPendingDelete(true);
	}
	else 
	{
		if(message->getRouted() && mOutgoingMessageQueue.front()->getRouted())
		{
			mRoutedMultiMessageQueue.push(message);

			//cave we *might* have 2bytes for Size !!!!!!  (if size 255 or bigger)
			uint16 baseSize = 19 + message->getSize(); // 2 header, 2 sequence, 2 0019, 1(3) size,7 prio/routing, 3 comp/crc
			packetsbuild++;
			while(baseSize < mMaxPacketSize && mOutgoingMessageQueue.size())
			{
				message = mOutgoingMessageQueue.front();
							
				baseSize += (message->getSize() + 10); // size + prio + routing	 //thats supposed to be 8
				//cave size *might* be > 255  so using 3 (1 plus 2) for size as a standard!!

				if(baseSize >= (mMaxPacketSize) || (!message->getRouted()) )
					break;

				mOutgoingMessageQueue.pop();
				mRoutedMultiMessageQueue.push(message);
			}
			_buildRoutedMultiDataPacket();
		}
		else if((!message->getRouted()) && (!mOutgoingMessageQueue.front()->getRouted()) )
		{
			mMultiMessageQueue.push(message);

			uint16 baseSize = 14 + message->getSize(); // 2 header, 2 sequence, 2 0019, 1 size(3) ,2 prio/routing, 3 comp/crc
			packetsbuild++;
			while(baseSize < mMaxPacketSize && mOutgoingMessageQueue.size())
			{
				message->mPath = MP_Multi;
				message = mOutgoingMessageQueue.front();
							
				baseSize += (message->getSize() + 5); // size + prio + routing   cave size *might be > 255 so using 3 (1+2) for size as a standard!!

				if(baseSize >= mMaxPacketSize || message->getRouted() || message->getSize() > 252)
					break;

				mOutgoingMessageQueue.pop();
				mMultiMessageQueue.push(message);
		
			}

			_buildMultiDataPacket();
		}
		else
		{
			packetsbuild++;
			if(!message->getRouted())
				_buildOutgoingReliablePackets(message);
			else
			   _buildOutgoingReliableRoutedPackets(message);
			message->setPendingDelete(true);
		}
	}
	if(message)
		message->mPath = MP_buildPacketEnded;
	return(packetsbuild);
}

//======================================================================================================================

uint32 Session::_buildPacketsUnreliable()
{

	uint32 packetsbuild = 0;
	boost::recursive_mutex::scoped_lock lk(mSessionMutex);

	//gLogger->logMsgF("session build packets queue size : %u ",MSG_NORMAL,mOutgoingMessageQueue.size());

	Message* message = mUnreliableMessageQueue.front();
	mUnreliableMessageQueue.pop();
	message->mPath = MP_buildPacketUnreliable;

	// no larger ones than ff yet, we want at least 2 messages to fit in, dont use routed mesages, so the frontline server does packing only
	if(!mUnreliableMessageQueue.size()
	|| message->getRouted() || mUnreliableMessageQueue.front()->getRouted() 
	|| message->getSize() > 252 || mUnreliableMessageQueue.front()->getSize() > 252 //sizebyte so 255 is max including header
	|| message->getSize() + mUnreliableMessageQueue.front()->getSize() > mMaxUnreliableSize - 16)
	{
		packetsbuild++;
		_buildOutgoingUnreliablePackets(message);
	
		message->setPendingDelete(true);
	}
	else 
	{
		mMultiUnreliableQueue.push(message);

		uint16 baseSize = 12 + message->getSize(); // 2 header, 2 sequence, 2 0019, 1 size,2 prio/routing, 3 comp/crc
		packetsbuild++;
		while(baseSize < mMaxUnreliableSize && mUnreliableMessageQueue.size())
		{
			message = mUnreliableMessageQueue.front();
			message->mPath = MP_buildPacketUnreliable;
						
			baseSize += message->getSize() + 3; // size + prio + routing

			if(baseSize >= mMaxPacketSize || message->getRouted() || message->getSize() > 252)
				break;

			mUnreliableMessageQueue.pop();
			mMultiUnreliableQueue.push(message);
		}

		_buildUnreliableMultiDataPacket();
	}

	return(packetsbuild);
}


void Session::_buildMultiDataPacket()
{
	Packet*		newPacket = mPacketFactory->CreatePacket();
	Message*	message = 0;

	newPacket->addUint16(SESSIONOP_DataChannel1);
	newPacket->addUint16(htons(mOutSequenceNext));
	newPacket->addUint16(0x1900);

	while(!mMultiMessageQueue.empty())
	{
		message = mMultiMessageQueue.front();
		mMultiMessageQueue.pop();
		
		if((message->getSize() + 2)> 254)
		{
			newPacket->addUint8(0xff);
			newPacket->addUint16(htons(message->getSize() + 2)); // count priority + routing flag;
		}
		else
			newPacket->addUint8(message->getSize() + 2); // count priority + routing flag

		newPacket->addUint8(message->getPriority());
		newPacket->addUint8(0);	//Routing byte -> zero for channel1
		newPacket->addData(message->getData(), message->getSize()); 

		message->setPendingDelete(true);
	}

	newPacket->setIsCompressed(true);
	newPacket->setIsEncrypted(true);

	boost::recursive_mutex::scoped_lock lk(mSessionMutex);

	mOutgoingPackets.push(newPacket);
	mOutgoingPacketCache[mOutSequenceNext] = newPacket;
	mOutSequenceNext++;
}

//======================================================================
// Routed multidatas for server server communication only !!!!


void Session::_buildRoutedMultiDataPacket()
{
	Packet*		newPacket = mPacketFactory->CreatePacket();
	Message*	message = 0;
	
	newPacket->addUint16(SESSIONOP_DataChannel1); //server server communication !!!!!
	newPacket->addUint16(htons(mOutSequenceNext));
	newPacket->addUint16(0x1900);
	//newPacket->addUint8(0);
	//newPacket->addUint8(0x19);

	while(!mRoutedMultiMessageQueue.empty())
	{
		message = mRoutedMultiMessageQueue.front();
		mRoutedMultiMessageQueue.pop();

		//assert((message->getSize() + 7)< 255);
		if((message->getSize() + 7) > 254)
		{
			newPacket->addUint8(0xff);
			newPacket->addUint16(htons(message->getSize() + 7)); // count priority + routing flag;
		}
		else
			newPacket->addUint8(message->getSize() + 7); // count priority + routing flag


		newPacket->addUint8(message->getPriority());
		newPacket->addUint8(1);		//routed
		
		newPacket->addUint8(message->getDestinationId());
		newPacket->addUint32(message->getAccountId());
			//dest = fragment->getUint8();
            //accountId = fragment->getUint32();
		

		newPacket->addData(message->getData(), message->getSize()); 

		message->setPendingDelete(true);

	}

	newPacket->setIsCompressed(false); //server server !!! save the cycles!!!
	newPacket->setIsEncrypted(true);

	boost::recursive_mutex::scoped_lock lk(mSessionMutex);
	mOutgoingPackets.push(newPacket);
	mOutgoingPacketCache[mOutSequenceNext] = newPacket;
	mOutSequenceNext++;
}

//======================================================================
// Unreliable Multi DataPacket
// thats server client communication
void Session::_buildUnreliableMultiDataPacket()
{
	Packet*		newPacket = mPacketFactory->CreatePacket();
	Message*	message = 0;

	newPacket->addUint16(SESSIONOP_MultiPacket);
	//newPacket->addUint16(htons(mOutSequenceNext));

	

	while(!mMultiUnreliableQueue.empty())
	{
		message = mMultiUnreliableQueue.front();
		mMultiUnreliableQueue.pop();

		assert((message->getSize() + 2)<= 255 && "Message size should never be exceeded by this point");
		assert(!message->getRouted() && "Message should be routed by this point");
		assert(message->getFastpath() && "Only fastpath messages should be handled here");

		newPacket->addUint8(message->getSize() + 2); // count priority + routing flag
		newPacket->addUint8(message->getPriority());
		newPacket->addUint8(0);
		newPacket->addData(message->getData(), message->getSize()); 

		message->setPendingDelete(true);
	}

	newPacket->setIsCompressed(true);
	newPacket->setIsEncrypted(true);

	//unreliables go on wire directly
	_addOutgoingUnreliablePacket(newPacket);
	
}

//======================================================================================================================

bool Session::ValidatePacketCrc(Packet* packet)
{
	uint32 packetCrc = mCompCryptor.GenerateCRC(packet->getData(), packet->getSize() - 2, getEncryptKey());
	
	uint8 crcLow = (uint8)*(packet->getData() + (packet->getSize() - 1));
	uint8 crcHigh = (uint8)*(packet->getData() + (packet->getSize() - 2));

	if( crcLow != (uint8)packetCrc || crcHigh != (uint8)(packetCrc >> 8) )
	{
		gLogger->logMsg("*** Reliable Packet dropped. CRC mismatch.");
		return false;
	}

	return true;
}

//======================================================================================================================

void Session::DeCompCryptPacket(Packet* packet)
{

	mCompCryptor.Decrypt(packet->getData() + 2, packet->getSize() - 4, getEncryptKey());

	Packet* decompressPacket = mPacketFactory->CreatePacket();
	uint16 decompressLen = mCompCryptor.Decompress( packet->getData() + 2, packet->getSize() - 5, decompressPacket->getData()+2, decompressPacket->getMaxPayload() - 5);

	if(decompressLen > 0)
	{
		packet->setIsCompressed(true);
		memcpy( packet->getData()+2, decompressPacket->getData()+2, decompressPacket->getSize());
	}
	else
	{
		packet->setSize( packet->getSize() - 3 );
	}

	mPacketFactory->DestroyPacket(decompressPacket);

}

//======================================================================================================================