					   /*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_PLAYERSTRUCTURE_OBJECT_H
#define ANH_ZONESERVER_PLAYERSTRUCTURE_OBJECT_H

#include "TangibleObject.h"
#include "BuildingEnums.h"
#include "StructureManager.h"
#include "UICallback.h"
#include <vector>

class CellObject;

//=============================================================================

typedef std::vector<CellObject*>		CellObjectList;
typedef	std::vector<TangibleObject*>	ItemList;


//=============================================================================

enum timerTodoEnum
{
	ttE_Nothing			= 1,
	ttE_Delete			= 2,
	ttE_BuildingFence	= 3,
	ttE_UpdateHopper	= 4,
	ttE_UpdateEnergy	= 5,

};

struct timerTodoStruct
{
	timerTodoEnum	todo;
	uint64			buildingFence;
	uint64			playerId;
	uint64			projectedTime;

};

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
		bool					decCondition(uint32 dec){int64 altered = static_cast<int64>(mCondition-dec);if(altered <= 0) return false; else mCondition = static_cast<uint32>(altered);return true; }

		uint32					getCurrentMaintenance();
		void					setCurrentMaintenance(uint32 maintenance);

		uint32					getMaintenanceRate();
		void					setMaintenanceRate(uint32 maintenance);

		uint32					getMaxCondition(){ return mMaxCondition; }
		void					setMaxCondition(uint32 condition){ mMaxCondition = condition; }
		bool					decMaxCondition(uint32 dec){int64 altered = static_cast<int64>(mMaxCondition-dec);if(altered <= 0) return false; else mMaxCondition = static_cast<uint32>(altered);return true; }
		
		
		bool					canRedeed();

		void					setRedeed(bool yesORno){mWillRedeed = yesORno;}
		bool					getRedeed(){return mWillRedeed;}

		// the code we need to enter to destroy a structure
		string					getCode(){return mCode;}
		void					setCode(){mCode = gStructureManager->getCode();}

		// is called by the structuremanager after reading maintenance data from the db
		void					deleteStructureDBDataRead(uint64 playerId);

		// sends an UI List with the Admin list
		void					sendStructureAdminList(uint64 playerId);

		void					handleUIEvent(uint32 action,int32 element,string inputStr,UIWindow* window);

		//camps dont have cells
		ObjectList				getAllCellChilds();


		// thats the camps / structures lit of additionally created item like signs and stuff and fires and chairs
		void					addItem(TangibleObject* tO)	{ mItemList.push_back(tO); }
		ItemList*				getItemList()				{ return(&mItemList); }

		timerTodoStruct*		getTTS(){return &mTTS;}

		// thats the structures admin list
		BStringVector			getStrucureAdminList(){return mStructureAdminList;}
		void					addStructureAdmin(string name){mStructureAdminList.push_back(name);}

		

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
		bool						mWillRedeed;
		string						mCode;
		timerTodoStruct				mTTS;

		BStringVector				mStructureAdminList;
};


//=============================================================================

#endif