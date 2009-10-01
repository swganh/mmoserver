					   /*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_PLAYERSTRUCTURE_OBJECT_H
#define ANH_ZONESERVER_PLAYERSTRUCTURE_OBJECT_H

#include "tangibleobject.h"
//#include "StructureManager.h"
#include "BuildingEnums.h"
#include "CellObject.h"
#include <vector>


//=============================================================================

typedef std::vector<CellObject*>		CellObjectList;
typedef	std::vector<TangibleObject*>	ItemList;


//=============================================================================

class PlayerStructure :	public TangibleObject
{
	friend class HarvesterFactory;

	public:

		PlayerStructure();
		~PlayerStructure();

		PlayerStructureFamily	getPlayerStructureFamily(){ return mPlayerStructureFamily; }
		void					setPlayerStructureFamily(PlayerStructureFamily bf){ mPlayerStructureFamily = bf; }

		uint32					getLoadCount(){ return mTotalLoadCount; }
		uint32					decLoadCount(){ return (mTotalLoadCount-1); }
		void					setLoadCount(uint32 count){ mTotalLoadCount = count; }

		//for structures which can be entered like camps and houses
		float					getWidth(){ return mWidth; }
		void					setWidth(float width){ mWidth = width; }
		float					getHeight(){ return mHeight; }
		void					setHeight(float height){ mHeight = height; }

		uint64					getOwner(){ return mOwner; }
		void					setOwner(uint64 owner){ mOwner = owner; }

		uint32					getCondition(){ return mCondition; }
		void					setCondition(uint32 condition){ mCondition = condition; }
		bool					decCondition(uint32 dec){int64 altered = mCondition-dec;if(altered <= 0) return false; else mCondition = altered;return true; }

		uint32					getMaintenance(){ return mMaintenance; }
		void					setMaintenance(uint32 maintenance){ mMaintenance = maintenance; }
		bool					decMaintenance(uint32 dec){int64 altered = mMaintenance-dec;if(altered <= 0) return false; else mMaintenance= altered;return true; }

		uint32					getMaxMaintenance();
		void					setMaxMaintenance(uint32 maintenance);

		uint32					getMaxCondition(){ return mMaxCondition; }
		void					setMaxCondition(uint32 condition){ mMaxCondition = condition; }
		bool					decMaxCondition(uint32 dec){int64 altered = mMaxCondition-dec;if(altered <= 0) return false; else mMaxCondition = altered;return true; }
		
		
		bool					canRedeed();

		//camps dont have cells
		ObjectList				getAllCellChilds();


		void					addItem(TangibleObject* tO)	{ mItemList.push_back(tO); }
		ItemList*				getItemList()				{ return(&mItemList); }


		

	private:


		PlayerStructureFamily		mPlayerStructureFamily;

		uint32						mTotalLoadCount;
		
		CellObjectList				mCells;

		float						mWidth;
		float						mHeight;

		//structures Owner
		uint64						mOwner;
		ItemList					mItemList;
		uint32						mCondition;
		uint32						mMaintenance;
		uint32						mMaxCondition;
};

//=============================================================================

#endif