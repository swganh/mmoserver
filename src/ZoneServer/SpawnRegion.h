/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_SPAWNREGION_H
#define ANH_ZONESERVER_SPAWNREGION_H

#include "RegionObject.h"
#include "MathLib/Rectangle.h"
#include "Utils/typedefs.h"

//=============================================================================

class ZoneTree;
class PlayerObject;
class QTRegion;

//=============================================================================

class SpawnRegion : public RegionObject
{
	friend class SpawnRegionFactory;
	friend class MissionSpawnRegionFactory;

	public:

		SpawnRegion();
		virtual ~SpawnRegion();

		uint32			getSpawnType(){ return mSpawnType; }
		void			setSpawnType(uint32 type){ mSpawnType = type; }
		bool			isMission(){return (mMission != 0);}

		virtual void	update();
		virtual void	onObjectEnter(Object* object);
		virtual void	onObjectLeave(Object* object);

	protected:

		Anh_Math::Rectangle mQueryRect;
		QTRegion*			mQTRegion;
		ZoneTree*			mSI;
		uint32				mMission;
		uint32				mSpawnType;
};


#endif



