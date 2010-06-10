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

#include "Utils/typedefs.h"
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
                              : mCreateTime(0)
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
							  , mLogged(false)
							  , mLogTime(0)
							  , mSession(NULL)
                              {}

  void                        Init(int8* data, uint16 len)      { mData = data; mSize = len; mIndex = 0;}
  void                        ResetIndex(void)                  { mIndex = 0; }

  uint16                      getSize(void)                     { return mSize; }
  int8*                       getData(void)                     { return mData; }
  uint16                      getIndex(void)                    { return mIndex; }
  inline uint8                getPriority(void)                 { return mPriority; }
  uint32                      getAccountId(void)                { return mAccountId; }
  uint8                       getDestinationId(void)            { return mDestinationId; }
  bool                        getRouted(void)                   { return mRouted; }
  uint64                      getCreateTime(void)               { return mCreateTime; }
  uint64                      getQueueTime(void)                { return mQueueTime; }
  bool                        getFastpath(void)                 { return mFastpath; }
  bool                        getPendingDelete(void)            { return mPendingDelete; }

  void                        setData(int8* data)               { mData = data; }
  void                        setSize(uint16 size)              { mSize = size; }
  void                        setIndex(uint16 index)            { mIndex = index; }
  void                        setPriority(uint8 priority)       { mPriority = priority; }
  void                        setAccountId(uint32 id)           { mAccountId = id; }
  void                        setDestinationId(uint8 id)        { mDestinationId = id; }
  void                        setRouted(bool routed)            { mRouted = routed; }
  void                        setCreateTime(uint64 time)        { mCreateTime = time; }
  void                        setQueueTime(uint64 time)         { mQueueTime = time; }
  void                        setFastpath(bool fastpath)        { mFastpath = fastpath; }
  void                        setPendingDelete(bool pending)    { mPendingDelete = pending; }

  void                        getInt8(int8& data)               { data = *(int8*)&mData[mIndex]; mIndex += sizeof(int8); }
  void                        getUint8(uint8& data)             { data = *(uint8*)&mData[mIndex]; mIndex += sizeof(uint8); }
  void                        getInt16(int16& data)             { data = *(int16*)&mData[mIndex]; mIndex += sizeof(int16); }
  void                        getUint16(uint16& data)           { data = *(uint16*)&mData[mIndex]; mIndex += sizeof(uint16); }
  void                        getInt32(int32& data)             { data = *(int32*)&mData[mIndex]; mIndex += sizeof(int32); }
  void                        getUint32(uint32& data)           { data = *(uint32*)&mData[mIndex]; mIndex += sizeof(uint32); }
  void                        getInt64(int64& data)             { data = *(int64*)&mData[mIndex]; mIndex += sizeof(int64); }
  void                        getUint64(uint64& data)           { data = *(uint64*)&mData[mIndex]; mIndex += sizeof(uint64); }

  int8                        getInt8(void)                     { int8 value = *(int8*)&mData[mIndex]; mIndex += sizeof(int8); return value; }
  uint8                       getUint8(void)                    { uint8 value = *(uint8*)&mData[mIndex]; mIndex += sizeof(uint8); return value; }
  int16                       getInt16(void)                    { int16 value = *(int16*)&mData[mIndex]; mIndex += sizeof(int16); return value; }
  uint16                      getUint16(void)                   { uint16 value = *(uint16*)&mData[mIndex]; mIndex += sizeof(uint16); return value; }
  int32                       getInt32(void)                    { int32 value = *(int32*)&mData[mIndex]; mIndex += sizeof(int32); return value; }
  uint32                      getUint32(void)                   { uint32 value = *(uint32*)&mData[mIndex]; mIndex += sizeof(uint32); return value; }
  int64                       getInt64(void)                    { int64 value = *(int64*)&mData[mIndex]; mIndex += sizeof(int64); return value; }
  uint64                      getUint64(void)                   { uint64 value = *(uint64*)&mData[mIndex];mIndex += sizeof(uint64); return value; }
  float                       getFloat(void)                    { float value = *(float*)&mData[mIndex];mIndex += sizeof(float); return value; }
  double                      getDouble(void)                   { double value = *(double*)&mData[mIndex];mIndex += sizeof(double); return value; }

  int8                        peekInt8(void)                    { int8 value = *(int8*)&mData[mIndex]; return value; }
  uint8                       peekUint8(void)                   { uint8 value = *(uint8*)&mData[mIndex]; return value; }
  int16                       peekInt16(void)                   { int16 value = *(int16*)&mData[mIndex]; return value; }
  uint16                      peekUint16(void)                  { uint16 value = *(uint16*)&mData[mIndex]; return value; }
  int32                       peekInt32(void)                   { int32 value = *(int32*)&mData[mIndex]; return value; }
  uint32                      peekUint32(void)                  { uint32 value = *(uint32*)&mData[mIndex]; return value; }
  int64                       peekInt64(void)                   { int64 value = *(int64*)&mData[mIndex]; return value; }
  uint64                      peekUint64(void)                  { uint64 value = *(uint64*)&mData[mIndex]; return value; }
  float                       peekFloat(void)                   { float value = *(float*)&mData[mIndex]; return value; }
  double                      peekDouble(void)                  { double value = *(double*)&mData[mIndex]; return value; }

  // Max length of a string is uint16 - 4 (two bytes for length at beginning of buffer, and 2 bytes at end for NULL terminator.)
  uint16                      getStringAnsi(BString& data)       { data.initRawBSTR(&mData[mIndex], BSTRType_ANSI); mIndex += data.getLength()+2; return data.getLength(); }
  uint16                      getStringUnicode16(BString& data)  { data.initRawBSTR(&mData[mIndex], BSTRType_Unicode16); mIndex += data.getLength() * 2 + 4; return data.getLength(); }
  
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
  
  int8*                       mData;

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



