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

#ifndef ANH_ZONESERVER_RESOURCECATEGORY_H
#define ANH_ZONESERVER_RESOURCECATEGORY_H

#include "Utils/bstring.h"
#include "Utils/typedefs.h"
#include <list>

//=============================================================================

class Resource;
class ResourceCategory;

typedef std::list<ResourceCategory*>	ResourceCategoryList;
typedef std::list<Resource*>			ResourceList;

//=============================================================================

class ResourceCategory
{
    friend class ResourceManager;

public:

    ResourceCategory();
    ~ResourceCategory();

    uint32					getId() {
        return mId;
    }
    void					setId(uint32 id) {
        mId = id;
    }
    uint32					getParentId() {
        return mParentId;
    }
    void					setParentId(uint32 id) {
        mParentId = id;
    }
    ResourceCategoryList*	getChildren() {
        return &mChildren;
    }
    ResourceList*			getSiblings() {
        return &mSiblings;
    }
    BString					getName() {
        return mName;
    }
    void					setName(const BString name) {
        mName = name;
    }
    BString					getDescriptor() {
        return mDescriptor;
    }
    void					setDescriptor(const BString desc) {
        mDescriptor = desc;
    }
    void					insertCategory(ResourceCategory* cat) {
        mChildren.push_back(cat);
    }
    void					insertResource(Resource* res) {
        mSiblings.push_back(res);
    }
    void					removeResource(Resource* res) {
        mSiblings.remove(res);
    }
    void					getResources(ResourceList& resourceList,bool currentOnly);

private:

    uint32					mId;
    uint32					mParentId;
    BString					mName;
    BString					mDescriptor;
    ResourceCategoryList	mChildren;
    ResourceList			mSiblings;
};

//=============================================================================

#endif

