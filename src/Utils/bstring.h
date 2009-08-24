/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_UTILS_BSTRING_H
#define ANH_UTILS_BSTRING_H


#include <vector>

class BString;

typedef std::vector<BString> BStringVector;

//======================================================================================================================

#define BSTRING_ALLOC_BLOCK_SIZE      64

enum BStringType
{
  BSTRType_ANSI,
  BSTRType_Unicode16,
  BSTRType_UTF8
};

//======================================================================================================================

class BString
{
public:
                                      BString(void);
									  BString(BStringType type,uint16 length);
                                      BString(int8* data);
                                      BString(uint16* data);
                                      BString(const BString& data);
                                      ~BString(void)            { if (mString) delete [] mString; }

  uint16                              initRawBSTR(int8* data, BStringType type);


  // Operator overlaods
  bool                                operator ==(char* string);
  bool                                operator ==(const char* data);
  bool								  operator ==(BString& data);

  BString&                            operator =(const int8* data);
  
  BString&                            operator =(int8* data);
  BString&                            operator =(const uint16* data);
  BString&                            operator =(const BString& data);
  
  BString&                            operator <<(const int8* data);
  BString&                            operator <<(int8* data);

  // Accessor Methods
  int8*                               getAnsi(void)                 { if (mType == BSTRType_ANSI) return mString; else return 0; };
  uint16*                             getUnicode16(void)            { if (mType == BSTRType_Unicode16) return (uint16*)mString; else return 0; };
  int8*                               getUTF8(void)                 { if (mType == BSTRType_UTF8) return mString; else return 0; };

  void                                convert(BStringType type);
  void                                substring(BString& dest, uint16 start, uint16 end);

  int                                 split(BStringVector& retVec,char delimiter);
  void                                toLower();
  void                                toUpper();
  bool								  isNumber();

  void                                setLength(uint16 length);
  void                                setType(BStringType type);

  uint16                              getLength(void) const         { return mLength; }
  uint32                              getDataLength(void) const     { return mLength * mCharacterWidth; }
  uint32                              getCharacterWidth(void) const { return mCharacterWidth; }


  BStringType                         getType(void) const           { return mType; }
  int8*                               getRawData(void) const        { return mString; }
  uint32                              getAllocated(void) const      { return mAllocated; }
  uint32                              getCrc(void);
  
  static uint32                       CRC(char* data);

private:
  void                                _allocate(void);

  uint16                              mLength;          // Length of the string itself.  BStrings are NOT null terminated
  uint16                              mAllocated;       // Length of the allocated buffer which should be a multiple of
                                                        // BSTRING_ALLOC_BLOCK_SIZE.
  uint16                              mCharacterWidth;  // Size of a single character in bytes
  BStringType                         mType;            // What format is the current string in.
  int8*                               mString;          // Pointer to the allocated buffer

  static uint32                       mCrcTable[256];
};


//======================================================================================================================
inline void BString::setLength(uint16 length)
{
  mLength = length;
  _allocate();
}


//======================================================================================================================
inline void BString::setType(BStringType type)   
{ 
  mType = type;     // what's the target type and how much space will we need
  switch (mType)
  {
    case BSTRType_ANSI:
      mCharacterWidth = 1;
      break;
    case BSTRType_Unicode16:
    case BSTRType_UTF8:
      mCharacterWidth = 2;
      break;
  }
}


//======================================================================================================================
inline uint32 BString::getCrc(void)
{
  uint32 crc = 0xffffffff;  // starting seed
  for (uint32 i = 0; i < mLength; i++)
  {
    crc = mCrcTable[mString[i] ^ (crc >> 24)] ^ (crc << 8);
  }

  return ~crc;
}


#endif //MMOSERVER_UTILS_BSTRING_H


