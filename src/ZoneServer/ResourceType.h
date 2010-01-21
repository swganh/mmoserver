/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_RESOURCETYPE_H
#define ANH_ZONESERVER_RESOURCETYPE_H

#include "Utils/typedefs.h"


//=============================================================================

class ResourceType
{
	friend class ResourceManager;

	public:

		ResourceType();
		~ResourceType();

		uint32	getId(){ return mId; }
		void	setId(uint32 id){ mId = id; }
		string	getName(){ return mName; }
		void	setName(const string name){ mName = name; }
		string	getDescriptor(){ return mDescriptor; }
		void	setDescriptor(const string desc){ mDescriptor = desc; }
		uint16	getCategoryId(){ return mCatId; }
		void	setCategoryId(uint16 cat){ mCatId = cat; }
		string	getModel(){ return mModel; }
		void	setModel(const string model){ mModel = model; }
		string	getContainerModel();
		string	getTypeName(){ return mTypeName; }
		void	setTypeName(const string name){ mTypeName = name; }
		uint32	getCategoryBazaar(){ return mCategoryBazaar; }
		void	setCategoryBazaar(uint32 cat) { mCategoryBazaar = cat; }
		string	getResourceType() { return mResourceType; }
		void	setResourceType(const string type) { mResourceType = type; }

	private:

		uint32	mId;
		string	mName;
		string	mTypeName;
		string	mDescriptor;
		string	mModel;
		uint16	mCatId;
		uint32  mCategoryBazaar;
		string	mResourceType;
};

//=============================================================================

#endif

