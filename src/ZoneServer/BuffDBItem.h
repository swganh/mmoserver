/*
 This file is part of MMOServer. For more information, visit http://swganh.com
 
 Copyright (c) 2006 - 2011 The SWG:ANH Team

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#pragma once

#ifndef ANH_BUFFDBITEM_H
#define ANH_BUFFDBITEM_H

#include "Utils/bstring.h"
#include "Utils/typedefs.h"

class BuffDBItem
{
public:
    BuffDBItem(void);
    ~BuffDBItem(void);
    uint64 mBuffId;
    uint64 mTargetId;
    uint64 mInstigatorId;
    uint64 mMaxTicks;
    uint64 mTickLength;
    uint64 mCurrentTick;
    uint32 mIconCRC;
    uint64 mPausedGlobalTick;
    uint64 mStartGlobalTick;
    BString mName;
};
class BuffAttributeDBItem
{
public:

    BuffAttributeDBItem(void) {
        ;
    }
    ~BuffAttributeDBItem(void) {
        ;
    }
    int32 mType, mInitialValue, mTickValue, mFinalValue;
};


#endif // ANH_BUFFDBITEM_H
