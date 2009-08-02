 /*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_TANGIBLE_RESOURCECONTAINER_H
#define ANH_ZONESERVER_TANGIBLE_RESOURCECONTAINER_H

#include "TangibleObject.h"
#include "Resource.h"

//=============================================================================

class ResourceContainer : public TangibleObject
{
	friend class ObjectFactory;
	friend class ResourceContainerFactory;

	public:

		ResourceContainer();
		~ResourceContainer();

		uint64		getResourceId(){ return mResourceId; }
		void		setResourceId(uint64 id){ mResourceId = id; }
		Resource*	getResource(){ return mResource; }
		void		setResource(Resource* resource){ mResource = resource; }
		uint32		getAmount(){ return mAmount; }
		void		setAmount(uint32 amount){ mAmount = amount; }
		uint32		getMaxAmount(){ return mMaxAmount; }
		void		setMaxAmount(uint32 max){ mMaxAmount = max; }

		void		sendAttributes(PlayerObject* playerObject);
		string		getBazaarName();
		string		getBazaarTang(){ return getModelString(); }
		uint32		getCategoryBazaar(){ return getResource()->getType()->getCategoryBazaar(); }

	private:

		uint64		mResourceId;
		uint32		mAmount;
		uint32		mMaxAmount;
		Resource*	mResource;
};

//=============================================================================

#endif

