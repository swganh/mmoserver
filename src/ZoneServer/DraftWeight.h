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

#ifndef ANH_ZONESERVER_DRAFTWEIGHT_H
#define ANH_ZONESERVER_DRAFTWEIGHT_H

#include "Utils/typedefs.h"


//=============================================================================

class DraftWeight
{
    friend class SchematicManager;

public:

    DraftWeight();
    ~DraftWeight();

    uint8	getDataType() {
        return mDataType;
    }
    void	setDataType(uint8 type) {
        mDataType = type;
    }
    uint8	getDistribution() {
        return mDistribution;
    }
    void	setDistribution(uint8 dist) {
        mDistribution = dist;
    }

    void	prepareData();
    uint8	getData() {
        return mData;
    }
    uint32  getSchemWeightBatch() {
        return mSchemWeightBatch;
    }
    uint32  getBatchListId() {
        return mBatchListId;
    }

private:
    uint32  mSchemWeightBatch;
    uint8	mDataType;
    uint8	mDistribution;
    uint8	mData;
    uint32  mBatchListId;
};

#endif


