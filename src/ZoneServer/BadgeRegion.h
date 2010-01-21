/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_BADGEREGION_H
#define ANH_ZONESERVER_BADGEREGION_H

#include "RegionObject.h"
#include "MathLib/Rectangle.h"
#include "Utils/typedefs.h"

//=============================================================================

class ZoneTree;
class PlayerObject;
class QTRegion;

//=============================================================================

class BadgeRegion : public RegionObject
{
	friend class BadgeRegionFactory;

	public:

		BadgeRegion();
		virtual ~BadgeRegion();

		uint32			getBadgeId(){ return mBadgeId; }
		void			setBadgeId(uint32 id){ mBadgeId = id; }

		virtual void	update();
		virtual void	onObjectEnter(Object* object);
		virtual void	onObjectLeave(Object* object);

	protected:

		uint32				mBadgeId;
		ZoneTree*			mSI;
		QTRegion*			mQTRegion;
		Anh_Math::Rectangle mQueryRect;
};


#endif



