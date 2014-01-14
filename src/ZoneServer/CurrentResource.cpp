 /*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/
#include "CurrentResource.h"
#include "ResourceType.h"
#include "LogManager/LogManager.h"
#include "ConfigManager/ConfigManager.h"


//=============================================================================

CurrentResource::CurrentResource() : Resource()
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

	gLogger->logMsgF("Building DistributionMap for %s(%s)",MSG_LOW,mName.getAnsi(),mType->getName().getAnsi());
	mapBuilder.Build();

	if(gConfig->read<int>("writeResourceMaps"))
	{
		string fileName = (int8*)(gConfig->read<std::string>("ZoneName")).c_str();
		fileName << "_" << mName.getAnsi() << ".bmp";

		gLogger->logMsgF("Writing File %s",MSG_LOW,fileName.getAnsi());

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
		gLogger->logMsg("WARNING: invalid NoisemapBounds set");

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

