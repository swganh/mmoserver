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

#ifndef ANH_ZONESERVER_CURRENTRESOURCE_H
#define ANH_ZONESERVER_CURRENTRESOURCE_H

#include "Utils/typedefs.h"
#include "Resource.h"
#include "ZoneServer/noiseutils.h"
#include <noise/noise.h>
#include <string>


//=============================================================================

class CurrentResource : public Resource
{
    friend class ResourceManager;

public:

    CurrentResource(bool writeResourceMaps, std::string zoneName);
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
	bool					mWriteResourceMaps;
	std::string				mZoneName;
};

#endif

