/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

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

MessageFactory::MessageFactory()
: mCurrentMessage(0)
, mCurrentMessageEnd(0)
, mCurrentMessageStart(0)
, mHeapStart(0)
, mHeapEnd(0)
, mHeapRollover(0)
, mMessageHeap(NULL)
, mHeapTotalSize(0)
, mMessagesCreated(0)
, mMessagesDestroyed(0)
, mServiceId(0)
, mHeapWarnLevel(80.0)
, mMaxHeapUsedPercent(0)
{
	// gLogger->logMsgF("MessageFactory::MessageFactory() CONSTRUCTED",MSG_NORMAL);
}

//======================================================================================================================

MessageFactory::~MessageFactory()
{
	// Here is the place for deletes of member data! Not in the Shutdown().
	// But now start to pray that no one still uses these messages. Who knows in this mess?
	// gLogger->logMsgF("MessageFactory::~MessageFactory() DESTRUCTED",MSG_NORMAL);
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

void MessageFactory::Startup(uint32 heapSize,uint32 serviceId)
{
	mHeapTotalSize = heapSize;

	// the singleton is only for use with the zone - the services use their own instantiations as we need 1 factory per thread
	// as the factory is not thread safe

	// Allocate our message heap.
	mMessageHeap = new int8[mHeapTotalSize];
	memset(mMessageHeap, 0xed, mHeapTotalSize);
	mHeapStart = mMessageHeap;
	mHeapEnd = mMessageHeap;

	mServiceId = serviceId;
	mLastTime = Anh_Utils::Clock::getSingleton()->getLocalTime();
}

//======================================================================================================================

void MessageFactory::Shutdown(void)
{
	// Can't / should not delete the member data without deleting the instance...

	// destroy our message heap

	//delete[] mMessageHeap;

}

//======================================================================================================================

void MessageFactory::Process(void)
{
	// Do some garbage collection if we can.
	_processGarbageCollection();
}

//======================================================================================================================
// important is, that the size of the message class is not nuking our heap bounds! 
void MessageFactory::StartMessage(void)
{
	// Do some garbage collection if we can.
	_processGarbageCollection();

	// Initialize the message start and end.
	mCurrentMessageStart = mHeapStart;

	assert(mCurrentMessage==0);	// Can't handle more than one message at once.

	mCurrentMessageEnd = mCurrentMessageStart;

	// Adjust start bounds if necessary.
	_adjustMessageStart(sizeof(Message));

	mCurrentMessage = new(mCurrentMessageStart) Message();
	mCurrentMessageEnd = mCurrentMessageStart + sizeof(Message);
}

//======================================================================================================================

Message* MessageFactory::EndMessage(void)
{
  assert(mCurrentMessage);

  // Do some garbage collection if we can.
  _processGarbageCollection();

  // Add one more zero byte at the end of the message
  //addUint8(0);

  // Just cast the message start
  Message* message = mCurrentMessage;

  message->setData(mCurrentMessageStart + sizeof(Message));
  message->setSize((uint16)(mCurrentMessageEnd - mCurrentMessageStart) - sizeof(Message));
  message->setCreateTime(Anh_Utils::Clock::getSingleton()->getLocalTime());
  mCurrentMessageStart = mCurrentMessageEnd;

  // Zero out our mCurrentMessage so we know we're not working on one.
  mCurrentMessage = 0;
  mHeapStart += message->getSize() + sizeof(Message);

  //Update our stats.
  mMessagesCreated++;
  float currentUsed = ((float)_getHeapSize() / (float)mHeapTotalSize)* 100.0f;
  mMaxHeapUsedPercent = std::max<float>(mMaxHeapUsedPercent,  currentUsed);



  // warn if we get near our boundaries
  if(currentUsed > mHeapWarnLevel)
  {
	  mHeapWarnLevel = static_cast<float>(currentUsed+1.2);
	  gLogger->logMsgF("WARNING: MessageFactory Heap at %2.2f usage",MSG_NORMAL,currentUsed);
  } else
  if (((currentUsed+2.2) < mHeapWarnLevel) && mHeapWarnLevel > 80.0)
	  mHeapWarnLevel = currentUsed;

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
	assert(mCurrentMessage);

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
	assert(mCurrentMessage);

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
	assert(mCurrentMessage);

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
	assert(mCurrentMessage);

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
	assert(mCurrentMessage);

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
	assert(mCurrentMessage);

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
	assert(mCurrentMessage);

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
	assert(mCurrentMessage);

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
	assert(mCurrentMessage);

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
	assert(mCurrentMessage);

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
	assert(mCurrentMessage);

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
	Message* message = reinterpret_cast<Message*>(mHeapEnd);
	assert(mHeapEnd < mMessageHeap + mHeapTotalSize);

	//when the oldest Message wont get deleted No other messages get deleted from the heap !!!!!!!!


	uint64 timestart, time;
	bool further = true;
	timestart = time = Anh_Utils::Clock::getSingleton()->getLocalTime();
    while(((time - timestart ) < 1) && further)
	{
		if (mHeapEnd != mHeapStart)
		{
			if (message->getPendingDelete())
			{

				uint32 size = message->getSize();
				message->~Message();
				memset(mHeapEnd, 0xed, size + sizeof(Message));
				mHeapEnd += size + sizeof(Message);

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
				
				assert(mHeapEnd < mMessageHeap + mHeapTotalSize);

				further = (mHeapEnd != mHeapStart) && message->getPendingDelete();

			}//pending delete

			else if(Anh_Utils::Clock::getSingleton()->getLocalTime() - message->getCreateTime() > MESSAGE_MAX_LIFE_TIME)
			{
				further = false;
				if (!message->mLogged)
				{
					gLogger->logMsgF("MessageFactory::_processGarbageCollection : New stuck Message !!! ",MSG_HIGH);
					gLogger->logMsgF("age : %u ",MSG_HIGH, uint32((Anh_Utils::Clock::getSingleton()->getLocalTime() - message->getCreateTime())/1000));
					gLogger->logMsgF("Source : %u ",MSG_HIGH, message->mSourceId);
					gLogger->logMsgF("Path : %u ",MSG_HIGH, message->mPath);
					
					

					gLogger->hexDump(message->getData(), message->getSize());
					message->mLogged = true;
					message->mLogTime = Anh_Utils::Clock::getSingleton()->getLocalTime();

					Session* session = (Session*)message->mSession;

					if(!session)
					{
						gLogger->logMsgF("Packet is Sessionless ",MSG_HIGH);
						message->setPendingDelete(true);
					}
					else
					if(session->getStatus() > SSTAT_Disconnected)
					{
						gLogger->logMsgF("Session is about to be destroyed ",MSG_HIGH);

					}
					else
					if(session->getStatus() == SSTAT_Disconnecting)
					{
						gLogger->logMsgF("Session is about to be destroyed ",MSG_HIGH);

					}

				}

				Session* session = (Session*)message->mSession;
				
				if(!session)
				{
					gLogger->logMsgF("Packet is Sessionless ",MSG_HIGH);
					message->setPendingDelete(true);
				}
				else
				if(Anh_Utils::Clock::getSingleton()->getLocalTime() >(message->mLogTime +10000))
				{
					gLogger->logMsgF("MessageFactory::_processGarbageCollection : Old stuck Message !!! ",MSG_HIGH);
					gLogger->logMsgF("age : %u ",MSG_HIGH, uint32((Anh_Utils::Clock::getSingleton()->getLocalTime() - message->getCreateTime())/1000));
					gLogger->logMsgF("Source : %u ",MSG_HIGH, message->mSourceId);
					gLogger->logMsgF("Path : %u ",MSG_HIGH, message->mPath);
					gLogger->logMsgF("Session status : %u ",MSG_HIGH, session->getStatus());
					gLogger->hexDump(message->getData(), message->getSize());
					message->mLogTime  = Anh_Utils::Clock::getSingleton()->getLocalTime();
					return;
				}
				else
				if(Anh_Utils::Clock::getSingleton()->getLocalTime() - message->getCreateTime() > MESSAGE_MAX_LIFE_TIME*mlt)
				{
					if(session)
					{
						// make sure that the status is not set again from Destroy to Disconnecting
						// otherwise we wont ever get rid of that session
						if(session->getStatus() < SSTAT_Disconnecting)
						{
							session->setCommand(SCOM_Disconnect);
						 	gLogger->logErrorF("MessageLayer","MessageFactory::_processGarbageCollection Message Heap TimeOut destroying Session ",MSG_HIGH);
						}
						if(session->getStatus() == SSTAT_Destroy)
						{
						 	gLogger->logErrorF("MessageLayer","MessageFactory::_processGarbageCollection Session about to be destroyed",MSG_HIGH);
						}
						return;
					}
					else
					{
						message->setPendingDelete(true);
						gLogger->logErrorF("MessageLayer","MessageFactory::_processGarbageCollection Message Heap TimeOut session already destroyed - tagging message as destroyable",MSG_HIGH);
						return;
					}

				
				}

			}
			else
				return;

			time = Anh_Utils::Clock::getSingleton()->getLocalTime();
		}//Heap start != Heapend
		else
		{
			return;
		}

		time = Anh_Utils::Clock::getSingleton()->getLocalTime();
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
 	assert(mHeapTotalSize > messageSize + heapSize );

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

		gLogger->logMsgF("Heap Rollover Service %u STATS: MessageHeap - size: %u, maxUsed: %2.2f%, created: %u, destroyed: %u\n", MSG_HIGH, mServiceId,heapSize, mMaxHeapUsedPercent, mMessagesCreated, mMessagesDestroyed);
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
 	assert(mHeapTotalSize > messageSize + heapSize );

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

		gLogger->logMsgF("Heap Rollover Service %u STATS: MessageHeap - size: %u, maxUsed: %2.2f%, created: %u, destroyed: %u\n", MSG_HIGH, mServiceId,heapSize, mMaxHeapUsedPercent, mMessagesCreated, mMessagesDestroyed);
	}
}

//======================================================================================================================








