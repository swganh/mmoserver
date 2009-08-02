/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_RESOURCECATEGORY_H
#define ANH_ZONESERVER_RESOURCECATEGORY_H

#include "Utils/typedefs.h"
#include "Resource.h"
#include <vector>

//=============================================================================

class ResourceCategory;

typedef std::vector<ResourceCategory*>	ResourceCategoryList;
typedef std::vector<Resource*>			ResourceList;

//=============================================================================

class ResourceCategory
{
	friend class ResourceManager;

	public:

		ResourceCategory();
		~ResourceCategory();

		uint32					getId(){ return mId; }
		void					setId(uint32 id){ mId = id; }
		uint32					getParentId(){ return mParentId; }
		void					setParentId(uint32 id){ mParentId = id; }
		ResourceCategoryList*	getChildren(){ return &mChildren; }
		ResourceList*			getSiblings(){ return &mSiblings; }
		string					getName(){ return mName; }
		void					setName(const string name){ mName = name; }
		string					getDescriptor(){ return mDescriptor; }
		void					setDescriptor(const string desc){ mDescriptor = desc; }
		void					insertCategory(ResourceCategory* cat){ mChildren.push_back(cat); }
		void					insertResource(Resource* res){ mSiblings.push_back(res); }
		void					getResources(ResourceList& resourceList,bool currentOnly);

	private:

		uint32					mId;
		uint32					mParentId;
		string					mName;
		string					mDescriptor;
		ResourceCategoryList	mChildren;
		ResourceList			mSiblings;
};

//=============================================================================

#endif

