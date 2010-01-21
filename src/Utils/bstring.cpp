/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "typedefs.h"  // This must be first here to remove the dependency from the header.
#include "bstring.h"

#include <algorithm>

#include <cassert>
#include <cctype>
#include <cstdlib>
#include <cstring>

#include <wchar.h>
#include <wctype.h>

//======================================================================================================================

BString::BString()
: mString(0)
, mType(BSTRType_ANSI)
, mAllocated(0)
, mCharacterWidth(1)
, mLength(0)
{
  _allocate();
  *(uint32*)mString = 0;   // Make sure a new empty string is null terminated.
}

//======================================================================================================================

BString::BString(BStringType type,uint16 length)
	: mString(0)
  , mType(type)
  , mAllocated(0)
  , mLength(length)
{
	if(type == BSTRType_ANSI)
		mCharacterWidth = 1;
	else if(type == BSTRType_Unicode16)
		mCharacterWidth = 2;

	_allocate();
	*(uint32*)mString = 0;
}

//======================================================================================================================

BString::~BString()
{
    if (mString) delete [] mString;
}

//======================================================================================================================

BString::BString(const int8* data)
: mString(0)
, mType(BSTRType_ANSI)
, mAllocated(0)
, mCharacterWidth(1)
, mLength(0)
{
	_allocate();
	// we might get a null pointer from db queries
	if(data != NULL)
		*this = data;
	else
		*(uint32*)mString = 0;
}

//======================================================================================================================

BString::BString(const uint16* data)
: mString(0)
, mType(BSTRType_Unicode16)
, mAllocated(0)
, mCharacterWidth(1)
, mLength(0)
{
	_allocate();
	// we might get a null pointer from db queries
	if(data != NULL)
		*this = data;
	else
		*(uint32*)mString = 0;
}

//======================================================================================================================

BString::BString(const wchar_t* data)
: mString(0)
, mType(BSTRType_Unicode16)
, mAllocated(0)
, mCharacterWidth(1)
, mLength(0)
{
	_allocate();
	// we might get a null pointer from db queries
	if(data != NULL)
		*this = data;
	else
		*(uint32*)mString = 0;
}


//======================================================================================================================

BString::BString(const BString& data)
: mString(0)
, mType(BSTRType_ANSI)
, mAllocated(0)
, mCharacterWidth(1)
, mLength(0)
{
	_allocate();
	*this = data;
}

//======================================================================================================================

uint16 BString::initRawBSTR(int8* data, BStringType type)
{
	uint16	totalLen = *(uint16*)data;

	mType = type;

	switch(type)
	{
		case BSTRType_ANSI:
		{
			mCharacterWidth = 1;
		}
		break;

		case BSTRType_Unicode16:
		case BSTRType_UTF8:
		{
			mCharacterWidth = 2;
		}
		break;
	}

	uint32 charLen = (totalLen + 1) * mCharacterWidth;

	// If we don't have enough room in our buffer, re-allocate a new one
	if(charLen > mAllocated)
	{
		if(mString)
			delete [] mString;

		mAllocated = (((static_cast<uint16>(charLen) / BSTRING_ALLOC_BLOCK_SIZE) + 1) * BSTRING_ALLOC_BLOCK_SIZE);
		mString = new int8[mAllocated];

		memset(mString,0,mAllocated);
	}

	// Copy our string into the buffer.
	if(mAllocated)
	{
		memcpy(mString,data+(mCharacterWidth*2),charLen - mCharacterWidth);
		memset(&mString[charLen - mCharacterWidth], 0, mCharacterWidth);
	}

	mLength = totalLen;

	return totalLen;
}

//======================================================================================================================

bool BString::operator ==(char* data) // compare against ansi null terminated string
{
	return(strcmp(data,mString) == 0);
}

//======================================================================================================================

bool BString::operator ==(const char* data) // compare against const ansi null terminated string
{
	return(strcmp(data,mString) == 0);
}

//======================================================================================================================

bool BString::operator ==(const BString& data)
{
	return (strcmp(mString, data.getAnsi()) == 0);
}

//======================================================================================================================

bool BString::operator ==(BString data)
{
	return (strcmp(mString, data.getAnsi()) == 0);
}

//======================================================================================================================

BString& BString::operator =(const int8* data)
{
	// we might get a null pointer from db queries
	if(data != NULL)
	{
	  mType = BSTRType_ANSI;
	  mCharacterWidth = 1;
	  mLength = (uint16)strlen(data);

	  // If we don't have enough room in our buffer, re-allocate a new one
	  _allocate();

	  // Cpoy our string into the new buffer.
	  if (mAllocated)
	  {
		memcpy(mString, data, mLength);
		memset(&mString[mLength], 0, 1);
	  }
	}

  return *this;
}

//======================================================================================================================
BString& BString::operator =(const uint16* data)
{
	// we might get a null pointer from db queries
	if(data != NULL)
	{
	  mType = BSTRType_Unicode16;
	  mCharacterWidth = 2;
	  mLength = static_cast<uint16>(wcslen(reinterpret_cast<const wchar_t*>(data)));

	  // If we don't have enough room in our buffer, re-allocate a new one
	  _allocate();

	  // Cpoy our string into the new buffer.
	  if (mAllocated)
	  {
		memcpy(mString, data, mLength*2);
		memset(&mString[mLength*2], 0, 1);
	  }
	}
  return *this;
}

//======================================================================================================================
BString& BString::operator =(const wchar_t* data)
{
	// we might get a null pointer from db queries
	if(data != NULL)
	{
	  mType = BSTRType_Unicode16;
	  mCharacterWidth = 2;
	  mLength = static_cast<uint16>(wcslen(data));

	  // If we don't have enough room in our buffer, re-allocate a new one
	  _allocate();

	  // Cpoy our string into the new buffer.
	  if (mAllocated)
	  {
		memcpy(mString, data, mLength*2);
		memset(&mString[mLength*2], 0, 1);
	  }
	}
  return *this;
}


//======================================================================================================================

BString& BString::operator =(int8* data)
{
	// we might get a null pointer from db queries
	if(data != NULL)
	{
		mType = BSTRType_ANSI;
		mCharacterWidth = 1;
		mLength = (uint16)strlen(data);

		_allocate();

		// Copy our string into the new buffer.
		if(mAllocated)
		{
			memcpy(mString, data, mLength);
			memset(&mString[mLength], 0, 1);
		}
	}

	return *this;
}

//======================================================================================================================

BString& BString::operator =(const BString& data)
{
	mType = data.getType();
	mCharacterWidth = static_cast<uint16>(data.getCharacterWidth());
	mLength = data.getLength();
    mAllocated = static_cast<uint16>(data.getAllocated());

  if (mString)
    delete [] mString;
	mString = new int8[mAllocated];

  // Copy our string into the new buffer.
  memcpy(mString, data.getRawData(), mAllocated);

	return *this;
}


//======================================================================================================================


BString& BString::operator <<(const uint16* data)
{
	if(mType != BSTRType_Unicode16)
		return *this;

	// we might get a null pointer from db queries
	if(data != NULL)
	{
	  // Get our source string length
	  uint16 dataLength  = static_cast<uint16>(wcslen(reinterpret_cast<const wchar_t*>(data)));
	  //uint16 dataLength = (uint16)strlen(data);

	  // Set our new length and allocate
	  uint16 oldLength = mLength;
	  mLength += dataLength;
	  _allocate();

	  // Append our string into the bugger.
	  memcpy(mString + oldLength * mCharacterWidth, data, dataLength* mCharacterWidth);
	  memset(mString + mLength * mCharacterWidth, 0, mCharacterWidth);
	}
  return *this;
}


BString& BString::operator <<(uint16* data)
{
	if(mType != BSTRType_Unicode16)
		return *this;

	// we might get a null pointer from db queries
	if(data != NULL)
	{
	  // Get our source string length
	  uint16 dataLength  = static_cast<uint16>(wcslen(reinterpret_cast<const wchar_t*>(data)));
	  //uint16 dataLength = (uint16)strlen(data);

	  // Set our new length and allocate
	  uint16 oldLength = mLength;
	  mLength += dataLength;
	  _allocate();

	  // Append our string into the bugger.
	  memcpy(mString + oldLength * mCharacterWidth, data, dataLength* mCharacterWidth);
	  memset(mString + mLength * mCharacterWidth, 0, mCharacterWidth);
	}
  return *this;
}

BString& BString::operator <<(const int8* data)
{
	// we might get a null pointer from db queries
	if(data != NULL)
	{
	  // Get our source string length
	  uint16 dataLength = (uint16)strlen(data);

	  // Set our new length and allocate
	  uint16 oldLength = mLength;
	  mLength += dataLength;
	  _allocate();

	  // Append our string into the bugger.
	  memcpy(mString + oldLength * mCharacterWidth, data, dataLength);
	  memset(mString + mLength * mCharacterWidth, 0, mCharacterWidth);
	}
  return *this;
}


//======================================================================================================================
BString& BString::operator <<(int8* data)
{
	// we might get a null pointer from db queries
	if(data != NULL)
	{
	  // Get our source string length
	  uint16 dataLength = (uint16)strlen(data);

	  // Set our new length and allocate
	  uint16 oldLength = mLength;
	  mLength += dataLength;
	  _allocate();

	  // Append our string into the bugger.
	  memcpy(mString + oldLength * mCharacterWidth, data, dataLength);
	  memset((mString + mLength * mCharacterWidth), 0, mCharacterWidth);
	}
  return *this;
}

int8* BString::getAnsi()
{
    if (mType == BSTRType_ANSI) {
        return mString;
    } else {
        return 0;
    }
}

//======================================================================================================================

const int8* BString::getAnsi() const
{
    if (mType == BSTRType_ANSI) {
        return mString;
    } else {
        return 0;
    }
}

//======================================================================================================================
/*
uint16* BString::getUnicode16()
{
    if (mType == BSTRType_Unicode16)
    {
        return (uint16*)mString;
    } else {
        return 0;
    }
}
 */
//======================================================================================================================

wchar_t* BString::getUnicode16()
{
    if (mType == BSTRType_Unicode16)
    {
        return reinterpret_cast<wchar_t*>(mString);
    } else {
        return 0;
    }
}

const wchar_t* BString::getUnicode16() const
{
    if (mType == BSTRType_Unicode16)
    {
        return reinterpret_cast<wchar_t*>(mString);
    } else {
        return 0;
    }
}

//======================================================================================================================

int8* BString::getUTF8()
{
    if (mType == BSTRType_UTF8)
    {
        return mString;
    } else {
        return 0;
    }
}

//======================================================================================================================

void BString::convert(BStringType type)
{
	// Try not to use this often as it is slow.
	// If we're trying to convert to the same type, just return.
	if(mType == type)
		return;

	//  Locals
	int8*   newBuffer = 0;
	uint16  allocated = 0;

	// what's the target type and how much space will we need
	switch(type)
	{
		case BSTRType_ANSI:
		{
			mCharacterWidth = 1;
			allocated = ((((mLength+1) / BSTRING_ALLOC_BLOCK_SIZE) + 1) * BSTRING_ALLOC_BLOCK_SIZE);

			// Allocate a new buffer for the converted string.
			newBuffer = new int8[allocated];
			//Initial null terminator
			memset(newBuffer,0,allocated);

			// Convert the string if needed.
			if(mType == BSTRType_Unicode16)
			{
                wcstombs(newBuffer, reinterpret_cast<wchar_t*>(mString), std::min(allocated, mAllocated));
			}
			else if(mType == BSTRType_UTF8)
			{
				// FIXME: Implement
				//int i = 0;
			}
		}
		break;

		case BSTRType_Unicode16:
		{
			mCharacterWidth = 2;
			allocated = (((((mLength+1) * mCharacterWidth) / BSTRING_ALLOC_BLOCK_SIZE) + 1) * BSTRING_ALLOC_BLOCK_SIZE);

			// Allocate a new buffer for the converted string.
			newBuffer = new int8[allocated];
			//Initial null terminator
			memset(newBuffer,0,allocated);

			if(mType == BSTRType_ANSI || mType == BSTRType_UTF8)
			{
                mbstowcs(reinterpret_cast<wchar_t*>(newBuffer),mString, std::min(allocated/2, mLength+1));
			}
		}
		break;

		case BSTRType_UTF8:
		{
			mCharacterWidth = 2;

			allocated = (((((mLength+1) * mCharacterWidth) / BSTRING_ALLOC_BLOCK_SIZE) + 1) * BSTRING_ALLOC_BLOCK_SIZE);
			// Allocate a new buffer for the converted string.
			newBuffer = new int8[allocated];
			//Initial null terminator
			*(uint32*)newBuffer = 0;

			if(mType == BSTRType_ANSI)
			{
				// FIXME: Implement, not sure if it needs to be though
				//int i = 0;
			}
			else if(mType == BSTRType_Unicode16)
			{
				wcstombs((int8*)newBuffer, reinterpret_cast<wchar_t*>(mString), mLength+1);
			}
		}
		break;
	}

	// We are now the new type of string
	if(mString != 0)
		delete [] mString;

	mString		= newBuffer;
	mAllocated	= allocated;
	mType		= type;
}

//======================================================================================================================

	// BS code,
	// an internal data modifier as toLower(), toUpper() etc ... should NEVER change anything else of the string.
	// In this case, the string is FORCED to BSTRType_ANSI.

	// Implemeting hidden side effects in a function is never good, on a STANDARD object like stings is a disaster.
	// Better halt controlled than CTD or server crash.

void BString::toLower()
{
	if(mType == BSTRType_Unicode16)
	{
		assert(false);
		//convert(BSTRType_ANSI);
		uint16* data = (uint16*)mString;

		while(*data)
		{
			*data = towlower(*data);
			data++;
		}
		return;

	}

	int8* data = mString;

	while(*data)
	{
		*data = tolower(*data);
		++data;
	}
}

//======================================================================================================================

void BString::toUpper()
{
	if(mType == BSTRType_Unicode16)
	{
		assert(false);
		convert(BSTRType_ANSI);
	}

	int8* data = mString;

	while(*data)
	{
		*data = toupper(*data);
		++data;
	}
}

//======================================================================================================================

void BString::toUpperFirst()
{
	if(mType == BSTRType_Unicode16)
	{
		uint16* data = (uint16*)mString;

		*data = towupper(*data);
		return;
	}

	int8* data = mString;

	*data = toupper(*data);

}

//======================================================================================================================

void BString::toLowerFirst()
{
	if(mType == BSTRType_Unicode16)
	{
		uint16* data = (uint16*)mString;

		*data = towlower(*data);
		return;
	}

	int8* data = mString;

	*data = tolower(*data);

}

//======================================================================================================================

bool BString::isNumber()
{
	if(mType == BSTRType_Unicode16)
	{
		assert(false);
		convert(BSTRType_ANSI);
	}

	int8* data = mString;

	while(*data)
	{
		if(!(isdigit(*data)))
			return(false);

		++data;
	}

	return(true);
}

//======================================================================================================================

int BString::split(BStringVector& retVec,char delimiter)
{
	retVec.clear();

	if(mType == BSTRType_Unicode16)
	{
		assert(false);
		convert(BSTRType_ANSI);
	}

	int8* data = mString;
	uint16 beginIndex = 0;
	uint16 endIndex = 0;

	while(1)
	{
		string tmpStr;

		beginIndex = endIndex;

		while(*data != delimiter && *data)
		{
			++data;
			++endIndex;
		}

		substring(tmpStr,beginIndex,endIndex);
		tmpStr.getRawData()[endIndex - beginIndex] = 0;

		retVec.push_back(BString(tmpStr.getAnsi()));

		if(!*data)
			break;

		if(!*++data)
			break;

		++endIndex;
	}
	return retVec.size();
}

//======================================================================================================================

void BString::substring(BString& dest, uint16 start, uint16 end)
{
  // Validate our parameters
  if (start > mLength || end > mLength || end < start)
    return;

  // Setup our destination string
  dest.setType(mType);
  // dest.setLength(mLength);
  dest.setLength(end - start);

  // what's the target type and how much space will we need
  switch (mType)
  {
    case BSTRType_UTF8:
    case BSTRType_ANSI:
      {
        int8* destBuffer = dest.getRawData();
        strncpy(destBuffer, mString + start, end - start);
        break;
      }
    case BSTRType_Unicode16:
      {
        uint16* destBuffer = (uint16*)dest.getRawData();
        wcsncpy(reinterpret_cast<wchar_t*>(destBuffer), reinterpret_cast<wchar_t*>(mString) + start, end - start);
        break;
      }
  }
}

//======================================================================================================================

void BString::_allocate()
{
	// If we don't have enough room in our buffer, re-allocate a new one
	if(mLength * mCharacterWidth >= mAllocated)
	{
		int8* newString = 0;
		int32 previousAllocated = mAllocated;

		// what's the target type and how much space will we need
		switch(mType)
		{
			case BSTRType_ANSI:
			{
				mAllocated = ((((mLength+1) / BSTRING_ALLOC_BLOCK_SIZE) + 1) * BSTRING_ALLOC_BLOCK_SIZE);
				newString = new char[mAllocated];

				memset(newString,0,mAllocated);

				if(mString)
				{
                    memcpy(newString,mString, std::min<uint32>(mLength + mCharacterWidth,previousAllocated));
				}
			}
			break;

			case BSTRType_Unicode16:
			case BSTRType_UTF8:
			{
				mAllocated = ((((mLength+1) / BSTRING_ALLOC_BLOCK_SIZE) + 1) * BSTRING_ALLOC_BLOCK_SIZE) * mCharacterWidth;
				newString = new char[mAllocated];

				memset(newString,0,mAllocated);

				if(mString)
				{
                    memcpy(newString, mString, std::min<uint32>((mLength + mCharacterWidth) * 2, previousAllocated));
				}
			}
			break;
		}

		if(mString != 0)
			delete [] mString;

		mString = newString;
	}
}

//======================================================================================================================

uint16 BString::getLength() const
{
    return mLength;
}

//======================================================================================================================

uint32 BString::getDataLength() const
{
    return mLength * mCharacterWidth;
}

//======================================================================================================================

uint32 BString::getCharacterWidth() const
{
    return mCharacterWidth;
}

//======================================================================================================================

BStringType BString::getType() const
{
    return mType;
}

//======================================================================================================================

int8* BString::getRawData() const
{
    return mString;
}

//======================================================================================================================

uint32 BString::getAllocated() const
{
    return mAllocated;
}

//======================================================================================================================
uint32 BString::CRC(char* data)
{
  uint32 length = (uint32)strlen(data);

  uint32 crc = 0xffffffff;  // starting seed
  for (uint32 i = 0; i < length; i++)
  {
    crc = mCrcTable[data[i] ^ (crc >> 24)] ^ (crc << 8);
  }

  return ~crc;
}


//======================================================================================================================
uint32 BString::mCrcTable[256] =
{
    0x0000000,
    0x04C11DB7, 0x09823B6E, 0x0D4326D9, 0x130476DC, 0x17C56B6B,
    0x1A864DB2, 0x1E475005, 0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6,
    0x2B4BCB61, 0x350C9B64, 0x31CD86D3, 0x3C8EA00A, 0x384FBDBD,
    0x4C11DB70, 0x48D0C6C7, 0x4593E01E, 0x4152FDA9, 0x5F15ADAC,
    0x5BD4B01B, 0x569796C2, 0x52568B75, 0x6A1936C8, 0x6ED82B7F,
    0x639B0DA6, 0x675A1011, 0x791D4014, 0x7DDC5DA3, 0x709F7B7A,
    0x745E66CD, 0x9823B6E0, 0x9CE2AB57, 0x91A18D8E, 0x95609039,
    0x8B27C03C, 0x8FE6DD8B, 0x82A5FB52, 0x8664E6E5, 0xBE2B5B58,
    0xBAEA46EF, 0xB7A96036, 0xB3687D81, 0xAD2F2D84, 0xA9EE3033,
    0xA4AD16EA, 0xA06C0B5D, 0xD4326D90, 0xD0F37027, 0xDDB056FE,
    0xD9714B49, 0xC7361B4C, 0xC3F706FB, 0xCEB42022, 0xCA753D95,
    0xF23A8028, 0xF6FB9D9F, 0xFBB8BB46, 0xFF79A6F1, 0xE13EF6F4,
    0xE5FFEB43, 0xE8BCCD9A, 0xEC7DD02D, 0x34867077, 0x30476DC0,
    0x3D044B19, 0x39C556AE, 0x278206AB, 0x23431B1C, 0x2E003DC5,
    0x2AC12072, 0x128E9DCF, 0x164F8078, 0x1B0CA6A1, 0x1FCDBB16,
    0x018AEB13, 0x054BF6A4, 0x0808D07D, 0x0CC9CDCA, 0x7897AB07,
    0x7C56B6B0, 0x71159069, 0x75D48DDE, 0x6B93DDDB, 0x6F52C06C,
    0x6211E6B5, 0x66D0FB02, 0x5E9F46BF, 0x5A5E5B08, 0x571D7DD1,
    0x53DC6066, 0x4D9B3063, 0x495A2DD4, 0x44190B0D, 0x40D816BA,
    0xACA5C697, 0xA864DB20, 0xA527FDF9, 0xA1E6E04E, 0xBFA1B04B,
    0xBB60ADFC, 0xB6238B25, 0xB2E29692, 0x8AAD2B2F, 0x8E6C3698,
    0x832F1041, 0x87EE0DF6, 0x99A95DF3, 0x9D684044, 0x902B669D,
    0x94EA7B2A, 0xE0B41DE7, 0xE4750050, 0xE9362689, 0xEDF73B3E,
    0xF3B06B3B, 0xF771768C, 0xFA325055, 0xFEF34DE2, 0xC6BCF05F,
    0xC27DEDE8, 0xCF3ECB31, 0xCBFFD686, 0xD5B88683, 0xD1799B34,
    0xDC3ABDED, 0xD8FBA05A, 0x690CE0EE, 0x6DCDFD59, 0x608EDB80,
    0x644FC637, 0x7A089632, 0x7EC98B85, 0x738AAD5C, 0x774BB0EB,
    0x4F040D56, 0x4BC510E1, 0x46863638, 0x42472B8F, 0x5C007B8A,
    0x58C1663D, 0x558240E4, 0x51435D53, 0x251D3B9E, 0x21DC2629,
    0x2C9F00F0, 0x285E1D47, 0x36194D42, 0x32D850F5, 0x3F9B762C,
    0x3B5A6B9B, 0x0315D626, 0x07D4CB91, 0x0A97ED48, 0x0E56F0FF,
    0x1011A0FA, 0x14D0BD4D, 0x19939B94, 0x1D528623, 0xF12F560E,
    0xF5EE4BB9, 0xF8AD6D60, 0xFC6C70D7, 0xE22B20D2, 0xE6EA3D65,
    0xEBA91BBC, 0xEF68060B, 0xD727BBB6, 0xD3E6A601, 0xDEA580D8,
    0xDA649D6F, 0xC423CD6A, 0xC0E2D0DD, 0xCDA1F604, 0xC960EBB3,
    0xBD3E8D7E, 0xB9FF90C9, 0xB4BCB610, 0xB07DABA7, 0xAE3AFBA2,
    0xAAFBE615, 0xA7B8C0CC, 0xA379DD7B, 0x9B3660C6, 0x9FF77D71,
    0x92B45BA8, 0x9675461F, 0x8832161A, 0x8CF30BAD, 0x81B02D74,
    0x857130C3, 0x5D8A9099, 0x594B8D2E, 0x5408ABF7, 0x50C9B640,
    0x4E8EE645, 0x4A4FFBF2, 0x470CDD2B, 0x43CDC09C, 0x7B827D21,
    0x7F436096, 0x7200464F, 0x76C15BF8, 0x68860BFD, 0x6C47164A,
    0x61043093, 0x65C52D24, 0x119B4BE9, 0x155A565E, 0x18197087,
    0x1CD86D30, 0x029F3D35, 0x065E2082, 0x0B1D065B, 0x0FDC1BEC,
    0x3793A651, 0x3352BBE6, 0x3E119D3F, 0x3AD08088, 0x2497D08D,
    0x2056CD3A, 0x2D15EBE3, 0x29D4F654, 0xC5A92679, 0xC1683BCE,
    0xCC2B1D17, 0xC8EA00A0, 0xD6AD50A5, 0xD26C4D12, 0xDF2F6BCB,
    0xDBEE767C, 0xE3A1CBC1, 0xE760D676, 0xEA23F0AF, 0xEEE2ED18,
    0xF0A5BD1D, 0xF464A0AA, 0xF9278673, 0xFDE69BC4, 0x89B8FD09,
    0x8D79E0BE, 0x803AC667, 0x84FBDBD0, 0x9ABC8BD5, 0x9E7D9662,
    0x933EB0BB, 0x97FFAD0C, 0xAFB010B1, 0xAB710D06, 0xA6322BDF,
    0xA2F33668, 0xBCB4666D, 0xB8757BDA, 0xB5365D03, 0xB1F740B4,
};




//======================================================================================================================
void BString::setLength(uint16 length)
{
  mLength = length;
  _allocate();
}


//======================================================================================================================
void BString::setType(BStringType type)
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
uint32 BString::getCrc() const
{
  uint32 crc = 0xffffffff;  // starting seed
  for (uint32 i = 0; i < mLength; i++)
  {
    crc = mCrcTable[mString[i] ^ (crc >> 24)] ^ (crc << 8);
  }

  return ~crc;
}
