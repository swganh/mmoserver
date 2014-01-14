#include "ObjectController.h"
#include "HeightmapAsyncContainer.h"

void ObjectController::heightMapCallback(HeightmapAsyncContainer* ref)
{
	switch(ref->type)
	{
	case HeightmapCallback_ArtisanSurvey:
	{
		HeightmapArtisanHandler(ref);
		break;
	}

	case HeightmapCallback_StructureHarvester:
	case HeightmapCallback_StructureFactory:
	case HeightmapCallback_StructureHouse:
	{
		HeightmapStructureHandler(ref);
		break;
	}

	}

	delete ref;
}