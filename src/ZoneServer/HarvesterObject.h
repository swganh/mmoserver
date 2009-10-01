/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_HARVESTER_OBJECT_H
#define ANH_ZONESERVER_HARVESTER_OBJECT_H

#include "PlayerStructure.h"
#include "BuildingEnums.h"
//#include <vector>


//=============================================================================



//=============================================================================

class HarvesterObject :	public PlayerStructure
{
	friend class HarvesterFactory;

	public:

		HarvesterObject();
		~HarvesterObject();

		
		HarvesterFamily	getHarvesterFamily(){ return mHarvesterFamily; }
		void			setHarvesterFamily(HarvesterFamily hf){ mHarvesterFamily = hf; }

		uint32			getLoadCount(){ return mTotalLoadCount; }
		uint32			decLoadCount(){ return (mTotalLoadCount-1); }
		void			setLoadCount(uint32 count){ mTotalLoadCount = count; }

		void			prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount);
		void			handleObjectMenuSelect(uint8 messageType,Object* srcObject);
		void			handleUIEvent(uint32 action,int32 element,string inputStr,UIWindow* window);
		

	private:

		uint32			mResourceHarvested;
		uint32			mHoperSize;
		uint64			mResourceType;
		HarvesterFamily	mHarvesterFamily;

		uint32			mTotalLoadCount;
};

//=============================================================================

#endif