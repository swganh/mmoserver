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

#include "ResourceCategory.h"
#include "Resource.h"


//=============================================================================

ResourceCategory::ResourceCategory()
{
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

