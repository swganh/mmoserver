/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_WEATHER_H
#define ANH_ZONESERVER_WEATHER_H

#include "Utils/typedefs.h"
#include "MathLib/Vector3.h"


//======================================================================================================================

class Weather
{
	public:

		Weather() : mWeather(0) {}
		~Weather(){}

		uint32    mWeather;
        glm::vec3 mClouds;
};

//======================================================================================================================

#endif

