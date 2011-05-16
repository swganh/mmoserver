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

#ifndef ANH_ZONESERVER_HOUSE_OBJECT_H
#define ANH_ZONESERVER_HOUSE_OBJECT_H

#include "BuildingObject.h"
#include "BuildingEnums.h"
#include "ObjectFactory.h"


class HouseObject :	public BuildingObject, public DatabaseCallback
{
    friend class HouseFactory;
    friend class BuildingFactory;

public:

    HouseObject();
    ~HouseObject();

    virtual void	handleDatabaseJobComplete(void* ref,DatabaseResult* result);

    virtual void	handleObjectReady(Object* object,DispatchClient* client, uint64 hopper);

    HouseFamily	getHouseFamily() {
        return mHouseFamily;
    }
    void			setHouseFamily(HouseFamily ff) {
        mHouseFamily = ff;
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

    void			prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount);
    void			handleObjectMenuSelect(uint8 messageType,Object* srcObject);

    PlayerStructure*	getSign() {
        return mSign;
    }
    void				setSign(PlayerStructure* sign) {
        mSign = sign;
    }

    bool			hasAdmin(uint64 id);

    void			checkCellPermission(PlayerObject* player);


private:

    BuildingFamily	mBuildingFamily;
    HouseFamily		mHouseFamily;

    uint32			mTotalLoadCount;

    PlayerStructure* mSign;

};

//=============================================================================

#endif
