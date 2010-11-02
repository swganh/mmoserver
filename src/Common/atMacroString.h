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

#pragma once

#include "Utils/typedefs.h"
#include "Utils/bstring.h"

class atMacroString
{
public:
    atMacroString();
    ~atMacroString();
    void			addTO(BString TO) {
        mTO = TO;
    }
    void			addTOId(uint64 toid) {
        mTOId = toid;
    }
    void			addTOstf(BString dir, BString str) {
        mTOdir = dir;
        mTOstr = str;
    }

    void			addDI(uint32 di) {
        mDI = di;
    }

    void			addTT(BString tt) {
        mTT = tt;
    }
    void			addTTId(uint64 ttid) {
        mTTId = ttid;
    }
    void			addTTstf(BString dir, BString str) {
        mTTdir = dir;
        mTTstr = str;
    }

    void			addTU(BString tu) {
        mTU = tu;
    }
    void			addTUId(uint64 tuid) {
        mTUId = tuid;
    }
    void			addTUstf(BString dir, BString str) {
        mTUdir = dir;
        mTUstr = str;
    }

    void			addMB(BString mb) {
        mMB = mb;
    }
    void			addMBstf(BString dir, BString str) {
        mMBdir = dir;
        mMBstr = str;
    }

    BString			assemble();
    void			addTextModule();

    void			setPlanetString(BString planet);
    void			addWaypoint();

    void			setWP(float x, float y, float z, BString name) {
        mX = x;
        mY = y;
        mZ = z;
        mWPName = name;
    }


private:

    void			_addByte(uint8 mByte);
    void			_addString(BString mString);
    void			_adduint16(uint16 mWord);
    void			_adduint32(uint32 mInteger);
    void			_adduint64(uint64 mLong);
    void			_addfloat(float mFloat);


    bool	mAssembled;

    uint64	mTOId;
    BString	mTO;
    BString	mTOdir;
    BString	mTOstr;

    uint64	mTTId;
    BString	mTT;
    BString	mTTdir;
    BString	mTTstr;


    uint64	mTUId;
    BString	mTU;
    BString	mTUdir;
    BString	mTUstr;

    BString	mMB;
    BString	mMBdir;
    BString	mMBstr;

    float		mX;
    float		mY;
    float		mZ;
    uint32		mPlanetCRC;
    BString		mWPName;

    uint8 Hallo[5000];
    uint8* mPoint;
    uint8* mPoint1;
    uint32 mSize;
    uint32 mDI;
    uint16 mCounter;
};
