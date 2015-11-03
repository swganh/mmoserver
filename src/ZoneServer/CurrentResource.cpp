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
#include "CurrentResource.h"
#include "ResourceType.h"




#include "Utils/logger.h"


//=============================================================================

CurrentResource::CurrentResource(bool writeResourceMaps, std::string zoneName) 
	: Resource()
	, mWriteResourceMaps(writeResourceMaps)
	, mZoneName(zoneName)
{
}

//=============================================================================

CurrentResource::~CurrentResource()
{
}

//=============================================================================

float CurrentResource::getDistribution(int x,int z)
{
    // translates to 1:32
    return mResourceDistributionMap.GetValue((x >> 6),(z >> 6));
}

//=============================================================================

void CurrentResource::buildDistributionMap()
{
    noise::utils::NoiseMapBuilderPlane	mapBuilder;
    noise::module::ScaleBias			flattenModule;

    flattenModule.SetSourceModule(0,mNoiseModule);
    flattenModule.SetScale(mNoiseMapScale);
    flattenModule.SetBias(mNoiseMapBias);

    _verifyNoiseSettings();

    mNoiseModule.SetPersistence(mNoiseMapPersistence);
    mNoiseModule.SetOctaveCount(mNoiseMapOctaves);
    mNoiseModule.SetFrequency(mNoiseMapFrequency);

    mapBuilder.SetSourceModule(flattenModule);
    mapBuilder.SetDestNoiseMap(mResourceDistributionMap);

    // translates to 1:32
    mapBuilder.SetDestSize(512,512);
    mapBuilder.SetBounds(mNoiseMapBoundsX1,mNoiseMapBoundsX2,mNoiseMapBoundsY1,mNoiseMapBoundsY2);

    LOG(INFO) << "Building DistributionMap for " << mName.getAnsi() << " " << mType->getName().getAnsi();
    mapBuilder.Build();

    if(mWriteResourceMaps)
    {
        BString fileName = (int8*)(mZoneName).c_str();
        fileName << "_" << mName.getAnsi() << ".bmp";

        LOG(INFO) << "Writing File " << fileName.getAnsi();

        noise::utils::RendererImage renderer;
        noise::utils::Image image;
        renderer.SetSourceNoiseMap(mResourceDistributionMap);
        renderer.SetDestImage(image);

        renderer.ClearGradient();
        renderer.AddGradientPoint(-1.0000,noise::utils::Color(0,0,0,255));
        renderer.AddGradientPoint(-0.9999,noise::utils::Color(0,0,0,255));
        renderer.AddGradientPoint(0.0000,noise::utils::Color(255,255,0,255));
        renderer.AddGradientPoint(1.0000,noise::utils::Color(255,0,0,255));

        renderer.EnableLight();
        renderer.SetLightContrast(1.5);
        renderer.SetLightBrightness(2.0);
        renderer.Render();

        noise::utils::WriterBMP writer;
        writer.SetSourceImage(image);
        writer.SetDestFilename(fileName.getAnsi());
        writer.WriteDestFile();
    }
}

//=============================================================================

void CurrentResource::_verifyNoiseSettings()
{
    if(mNoiseMapBoundsX1 < 0.0 || mNoiseMapBoundsX1 > 24.9)
        mNoiseMapBoundsX1 = 1.0;

    if(mNoiseMapBoundsX2 < 0.1 || mNoiseMapBoundsX2 > 25.0)
        mNoiseMapBoundsX2 = 3.0;

    if(mNoiseMapBoundsY1 < 0.0 || mNoiseMapBoundsY1 > 24.9)
        mNoiseMapBoundsY1 = 1.0;

    if(mNoiseMapBoundsX1 < 0.0 || mNoiseMapBoundsX1 > 25.0)
        mNoiseMapBoundsX1 = 3.0;

    if(mNoiseMapBoundsX1 >= mNoiseMapBoundsX2 || mNoiseMapBoundsY1 >= mNoiseMapBoundsY2)
        LOG(WARNING) << "NoiseMap: Invalid NoisemapBounds set!";

    if(mNoiseMapOctaves < 1 || mNoiseMapOctaves > 6)
        mNoiseMapOctaves = 4;

    if(mNoiseMapFrequency < 1.0 || mNoiseMapFrequency > 32.0)
        mNoiseMapFrequency = 6.0;

    if(mNoiseMapPersistence < 0.0 || mNoiseMapPersistence > 1.0)
        mNoiseMapPersistence = 0.2;

    if(mNoiseMapScale < 0 || mNoiseMapScale > 1)
        mNoiseMapScale = 1;

    if(mNoiseMapBias < - 1 || mNoiseMapBias > 1)
        mNoiseMapBias = 0;
}

//=============================================================================

