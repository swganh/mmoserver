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

#ifndef ANH_ZONESERVER_BADGE_H
#define ANH_ZONESERVER_BADGE_H

#include "Utils/typedefs.h"


//======================================================================================================================

class Badge
{
    friend class CharSheetManager;

public:

    Badge() {}
    ~Badge() {}

    uint32	getId() {
        return mId;
    }
    void	setId(uint32 id) {
        mId = id;
    }

    BString	getName() {
        return mName;
    }
    void	setName(const BString name) {
        mName = name;
    }

    uint32	getSoundId() {
        return mSoundId;
    }
    void	setSoundId(uint32 sound) {
        mSoundId = sound;
    }

    uint8	getCategory() {
        return mCategory;
    }
    void	setCategory(uint8 category) {
        mCategory = category;
    }

private:

    uint32	mId;
    BString	mName;
    uint32	mSoundId;
    uint8	mCategory;
};

#endif

