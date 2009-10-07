/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "ResourceCategory.h"
#include "Resource.h"


//=============================================================================

ResourceCategory::ResourceCategory()
{
	mChildren.reserve(5);
	mSiblings.reserve(5);
}

//=============================================================================

ResourceCategory::~ResourceCategory()
{
}

//=============================================================================

void ResourceCategory::getResources(ResourceList& resourceList,bool currentOnly)
{
	ResourceList::iterator resourceIt = mSiblings.begin();
	while(resourceIt != mSiblings.end())
	{
		if(currentOnly)
		{
			if(((*resourceIt)->getCurrent()) != 0)
				resourceList.push_back(*resourceIt);
		}
		else
			resourceList.push_back(*resourceIt);

		++resourceIt;
	}

	ResourceCategoryList::iterator rescatIt = mChildren.begin();
	while(rescatIt != mChildren.end())
	{
		ResourceCategory* tmpCat = (*rescatIt);
		tmpCat->getResources(resourceList,currentOnly);

		++rescatIt;
	}
}

//=============================================================================

