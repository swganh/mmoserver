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

		uint32			getResourceCategory(){ return mResourceCategory; }
		uint32			getUpdateCounter(){ return mHino7UpdateCounter; }
		void			setUpdateCounter(uint32 value){ mHino7UpdateCounter = value; }

		uint64			getCurrentResource(){ return mCurrentResource; }
		void			setCurrentResource(uint64 value){ mCurrentResource = value; }

		bool			getActive(){ return mActive; }
		void			setActive(bool value){ mActive = value; }

		float			getCurrentExtractionRate(){ return mCurrentExtractionRate; }
		void			setCurrentExtractionRate(float value){ mCurrentExtractionRate = value; }

		void			prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount);
		void			handleObjectMenuSelect(uint8 messageType,Object* srcObject);
		//void			handleUIEvent(uint32 action,int32 element,string inputStr,UIWindow* window);
		

	private:

		bool			mActive;

		uint32			mResourceHarvested;
		uint32			mHoperSize;
		uint64			mCurrentResource;
		HarvesterFamily	mHarvesterFamily;

		uint32			mTotalLoadCount;
		float			mCurrentExtractionRate;

		uint32			mResourceCategory;
		uint32			mHino7UpdateCounter;

};

//=============================================================================

#endif