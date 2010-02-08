 /*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_TANGIBLE_RESOURCECONTAINER_H
#define ANH_ZONESERVER_TANGIBLE_RESOURCECONTAINER_H

#include "Utils/typedefs.h"
#include "TangibleObject.h"

class Resource;
class PlayerObject;

//=============================================================================

class ResourceContainer : public TangibleObject
{
	friend class ObjectFactory;
	friend class ResourceContainerFactory;

	public:

		ResourceContainer();
		~ResourceContainer();

		virtual void	setParentIdIncDB(uint64 parentId);
	

		uint64		getResourceId();
		void		setResourceId(uint64 id);
		Resource*	getResource();
		void		setResource(Resource* resource);
		uint32		getAmount();
		void		setAmount(uint32 amount);
		uint32		getMaxAmount();
		void		setMaxAmount(uint32 max);

		void		sendAttributes(PlayerObject* playerObject);
		string		getBazaarName();
		string		getBazaarTang();
		uint32		getCategoryBazaar();

	private:

		uint64		mResourceId;
		uint32		mAmount;
		uint32		mMaxAmount;
		Resource*	mResource;
};

//=============================================================================

#endif

