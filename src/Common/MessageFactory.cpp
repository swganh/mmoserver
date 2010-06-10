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
#include "Message.h"
#include "MessageFactory.h"

#include "LogManager/LogManager.h"

#include "Utils/clock.h"
#include "NetworkManager/Session.h"

#include <algorithm>
#include <cassert>
#include <cstring>

// TODO: There is no bounds checking on the heap yet.
//======================================================================================================================

MessageFactory* MessageFactory::mSingleton = 0;

//======================================================================================================================

MessageFactory::MessageFactory(uint32 heapSize,uint32 serviceId)
: mCurrentMessage(0)
, mCurrentMessageEnd(0)
, mCurrentMessageStart(0)
, mHeapStart(0)
, mHeapEnd(0)
, mHeapRollover(0)
, mMessageHeap(NULL)
, mHeapTotalSize(heapSize)
, mMessagesCreated(0)
, mMessagesDestroyed(0)
, mServiceId(0)
, mHeapWarnLevel(80.0)
, mMaxHeapUsedPercent(0)
{
	// the singleton is only for use with the zone - the services use their own instantiations as we need 1 factory per thread
	// as the factory is not thread safe

	// Allocate our message heap.
	mMessageHeap = new int8[mHeapTotalSize];
	memset(mMessageHeap, 0xed, mHeapTotalSize);
	mHeapStart = mMessageHeap;
	mHeapEnd = mMessageHeap;

	mLastHeapLevel = 0;
	mLastHeapLevelTime = gClock->getSingleton()->getStoredTime();

	mServiceId = serviceId;
	mLastTime = Anh_Utils::Clock::getSingleton()->getLocalTime();
}

//======================================================================================================================

MessageFactory::~MessageFactory()
{
	// Here is the place for deletes of member data! Not in the Shutdown().
	// But now start to pray that no one still uses these messages. Who knows in this mess?
	delete[] mMessageHeap;

	// mSingleton = 0;
	// Actually, we can't null mSingleton since network manager calls this code directly,
	// using instances created by new(), bypassing the singleton concept,
	// and assign null to mSingleton would invalidate the possibility to delete the "singleton-version" used by zoneserver.

	// mSingleton = 0; IS executed, but in destroySingleton();

	// Important to notice is please that message factory isnt threadsafe
	// thus we need several instances of it !!!!!!
}

//======================================================================================================================

void MessageFactory::Process(void)
{
	// Do some garbage collection if we can.
	_processGarbageCollection();
	
	//maintain a 1sec resolution clock to timestamp messages
	gClock->process();
}

//======================================================================================================================
// important is, that the size of the message class is not nuking our heap bounds! 
void MessageFactory::StartMessage(void)
{
	// Do some garbage collection if we can.
	_processGarbageCollection();

	// Initialize the message start and end.
	mCurrentMessageStart = mHeapStart;

	assert(mCurrentMessage==0 && "Can't handle more than one message at once.");

	mCurrentMessageEnd = mCurrentMessageStart;

	// Adjust start bounds if necessary.
	_adjustMessageStart(sizeof(Message));

	mCurrentMessage = new(mCurrentMessageStart) Message();
	mCurrentMessageEnd = mCurrentMessageStart + sizeof(Message);
}

//======================================================================================================================

uint32 MessageFactory::HeapWarningLevel(void)
{
	uint64 now = gClock->getSingleton()->getStoredTime();
	
	uint32 warnLevel = (uint32)(mCurrentUsed/10);
	if((mCurrentUsed > mLastHeapLevel)&&(mCurrentUsed - mLastHeapLevel) > 10.0)
		warnLevel += 2;

	if((mCurrentUsed > mLastHeapLevel)&&(mCurrentUsed - mLastHeapLevel) > 20.0)
		warnLevel += 4;
	
	if((now - mLastHeapLevelTime) > 1000)
	{
		mLastHeapLevelTime =  now;
		mLastHeapLevel = mCurrentUsed;
	}

	
	return warnLevel;
	
}

//======================================================================================================================

Message* MessageFactory::EndMessage(void)
{
	assert(mCurrentMessage && "Must call StartMessage before EndMessage.");

  // Do some garbage collection if we can.
  //_processGarbageCollection();

  // Just cast the message start
  Message* message = mCurrentMessage;

  message->setData(mCurrentMessageStart + sizeof(Message));
  message->setSize((uint16)(mCurrentMessageEnd - mCurrentMessageStart) - sizeof(Message));
  message->setCreateTime(gClock->getSingleton()->getStoredTime());
  mCurrentMessageStart = mCurrentMessageEnd;

  // Zero out our mCurrentMessage so we know we're not working on one.
  mCurrentMessage = 0;

  //adjust heapstart to past our new message
  mHeapStart += message->getSize() + sizeof(Message);

  //Update our stats.
  mMessagesCreated++;
  mCurrentUsed = ((float)_getHeapSize() / (float)mHeapTotalSize)* 100.0f;
  mMaxHeapUsedPercent = std::max<float>(mMaxHeapUsedPercent,  mCurrentUsed);



  // warn if we get near our boundaries
  if(mCurrentUsed > mHeapWarnLevel)
  {
	  mHeapWarnLevel = static_cast<float>(mCurrentUsed+1.2);
	  gLogger->log(LogManager::EMERGENCY,"MessageFactory Heap at %2.2f usage", mCurrentUsed);
  } else
  if (((mCurrentUsed+2.2) < mHeapWarnLevel) && mHeapWarnLevel > 80.0)
	  mHeapWarnLevel = mCurrentUsed;

  return message;
}

//======================================================================================================================

void MessageFactory::DestroyMessage(Message* message)
{
	// Just flag the message for deletion
	message->setPendingDelete(true);
}

//======================================================================================================================

void MessageFactory::addInt8(int8 data)
{
	// Make sure we've called StartMessage()
	assert(mCurrentMessage && "Must call StartMessage before adding data");

	// Adjust start bounds if necessary.
	_adjustHeapStartBounds(sizeof(data));

	// Insert our data and move our end pointer.
	*mCurrentMessageEnd = data;
	mCurrentMessageEnd += sizeof(int8);
}

//======================================================================================================================

void MessageFactory::addUint8(uint8 data)
{
	// Make sure we've called StartMessage()
	assert(mCurrentMessage && "Must call StartMessage before adding data");

	// Adjust start bounds if necessary.
	_adjustHeapStartBounds(sizeof(data));

	// Insert our data and move our end pointer.
	*mCurrentMessageEnd = (uint8)data;
	mCurrentMessageEnd += sizeof(uint8);
}

//======================================================================================================================

void MessageFactory::addInt16(int16 data)
{
	// Make sure we've called StartMessage()
	assert(mCurrentMessage && "Must call StartMessage before adding data");

	// Adjust start bounds if necessary.
	_adjustHeapStartBounds(sizeof(data));

	// Insert our data and move our end pointer.
	*((int16*)mCurrentMessageEnd) = data;
	mCurrentMessageEnd += sizeof(uint16);
}

//======================================================================================================================

void MessageFactory::addUint16(uint16 data)
{
	// Make sure we've called StartMessage()
	assert(mCurrentMessage);

	// Adjust start bounds if necessary.
	_adjustHeapStartBounds(sizeof(data));

	// Insert our data and move our end pointer.
	*((uint16*)mCurrentMessageEnd) = data;
	mCurrentMessageEnd += sizeof(uint16);
}

//======================================================================================================================

void MessageFactory::addInt32(int32 data)
{
	// Make sure we've called StartMessage()
	assert(mCurrentMessage && "Must call StartMessage before adding data");

	// Adjust start bounds if necessary.
	_adjustHeapStartBounds(sizeof(data));

	// Insert our data and move our end pointer.
	*((int32*)mCurrentMessageEnd) = data;
	mCurrentMessageEnd += sizeof(int32);
}

//======================================================================================================================

void MessageFactory::addUint32(uint32 data)
{
	// Make sure we've called StartMessage()
	assert(mCurrentMessage && "Must call StartMessage before adding data");

	// Adjust start bounds if necessary.
	_adjustHeapStartBounds(sizeof(data));

	// Insert our data and move our end pointer.
	*((uint32*)mCurrentMessageEnd) = data;
	mCurrentMessageEnd += sizeof(uint32);
}

//======================================================================================================================

void MessageFactory::addInt64(int64 data)
{
	// Make sure we've called StartMessage()
	assert(mCurrentMessage && "Must call StartMessage before adding data");

	// Adjust start bounds if necessary.
	_adjustHeapStartBounds(sizeof(data));

	// Insert our data and move our end pointer.
	*((int64*)mCurrentMessageEnd) = data;
	mCurrentMessageEnd += sizeof(int64);
}

//======================================================================================================================

void MessageFactory::addUint64(uint64 data)
{
	// Make sure we've called StartMessage()
	assert(mCurrentMessage && "Must call StartMessage before adding data");

	// Adjust start bounds if necessary.
	_adjustHeapStartBounds(sizeof(data));

	// Insert our data and move our end pointer.
	*((uint64*)mCurrentMessageEnd) = data;
	mCurrentMessageEnd += sizeof(uint64);
}

//======================================================================================================================

void MessageFactory::addFloat(float data)
{
	// Make sure we've called StartMessage()
	assert(mCurrentMessage && "Must call StartMessage before adding data");

	// Adjust start bounds if necessary.
	_adjustHeapStartBounds(sizeof(data));

	// Insert our data and move our end pointer.
	*((float*)mCurrentMessageEnd) = data;
	mCurrentMessageEnd += sizeof(float);
}

//======================================================================================================================

void MessageFactory::addDouble(double data)
{
	// Make sure we've called StartMessage()
	assert(mCurrentMessage && "Must call StartMessage before adding data");

	// Adjust start bounds if necessary.
	_adjustHeapStartBounds(sizeof(data));

	// Insert our data and move our end pointer.
	*((double*)mCurrentMessageEnd) = data;
	mCurrentMessageEnd += sizeof(double);
}

//======================================================================================================================

void MessageFactory::addString(const char* cstring)
{
	BString str;
	str = cstring;
	addString(str);
//return;
}
//======================================================================================================================

void MessageFactory::addString(const unsigned short* ustring)
{
	BString ustr;
	ustr = ustring;
	addString(ustr);
//return;
}
//======================================================================================================================

void MessageFactory::addString(const wchar_t* ustring)
{
	BString ustr;
	ustr = ustring;
	addString(ustr);
}
//======================================================================================================================

void MessageFactory::addString(const string& data)
{
	// Make sure we've called StartMessage()
	assert(mCurrentMessage && "Must call StartMessage before adding data");

	// Adjust start bounds if necessary.
	_adjustHeapStartBounds(data.getDataLength());

	// Insert our data and move our end pointer.
	switch(data.getType())
	{
		case BSTRType_UTF8:
		case BSTRType_ANSI:
		{
			// First insert the string length
			*((uint16*)mCurrentMessageEnd) = data.getLength();
			mCurrentMessageEnd += 2;

			memcpy(mCurrentMessageEnd, data.getAnsi(), data.getLength());
			mCurrentMessageEnd += data.getLength();
		}
		break;

		case BSTRType_Unicode16:
		{
			// First insert the string length
			*((uint32*)mCurrentMessageEnd) = data.getLength();
			mCurrentMessageEnd += 4;

			memcpy(mCurrentMessageEnd, data.getUnicode16(), data.getLength() * 2);
			mCurrentMessageEnd += data.getLength() * 2;
		}
		break;
	}
}

//======================================================================================================================

void MessageFactory::addData(int8* data, uint16 len)
{
	// Make sure we've called StartMessage()
	assert(mCurrentMessage && "Must call StartMessage before adding data");

	// Adjust start bounds if necessary.
	_adjustHeapStartBounds(len);

	// Insert our data and move our end pointer.
	memcpy(mCurrentMessageEnd, data, len);
	mCurrentMessageEnd += len;
}

//======================================================================================================================

void MessageFactory::_processGarbageCollection(void)
{
	uint32 mlt = 3;
	if(_getHeapSize() > 70.0)
			mlt = 2;

	// Just check to see if the oldest message is ready to be deleted yet.
	//start with the oldest message
	assert(mHeapEnd < mMessageHeap + mHeapTotalSize && "mHeapEnd not within mMessageHeap bounds");
	Message* message = reinterpret_cast<Message*>(mHeapEnd);

	//when the oldest Message wont get deleted No other messages get deleted from the heap !!!!!!!!


	uint32 count = 0;
	bool further = true;
	
	
    while((count < 50) && further)
	{
		if (mHeapEnd != mHeapStart)
		{
			if (message->getPendingDelete())
			{

				//uint32 size = message->getSize();
				message->~Message();
				//memset(mHeapEnd, 0xed, size + sizeof(Message));
				mHeapEnd += message->getSize() + sizeof(Message);

				mMessagesDestroyed++;

				// If we're at the end of the queue, rollover to the front again.
				if (mHeapEnd == mHeapRollover)
				{
					if (mHeapEnd == mHeapStart)
					{
						mHeapStart = mMessageHeap;
					}

					mHeapEnd		= mMessageHeap;
					mHeapRollover	= 0;
				}

				message = reinterpret_cast<Message*>(mHeapEnd);
				
				if(!message)
					return;

				assert(mHeapEnd < mMessageHeap + mHeapTotalSize  && "mHeapEnd not within mMessageHeap bounds");

				further = (mHeapEnd != mHeapStart) && message->getPendingDelete();

			}//pending delete

			else if(Anh_Utils::Clock::getSingleton()->getStoredTime() - message->getCreateTime() > MESSAGE_MAX_LIFE_TIME)
			{
				further = false;
				if (!message->mLogged)
				{
					gLogger->log(LogManager::WARNING, "Garbage Collection found a new stuck message!");
					gLogger->logCont(LogManager::INFORMATION, "age : %u ", uint32((Anh_Utils::Clock::getSingleton()->getStoredTime() - message->getCreateTime())/1000));
					
					message->mLogged = true;
					message->mLogTime = Anh_Utils::Clock::getSingleton()->getStoredTime();

					Session* session = (Session*)message->mSession;

					if(!session)
					{
						gLogger->logCont(LogManager::INFORMATION, "Packet is Sessionless.");
						message->setPendingDelete(true);
					}
					else if(session->getStatus() > SSTAT_Disconnected || session->getStatus() == SSTAT_Disconnecting)
					{
						gLogger->logCont(LogManager::INFORMATION, "Session is about to be destroyed.");
					}
				}

				Session* session = (Session*)message->mSession;
				
				if(!session)
				{
					gLogger->log(LogManager::INFORMATION, "Garbage Collection found sessionless packet");
					message->setPendingDelete(true);
				}
				else if(Anh_Utils::Clock::getSingleton()->getStoredTime() >(message->mLogTime +10000))
				{
					gLogger->log(LogManager::EMERGENCY, "Garbage Collection found a old stuck message!");
					gLogger->logCont(LogManager::INFORMATION, "age : %u ", uint32((Anh_Utils::Clock::getSingleton()->getStoredTime() - message->getCreateTime())/1000));
					gLogger->logCont(LogManager::INFORMATION, "Session status : %u ", session->getStatus());
					message->mLogTime  = Anh_Utils::Clock::getSingleton()->getStoredTime();
					return;
				}
				else if(Anh_Utils::Clock::getSingleton()->getStoredTime() - message->getCreateTime() > MESSAGE_MAX_LIFE_TIME*mlt)
				{
					if(session)
					{
						// make sure that the status is not set again from Destroy to Disconnecting
						// otherwise we wont ever get rid of that session
						if(session->getStatus() < SSTAT_Disconnecting)
						{
							session->setCommand(SCOM_Disconnect);
						 	gLogger->log(LogManager::EMERGENCY, "Garbage Collection Message Heap Time out. Destroying Session");
						}
						if(session->getStatus() == SSTAT_Destroy)
						{
						 	gLogger->log(LogManager::EMERGENCY, "Garbage Collection Message Heap Time out. Session about to Destroyed.");
						}
						return;
					}
					else
					{
						message->setPendingDelete(true);
						gLogger->log(LogManager::EMERGENCY, "Garbage Collection Message Heap Time out. Session Already Destroyed. Tagged Message as Deletable.");
						return;
					}

				
				}

			}
			else
				return;
			
		}//Heap start != Heapend
		else
		{
			return;
		}

		//we need to be accurate here
		count++;
	}

}

//======================================================================================================================

void MessageFactory::_adjustHeapStartBounds(uint32 size)
{
	// Are we going to overflow our heap?
	uint32 heapSize = _getHeapSize();

	// _getHeapSize() returns the size of already constructed packets,
	// but NOT the parts we already have been added for THIS message under construction.

	uint32 messageSize = (mCurrentMessageEnd - mCurrentMessageStart);

	//assert(mHeapTotalSize > messageSize + heapSize + size);
 	assert(mHeapTotalSize > messageSize + heapSize && "Message heap overflow.");

	// Check to see if this add is going to push past the heap boundry.
	if(mCurrentMessageEnd + size > mMessageHeap + mHeapTotalSize)
	{
		// We've gone past the end of our heap, copy this message to the front of the heap and continue

		memcpy(mMessageHeap, mCurrentMessageStart, messageSize);

		// Reset our main heap pointer(s)
		if(mHeapStart == mHeapEnd)
		{
			mHeapEnd = mMessageHeap;
		}

		mHeapStart		= mMessageHeap;
		mHeapRollover	= mCurrentMessageStart;

		// Reinit our message pointers.
		mCurrentMessage			= (Message*)mMessageHeap;
		mCurrentMessageStart	= mMessageHeap;
		mCurrentMessageEnd		= mMessageHeap + messageSize;

		mCurrentMessage->setData(mMessageHeap + sizeof(Message));

		gLogger->log(LogManager::WARNING, "Heap Rollover Service %u STATS: MessageHeap - size: %u, maxUsed: %2.2f%, created: %u, destroyed: %u\n", mServiceId,heapSize, mMaxHeapUsedPercent, mMessagesCreated, mMessagesDestroyed);
	}
}

// the trouble is, that if we start a new Message we need to check whether the size of the message class is still inside the heap bounds.
// However mCurrentMessage is 0 at that point
void MessageFactory::_adjustMessageStart(uint32 size)
{
	// Are we going to overflow our heap?
	uint32 heapSize = _getHeapSize();

	// _getHeapSize() returns the size of already constructed packets,
	// but NOT the parts we already have been added for THIS message under construction.

	uint32 messageSize = (mCurrentMessageEnd - mCurrentMessageStart);

	//assert(mHeapTotalSize > messageSize + heapSize + size);
 	assert(mHeapTotalSize > messageSize + heapSize && "Message heap overflow.");

	// Check to see if this add is going to push past the heap boundry.
	if(mCurrentMessageEnd + size > mMessageHeap + mHeapTotalSize)
	{
		// We've gone past the end of our heap, copy this message to the front of the heap and continue

		memcpy(mMessageHeap, mCurrentMessageStart, messageSize);

		// Reset our main heap pointer(s)
		if(mHeapStart == mHeapEnd)
		{
			mHeapEnd = mMessageHeap;
		}

		mHeapStart		= mMessageHeap;
		mHeapRollover	= mCurrentMessageStart;

		// Reinit our message pointers.
		mCurrentMessage			= (Message*)mMessageHeap;
		mCurrentMessageStart	= mMessageHeap;
		mCurrentMessageEnd		= mMessageHeap + messageSize;

		//mCurrentMessage->setData(mMessageHeap + sizeof(Message));

		gLogger->log(LogManager::WARNING, "Heap Rollover Service %u STATS: MessageHeap - size: %u, maxUsed: %2.2f%, created: %u, destroyed: %u\n", mServiceId,heapSize, mMaxHeapUsedPercent, mMessagesCreated, mMessagesDestroyed);
	}
}

//======================================================================================================================








