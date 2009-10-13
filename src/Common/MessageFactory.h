/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_LOGINSERVER_MESSAGEFACTORY_H
#define ANH_LOGINSERVER_MESSAGEFACTORY_H

#include "Utils/typedefs.h"
#include "ConfigManager/ConfigManager.h"

//======================================================================================================================

class Message;

#define gMessageFactory			MessageFactory::getSingleton()

// force a delete when we exceed this limit
#define MESSAGE_MAX_LIFE_TIME	60000

//======================================================================================================================

class MessageFactory
{
	public:

		MessageFactory(void);
		~MessageFactory(void);

		void                    Startup(uint32 heapSize,uint32 serviceId = 0);
		void                    Shutdown(void);
		void                    Process(void);

		void                    StartMessage(void);
		Message*                EndMessage(void);
		void                    DestroyMessage(Message* message);

		static MessageFactory*	getSingleton(void);
		static void             destroySingleton(void);

		// Data packing methods.
		void                    addInt8(int8 data);
		void                    addUint8(uint8 data);
		void                    addInt16(int16 data);
		void                    addUint16(uint16 data);
		void                    addInt32(int32 data);
		void                    addUint32(uint32 data);
		void                    addInt64(int64 data);
		void                    addUint64(uint64 data);
		void                    addFloat(float data);
		void                    addDouble(double data);
		void                    addString(string& data);
		void					addString(const char* cstring);
		void					addString(const wchar_t* ustring);
		void					addString(const unsigned short* ustring);
		void                    addData(int8* data, uint16 len);

	private:

		void                    _processGarbageCollection(void);
		void                    _adjustHeapStartBounds(uint32 size);
		void                    _adjustHeapEndBounds(uint32 size);
		uint32                  _getHeapSize(void);

		Message*                mCurrentMessage;
		int8*                   mCurrentMessageStart;
		int8*                   mCurrentMessageEnd;

		int8*                   mHeapStart;
		int8*                   mHeapEnd; 
		int8*                   mHeapRollover;
		uint32                  mHeapTotalSize;		//total heapsize used AND unused

		int8*                   mMessageHeap;

		// Statistics
		uint32                  mMessagesCreated;
		uint32                  mMessagesDestroyed;
		float                   mMaxHeapUsedPercent;
		float					mHeapWarnLevel;
		uint64					mLastTime;//last message about stuck messages

		static MessageFactory*	mSingleton;
		uint32					mServiceId;
		// Anh_Utils::Clock*		mClock;
};

//======================================================================================================================

inline MessageFactory* MessageFactory::getSingleton(void)    
{ 
	if(!mSingleton)
	{
		// gLogger->logMsgF("MessageFactory::getSingleton()",MSG_NORMAL);
		mSingleton = new MessageFactory();
		mSingleton->Startup(gConfig->read<uint32>("GlobalMessageHeap")*1024);
	}

	return mSingleton;
}

//======================================================================================================================

inline void MessageFactory::destroySingleton(void)    
{ 
	if (mSingleton)
	{
		mSingleton->Shutdown();
		delete mSingleton;
		mSingleton = 0;
	}
}

//======================================================================================================================

inline uint32 MessageFactory::_getHeapSize(void)
{
	if (mHeapStart >= mHeapEnd)
	{
		return (uint32)(mHeapStart - mHeapEnd);
	}

	return (uint32)(mMessageHeap + mHeapTotalSize - mHeapEnd + mHeapStart - mMessageHeap);
}

//======================================================================================================================

#endif  //MMOSERVER_LOGINSERVER_MESSAGEFACTORY_H



