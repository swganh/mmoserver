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

#ifndef ANH_ZONESERVER_SPAWNREGION_H
#define ANH_ZONESERVER_SPAWNREGION_H

#include "RegionObject.h"
#include "MathLib/Rectangle.h"
#include "Utils/typedefs.h"

//=============================================================================

class SpatialIndexManager;
class PlayerObject;

//=============================================================================

class SpawnRegion : public RegionObject
{
	friend class SpawnRegionFactory;
	friend class MissionSpawnRegionFactory;

public:

	SpawnRegion();
	virtual ~SpawnRegion();

    uint32			getSpawnType() {
        return mSpawnType;
    }
    void			setSpawnType(uint32 type) {
        mSpawnType = type;
    }
    uint32 getMission() { return mMission; }
    void    setMission(uint32 mission) { mMission = mission;  }

    bool			isMission() {
        return (mMission != 0);
    }

	virtual void	update();
	virtual void	onObjectEnter(Object* object);
	virtual void	onObjectLeave(Object* object);

protected:
	Anh_Math::Rectangle mQueryRect;

	SpatialIndexManager*	mSIM;
	uint32					mMission;
	uint32					mSpawnType;
};


#endif



