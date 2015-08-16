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

#ifndef ANH_UTILS_BSTRING_H
#define ANH_UTILS_BSTRING_H

#include <cstdint>
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
    BString();
    BString(BStringType type, uint16_t length);
    ~BString();

    BString(const char* data);
    BString(const uint16_t* data);
    BString(const wchar_t* data);
    BString(const BString& data);

    uint16_t initRawBSTR(char* data, BStringType type);

    // Operator overloads
    bool operator ==(char* string);
    bool operator ==(const char* data);
    bool operator ==(const BString& data);
    bool operator ==(BString data);

    BString& operator =(const char* data);

    BString& operator =(char* data);
    BString& operator =(const uint16_t* data);
    BString& operator =(const wchar_t* data);
    BString& operator =(const BString& data);

    BString& operator <<(const char* data);
    BString& operator <<(const uint16_t* data);
    BString& operator <<(char* data);
    BString& operator <<(uint16_t* data);

    // Accessor Methods
    const char* getAnsi() const;
    const wchar_t* getUnicode16() const;

    void convert(BStringType type);
    void substring(BString& dest, uint16_t start, uint16_t end);

    int  split(BStringVector& retVec,char delimiter);
    void toLower();
    void toUpper();
    void toUpperFirst();
    void toLowerFirst();
    bool isNumber();

    void setLength(uint16_t length);
    void setType(BStringType type);

    uint16_t getLength() const;
    uint32_t getDataLength() const;
    uint32_t getCharacterWidth() const;

    BStringType getType() const;
    char* getRawData() const;
    uint32_t getAllocated() const;
    uint32_t getCrc() const;

    static uint32_t CRC(char* data);

private:
    void _allocate();

    char* mString;          // Pointer to the allocated buffer
    BStringType mType;            // What format is the current string in.
    uint16_t mAllocated;       // Length of the allocated buffer which should be a multiple of
    uint16_t mCharacterWidth;  // Size of a single character in bytes
    uint16_t mLength;          // Length of the string itself.  BStrings are NOT null terminated

    static uint32_t mCrcTable[256];
};

#endif //MMOSERVER_UTILS_BSTRING_H


