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

#ifndef ANH_NETWORKMANAGER_PACKET_H
#define ANH_NETWORKMANAGER_PACKET_H

#include "Utils/typedefs.h"
#include <memory.h>   //really hate doing this, but I don't think this one file should cause much trouble.
//  And with that comment, I just cursed us all.
#include <assert.h>
#include <NetworkManager/Session.h>


//======================================================================================================================
class SessionPacket;
class Session;


//======================================================================================================================
enum SESSIONOP
{
    SESSIONOP_First                 = 0x0000,
    SESSIONOP_SessionRequest        = 0x0100,
    SESSIONOP_SessionResponse       = 0x0200,
    SESSIONOP_MultiPacket           = 0x0300,
    SESSIONOP_Unknown1              = 0x0400,
    SESSIONOP_Disconnect            = 0x0500,
    SESSIONOP_Ping                  = 0x0600,
    SESSIONOP_NetStatRequest        = 0x0700,
    SESSIONOP_NetStatResponse       = 0x0800,
    SESSIONOP_DataChannel1          = 0x0900,
    SESSIONOP_DataChannel2          = 0x0a00,
    SESSIONOP_DataChannel3          = 0x0b00,
    SESSIONOP_DataChannel4          = 0x0c00,
    SESSIONOP_DataFrag1             = 0x0d00,
    SESSIONOP_DataFrag2             = 0x0e00,
    SESSIONOP_DataFrag3             = 0x0f00,
    SESSIONOP_DataFrag4             = 0x1000,
    SESSIONOP_DataOrder1            = 0x1100,
    SESSIONOP_DataOrder2            = 0x1200,
    SESSIONOP_DataOrder3            = 0x1300,
    SESSIONOP_DataOrder4            = 0x1400,
    SESSIONOP_DataAck1              = 0x1500,
    SESSIONOP_DataAck2              = 0x1600,
    SESSIONOP_DataAck3              = 0x1700,
    SESSIONOP_DataAck4              = 0x1800,
    SESSIONOP_FatalError            = 0x1900,
    SESSIONOP_FatalErrorResponse    = 0x1a00,
    SESSIONOP_Reset                 = 0x1d00,
    SESSIONOP_CriticalError		  = 0x1e00
};



//======================================================================================================================
class Packet
{
public:
    //we want to have bigger packets for zone connection communication
    Packet(void)
        : mTimeCreated(0)
        , mTimeQueued(0)
        , mTimeOOHSent(0)
        , mTimeSent(0)
        , mCRC(0)
        , mResends(0)
        , mSize(0)
        , mReadIndex(0)
        , mWriteIndex(0)
        , mCompressed(0)
        , mEncrypted(0)
    {}

    void                          Reset(void);

    void                          setReadIndex(uint16 index)          {
        mReadIndex = index;
    }
    void                          setWriteIndex(uint16 index)         {
        mReadIndex = index;
    }

    uint16                        getReadIndex(void)                  {
        return mReadIndex;
    }
    uint16                        getWriteIndex(void)                 {
        return mReadIndex;
    }

    uint64                        getTimeCreated(void)                {
        return mTimeCreated;
    }
    uint64                        getTimeQueued(void)                 {
        return mTimeQueued;
    }
    uint64                        getTimeSent(void)                   {
        return mTimeSent;
    }
    uint64                        getTimeOOHSent(void)                {
        return mTimeOOHSent;
    }
    uint32                        getResends(void)                    {
        return mResends;
    }
    uint16                        getSize(void)                       {
        return mSize;
    }
    int8*                         getData(void)                       {
        return mData;
    }
    uint16                        getMaxPayload(void)                 {
        return mMaxPayLoad;
    }
    uint16                        getPacketType(void)                 {
        return *(uint16*)mData;
    }
    bool                          getIsCompressed(void)               {
        return mCompressed;
    }
    bool                          getIsEncrypted(void)                {
        return mEncrypted;
    }
    uint32                        getCRC(void)                        {
        return mCRC;
    }

    void                          setMaxPayload(uint16 pl)            {
        mMaxPayLoad = pl;
    }
    void                          setTimeCreated(uint64 time)         {
        mTimeCreated = time;
    }
    void                          setTimeQueued(uint64 time)          {
        mTimeQueued = time;
    }
    void                          setTimeSent(uint64 time)            {
        mTimeSent = time;
    }
    void                          setTimeOOHSent(uint64 time)         {
        mTimeOOHSent = time;
    }
    void                          setResends(uint32 resends)          {
        mResends = resends;
    }
    void                          setSize(uint16 size)                {
        assert(size <= mMaxPayLoad && "Packet size larger than MaxPayLoad");
        mSize = size;
    }
    void                          setPacketType(uint16 type)          {
        *((int16*)mData) = type;
    }
    void                          setIsCompressed(bool compressed)    {
        mCompressed = compressed;
    }
    void                          setIsEncrypted(bool encrypted)      {
        mEncrypted = encrypted;
    }
    void                          setCRC(uint32 crc)                  {
        mCRC = crc;
    }


    // Two generic interfaces to mData.  Temporary until a better ones can be implemented.
    void                          addUint8(uint8 data)                {
        *(uint8*)&mData[mWriteIndex] = data;
        mWriteIndex += sizeof(uint8);
        if (mWriteIndex > mSize) mSize = mWriteIndex;
        assert(mSize <= mMaxPayLoad && "Packet size larger than MaxPayLoad");
    }
    void                          addUint16(uint16 data)              {
        *(uint16*)&mData[mWriteIndex] = data;
        mWriteIndex += sizeof(uint16);
        if (mWriteIndex > mSize) mSize = mWriteIndex;
        assert(mSize <= mMaxPayLoad && "Packet size larger than MaxPayLoad");
    }
    void                          addUint32(uint32 data)              {
        *(uint32*)&mData[mWriteIndex] = data;
        mWriteIndex += sizeof(uint32);
        if (mWriteIndex > mSize) mSize = mWriteIndex;
        assert(mSize <= mMaxPayLoad && "Packet size larger than MaxPayLoad");
    }
    void                          addUint64(uint64 data)              {
        *(uint64*)&mData[mWriteIndex] = data;
        mWriteIndex += sizeof(uint64);
        if (mWriteIndex > mSize) mSize = mWriteIndex;
        assert(mSize <= mMaxPayLoad && "Packet size larger than MaxPayLoad");
    }
    void                          addData(int8* data, uint16 len)     {
        memcpy((void*)&mData[mWriteIndex], data, len);
        mWriteIndex += len;
        if (mWriteIndex > mSize) mSize = mWriteIndex;
        assert(mSize <= mMaxPayLoad && "Packet size larger than MaxPayLoad");
    }

    int8                          getInt8(void)                       {
        int8 value = *(int8*)&mData[mReadIndex];
        mReadIndex += sizeof(int8);
        return value;
    }
    uint8                         getUint8(void)                      {
        uint8 value = *(uint8*)&mData[mReadIndex];
        mReadIndex += sizeof(uint8);
        return value;
    }
    int16                         getInt16(void)                      {
        int16 value = *(int16*)&mData[mReadIndex];
        mReadIndex += sizeof(int16);
        return value;
    }
    uint16                        getUint16(void)                     {
        uint16 value = *(uint16*)&mData[mReadIndex];
        mReadIndex += sizeof(uint16);
        return value;
    }
    int32                         getInt32(void)                      {
        int32 value = *(int32*)&mData[mReadIndex];
        mReadIndex += sizeof(int32);
        return value;
    }
    uint32                        getUint32(void)                     {
        uint32 value = *(uint32*)&mData[mReadIndex];
        mReadIndex += sizeof(uint32);
        return value;
    }
    int64                         getInt64(void)                      {
        int64 value = *(int64*)&mData[mReadIndex];
        mReadIndex += sizeof(int64);
        return value;
    }
    uint64                        getUint64(void)                     {
        uint64 value = *(uint64*)&mData[mReadIndex];
        mReadIndex += sizeof(uint64);
        return value;
    }
    float                         getFloat(void)                      {
        float value = *(float*)&mData[mReadIndex];
        mReadIndex += sizeof(float);
        return value;
    }
    double                        getDouble(void)                     {
        double value = *(double*)&mData[mReadIndex];
        mReadIndex += sizeof(double);
        return value;
    }

    int8                          peekInt8(void)                      {
        int8 value = *(int8*)&mData[mReadIndex];
        return value;
    }
    uint8                         peekUint8(void)                     {
        uint8 value = *(uint8*)&mData[mReadIndex];
        return value;
    }
    int16                         peekInt16(void)                     {
        int16 value = *(int16*)&mData[mReadIndex];
        return value;
    }
    uint16                        peekUint16(void)                    {
        uint16 value = *(uint16*)&mData[mReadIndex];
        return value;
    }
    int32                         peekInt32(void)                     {
        int32 value = *(int32*)&mData[mReadIndex];
        return value;
    }
    uint32                        peekUint32(void)                    {
        uint32 value = *(uint32*)&mData[mReadIndex];
        return value;
    }
    int64                         peekInt64(void)                     {
        int64 value = *(int64*)&mData[mReadIndex];
        return value;
    }
    uint64                        peekUint64(void)                    {
        uint64 value = *(uint64*)&mData[mReadIndex];
        return value;
    }
    float                         peekFloat(void)                     {
        float value = *(float*)&mData[mReadIndex];
        return value;
    }
    double                        peekDouble(void)                    {
        double value = *(double*)&mData[mReadIndex];
        return value;
    }


    Session*                      getSession(void)                    {
        return mSession;
    }
    void							setSession(Session* session)		{
        mSession = session;
    }

protected:
    Session*						mSession;

    //we want to have bigger packets for zone connection communication
    int8                          mData[MAX_SERVER_PACKET_SIZE];
    uint64                        mTimeCreated;
    uint64                        mTimeQueued;
    uint64												mTimeOOHSent;
    uint64                        mTimeSent;
    uint32                        mCRC;
    uint32                        mResends;
    uint16												mMaxPayLoad;
    uint16                        mSize;
    uint16                        mReadIndex;
    uint16                        mWriteIndex;
    bool                          mCompressed;
    bool                          mEncrypted;

};


//======================================================================================================================
inline void Packet::Reset(void)
{
    mTimeCreated      = 0;
    mTimeSent         = 0;
    mResends          = 0;
    mSize             = 0;
    mReadIndex        = 0;
    mWriteIndex       = 0;
    mCompressed       = false;
    mEncrypted        = false;
    mCRC              = 0;
}

#endif //ANH_NETWORKMANAGER_PACKET_H


