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

#ifndef ANH_ZONESERVER_HARVESTER_OBJECT_H
#define ANH_ZONESERVER_HARVESTER_OBJECT_H

#include "PlayerStructure.h"
#include "BuildingEnums.h"

//=============================================================================

typedef std::pair<uint64,float>					HarvesterResourcePair;
typedef std::vector<HarvesterResourcePair>		HResourceList;


//=============================================================================

struct HarvesterHopperItem
{
    uint64			HarvesterID;
    uint64			ResourceID;
    float			Quantity;
};

class HarvesterObject :	public PlayerStructure, public DatabaseCallback
{
    friend class HarvesterFactory;

public:

    HarvesterObject();
    ~HarvesterObject();

    virtual void	handleDatabaseJobComplete(void* ref,DatabaseResult* result);

    HarvesterFamily	getHarvesterFamily() {
        return mHarvesterFamily;
    }
    void			setHarvesterFamily(HarvesterFamily hf) {
        mHarvesterFamily = hf;
    }

    uint32			getLoadCount() {
        return mTotalLoadCount;
    }
    uint32			decLoadCount() {
        return (mTotalLoadCount-1);
    }
    void			setLoadCount(uint32 count) {
        mTotalLoadCount = count;
    }

    uint32			getResourceCategory() {
        return mResourceCategory;
    }
    uint32			getUpdateCounter() {
        return mHino7UpdateCounter;
    }
    void			setUpdateCounter(uint32 value) {
        mHino7UpdateCounter = value;
    }

    uint64			getCurrentResource() {
        return mCurrentResource;
    }
    void			setCurrentResource(uint64 value) {
        mCurrentResource = value;
    }

    bool			getActive() {
        return mActive;
    }
    void			setActive(bool value) {
        mActive = value;
    }

    float			getCurrentExtractionRate() {
        return mCurrentExtractionRate;
    }
    void			setCurrentExtractionRate(float value) {
        mCurrentExtractionRate = value;
    }

    void			createResourceContainer(uint64 id, PlayerObject* player, uint32 amount);

    void			prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount);
    void			handleObjectMenuSelect(uint8 messageType,Object* srcObject);

    float			getSpecExtraction();
    float			getHopperSize();
    float			getCurrentHopperSize();

    HResourceList*	getResourceList() {
        return &mResourceList;
    }
    uint32			getRListUpdateCounter() {
        return mRListUpdateCounter;
    }
    void			setRListUpdateCounter(uint32 value) {
        mRListUpdateCounter = value;
    }
    bool			checkResourceList(uint64 id);


private:

    bool			mActive;

    uint32			mResourceHarvested;
    uint32			mHoperSize;
    uint64			mCurrentResource;
    HarvesterFamily	mHarvesterFamily;

    uint32			mTotalLoadCount;
    float			mCurrentExtractionRate;

    uint32			mResourceCategory;
    uint32			mHino7UpdateCounter;


    HResourceList	mResourceList;
    uint32			mRListUpdateCounter;

};

//=============================================================================

#endif
