/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

// this will send out of orders and just skip the packets missing
//#define SEND_OUT_OF_ORDERS 

#include "Session.h"

#include "NetworkClient.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Service.h"
#include "SocketReadThread.h"
#include "SocketWriteThread.h"

#include "LogManager/LogManager.h"

#include "Common/MessageFactory.h"

#include "Utils/rand.h"
#include "Utils/utils.h"

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
mAddress(0),
mPort(0),
mEncryptKey(0),
mRequestId(0),
mOutgoingPingSequence(1),
mFragmentedPacketTotalSize(0),
mFragmentedPacketCurrentSize(0),
mFragmentedPacketStartSequence(0),
mFragmentedPacketCurrentSequence(0),
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
mOutSequenceRollover(false),
mNextPacketSequenceSent(0),
mLastRemotePacketAckReceived(0),
mWindowSizeCurrent(8000),
mWindowResendSize(8000),
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
	// We do have a global clock object, don't use seperate clock and times for every process.
	// mClock = new Anh_Utils::Clock(); 

	mConnectStartEvent = lasttime = Anh_Utils::Clock::getSingleton()->getLocalTime();       // For SCOM_Connect commands
	mLastConnectRequestSent = mConnectStartEvent;  

	mLastPacketReceived = mConnectStartEvent;      // General session timeout
	mLastPacketSent = mConnectStartEvent;          // General session timeout
	mLastRemotePacketAckReceived = mConnectStartEvent;          // General session timeout 
	

	mServerService = false;
	mMaxPacketSize = MAX_PACKET_SIZE;
	mMaxUnreliableSize= (uint32) MAX_PACKET_SIZE/2;
	
	endCount = 0;

}

//======================================================================================================================

Session::~Session(void)
{					  
	//gLogger->logMsgF("Session::~Session ",MSG_HIGH,this->getId());
	Message* message = 0;

	
	mSessionMutex.acquire();

    while(!mOutgoingMessageQueue.empty())
    {
      message = mOutgoingMessageQueue.front();
      mOutgoingMessageQueue.pop();

      // We're done with this message.
      mMessageFactory->DestroyMessage(message);
    }

	while(!mIncomingMessageQueue.empty())
	{
		message = mIncomingMessageQueue.front();
		mIncomingMessageQueue.pop();

		// We're done with this message.
		mMessageFactory->DestroyMessage(message);
	}
	

	mSessionMutex.release();

	// delete(mClock);
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
  
  
  if((wholeTime - lasttime )>5000 && (mWindowPacketList.size() > 100))
  {
	  say = true;
	  lasttime =	packetBuildTime;
	  
  }

  // Process our delayed ack here
  if(mSendDelayedAck)
  {
	  
    //please note that we push it directly on the packet queue it wont be build as unreliable and wont end up in a 00 03
    // clear our send flag
    mSendDelayedAck = false;

    // send an ack for this packet.
    Packet* ackPacket = mPacketFactory->CreatePacket();
    ackPacket->addUint16(SESSIONOP_DataAck1);
    ackPacket->addUint16(htons(mInSequenceNext - 1));
    
    //if (mService->getId() == 1)
    //{
     //gLogger->logMsgF("Sending ACK - Sequence: %u, Session:0x%x%.4x", MSG_HIGH, mInSequenceNext - 1, mService->getId(), getId());
    //}

    // Acks only need encryption
    ackPacket->setIsCompressed(false);
    ackPacket->setIsEncrypted(true);
  
    // Push the packet on our outgoing queue
    _addOutgoingUnreliablePacket(ackPacket);
  }

  // We need to prepare any outgoing packets here
  // Do we have any room for more reliable packets?



  
  uint32 pBuild = 0;
  uint32 pUnreliableBuild = 0;
  //while(((packetBuildTime - packetBuildTimeStart) < mPacketBuildTimeLimit) && ((mRolloverWindowPacketList.size() + mWindowPacketList.size()) < mWindowSizeCurrent) && mOutgoingMessageQueue.size())


 while(((packetBuildTime - packetBuildTimeStart) < mPacketBuildTimeLimit) && ((mRolloverWindowPacketList.size() + mWindowPacketList.size()) < mWindowSizeCurrent) && mOutgoingMessageQueue.size())
  {
	pBuild += _buildPackets();
	packetBuildTime = Anh_Utils::Clock::getSingleton()->getLocalTime();
	
  }

 uint32 resendPackets = 0;
 
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

	// Now check to see if we can send any more reliable packets out the wire yet.  NOT optimized
	PacketWindowList::iterator	iter;
	PacketWindowList::iterator	iterRoll;

	Packet*						windowPacket	= NULL;
	uint32						packetsSent		= 0;

	//Rollover happens when our sequence reaches 65535
	//the old (< sequence = 0)packets go in the rolloverqueue and wait for being send and/or acknowledged and then deleted
	if(mOutSequenceRollover)
	{
		mSessionMutex.acquire();

		iterRoll = mRolloverWindowPacketList.begin();
		
		//the list contains all packets those already send / resend and those not already send are in the back
		//this means we always iterate through the oldest packets first until we get to the new packets
		//up to the point that our packetwindowsize of packets is send

		//it will make sense to use a separate list for already send packets as we do not want to iterate constantly through
		//old but not yet acknowledged packets - this doesnt seem to be a problem though

		while(iterRoll != mRolloverWindowPacketList.end())
		{

			windowPacket = *iterRoll;
			windowPacket->setReadIndex(2);
			uint16 sequence = ntohs(windowPacket->getUint16());

			
			if(Anh_Utils::Clock::getSingleton()->getLocalTime() - windowPacket->getTimeSent() > 5000) //(mAverageRoundtripTime + mLastRoundtripTime) / 2)
			{
				uint32 resends = windowPacket->getResends();

				if(resends > 10)
				{
					//check whether server!!!
					mCommand = SCOM_Disconnect;

					say = true;
					mLastPacketDestroyed  = Anh_Utils::Clock::getSingleton()->getLocalTime();
					gLogger->logMsgF("_buildPackets() destroying packet resends > 10 session: %u", MSG_HIGH,this->mId);

					mPacketFactory->DestroyPacket(windowPacket);
					iterRoll = mRolloverWindowPacketList.erase(iterRoll);

					continue;
				}
				else
				{
					resendPackets ++;
					// Resend this packet as it's been too here too long.
					//gLogger->logMsgF("*** Rollover Resending last reliable packet. seqthis: %u nextseqsent: %u nextout: %u Session:0x%x%.4x", MSG_NORMAL,sequence,mNextPacketSequenceSent,mOutSequenceNext,mService->getId(), getId());

					windowPacket->setResends(windowPacket->getResends() + 1);
					_addOutgoingReliablePacket(windowPacket);
					

					//we need to leave the window open otherwise we have to much stalling
					//as it is the threads on the connectionserver might be stalled if we have to much clients
					//and we might not get our acks fast enough
					if(mWindowSizeCurrent > 20)
						mWindowSizeCurrent--;

					packetsSent++;

			
					if (packetsSent >= (mWindowSizeCurrent))
						break;
				}
			}
			else
				//packet not old enough for resend so break out!
				break;
			
			++iterRoll;
		}
		
		iterRoll = mNewRolloverWindowPacketList.begin();

		while(iterRoll != mNewRolloverWindowPacketList.end())
		{

			windowPacket = *iterRoll;
			windowPacket->setReadIndex(2);
			uint16 sequence = ntohs(windowPacket->getUint16());

			
			// If we've sent our mWindowSizeCurrent of packets, break out and wait for some acks.
			// make sure we send at least a minimum as we dont wont any stalling
			if (packetsSent >= mWindowSizeCurrent)
				break;

			_addOutgoingReliablePacket(windowPacket);
			iterRoll = mNewRolloverWindowPacketList.erase(iterRoll);
			mRolloverWindowPacketList.push_back(windowPacket);
			packetsSent++;

			mNextPacketSequenceSent++;
		
		}


		mSessionMutex.release();
	}

	resendPackets = 0;

	mSessionMutex.acquire();

	iter = mWindowPacketList.begin();

	//WindowPacketList is our current window of send and not yet acknowledged packets
	//A Rollover might still be in existance

	while(iter != mWindowPacketList.end())
	{

		windowPacket = *iter;
		windowPacket->setReadIndex(2);
		uint16 sequence = ntohs(windowPacket->getUint16());

		if(Anh_Utils::Clock::getSingleton()->getLocalTime() - windowPacket->getTimeSent() > 5000) //(mAverageRoundtripTime + mLastRoundtripTime) / 2)
		{
			uint32 resends = windowPacket->getResends();

			if(resends > 10)
			{

				say = true;
				mCommand = SCOM_Disconnect;
				mLastPacketDestroyed  = Anh_Utils::Clock::getSingleton()->getLocalTime();
				gLogger->logMsgF("_buildPackets() destroying packet resends > 10 session: %u", MSG_HIGH,this->mId);

				mPacketFactory->DestroyPacket(windowPacket);
				iter = mWindowPacketList.erase(iter);

				continue;
			}
			else
			{
				resendPackets ++;
				// Resend this packet as it's been too here too long.
				//gLogger->logMsgF("*** Resending last reliable packet. seqthis: %u nextseqsent: %u nextout: %u Session:0x%x%.4x", MSG_NORMAL, sequence,mNextPacketSequenceSent,mOutSequenceNext,mService->getId(), getId());
				 _addOutgoingReliablePacket(windowPacket);

				windowPacket->setResends(windowPacket->getResends() + 1);

				//ensure we do at least a minimum of sends or resends so stalling wont happen
				if(mWindowSizeCurrent > 20)
					mWindowSizeCurrent--;
				
				packetsSent++;

				
				if (packetsSent >= (mWindowSizeCurrent))
					break;
			}
		}
		else
			//packets not old enough for resend - dont waste any more time here
			break;


		++iter;
	
	}

	iter = mNewWindowPacketList.begin();

	while(iter != mNewWindowPacketList.end())
	{

		windowPacket = *iter;
		windowPacket->setReadIndex(2);
		uint16 sequence = ntohs(windowPacket->getUint16());

		
		// If we've sent our mWindowSizeCurrent of packets, break out and wait for some acks.
		// make sure we send at least a minimum as we dont wont any stalling
		if (packetsSent >= mWindowSizeCurrent)
			break;

		_addOutgoingReliablePacket(windowPacket);
		iter = mNewWindowPacketList.erase(iter);
		mWindowPacketList.push_back(windowPacket);
		packetsSent++;

		++mNextPacketSequenceSent;
			
	}

	if(resendPackets)
		say = true;

	if(say)
	{
		gLogger->logMsgF("_buildPackets() session: %u", MSG_HIGH,this->mId);
	    gLogger->logMsgF("_buildPackets() %u reliable messages waiting %u unreliable Messages waiting Session : %u", MSG_HIGH,outSize,mUnreliableMessageQueue.size(),this->getId() );
	    gLogger->logMsgF("_buildPackets() Rollover List size : %u WindowPacketList size %u current Window %u", MSG_HIGH,mRolloverWindowPacketList.size(), mWindowPacketList.size(),mWindowSizeCurrent);
	  //gLogger->logMsgF("_buildPackets() WindowResendsize : %u", MSG_HIGH,mWindowResendSize );
	    gLogger->logMsgF("_buildPackets() last Packet received :  %I64u ms ago", MSG_HIGH,(packetBuildTimeStart-mLastPacketReceived));
	    gLogger->logMsgF("_buildPackets() last Packet send :  %I64u ms ago", MSG_HIGH,(packetBuildTimeStart-mLastPacketSent));
	
		gLogger->logMsgF("_buildPackets() Current Window resend : %u Packets", MSG_HIGH,resendPackets);
		gLogger->logMsgF("_buildPackets() send in total : %u Packets", MSG_HIGH,packetsSent);
		uint64 current = Anh_Utils::Clock::getSingleton()->getLocalTime();	
		gLogger->logMsgF("_buildPackets() last ack received : %I64u ms", MSG_HIGH,current - mLastRemotePacketAckReceived);
		gLogger->logMsgF("_buildPackets() time used : %I64u ms", MSG_HIGH,current - wholeTime);
		

		
	}

	mSessionMutex.release();
  
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
  if (mStatus == SSTAT_Connected)
  {
    // If we haven't received a packet in 30s, disconnect us.
	  uint64 t = Anh_Utils::Clock::getSingleton()->getLocalTime() - mLastPacketReceived;
	  t = t/1000;
    if ((Anh_Utils::Clock::getSingleton()->getLocalTime() - mLastPacketReceived) > 30000)
    {

		gLogger->logMsgF("Session disconnect last received packet > 30 (%u) seconds session Id :%u", MSG_HIGH, t, this->getId());   
 
      mCommand = SCOM_Disconnect;
    }

    // If we haven't sent a packet in over 10 seconds, send a ping
    if (Anh_Utils::Clock::getSingleton()->getLocalTime() - mLastPacketSent > 10000)
    {
      gLogger->logMsgF("Sending ping packet due to timeout. Session:0x%x%.4x", MSG_LOW, mService->getId(), getId());
      _sendPingPacket();
    }
  }
 
}


//======================================================================================================================
void Session::SendChannelA(Message* message)
{
	//check whether we are disconnecting  this happens when a client or server crashes without sending a disconnect
	//however in these cases we get a lot of stuck messages on the heap which are orphaned
	if(mStatus != SSTAT_Connected)
	{
		//gLogger->logMsgF("Session::SendChannelA :: we are not connected -destroy message", MSG_HIGH);
		message->setPendingDelete(true);		
		return;
	}

  // Do some boundschecking.
  assert(message->getPriority() < 0x10);
  
  //gLogger->logMsgF("Sending message - Session:0x%x%.4x", MSG_LOW, mService->getId(), getId());

  mSessionMutex.acquire();

  //the connectionserver puts a lot of fastpaths here  - so just put them were they belong
  //this alone takes roughly 5% cpu off of the connectionserver

  if(message->getFastpath()&& (message->getSize() < mMaxUnreliableSize))
	  mUnreliableMessageQueue.push(message);
  else
	mOutgoingMessageQueue.push(message);

  mSessionMutex.release();
}

void Session::SendChannelAUnreliable(Message* message)
{
	//check whether we are disconnecting
	if(mStatus != SSTAT_Connected)
	{
		//gLogger->logMsgF("Session::SendChannelA :: we are not connected -destroy packet", MSG_HIGH);
		message->setPendingDelete(true);		
		return;
	}

  // Do some boundschecking.
  assert(message->getPriority() < 0x10);
  
  //gLogger->logMsgF("Sending message - Session:0x%x%.4x", MSG_LOW, mService->getId(), getId());

  mSessionMutex.acquire();
  if(message->getSize() > mMaxUnreliableSize)	//I send the attribute messages as unreliables	 but they can be to big!!
	  mOutgoingMessageQueue.push(message);
  else
	mUnreliableMessageQueue.push(message);

  mSessionMutex.release();
}


void Session::SortSessionPacket(Packet* packet, uint16 type)
{
	packet->setReadIndex(2);
	switch (type)
	  {
		// New connection request.
	
		// our user data channels
		case SESSIONOP_DataChannel2:
		{
		// All data channels get handled here.
		  _processDataChannelB(packet, false);
		  break;
		}
		case SESSIONOP_DataChannel1:
		case SESSIONOP_DataChannel3:
		case SESSIONOP_DataChannel4:
		{
		  // All data channels get handled here.
		  _processDataChannelPacket(packet, false);
		  break;
		}
		
		// This is part of a message fragment.  Just add it to the packet queue as it must be handled further up.
		case SESSIONOP_DataFrag1:
		case SESSIONOP_DataFrag2:
		case SESSIONOP_DataFrag3:
		case SESSIONOP_DataFrag4:
		{
		  // Handle our fragmented packets here.
		  _processFragmentedPacket(packet);
		  break;
		}
		
		
		default:
		{
		  // Unknown SESSIONOP code
		//jack
		  int jack = 0;
			//tmr
			gLogger->logMsgF("Destroying packet because!!! --tmr",MSG_HIGH);
				// htx
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
	case SESSIONOP_DataOrder3:
	case SESSIONOP_DataOrder4:
	{
	  _processDataOrderPacket(packet);
	  return;
	}

	case SESSIONOP_DataOrder2:
	{
	  _processDataOrderChannelB(packet);
	  return;
	}

	// Multiple session packets 
	case SESSIONOP_MultiPacket:
	{
		// itself has no sequence but its contained packets might
		// however they get handled through this function anyway
	  _processMultiPacket(packet);
	  return;
	}
	// Remote side disconnceted
	case SESSIONOP_Disconnect:
	{
	  mStatus = SSTAT_Disconnecting;
	  _processDisconnectPacket(packet);
	  return;
	}
	
	// Data channel acks
	case SESSIONOP_DataAck1:
	case SESSIONOP_DataAck2:
	case SESSIONOP_DataAck3:
	case SESSIONOP_DataAck4:
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
	  _processNetStatRequestPacket(packet);
	  return;
	}
  }

  //now we have a sequenced packet
  //check if we are in sequence
   uint16 sequence = ntohs(packet->getUint16());

   //gLogger->logMsgF("Incoming data - seq: %i expect: %u Session:0x%x%.4x", MSG_LOW, sequence, mInSequenceNext, mService->getId(), getId());

   if (mInSequenceNext != sequence)
   {
	   //test if we have a rollover
	   int32 overflowTest = sequence - 50000;
	   if((overflowTest > mInSequenceNext)||(sequence<mInSequenceNext))
	   {
		   gLogger->logErrorF("Netcode","Session::HandleSessionPacket :: Double Packet!!! Incoming data - seq: %i expect: %u Session:0x%x%.4x", MSG_HIGH, sequence, mInSequenceNext, mService->getId(), getId());
		   //case 
		   //sequence 65500 (25500)
		   //mInSequenceNext 500
		   //packet is send twice - ack and delete
		   // ack our sequence - we might have a lost ack ...
			Packet* ackPacket = mPacketFactory->CreatePacket();
			ackPacket->addUint16(SESSIONOP_DataAck1);
			ackPacket->addUint16(htons(mInSequenceNext - 1));
		    
			// Acks only need encryption
			ackPacket->setIsCompressed(false);
			ackPacket->setIsEncrypted(true);
		  
			// Push the packet on our outgoing queue
			_addOutgoingUnreliablePacket(ackPacket);

			mPacketFactory->DestroyPacket(packet);
			return;

	   }

	   //were missing something
	   gLogger->logMsgF("Session::HandleSessionPacket :: Incoming data - seq: %i expect: %u Session:0x%x%.4x", MSG_HIGH, sequence, mInSequenceNext, mService->getId(), getId());

	   // now send an out of order
		//gLogger->logMsgF("OO %u < %u",MSG_NORMAL,mInSequenceNext,sequence);
			
		switch(packetType )
		{
			case SESSIONOP_DataFrag1:
			case SESSIONOP_DataChannel1:
			  {
					Packet* orderPacket;
					orderPacket = mPacketFactory->CreatePacket();
					orderPacket->addUint16(SESSIONOP_DataOrder1);
					orderPacket->addUint16(htons(sequence));
					orderPacket->setIsCompressed(false);
					orderPacket->setIsEncrypted(true);

					_addOutgoingUnreliablePacket(orderPacket);
			  }
			  break;
			
			case SESSIONOP_DataFrag2:
			case SESSIONOP_DataChannel2:
			  {
					Packet* orderPacket;
					orderPacket = mPacketFactory->CreatePacket();
					orderPacket->addUint16(SESSIONOP_DataOrder2);
					orderPacket->addUint16(htons(sequence));
					orderPacket->addUint16(htons(mInSequenceNext-1));
					orderPacket->setIsCompressed(false);
					orderPacket->setIsEncrypted(true);

					_addOutgoingUnreliablePacket(orderPacket);

			  }
			  break;

			default:
			{
				gLogger->logMsgF("Session::HandleSessionPacket :: *** wanted to send Out-of-Order packet with weird opcode - Sequence: %i, Service %u Session:0x%.4x", MSG_HIGH, sequence, mService->getId(), getId());
				mPacketFactory->DestroyPacket(packet);
				return;
			}
		}
		mPacketFactory->DestroyPacket(packet);
		return;
	   
   }

   

   // What type of SessionPacket is it?
   // let the specialist handle it
   if (mInSequenceNext == sequence)
		SortSessionPacket(packet,packetType);

  
}


//======================================================================================================================

void Session::HandleFastpathPacket(Packet* packet)
{
	// Fast path is raw data.  Just send it up.
	mLastPacketReceived = Anh_Utils::Clock::getSingleton()->getLocalTime();
	_processDataChannelPacket(packet, true);
}

//======================================================================================================================

void Session::DestroyIncomingMessage(Message* message)      
{ 
	mMessageFactory->DestroyMessage(message); 
}

//======================================================================================================================

void Session::DestroyPacket(Packet* packet)
{
	mPacketFactory->DestroyPacket(packet);
}

//======================================================================================================================

Packet* Session::getOutgoingReliablePacket(void)                   
{ 
  Packet* packet = 0;

  if (mOutgoingReliablePacketQueue.size() == 0)
    return packet;

  mServerPacketsSent++;
  
  // Get a new Outgoing packet
  mSessionMutex.acquire();
  
  packet =  mOutgoingReliablePacketQueue.front(); 
  mOutgoingReliablePacketQueue.pop(); 

  mSessionMutex.release();

  mLastPacketSent = Anh_Utils::Clock::getSingleton()->getLocalTime();

  //packet->setReadIndex(2);
 // uint16 sequence = ntohs(packet->getUint16());

  
  return packet;
}


//======================================================================================================================
Packet* Session::getOutgoingUnreliablePacket(void)                   
{ 
  Packet* packet = 0;

  if (mOutgoingUnreliablePacketQueue.size() == 0)
    return packet;
  
  mServerPacketsSent++;
  
  // Get a new Outgoing packet
  mSessionMutex.acquire();

  packet =  mOutgoingUnreliablePacketQueue.front(); 
  mOutgoingUnreliablePacketQueue.pop();

  mSessionMutex.release();

  mLastPacketSent = Anh_Utils::Clock::getSingleton()->getLocalTime();

  // Temp debug logging for client only.
  //packet->setReadIndex(2);
  

  return packet;
}


//======================================================================================================================
Message* Session::getIncomingQueueMessage()
{
  Message* message = 0;

  mSessionMutex.acquire();

  message = mIncomingMessageQueue.front();
  mIncomingMessageQueue.pop();

  mSessionMutex.release();

  return message;
}


//======================================================================================================================
void Session::_processSessionRequestPacket(Packet* packet)
{   
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
		// Otherwise ack this packet then send it up
		packet->setReadIndex(0);
		priority = packet->getUint8();
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
		assert(newMessage->getPriority() < 0x10);
		assert(newMessage->getPriority() < 0x10);
		newMessage->setDestinationId(dest);
		newMessage->setAccountId(accountId);
		newMessage->setFastpath(true);
		if (routed)
		  newMessage->setRouted(true);

		_addIncomingMessage(newMessage, priority);
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
		if (routed)
		  newMessage->setRouted(true);
		else
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
  if(destroyPacket)
	mPacketFactory->DestroyPacket(packet);
}

//======================================================================================================================
//Data Channel B is used for interserver communication (ie of routed Packets)
//this way we are able to pass around zipping
//======================================================================================================================

void Session::_processDataChannelB(Packet* packet, bool fastPath)
{
  uint8		priority		= 0;
  uint8		routed			= 0;
  uint8		dest			= 0;
  uint32	accountId		= 0;
  bool		destroyPacket	= true;
  

  // If we're fastpath, just send it up.  
  if(fastPath)
  {
    // Otherwise ack this packet then send it up
    packet->setReadIndex(0);
    priority = packet->getUint8();
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
	assert(newMessage->getPriority() < 0x10);
    newMessage->setDestinationId(dest);
    newMessage->setAccountId(accountId);
    newMessage->setFastpath(true);
    if (routed)
      newMessage->setRouted(true);

    _addIncomingMessage(newMessage, priority);
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

    if (mInSequenceNext == sequence)
    {

      // check to see if this is a multi-message message
      //uint16 len = packet->getSize() - 4;  // -2 header, -2 sequence
      priority = packet->getUint8();
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
			  //right now max size 255 (1 byte!!!)
			  size = ntohs(packet->getUint16());
          }
		  
          
          // We need to get these again.
          priority = packet->getUint8();
          routed =  packet->getUint8();  // Routing byte in a B 019 used!!! as we have interserver communication here
		 
		  dest = packet->getUint8();
          accountId = packet->getUint32();
		  

          // Init a new message for this data.
          mMessageFactory->StartMessage();
          mMessageFactory->addData(packet->getData() + packet->getReadIndex(), static_cast<uint16>(size) - 7); // -1 priority, -1 routing
          Message* newMessage = mMessageFactory->EndMessage();

          // set our account and server id's
          newMessage->setAccountId(accountId);
          newMessage->setDestinationId(dest);
          newMessage->setPriority(priority);
		  assert(newMessage->getPriority() < 0x10);

          // Need to specify whether this is routed or not here, so we know in the app
          newMessage->setRouted(true);

          // Push the message on our incoming queue
          _addIncomingMessage(newMessage, priority);

          // Advance the message index
          packet->setReadIndex(packet->getReadIndex() + static_cast<uint16>(size) - 7); // -1 priority, -1 routing

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
		assert(newMessage->getPriority() < 0x10);

        // Push the message on our incoming queue
        _addIncomingMessage(newMessage, priority);

      }

      // Set our inSequence number so we know we recieved this packet.
	  // in sequence is per packet not per message 
	  //gLogger->logMsgF("_processDataChannelPacket B::increasing mInSequence by 1 %u", MSG_HIGH, mInSequenceNext);  
	  mInSequenceNext++;
	  //gLogger->logMsgF("_processDataChannelPacket B::send ack sequence %u", MSG_HIGH, mInSequenceNext-1);  
	  mSendDelayedAck = true;
	  
    }
    
  }

  // Destroy our incoming packet, it's not needed any longer.
  if(destroyPacket)
	mPacketFactory->DestroyPacket(packet);
}


//======================================================================================================================
void Session::_processDataChannelAck(Packet* packet)
{
	uint32 oldsize = mWindowSizeCurrent;

	Packet* windowPacket = 0;
	uint16 windowPacketSequence = 0;
	PacketWindowList::iterator iter;
	PacketWindowList::iterator itN;

	// Get the sequence off our incoming packet
	packet->setReadIndex(2);  //skip the header
	uint16 sequence = ntohs(packet->getUint16());

	//gLogger->logMsgF("Received ACK  - Sequence: %u, Session:0x%x%.4x", MSG_HIGH, sequence, mService->getId(), getId());

	mSessionMutex.acquire();
	uint32 pDel = 0;

	// If our windowQueue is empty, this is a dupe ack for the last packet that was on it.  Just return.
	if(mOutSequenceRollover)
	{
		if((!mRolloverWindowPacketList.size()) && (!mNewRolloverWindowPacketList.size()))
		{
			mOutSequenceRollover = false;
		}
		// this is an ack for the new queue	 so the old queue can go completely
		else if((sequence < (0xFFFF - (mRolloverWindowPacketList.size()+mNewRolloverWindowPacketList.size()))))
		{
		
			gLogger->logMsgF("_processDataChannelAck::Rollover complete Windowsize %u ack seq new queue %u",MSG_HIGH,mWindowSizeCurrent,sequence);
			mLastRemotePacketAckReceived = Anh_Utils::Clock::getSingleton()->getLocalTime();

			mOutSequenceRollover = false;
			iter = mRolloverWindowPacketList.begin();
			
			//now that communication is reestablished we can resize our window again
			if(mWindowSizeCurrent < mWindowResendSize)
			{
				mWindowSizeCurrent += uint32(mWindowResendSize/10);
				if(mWindowSizeCurrent >mWindowResendSize)
					mWindowSizeCurrent = mWindowResendSize;
			}
			//destroy them now they are acknowleged
			//they dont have to be resend
			while(iter != mRolloverWindowPacketList.end())
			{
				pDel ++;
				mPacketFactory->DestroyPacket(*iter);
				mRolloverWindowPacketList.erase(iter);
				iter = mRolloverWindowPacketList.begin();
			

			}
		}
		else//else if(sequence < 0xFFFF - mRolloverWindowPacketList.size()) thats an ack purely for the old list
		{
			//only parts of the old queue can go
			iter = mRolloverWindowPacketList.begin();
			windowPacket = *iter;
			windowPacket->setReadIndex(2); 
			windowPacketSequence = ntohs(windowPacket->getUint16());

			if(sequence < windowPacketSequence)
			{
				//gLogger->logMsg("dupe rollover queue ack");
			}
			else
			{
				
				// This is a proper ack, so handle it.
				if(mWindowSizeCurrent < mWindowResendSize)
				{
					// I dont go with a set window of packets in our queues here as I think
					// that the servers (especially the zones) need to keep on sending
					// especially when loads of players log on to one zone in this situation we get easily a few thousand to tenthsnd messages
					// in a short time
					mWindowSizeCurrent += uint32(mWindowResendSize/10);
					if(mWindowSizeCurrent >mWindowResendSize)
						mWindowSizeCurrent = mWindowResendSize;
				}
				while(sequence >= windowPacketSequence)
				{	
					pDel ++;
					mPacketFactory->DestroyPacket(*iter);
					mRolloverWindowPacketList.erase(iter);

					// If the list is empty, break out
					if(mRolloverWindowPacketList.size() == 0)
					{
						gLogger->logMsgF("_processDataChannelAck::Rollover complete Windowsize %u ack seq %u",MSG_HIGH,mWindowSizeCurrent,sequence);
						windowPacket = 0;
						windowPacketSequence = 0;
						
						if(mNewRolloverWindowPacketList.size() == 0)
							mOutSequenceRollover = false;

						break;
					}

					iter = mRolloverWindowPacketList.begin();
					windowPacket = *iter;
					windowPacket->setReadIndex(2); 
					windowPacketSequence = ntohs(windowPacket->getUint16());
				}//while(sequence >= windowPacketSequence)

				mLastRemotePacketAckReceived = Anh_Utils::Clock::getSingleton()->getLocalTime();

				// Our current window increases here, as we know come communication is back.
			
			}//if(sequence < windowPacketSequence)

			mSessionMutex.release();

			mPacketFactory->DestroyPacket(packet);
			gLogger->logMsgF("_processDataChannelAck::Rollover Ack Windowsize %u ack seq new queue %u",MSG_HIGH,mWindowSizeCurrent,sequence);

			return;
		}//else if(sequence < 0xFFFF - mRolloverWindowPacketList.size())
	}	   //if the rollover was gone completely we continue with the regular list

	if (mWindowPacketList.size() == 0)
	{
		//gLogger->logMsgF("Dupe ACK received - Nothing in resend window - seq: %u, Session:0x%x%.4x", MSG_LOW, sequence, mService->getId(), getId());
		mPacketFactory->DestroyPacket(packet);

		mSessionMutex.release();
		return;
	}

	iter = mWindowPacketList.begin();
	windowPacket = *iter;
	windowPacket->setReadIndex(2); //skip the header
	windowPacketSequence = ntohs(windowPacket->getUint16());

	if (sequence < windowPacketSequence)
	{
		// Dpulicate ack, drop it.
		//gLogger->logMsgF("Dupe ACK received - No such packet in window - ackSeq: %u, expect: %u, Session:0x%x%.4x", MSG_HIGH, sequence, windowPacketSequence, mService->getId(), getId());
	}
	else if (sequence > windowPacketSequence + mWindowPacketList.size())
	{
		// This ack is way out of bounds, log a message and drop it. 
		gLogger->logMsgF("_processDataChannelAck::*** Ack out of bounds - ackSeq: %u, expect: %u, Session:0x%x%.4x", MSG_HIGH, sequence, windowPacketSequence, mService->getId(), getId());
	}
	else
	{
		// This is a proper ack, so handle it.
		if(mWindowSizeCurrent < mWindowResendSize)
		{
			mWindowSizeCurrent += uint32(mWindowResendSize/10);
			if(mWindowSizeCurrent >mWindowResendSize)
				mWindowSizeCurrent = mWindowResendSize;
		}

		while (sequence >= windowPacketSequence)
		{
			pDel ++;
			mPacketFactory->DestroyPacket(*iter);
			mWindowPacketList.erase(iter);
			

			// If the window is empty, break out
			if (mWindowPacketList.size() == 0)
				break;

			iter = mWindowPacketList.begin();
			windowPacket = *iter;
			windowPacket->setReadIndex(2); //skip the header
			windowPacketSequence = ntohs(windowPacket->getUint16());
		}

		mLastRemotePacketAckReceived = Anh_Utils::Clock::getSingleton()->getLocalTime();
		//gLogger->logMsgF("Received ACK  - Sequence: %u, Session:0x%x%.4x", MSG_HIGH, sequence, mService->getId(), getId());

	}

	mSessionMutex.release();

	// Destroy our incoming packet, it's not needed any longer.
	mPacketFactory->DestroyPacket(packet);

}


//======================================================================================================================
void Session::_processDataOrderPacket(Packet* packet)
{
  packet->setReadIndex(2);
  uint16 sequence = ntohs(packet->getUint16());

  PacketWindowList::iterator iter = mWindowPacketList.begin();
  PacketWindowList::iterator iterRoll = mRolloverWindowPacketList.begin();
  Packet* windowPacket = *iter;
  windowPacket->setReadIndex(2);
  uint16 windowSequence = ntohs(windowPacket->getUint16());

  //check if its just an increase in the sequence as new packets arrive or quite a new request
  /*
  if((lowestCount +1)== sequence)
  {
	  lowestCount++;
	  if(Anh_Utils::Clock::getSingleton()->getLocalTime() - windowPacket->getTimeOOHSent() < 100)
	  	  return;
  }
  else
  {
	  lowest = sequence;
	  lowestCount = sequence;
  }
	*/
  gLogger->logErrorF("Netcode","_processDataOrderPacket::Out-Of-order packet session 0x%x%.4x seq: %u, windowsequ : %u", MSG_HIGH, mService->getId(), mId, sequence, windowSequence);

  //Do some bounds checking
  if (sequence < windowSequence)
  {
	  gLogger->logErrorF("Netcode","_processDataOrderPacket::*** Order packet sequence too small, may be a duplicate or we handled our acks wrong.  seq: %u, expect >: %u", MSG_HIGH, sequence, windowSequence);
  }

  if (sequence > windowSequence + mWindowPacketList.size())
  {
	  gLogger->logErrorF("Netcode","_processDataOrderPacket::*** Rollover Order packet  seq: %u, expect >: %u", MSG_HIGH, sequence, windowSequence);
  }
	
	//The location of the packetsequence out of order has NOBEARING on the question on which list we will find the last properly received Packet!!!


	if(mRolloverWindowPacketList.size()&& (sequence > (65535-mRolloverWindowPacketList.size())))
	{
		//jupp its on the rolloverlist
		mSessionMutex.acquire(); //			   mRolloverWindowPacketList and WindowPacketList get accessed by the socketwritethread and by the socketreadthread both through the session

		uint16 count = 0;
		for (iterRoll = mRolloverWindowPacketList.begin(); iterRoll != mWindowPacketList.end(); iterRoll++)
		{
			// Grab our window packet
			windowPacket = (*iterRoll);
			windowPacket->setReadIndex(2);
			uint16 windowRollSequence = ntohs(windowPacket->getUint16());

			// If it's smaller than the order packet send it, otherwise break;
			if (windowRollSequence < sequence )
			{
				//count++;
				//if(count > 50)
				//	break;
					
				if(Anh_Utils::Clock::getSingleton()->getLocalTime() - windowPacket->getTimeOOHSent() < 200)
					break;
				
				_addOutgoingReliablePacket(windowPacket);
				
				windowPacket->setTimeOOHSent(Anh_Utils::Clock::getSingleton()->getLocalTime());

				if (mWindowSizeCurrent > (mWindowResendSize/10))
					mWindowSizeCurrent--;

			}
		}
		mSessionMutex.release();

	 }
 
 
	 mSessionMutex.acquire();//			   mRolloverWindowPacketList and WindowPacketList get accessed by the socketwritethread and by the socketreadthread both through the session
	 uint16 count = 0;
	 for (iter = mWindowPacketList.begin(); iter != mWindowPacketList.end(); iter++)
	 {
		// Grab our window packet
		windowPacket = (*iter);
		windowPacket->setReadIndex(2);
		uint16 windowSequence = ntohs(windowPacket->getUint16());

		// If it's smaller than the order packet send it, otherwise break;
		// do we want to throttle the amount of packets being send to 10 or 50 or 100 ???
		// if we receive a sequence on the rolloverlist (65530 for example) we will
		// always send ALL packets on the regular list - I dont anticipate a big deal here though!!!
		if (windowSequence < sequence )
		{
			        
			//gLogger->logMsgF("Resending reliable packet.  seq: %u, order: %u", MSG_HIGH, windowSequence, sequence);
				
			_addOutgoingReliablePacket(windowPacket);
					
			windowPacket->setTimeOOHSent(Anh_Utils::Clock::getSingleton()->getLocalTime());

			if (mWindowSizeCurrent > (mWindowResendSize/10))
				mWindowSizeCurrent--;
		
		}
		else
		{
			mSessionMutex.release();
			return;
		}
	}
	mSessionMutex.release();
  

  // Destroy our incoming packet, it's not needed any longer.
  mPacketFactory->DestroyPacket(packet);
}


//======================================================================================================================
void Session::_processDataOrderChannelB(Packet* packet)
{
  packet->setReadIndex(2);
  uint16 sequence = ntohs(packet->getUint16());
  uint16 bottomSequence = ntohs(packet->getUint16());

  PacketWindowList::iterator iter = mWindowPacketList.begin();
  PacketWindowList::iterator iterRoll = mRolloverWindowPacketList.begin();
  Packet* windowPacket = *iter;
  windowPacket->setReadIndex(2);
  uint16 windowSequence = ntohs(windowPacket->getUint16());
  

  if(bottomSequence > sequence)
  {
	  if(bottomSequence > (65535-mRolloverWindowPacketList.size()))
	  {
		  //rollover - all is well
	  }
	  else
	  {
		  //shit
		  gLogger->logErrorF("Netcode","_processDataOrderChannelB::Out-Of-order packet session 0x%x%.4x seq: %u, windowsequ : %u, bottom %u", MSG_HIGH, mService->getId(), mId, sequence, windowSequence, bottomSequence);
		  assert(false);
	  }
	  
	  //either we have screwed up majestically *or* we have a rollover at our hands

  }

  //check if its just an increase in the sequence as new packets arrive or quite a new request
  /*
  if((lowestCount +1)== sequence)
  {
	  lowestCount++;
	  if(Anh_Utils::Clock::getSingleton()->getLocalTime() - windowPacket->getTimeOOHSent() < 100)
	  	  return;
  }
  else
  {
	  lowest = sequence;
	  lowestCount = sequence;
  }
	*/
  gLogger->logErrorF("Netcode","_processDataOrderChannelB::Out-Of-order packet session 0x%x%.4x seq: %u, windowsequ : %u, bottom %u", MSG_HIGH, mService->getId(), mId, sequence, windowSequence, bottomSequence);

  //Do some bounds checking
  if (sequence < windowSequence)
  {
	  gLogger->logErrorF("Netcode","_processDataOrderChannelB::*** Order packet sequence too small, may be a duplicate or we handled our acks wrong.  seq: %u, expect >: %u", MSG_HIGH, sequence, windowSequence);
  }

  if (sequence > windowSequence + mWindowPacketList.size())
  {
	  gLogger->logErrorF("Netcode","_processDataOrderChannelB::*** Rollover Order packet  seq: %u, expect >: %u", MSG_HIGH, sequence, windowSequence);
  }
	
	//The location of the packetsequence out of order has NOBEARING on the question on which list we will find the last properly received Packet!!!


	if(mRolloverWindowPacketList.size()&& (sequence > (65535-mRolloverWindowPacketList.size())))
	{
		//jupp its on the rolloverlist
		mSessionMutex.acquire(); //			   mRolloverWindowPacketList and WindowPacketList get accessed by the socketwritethread and by the socketreadthread both through the session

		uint16 count = 0;
		for (iterRoll = mRolloverWindowPacketList.begin(); iterRoll != mWindowPacketList.end(); iterRoll++)
		{
			// Grab our window packet
			windowPacket = (*iterRoll);
			windowPacket->setReadIndex(2);
			uint16 windowRollSequence = ntohs(windowPacket->getUint16());

			// If it's smaller than the order packet send it, otherwise break;
			if ((windowRollSequence < sequence) && (windowRollSequence >= bottomSequence))
			{
				//count++;
				//if(count > 50)
				//	break;
					
				if(Anh_Utils::Clock::getSingleton()->getLocalTime() - windowPacket->getTimeOOHSent() < 200)
					break;
				
				_addOutgoingReliablePacket(windowPacket);
				
				windowPacket->setTimeOOHSent(Anh_Utils::Clock::getSingleton()->getLocalTime());

				if (mWindowSizeCurrent > (mWindowResendSize/10))
					mWindowSizeCurrent--;

			}
		}
		mSessionMutex.release();

	 }
 
 
	 mSessionMutex.acquire();//			   mRolloverWindowPacketList and WindowPacketList get accessed by the socketwritethread and by the socketreadthread both through the session
	 uint16 count = 0;
	 for (iter = mWindowPacketList.begin(); iter != mWindowPacketList.end(); iter++)
	 {
		// Grab our window packet
		windowPacket = (*iter);
		windowPacket->setReadIndex(2);
		uint16 windowSequence = ntohs(windowPacket->getUint16());

		// If it's smaller than the order packet send it, otherwise break;
		// do we want to throttle the amount of packets being send to 10 or 50 or 100 ???
		// if we receive a sequence on the rolloverlist (65530 for example) we will
		// always send ALL packets on the regular list - I dont anticipate a big deal here though!!!
		if ((windowSequence < sequence) && (windowSequence >= bottomSequence))
		{
			        
			//gLogger->logMsgF("Resending reliable packet.  seq: %u, order: %u", MSG_HIGH, windowSequence, sequence);
				
			_addOutgoingReliablePacket(windowPacket);
					
			windowPacket->setTimeOOHSent(Anh_Utils::Clock::getSingleton()->getLocalTime());

			if (mWindowSizeCurrent > (mWindowResendSize/10))
				mWindowSizeCurrent--;
		
		}
		else
		{
			mSessionMutex.release();
			return;
		}
	}
	mSessionMutex.release();
  

  // Destroy our incoming packet, it's not needed any longer.
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
		mFragmentedPacketCurrentSize = packet->getSize() - 8;  // -2 header, -2 sequence, -4 size
		mFragmentedPacketCurrentSequence = mFragmentedPacketStartSequence = sequence;

		//if (mService->getId() == 1)
		{
			//gLogger->logMsgF("Start incoming fragged packets - total: %u seq:%u", MSG_LOW, mFragmentedPacketTotalSize, sequence);
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
		  {
			newMessage->setRouted(true);
		  }
		  newMessage->setPriority(priority);
		  assert(newMessage->getPriority() < 0x10);
		  newMessage->setDestinationId(dest);
		  newMessage->setAccountId(accountId);

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
  // Create a new ping packet and send it on.
  Packet* packet = mPacketFactory->CreatePacket();
  packet->addUint8(0);
  packet->addUint8(0x06);     // Ping packet type;
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

  mSessionMutex.acquire();

  mIncomingMessageQueue.push(message);

  mSessionMutex.release();
  
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

  // fragments envelope sizes
  
  envelopeSize = 18; // -2 header -2 seq -4 size -1 priority -1 routed flag -1 route destination -4 account id -3 comp/CRC 

  // If we're too large to fit in a single packet, split us up.
  if(messageSize + envelopeSize >= mMaxPacketSize) 
  {
    // Build our first packet with the total size.
    newPacket = mPacketFactory->CreatePacket(); 

	newPacket->addUint16(SESSIONOP_DataFrag2);
	
    newPacket->addUint16(htons(mOutSequenceNext));

	 newPacket->addUint32(htonl(messageSize + 7));

	newPacket->addUint8(message->getPriority());

    newPacket->addUint8(1);                                       // There is a routing header next
    newPacket->addUint8(message->getDestinationId());
    newPacket->addUint32(message->getAccountId());
    newPacket->addData(message->getData(), mMaxPacketSize- envelopeSize); // -2 header, -2 sequence, -4 size, -2 priority/routing, -5 routing, -2 crc
    messageIndex += mMaxPacketSize - envelopeSize;                         // -2 header, -2 sequence, -4 size, -2 priority/routing, -5 routing, -2 crc
    

    // Data channels need compression and encryption
	// no compression in server server communication!
    newPacket->setIsCompressed(false);
	newPacket->setIsEncrypted(true);
    
    // Push the packet on our outgoing queue
    mSessionMutex.acquire();

	assert((newPacket->getSize()+3)<= mMaxPacketSize);

    mNewWindowPacketList.push_back(newPacket);

	if(!++mOutSequenceNext)
		_handleOutSequenceRollover();

    mSessionMutex.release();

    // Now build any remaining packets.
    while (messageSize > messageIndex)
    {
      newPacket = mPacketFactory->CreatePacket();

      // Build our remaining packets
	  newPacket->addUint16(SESSIONOP_DataFrag2);
	
      newPacket->addUint16(htons(mOutSequenceNext));
      newPacket->addData(message->getData() + messageIndex, std::min<uint16>(mMaxPacketSize - 7, messageSize - messageIndex));

	  //no new routing header necessary here
      messageIndex += mMaxPacketSize - 7;  // -2 header, -2 sequence, -3 comp/crc

      // Data channels need compression and encryption
	  // no compression in server server communication!
     newPacket->setIsCompressed(false);
	
     newPacket->setIsEncrypted(true);
      
      // Push the packet on our outgoing queue
      mSessionMutex.acquire();

	  assert((newPacket->getSize()+3)<= mMaxPacketSize);
      mNewWindowPacketList.push_back(newPacket);

	  if(!++mOutSequenceNext)
		  _handleOutSequenceRollover();

      mSessionMutex.release();
    }
  }
  else
  {
   
    // Create a new packet and push the data into it.
    newPacket = mPacketFactory->CreatePacket();
	
	newPacket->addUint16(SESSIONOP_DataChannel2);
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
    mSessionMutex.acquire();

	assert((newPacket->getSize()+3)<= mMaxPacketSize);
    mNewWindowPacketList.push_back(newPacket);

	if(!++mOutSequenceNext)
		_handleOutSequenceRollover();

    mSessionMutex.release();
  }
}



//======================================================================================================================
void Session::_buildOutgoingReliablePackets(Message* message)
{
  //NOT routed
  Packet*	newPacket = 0;
  uint16	messageIndex = 0;
  uint16	envelopeSize = 0;
  uint16	messageSize = message->getSize();

  // fragments envelope sizes
  if (message->getRouted())
  {
	  gLogger->logMsgF("_buildOutgoingReliablePackets::routed packet faultily sorted", MSG_HIGH);
	  _buildOutgoingReliableRoutedPackets(message);
  }

  envelopeSize = 13; // -2 header -2 seq -4 size -1 priority -1 routed flag -3 comp/CRC 
  
  // If we're too large to fit in a single packet, split us up.
  if(messageSize + envelopeSize >= mMaxPacketSize) 
  {
    // Build our first packet with the total size.
    newPacket = mPacketFactory->CreatePacket(); 

	newPacket->addUint16(SESSIONOP_DataFrag1);	

    newPacket->addUint16(htons(mOutSequenceNext));

    newPacket->addUint32(htonl(messageSize + 2));

	newPacket->addUint8(message->getPriority());

    // Do we need to add the routing header
    
    newPacket->addUint8(0);                                       // This byte is always 0 on the client
    newPacket->addData(message->getData(), mMaxPacketSize - envelopeSize); // -2 header, -2 sequence, -4 size, -2 priority/routing, -2 crc
    messageIndex += mMaxPacketSize - envelopeSize;                         // -2 header, -2 sequence, -4 size, -2 priority/routing, -2 crc
    
    // Data channels need compression and encryption
	// no compression in server server communication!
	newPacket->setIsCompressed(true);

    newPacket->setIsEncrypted(true);
    
    // Push the packet on our outgoing queue
    mSessionMutex.acquire();

    mNewWindowPacketList.push_back(newPacket);

	if(!++mOutSequenceNext)
		_handleOutSequenceRollover();

    mSessionMutex.release();

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
      mSessionMutex.acquire();

      mNewWindowPacketList.push_back(newPacket);

	  if(!++mOutSequenceNext)
		  _handleOutSequenceRollover();

      mSessionMutex.release();
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
	newPacket->addData(message->getData(), message->getSize());  // -2 header, -2 sequence, -2 priority/routing, -5 routing, -2 crc
	
	// Data channels need compression and encryption 
	// no compression in server server communication!
	newPacket->setIsCompressed(true);

    newPacket->setIsEncrypted(true);
    
    // Push the packet on our outgoing queue
    mSessionMutex.acquire();

    mNewWindowPacketList.push_back(newPacket);

	if(!++mOutSequenceNext)
		_handleOutSequenceRollover();

    mSessionMutex.release();
  }
  mMessageFactory->DestroyMessage(message);
}


//======================================================================================================================
void Session::_buildOutgoingUnreliablePackets(Message* message)
{
  Packet* newPacket = 0;
  uint16 messageIndex = 0;


  //if (mService->getId() == 1)
  {
    //gLogger->logMsgF("Building unreliable packet - Sequence: %u, Session:0x%x%.4x", MSG_LOW, mOutSequenceNext, mService->getId(), getId());
  }

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
  
}


//======================================================================================================================
void Session::_addOutgoingReliablePacket(Packet* packet)
{
  // Push the packet on our outgoing queue
  mSessionMutex.acquire();

  mOutgoingReliablePacketQueue.push(packet);

  // Set our last packet sent time index
  packet->setTimeQueued(Anh_Utils::Clock::getSingleton()->getLocalTime());

  mSessionMutex.release();

  //gLogger->logMsgF("ReliableQueueAdd: Type:0x%.4x, Session:0x%x%.4x", MSG_LOW, packet->getPacketType(), mService->getId(), getId());
}


//======================================================================================================================
void Session::_addOutgoingUnreliablePacket(Packet* packet)
{
  // Push the packet on our outgoing queue
  mSessionMutex.acquire();

  // Set our last packet sent time index
  packet->setTimeQueued(Anh_Utils::Clock::getSingleton()->getLocalTime());
  mOutgoingUnreliablePacketQueue.push(packet);

  mSessionMutex.release();

  //gLogger->logMsgF("UnreliableQueueAdd: Type:0x%.4x, Session:0x%x%.4x", MSG_LOW, packet->getPacketType(), mService->getId(), getId());
}


//======================================================================================================================
int8* Session::getAddressString(void)
{
  in_addr address;
  address.S_un.S_addr = mAddress;

  return inet_ntoa(address); 
}


//======================================================================================================================
uint16 Session::getPortHost(void)
{
  return ntohs(mPort); 
}

//======================================================================================================================

uint32 Session::_buildPackets()
{

	// 2 things
	// 1st try to make as few packets as possible !!!
	// thats a must for server server communication - the soe protocol is extremely expensive in cpu cycles

	// 2nd are there any ways a session can have to generate routed and not routed packets ?????

	uint32 packetsbuild = 0;
	mSessionMutex.acquire();

	//gLogger->logMsgF("session build packets queue size : %u ",MSG_NORMAL,mOutgoingMessageQueue.size());

	Message* message = mOutgoingMessageQueue.front();
	mOutgoingMessageQueue.pop();

	if((mOutgoingMessageQueue.size())&&(message->getRouted() ^ mOutgoingMessageQueue.front()->getRouted())	) 
		gLogger->logMsgF("this session contains routed AND notrouted packets",MSG_HIGH);		

	if(!mOutgoingMessageQueue.size()

	//question are there any possibilities that a networklayer encounters mixed routed and unrouted packets ??? I do not think so
	|| (message->getRouted() ^ mOutgoingMessageQueue.front()->getRouted())	 
//	|| message->getSize() > 247 || mOutgoingMessageQueue.front()->getSize() > 247
	|| message->getFastpath()
	|| message->getSize() + mOutgoingMessageQueue.front()->getSize() > mMaxPacketSize - 21)
	
	{
		if (message->getFastpath()&&(message->getSize()<=mMaxUnreliableSize))
		{
			packetsbuild++;
			_buildOutgoingUnreliablePackets(message);
		}
		else
		{
			packetsbuild++;
			
			if(message->getRouted())
				_buildOutgoingReliableRoutedPackets(message);
			else
				_buildOutgoingReliablePackets(message);
		}

		mMessageFactory->DestroyMessage(message);
	}
	else 
	{
		if(message->getRouted() && mOutgoingMessageQueue.front()->getRouted())
		{
			mRoutedMultiMessageQueue.push(message);

			//cave we *might* have 2bytes for Size !!!!!!  (if size 255 or bigger)
			uint16 baseSize = 18 + message->getSize(); // 2 header, 2 sequence, 2 0019, 1(2) size,7 prio/routing, 3 comp/crc
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

			uint16 baseSize = 12 + message->getSize(); // 2 header, 2 sequence, 2 0019, 1 size,2 prio/routing, 3 comp/crc
			packetsbuild++;
			while(baseSize < mMaxPacketSize && mOutgoingMessageQueue.size())
			{
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
			_buildOutgoingReliablePackets(message);
		}
	}

	mSessionMutex.release();
	return(packetsbuild);
}

//======================================================================================================================

uint32 Session::_buildPacketsUnreliable()
{

	uint32 packetsbuild = 0;
	mSessionMutex.acquire();

	//gLogger->logMsgF("session build packets queue size : %u ",MSG_NORMAL,mOutgoingMessageQueue.size());

	Message* message = mUnreliableMessageQueue.front();
	mUnreliableMessageQueue.pop();

	// no larger ones than ff yet, we want at least 2 messages to fit in, dont use routed mesages, so the frontline server does packing only
	if(!mUnreliableMessageQueue.size()
	|| message->getRouted() || mUnreliableMessageQueue.front()->getRouted() 
	|| message->getSize() > 252 || mUnreliableMessageQueue.front()->getSize() > 252 //sizebyte so 255 is max including header
	|| message->getSize() + mUnreliableMessageQueue.front()->getSize() > mMaxUnreliableSize - 16)
	{
		packetsbuild++;
		_buildOutgoingUnreliablePackets(message);
	
		mMessageFactory->DestroyMessage(message);
	}
	else 
	{
		mMultiUnreliableQueue.push(message);

		uint16 baseSize = 12 + message->getSize(); // 2 header, 2 sequence, 2 0019, 1 size,2 prio/routing, 3 comp/crc
		packetsbuild++;
		while(baseSize < mMaxUnreliableSize && mUnreliableMessageQueue.size())
		{
			message = mUnreliableMessageQueue.front();
						
			baseSize += message->getSize() + 3; // size + prio + routing

			if(baseSize >= mMaxPacketSize || message->getRouted() || message->getSize() > 252)
				break;

			mUnreliableMessageQueue.pop();
			mMultiUnreliableQueue.push(message);
		}

		_buildUnreliableMultiDataPacket();
	}

	mSessionMutex.release();
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

		mMessageFactory->DestroyMessage(message);
	}

	newPacket->setIsCompressed(true);
	newPacket->setIsEncrypted(true);

	mNewWindowPacketList.push_back(newPacket);
	assert(newPacket->getSize() <= (mMaxPacketSize-3));
	//sequence of packets uint16 +1 for every packet rollover from 0xffff to 0
	if(!++mOutSequenceNext)
		_handleOutSequenceRollover();
}

//-----------------------------------------------------------------------------
// Routed multidatas for server server communication only !!!!
//-----------------------------------------------------------------------------

void Session::_buildRoutedMultiDataPacket()
{
	Packet*		newPacket = mPacketFactory->CreatePacket();
	Message*	message = 0;

	newPacket->addUint16(SESSIONOP_DataChannel2); //server server communication !!!!!
	newPacket->addUint16(htons(mOutSequenceNext));
	newPacket->addUint16(0x1900);

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
		newPacket->addUint8(1);
		
		newPacket->addUint8(message->getDestinationId());
		newPacket->addUint32(message->getAccountId());
			//dest = fragment->getUint8();
            //accountId = fragment->getUint32();
		

		newPacket->addData(message->getData(), message->getSize()); 

		mMessageFactory->DestroyMessage(message);
	}

	newPacket->setIsCompressed(false); //server server !!! save the cycles!!!
	newPacket->setIsEncrypted(true);

	mNewWindowPacketList.push_back(newPacket);

	//sequence of packets uint16 +1 for every packet rollover from 0xffff to 0
	if(!++mOutSequenceNext)
		_handleOutSequenceRollover();
}

//need to get in multis /00 03s
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

		assert((message->getSize() + 2)<= 255);
		assert(!message->getRouted());

		newPacket->addUint8(message->getSize() + 2); // count priority + routing flag
		newPacket->addUint8(message->getPriority());
		newPacket->addUint8(0);
		newPacket->addData(message->getData(), message->getSize()); 

		mMessageFactory->DestroyMessage(message);
	}

	newPacket->setIsCompressed(true);
	newPacket->setIsEncrypted(true);

	//unreliables go on wire directly
	_addOutgoingUnreliablePacket(newPacket);
	
}

//======================================================================================================================

void Session::_handleOutSequenceRollover()
{
	//rollover of the packet sequence from 0xffff to 0
	gLogger->logMsgF("Session Sequence Rollover queuesize %u nextseqsent: %u Service %u",MSG_HIGH,mWindowPacketList.size(),mNextPacketSequenceSent,mService->getId());
	mOutSequenceRollover = true;
	
	mRolloverWindowPacketList = mWindowPacketList;
	mNewRolloverWindowPacketList = mNewWindowPacketList;
	
	mWindowPacketList.clear();
	mNewWindowPacketList.clear();
}

//======================================================================================================================


