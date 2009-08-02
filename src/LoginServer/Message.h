#ifndef MMOSERVER_LOGINSERVER_MESSAGE_H
#define MMOSERVER_LOGINSERVER_MESSAGE_H

#include "Utils/typedefs.h"


//======================================================================================================================
class Message
{
public:
                              Message(void) : mSize(0), mIndex(sizeof(Message)), mData(0) {}; 

  uint16                      getSize(void)                    { return mSize; };
  int8*                       getData(void)                    { return mData; };

  void                        setData(int8* data)              { mData = data; };
  void                        setSize(uint16 size)             { mSize = size; };

  void                        getInt8(int8& data)              { data = *(int8*)&mData[mIndex]; mIndex += sizeof(int8); };
  void                        getUint8(uint8& data)            { data = *(uint8*)&mData[mIndex]; mIndex += sizeof(uint8); };
  void                        getInt16(int16& data)            { data = *(int16*)&mData[mIndex]; mIndex += sizeof(int16); };
  void                        getUint16(uint16& data)          { data = *(uint16*)&mData[mIndex]; mIndex += sizeof(uint16); };
  void                        getInt32(int32& data)            { data = *(int32*)&mData[mIndex]; mIndex += sizeof(int32); };
  void                        getUint32(uint32& data)          { data = *(uint32*)&mData[mIndex]; mIndex += sizeof(uint32); };
  void                        getInt64(int64& data)            { data = *(int64*)&mData[mIndex]; mIndex += sizeof(int64); };
  void                        getUint64(uint64& data)          { data = *(uint64*)&mData[mIndex]; mIndex += sizeof(uint64); };

  // Max length of a string is uint16.
  uint16                      getStringAnsi(string& data)      { data.initRawBSTR(&mData[mIndex], BSTRType_ANSI); mIndex += data.getLength(); } ;
  uint16                      getStringUnicode16(string& data) { data.initRawBSTR(&mData[mIndex], BSTRType_Unicode16); mIndex += data.getLength() * 2; } ;
  uint16                      getStringUnicode32(string& data) { data.initRawBSTR(&mData[mIndex], BSTRType_Unicode32); mIndex += data.getLength() * 4; } ;

private:
  uint16                      mSize;
  uint16                      mIndex;
  int8*                       mData;
};


#endif //MMOSERVER_LOGINSERVER_MESSAGE_H



