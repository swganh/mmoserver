#include "NoBuildRegion.h"


NoBuildRegion::NoBuildRegion(void)
{
}


NoBuildRegion::~NoBuildRegion(void)
{
}

NoBuildPlanetList*	NoBuildRegion::getNoBuildPlanetList(uint32 planet)
{
	NoBuildPlanetList* nbPL = mNoBuildPlanetList;
	NoBuildPlanetList::iterator it = nbPL->begin();
	while(it != mNoBuildPlanetList->end())
	{
		// check if we're in the planet we're requesting
		//if ((*it)->)
	}
}