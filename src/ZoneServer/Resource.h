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

#ifndef ANH_ZONESERVER_RESOURCE_H
#define ANH_ZONESERVER_RESOURCE_H

#include "Utils/bstring.h"
#include "Utils/typedefs.h"

class ResourceType;

//=============================================================================

enum ResourceAttribute
{
    ResAttr_ER	= 0,
    ResAttr_CR	= 1,
    ResAttr_CD	= 2,
    ResAttr_DR	= 3,
    ResAttr_FL	= 4,
    ResAttr_HR	= 5,
    ResAttr_MA	= 6,
    ResAttr_PE	= 7,
    ResAttr_OQ	= 8,
    ResAttr_SR	= 9,
    ResAttr_UT	= 10

};

class PlayerObject;

//=============================================================================

class Resource
{
    friend class ResourceManager;

public:

    Resource();
    ~Resource();

    uint64			getId() {
        return mId;
    }
    void			setId(uint64 id) {
        mId = id;
    }
    BString			getName() {
        return mName;
    }
    void			setName(const BString name) {
        mName = name;
    }
    uint32			getTypeId() {
        return mTypeId;
    }
    uint16			getAttribute(uint8 nr) {
        return mAttributes[nr];
    }
    void			setAttribute(uint8 nr,uint16 value) {
        mAttributes[nr] = value;
    }
    uint8			getCurrent() {
        return mCurrent;
    }
    void			setCurrent(uint8 c) {
        mCurrent = c;
    }
    ResourceType*	getType() {
        return mType;
    }
    void			setType(ResourceType* type) {
        mType = type;
    }
    void			sendAttributes(PlayerObject* playerObject);

protected:

    uint64			mId;
    BString			mName;
    uint32			mTypeId;
    uint16			mAttributes[11];
    uint8			mCurrent;
    ResourceType*	mType;
};

#endif

