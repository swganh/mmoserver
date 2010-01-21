/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_CURRENTRESOURCE_H
#define ANH_ZONESERVER_CURRENTRESOURCE_H

#include "Utils/typedefs.h"
#include "Resource.h"
#include "ZoneServer/noiseutils.h"
#include <noise.h>
#include <string>


//=============================================================================

class CurrentResource : public Resource
{
	friend class ResourceManager;

	public:

		CurrentResource();
		~CurrentResource();

		void	buildDistributionMap();

		float	getDistribution(int x,int z);

	private:

		void	_verifyNoiseSettings();

		double	mNoiseMapBoundsX1,mNoiseMapBoundsX2;
		double	mNoiseMapBoundsY1,mNoiseMapBoundsY2;
		uint8	mNoiseMapOctaves;
		double	mNoiseMapFrequency;
		double	mNoiseMapPersistence;
		double	mNoiseMapScale;
		double	mNoiseMapBias;
		uint64	mUnitsTotal;
		uint64	mUnitsLeft;

		noise::module::Perlin	mNoiseModule;
		noise::utils::NoiseMap	mResourceDistributionMap;
};

#endif

