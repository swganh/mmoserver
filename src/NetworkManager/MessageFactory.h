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

#ifndef ANH_LOGINSERVER_MESSAGEFACTORY_H
#define ANH_LOGINSERVER_MESSAGEFACTORY_H

#include <cstdint>
#include <string>
#include <assert.h>
#include "Utils/typedefs.h"
#include "Utils/bstring.h"

//======================================================================================================================

class Message;

#define gMessageFactory			MessageFactory::getSingleton()

// NEVER DELETE MESSAGES THAT ARE STILL REFERENCED SOMEWHERE
#define MESSAGE_MAX_LIFE_TIME	60000

//======================================================================================================================

class MessageFactory
{
public:

    MessageFactory(uint32 heapSize,uint32 serviceId = 0);
    ~MessageFactory();

    void                    Process(void);

    //provides us with information on the current heap useage
    uint32					HeapWarningLevel(void);

    void                    StartMessage(void);
    Message*                EndMessage(void);

    void                    DestroyMessage(Message* message);

    static MessageFactory*	getSingleton(void);
	static MessageFactory*	getSingleton(uint32_t heap_size);
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
    void                    addString(const BString& data);
    void					addString(const std::string& string);
    void					addString(const std::wstring& string);
    void					addString(const char* cstring);
    void					addString(const wchar_t* ustring);
    void					addString(const unsigned short* ustring);
    void                    addData(const int8* data, uint16 len);
    void                    addData(const uint8_t* data, uint16 len);

    float					getHeapsize() {
        return mCurrentUsed;
    }
private:

    void                    _processGarbageCollection(void);
    void                    _adjustHeapStartBounds(uint32 size);
    //make sure our messageclass size is put inside heap bounds
    void					_adjustMessageStart(uint32 size);
    void                    _adjustHeapEndBounds(uint32 size);
    uint32                  _getHeapSize(void);

    Message*                mCurrentMessage;
    int8*                   mCurrentMessageEnd;
    int8*                   mCurrentMessageStart;

    int8*                   mHeapStart;
    int8*                   mHeapEnd;
    int8*                   mHeapRollover;
    int8*                   mMessageHeap;
    uint64									mLastTime; //last message about stuck messages
    uint32                  mHeapTotalSize; //total heapsize used AND unused


    // Statistics
    uint32                  mMessagesCreated;
    uint32                  mMessagesDestroyed;
    uint32					mServiceId;
    float					mHeapWarnLevel;
    float                   mMaxHeapUsedPercent;

    float					mLastHeapLevel;
    uint64					mLastHeapLevelTime;
    float					mCurrentUsed;

    static MessageFactory*	mSingleton;
    // Anh_Utils::Clock*		mClock;
};

//======================================================================================================================

inline MessageFactory* MessageFactory::getSingleton(void)
{
    if(!mSingleton)
    {
        assert(0 && "You should have called the overload in your Server's initalization method...");
    }

    return mSingleton;
}

//======================================================================================================================

inline MessageFactory* MessageFactory::getSingleton(uint32_t heap_size)
{
    if(!mSingleton)
    {
        mSingleton = new MessageFactory(heap_size*1024);
    }

    return mSingleton;
}

//======================================================================================================================

inline void MessageFactory::destroySingleton(void)
{
    if (mSingleton)
    {
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

    return (uint32)mHeapTotalSize-(mHeapEnd - mHeapStart);
    //return (uint32)(mMessageHeap + mHeapTotalSize - mHeapEnd + mHeapStart - mMessageHeap);
}

//======================================================================================================================

#endif  //MMOSERVER_LOGINSERVER_MESSAGEFACTORY_H



