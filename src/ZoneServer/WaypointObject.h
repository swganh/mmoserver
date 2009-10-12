/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_WAYPOINT_OBJECT_H
#define ANH_ZONESERVER_WAYPOINT_OBJECT_H

#include "Object.h"

//=============================================================================

enum waypoint_types
{
	Waypoint_blue	= 1,
	Waypoint_green	= 2,
	Waypoint_orange	= 3,
	Waypoint_yellow	= 4,
	Waypoint_red	= 5,
	Waypoint_white	= 6,
	Waypoint_JTL	= 7,
};


//=============================================================================

class WaypointObject : public Object
{
	friend class WaypointFactory;

	public:

		WaypointObject();
		~WaypointObject();

		bool				getActive(){ return mActive; }
		void				setActive(bool active){ mActive = active; }
		void				toggleActive(){ mActive = !mActive; };
		string				getName(){ return mName; }
		void				setName(const string name){ mName = name; }
		uint8				getWPType(){ return mWPType; }
		void				setWPType(uint8 type){ mWPType = type; }

		void				setCoords(Anh_Math::Vector3 coords){ mCoords = coords; }
		Anh_Math::Vector3	getCoords(){ return mCoords; }

		uint32				getPlanetCRC() { return mPlanetCRC; }
		void				setPlanetCRC(uint32 planet_crc) { mPlanetCRC = planet_crc; }

	private:

		Anh_Math::Vector3	mCoords;
		bool				mActive;
		string				mName;
		uint8				mWPType;
		uint32				mPlanetCRC;
};

//=============================================================================

#endif

