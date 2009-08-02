/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_CITY_H
#define ANH_ZONESERVER_CITY_H

#include "Utils/typedefs.h"
#include "Mathlib/Rectangle.h"
#include "RegionObject.h"


//=============================================================================

class ZoneTree;
class PlayerObject;
class QTRegion;

//=============================================================================

class City : public RegionObject
{
	friend class CityFactory;

	public:

		City();
		virtual ~City();

		string			getCityName(){ return mCityName; }
		void			setCityName(const string cityName){ mCityName = cityName; }

		virtual void	update();
		virtual void	onObjectEnter(Object* object);
		virtual void	onObjectLeave(Object* object);

	protected:

		string				mCityName;
		ZoneTree*			mSI;
		QTRegion*			mQTRegion;
		Anh_Math::Rectangle mQueryRect;
};


#endif



