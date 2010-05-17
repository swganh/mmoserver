/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/
#include "Container.h"
#include "ResourceContainer.h"
#include "WorldManager.h"


//=============================================================================

Container::Container() : TangibleObject(), mParent(NULL)
{
	mTanGroup = TanGroup_Container;
	mObjectLoadCounter = 1000;
	//mObjects.reserve(80);
}

//=============================================================================

Container::~Container()
{
	ObjectList::iterator	containerObjectIt = mObjects.begin();

	while(containerObjectIt != mObjects.end())
	{
		// Can we have busy crafting tool in a container??? 
		// in theory yes,albeit only shortly

		gWorldManager->destroyObject((*containerObjectIt));
		
		containerObjectIt = mObjects.erase(containerObjectIt);
	}


}

//=============================================================================

void Container::removeObject(Object* object)
{
	ObjectList::iterator it = mObjects.begin();
	while(it != mObjects.end())
	{
		if((*it)->getId() == object->getId())
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
			gWorldManager->destroyObject(object);
			mObjects.erase(it);
			break;
		}
		++it;
	}
}

//=============================================================================
