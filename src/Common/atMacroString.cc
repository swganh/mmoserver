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

#include "Common/atMacroString.h"

#include <cstring>

atMacroString::atMacroString(void)
{
    mPoint = &Hallo[0];
    mPoint1 = &Hallo[0];
    mSize = 0;
    mAssembled = false;
    mX = 0;
    mY = 0;
    mZ = 0;
    mCounter = 0;
    mTUId = 0;
    mTOId = 0;
    mTTId = 0;
}

atMacroString::~atMacroString(void)
{
}

//==============================================================================
// copies ASCII or Unicode string byte for byte into our array keeping the charwidth
// the length of the string gets added correctly in front as either uint16 or uint32

void	atMacroString::_addString(BString mString)
{
    uint32 length = (uint32)mString.getLength();
    //start by adding the size
    switch (mString.getType()) {
        //2bytes for ascii
    case BSTRType_UTF8:
    case BSTRType_ANSI: {
        _adduint16(mString.getLength());
    }
    break;

    //4 for unicode16
    case BSTRType_Unicode16: {
        //_adduint32((uint32)mString.getLength());
        memcpy(mPoint1,&length,4);
        mPoint1 +=4;
        mSize +=4;
    }
    break;
    }

    //now the String itself
    memcpy(mPoint1,mString.getRawData(),mString.getDataLength());
    mPoint1 += mString.getDataLength();
    mSize	+= mString.getDataLength();

}

//==============================================================================
// Adds a byte Value to our array. The value gets properly escaped if need be
//

void	atMacroString::_addByte(uint8 mByte)
{

    *mPoint1 = mByte;
    mPoint1 ++;
    mSize ++;


}

void	atMacroString::_adduint32(uint32 mInteger)
{

    memcpy(mPoint1,&mInteger,4);
    mPoint1 +=4;
    mSize +=4;
}

void	atMacroString::_addfloat(float mFloat)
{

    memcpy(mPoint1,&mFloat,4);
    mPoint1 +=4;
    mSize +=4;
}

void	atMacroString::_adduint16(uint16 mWord)
{

    memcpy(mPoint1,&mWord,2);
    mPoint1 +=2;
    mSize +=2;


}

void	atMacroString::_adduint64(uint64 mLong)
{

    memcpy(mPoint1,&mLong,8);
    mPoint1 +=8;
    mSize +=8;

}

void	atMacroString::setPlanetString(BString planet)
{

    mPlanetCRC = planet.getCrc();

}

void	atMacroString::addWaypoint()
{

    //byte 1 unknown
    _addByte(0);//perhaps counter, so this would be waypoint nr 1 ???
    _addByte(mCounter-1);//perhaps counter, so this would be waypoint nr 1 ???
    //mCounter++;
    //however why start macros then at 0 ????
    _addByte(0);
    _addByte(4);//1 for macros 4 for waypoints

    //unknown integer must be 0xfffffffd for waypoints
    _adduint32(0xfffffffd);


    _adduint32(0);//
    _addfloat(mX);// x
    _adduint32(0);//
    _addfloat(mY);//y
    _adduint32(0);//
    _addfloat(mZ);//z


    _adduint32(mPlanetCRC);//crc planet

    mWPName.convert(BSTRType_Unicode16);
    _addString(mWPName);
    _adduint32(0);//
    _adduint32(0);//

    _addByte(0);//
    _addByte(0);//

}

void	atMacroString::addTextModule()
{
    //byte 1 unknown
    _addByte(static_cast<uint8>(mCounter));
    mCounter++;
    _addByte(0);
    _addByte(1);

    //unknown integer  -1
    _adduint32(0xffffffff);


    //MessageBody
    //stf dir
    mMBdir.convert(BSTRType_ANSI);
    _addString(mMBdir);
    //??
    _adduint32(0);
    //stf text
    mMBstr.convert(BSTRType_ANSI);
    _addString(mMBstr);

    //TU Id
    _adduint64(mTUId);//not a string
    //TU stf  1
    mTTdir.convert(BSTRType_ANSI);
    _addString(mTUdir);
    //empty
    _adduint32(0);//not a string
    //TU stf  2
    //%TT str
    mTTstr.convert(BSTRType_ANSI);
    _addString(mTUstr);

    //%TU custom string
    mTU.convert(BSTRType_Unicode16);
    _addString(mTU);



    //TT Id
    _adduint64(mTTId);

    //%TT dir
    mTTdir.convert(BSTRType_ANSI);
    _addString(mTTdir);
    //empty
    _adduint32(0);
    //%TT str
    mTTstr.convert(BSTRType_ANSI);
    _addString(mTTstr);

    //%TT custom string
    mTT.convert(BSTRType_Unicode16);
    _addString(mTT);


    //TO Id
    _adduint64(mTOId);

    //%TO dir
    mTOdir.convert(BSTRType_ANSI);
    _addString(mTOdir);
    //TO empty
    _adduint32(0);
    //%TO str
    mTOstr.convert(BSTRType_ANSI);
    _addString(mTOstr);

    //%TO custom string
    mTO.convert(BSTRType_Unicode16);
    _addString(mTO);

    _adduint32(mDI); //
    _adduint32(0);
    _addByte(0);

}


BString	atMacroString::assemble()
{

    //make sure we have the right amount of bytes
    uint32 sizeT = mSize;
    _adduint32(0);
    _adduint32(0);

    sizeT = (sizeT <<31);
    sizeT = (sizeT >>31);
    if (sizeT == 1) {
        _addByte(0);
    }

    uint32 tempsize = mSize  >> 1;
    BString mContainer;
    mContainer.convert(BSTRType_Unicode16);
    mContainer.setLength(static_cast<uint16>(tempsize));
    memcpy(mContainer.getRawData(),mPoint,mSize);
    mContainer.setLength(static_cast<uint16>(tempsize));

    //gLogger->hexDump(mContainer.getRawData(),mContainer.getDataLength());
    return(mContainer);


}


