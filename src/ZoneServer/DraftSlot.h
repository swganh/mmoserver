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

#ifndef ANH_ZONESERVER_DRAFTSLOT_H
#define ANH_ZONESERVER_DRAFTSLOT_H

#include "Utils/bstring.h"
#include "Utils/typedefs.h"


//=============================================================================

enum DSType
{
    DST_Empty				= 0,
    DST_IdentComponent		= 2,
    DST_Resource			= 4,
    DST_SimiliarComponent	= 5
};

class DraftSlot
{
    friend class SchematicManager;

public:

    DraftSlot();
    ~DraftSlot();

    BString	getComponentName() {
        return mName;
    }
    void	setComponentName(BString name) {
        mName = name;
    }
    BString	getComponentFile() {
        return mFile;
    }
    void	setComponentFile(BString file) {
        mFile = file;
    }
    BString	getResourceName() {
        return mResourceName;
    }
    void	setResourceName(BString res) {
        mResourceName = res;
    }

    //the amount necessary to fill the slot
    uint32	getNecessaryAmount() {
        return mAmount;
    }
    void	setNecessaryAmount(uint32 amount) {
        mAmount = amount;
    }

    //the type of res / component to fill
    uint8	getType() {
        return mType;
    }
    void	setType(uint8 type) {
        mType = type;
    }

    uint8	getOptional() {
        return mOptional;
    }
    void	setOptional(uint8 optional) {
        mOptional = optional;
    }

    uint32  getSchemWeightBatch() {
        return mSchemWeightBatch;
    }
private:

    BString	mName;
    BString	mFile;
    BString	mResourceName;
    uint32	mAmount;
    uint32  mSchemWeightBatch;

    uint8	mType;
    uint8	mOptional;
};

#endif


