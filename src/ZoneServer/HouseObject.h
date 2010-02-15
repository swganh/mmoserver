/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_HOUSE_OBJECT_H
#define ANH_ZONESERVER_HOUSE_OBJECT_H

#include "PlayerStructure.h"
#include "BuildingEnums.h"
#include "ObjectFactory.h"

//=============================================================================



//=============================================================================


class HouseObject :	public PlayerStructure, public DatabaseCallback
{
	friend class HouseFactory;

	public:

		HouseObject();
		~HouseObject();

		virtual void	handleDatabaseJobComplete(void* ref,DatabaseResult* result);
		
		virtual void	handleObjectReady(Object* object,DispatchClient* client, uint64 hopper);

		HouseFamily	getHouseFamily(){ return mHouseFamily; }
		void			setHouseFamily(HouseFamily ff){ mHouseFamily = ff; }

		uint32			getLoadCount(){ return mTotalLoadCount; }
		uint32			decLoadCount(){ return (mTotalLoadCount-1); }
		void			setLoadCount(uint32 count){ mTotalLoadCount = count; }

		bool			getPublic(){ return mPublic; }
		void			setPublic(bool value){ mPublic = value; }

		void			prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount);
		void			handleObjectMenuSelect(uint8 messageType,Object* srcObject);

		

		float			getWidth(){ return mWidth; }
		void			setWidth(float width){ mWidth = width; }
		float			getHeight(){ return mHeight; }
		void			setHeight(float height){ mHeight = height; }
		


	private:
		
		
		float			mWidth;
		float			mHeight;

		bool			mPublic;

		
		BuildingFamily	mBuildingFamily;
		HouseFamily		mHouseFamily;

		uint32			mTotalLoadCount;
		
};

//=============================================================================

#endif