/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "MathLib/Quaternion.h"
#include "Container.h"
#include "WorldManager.h"
#include "ResourceContainer.h"


//=============================================================================

Container::Container() : TangibleObject(), mParent(NULL)
{
	// gLogger->logMsg("Container::Container()");

	mTanGroup = TanGroup_Container;
	mObjectLoadCounter = 1000;
	mObjects.reserve(80);
}

//=============================================================================

Container::~Container()
{
	ObjectList::iterator	containerObjectIt = mObjects.begin();

	while(containerObjectIt != mObjects.end())
	{
		Object* object = (*containerObjectIt);

		// Can we have busy crafting tool in a container??? 
		// in theory yes,albeit only shortly

		gWorldManager->destroyObject(object);
		
		containerObjectIt = mObjects.erase(containerObjectIt);
	}


}

//=============================================================================

void Container::removeObject(Object* object)
{
	ObjectList::iterator it = mObjects.begin();
	while(it != mObjects.end())
	{
		if((*it) == object)
		{
			mObjects.erase(it);
			break;
		}
		++it;
	}
}

//=============================================================================

void Container::deleteObject(Object* object)
{
	ObjectList::iterator it = mObjects.begin();
	while(it != mObjects.end())
	{
		if((*it) == object)
		{
			delete(*it);
			mObjects.erase(it);
			break;
		}
		++it;
	}
}

//=============================================================================
