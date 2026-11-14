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

#ifndef ANH_LOGINSERVER_MESSAGE_H
#define ANH_LOGINSERVER_MESSAGE_H

#include <cstdint>
#include <cstring>
#include <string>

#include "Utils/typedefs.h"
#include "Utils/bstring.h"
#include "Utils/logger.h"

enum MessagePath
{
    MP_None = 0,
    MP_DataChannelA = 1,
    MP_RoutedReliable= 2,
    MP_RoutedMulti= 3,
    MP_buildPacketStarted= 4,
    MP_buildPacketEnded= 5,
    MP_Multi= 6,
    MP_buildPacketUnreliable= 7


};



//======================================================================================================================
class Message
{
public:
    Message(void)
        : mLogged(false)
        , mLogTime(0)
        , mSession(nullptr)
        , mCreateTime(0)
        , mQueueTime(0)
        , mAccountId(0xffffffff)
        , mSize(0)
        , mIndex(0)
        , mPriority(0)
        , mDestinationId(0)
        , mRouted(false)
        , mFastpath(false)
        , mPendingDelete(false)
        , mData(0)
    {}

    void                        Init(int8* data, uint16 len)      {
        mData = data;
        mSize = len;
        mIndex = 0;
        mOverrun = false;
    }
    void                        ResetIndex(void)                  {
        mIndex = 0;
        mOverrun = false;
    }

    uint16                      getSize(void)                     {
        return mSize;
    }
    int8*                       getData(void)                     {
        return mData;
    }
    uint16                      getIndex(void)                    {
        return mIndex;
    }
    inline uint8                getPriority(void)                 {
        return mPriority;
    }
    uint32                      getAccountId(void)                {
        return mAccountId;
    }
    uint8                       getDestinationId(void)            {
        return mDestinationId;
    }
    bool                        getRouted(void)                   {
        return mRouted;
    }
    uint64                      getCreateTime(void)               {
        return mCreateTime;
    }
    uint64                      getQueueTime(void)                {
        return mQueueTime;
    }
    bool                        getFastpath(void)                 {
        return mFastpath;
    }
    bool                        getPendingDelete(void)            {
        return mPendingDelete;
    }

    void                        setData(int8* data)               {
        mData = data;
    }
    void                        setSize(uint16 size)              {
        mSize = size;
    }
    void                        setIndex(uint16 index)            {
        mIndex = index;
        mOverrun = false;
    }
    void                        setPriority(uint8 priority)       {
        mPriority = priority;
    }
    void                        setAccountId(uint32 id)           {
        mAccountId = id;
    }
    void                        setDestinationId(uint8 id)        {
        mDestinationId = id;
    }
    void                        setRouted(bool routed)            {
        mRouted = routed;
    }
    void                        setCreateTime(uint64 time)        {
        mCreateTime = time;
    }
    void                        setQueueTime(uint64 time)         {
        mQueueTime = time;
    }
    void                        setFastpath(bool fastpath)        {
        mFastpath = fastpath;
    }
    void                        setPendingDelete(bool pending)    {
        mPendingDelete = pending;
    }

    // Reads are bounds checked: a read past the end of the message (a
    // malformed or hostile packet) returns 0 or an empty string, does not
    // move the index, and marks the message overrun; so does every read
    // after it. Values are copied out, not read through unaligned pointers.
    bool                        isOverrun(void) const             {
        return mOverrun;
    }

    void                        getInt8(int8& data)            {
        data = read_<int8>();
    }
    void                        getUint8(uint8& data)            {
        data = read_<uint8>();
    }
    void                        getInt16(int16& data)            {
        data = read_<int16>();
    }
    void                        getUint16(uint16& data)            {
        data = read_<uint16>();
    }
    void                        getInt32(int32& data)            {
        data = read_<int32>();
    }
    void                        getUint32(uint32& data)            {
        data = read_<uint32>();
    }
    void                        getInt64(int64& data)            {
        data = read_<int64>();
    }
    void                        getUint64(uint64& data)            {
        data = read_<uint64>();
    }
    int8                        getInt8(void)                  {
        return read_<int8>();
    }
    uint8                       getUint8(void)                  {
        return read_<uint8>();
    }
    int16                       getInt16(void)                  {
        return read_<int16>();
    }
    uint16                      getUint16(void)                  {
        return read_<uint16>();
    }
    int32                       getInt32(void)                  {
        return read_<int32>();
    }
    uint32                      getUint32(void)                  {
        return read_<uint32>();
    }
    int64                       getInt64(void)                  {
        return read_<int64>();
    }
    uint64                      getUint64(void)                  {
        return read_<uint64>();
    }
    float                       getFloat(void)                  {
        return read_<float>();
    }
    double                      getDouble(void)                  {
        return read_<double>();
    }
    int8                        peekInt8(void)                 {
        return peek_<int8>();
    }
    uint8                       peekUint8(void)                 {
        return peek_<uint8>();
    }
    int16                       peekInt16(void)                 {
        return peek_<int16>();
    }
    uint16                      peekUint16(void)                 {
        return peek_<uint16>();
    }
    int32                       peekInt32(void)                 {
        return peek_<int32>();
    }
    uint32                      peekUint32(void)                 {
        return peek_<uint32>();
    }
    int64                       peekInt64(void)                 {
        return peek_<int64>();
    }
    uint64                      peekUint64(void)                 {
        return peek_<uint64>();
    }
    float                       peekFloat(void)                 {
        return peek_<float>();
    }
    double                      peekDouble(void)                 {
        return peek_<double>();
    }

    // Max length of a string is uint16 - 4 (two bytes for length at beginning of buffer, and 2 bytes at end for NULL terminator.)
    // The length comes from the packet: it is checked against the bytes left
    // before anything is copied.
    uint16                      getStringAnsi(BString& data)       {
        uint16 length = peek_<uint16>();
        if (!fits_(2 + static_cast<uint32_t>(length))) {
            return empty_(data, BSTRType_ANSI);
        }
        data.initRawBSTR(&mData[mIndex], BSTRType_ANSI);
        mIndex += data.getLength()+2;
        return data.getLength();
    }
    uint16                      getStringUnicode16(BString& data)  {
        uint32 length = peek_<uint32>();
        if (length > 0xFFFF || !fits_(4 + length * 2)) {
            return empty_(data, BSTRType_Unicode16);
        }
        data.initRawBSTR(&mData[mIndex], BSTRType_Unicode16);
        mIndex += data.getLength() * 2 + 4;
        return data.getLength();
    }

    std::string getStringAnsi() {
        uint16_t size = getUint16();
        if (!fits_(size)) {
            overrun_(size);
            return std::string();
        }
        std::string tmp(&mData[mIndex], &mData[mIndex + size]);

        mIndex += size;

        return tmp;
    }

    std::u16string getStringUnicode16() {
    	uint32_t size = getUint32();
    	if (size > 0xFFFF || !fits_(size * 2)) {
    		overrun_(size * 2);
    		return std::u16string();
    	}
    	std::u16string tmp(size, u'\0');
    	memcpy(&tmp[0], &mData[mIndex], size * 2);

    	mIndex += tmp.size() * 2;

    	return tmp;
    }

    bool                        mLogged;
    uint64                      mLogTime;
    void*						  mSession;

private:
    uint64                      mCreateTime;
    uint64                      mQueueTime;
    uint32                      mAccountId;
    uint16                      mSize;
    uint16                      mIndex;
    uint8                       mPriority;
    uint8                       mDestinationId;
    bool                        mRouted;
    bool                        mFastpath;
    bool                        mPendingDelete;
    bool                        mOverrun = false;

    int8*                       mData;

    // Whether n more bytes are left to read.
    bool                        fits_(uint32_t n) const {
        return mData && static_cast<uint32_t>(mIndex) + n <= mSize;
    }
    template<typename T> T      read_() {
        T value = T();
        if (!mOverrun && fits_(sizeof(T))) {
            memcpy(&value, &mData[mIndex], sizeof(T));
            mIndex += sizeof(T);
        } else {
            overrun_(sizeof(T));
        }
        return value;
    }
    // The first read past the end of this message: logged once, so a handler
    // that relied on reading past a wrong size shows up.
    void                        overrun_(uint32_t wanted) {
        if (!mOverrun) {
            LOG(WARNING) << "Message read past its end: " << wanted << " bytes at " << mIndex << " of " << mSize;
        }
        mOverrun = true;
    }
    template<typename T> T      peek_() const {
        T value = T();
        if (!mOverrun && fits_(sizeof(T))) {
            memcpy(&value, &mData[mIndex], sizeof(T));
        }
        return value;
    }
    uint16                      empty_(BString& data, BStringType type) {
        static char zeros[4] = { 0, 0, 0, 0 };
        overrun_(0);
        data.initRawBSTR(zeros, type);
        return 0;
    }

};

class CompareMsg
{
public:

    bool operator () (Message* left,Message* right)
    {
        return(left->getPriority() < right->getPriority());
    }
};



#endif //MMOSERVER_LOGINSERVER_MESSAGE_H



