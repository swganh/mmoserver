#ifndef HEIGHTMAPASYNCCONTAINER_H
#define HEIGHTMAPASYNCCONTAINER_H
#include <map>
#include "HeightMapCallback.h"

class heightResult
{
public:
	float height;
	bool hasWater;
};

typedef std::map<std::pair<float, float>, heightResult*> HeightResultMap;

enum HeightmapCallbackTypes
{
	HeightmapCallback_ArtisanSurvey,
	HeightmapCallback_StructureHarvester,
	HeightmapCallback_StructureFactory,
	HeightmapCallback_StructureHouse
};

class HeightmapAsyncContainer
{
public:
	HeightmapAsyncContainer(HeightMapCallBack* backCall, HeightmapCallbackTypes Type)
	: callback(backCall)
	, type(Type)
	{
		mHeightResults = new HeightResultMap();
	}

	~HeightmapAsyncContainer()
	{
		for(HeightResultMap::iterator it= mHeightResults->begin(); it != mHeightResults->end(); it++)
			delete it->second;

		delete mHeightResults;
	}
	
	void addToBatch(float x, float y) { mHeightResults->insert(std::make_pair(std::make_pair(x,y), reinterpret_cast<heightResult*>(0)));}
	HeightResultMap* getResults() {return mHeightResults;}
	HeightMapCallBack* getCallback() { return callback;}
	HeightmapCallbackTypes type;

private:
	HeightResultMap* mHeightResults;
	HeightMapCallBack* callback;

};
#endif HEIGHTMAPASYNCCONTAINER_H