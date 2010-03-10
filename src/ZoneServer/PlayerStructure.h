					   /*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_PLAYERSTRUCTURE_OBJECT_H
#define ANH_ZONESERVER_PLAYERSTRUCTURE_OBJECT_H

#include "TangibleObject.h"
#include "BuildingEnums.h"
#include "StructureManager.h"
#include "UICallback.h"
#include <vector>


//=============================================================================

typedef	std::vector<TangibleObject*>	ItemList;


//=============================================================================

enum timerTodoEnum
{
	ttE_Nothing			= 1,
	ttE_Delete			= 2,
	ttE_BuildingFence	= 3,
	ttE_UpdateHopper	= 4,
	ttE_UpdateEnergy	= 5

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
	friend class FactoryFactory;
	friend class BuildingFactory;
	friend class HouseFactory;
	

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

		uint32					getCondition(){ return (mMaxCondition - mDamage); }
		void					setCondition(uint32 condition){ mCondition = condition; }
		bool					decCondition(uint32 dec){int64 altered = static_cast<int64>(mCondition-dec);if(altered <= 0) return false; else mCondition = static_cast<uint32>(altered);return true; }

		//the lots this structure uses
		uint8					getLotCount(){ return mLotsUsed; }

		uint32					getCurrentMaintenance();
		void					setCurrentMaintenance(uint32 maintenance);

		uint32					getMaintenanceRate(){return(uint32(maint_cost_wk/(168)));}
		void					setMaintenanceRate(uint32 maintenance){maint_cost_wk = maintenance*168;}

		uint32					getRepairCost(){return(mRepairCost);}
		void					setRepairCost(uint32 m){mRepairCost= m;}

		uint32					getCurrentPower();
		void					setCurrentPower(uint32 maintenance);

		uint32					getPowerConsumption(){return(mPowerUsed);}
		void					setPowerConsumption(uint32 c){mPowerUsed = c;}

		uint32					getMaxCondition(){ return mMaxCondition; }
		void					setMaxCondition(uint32 condition){ mMaxCondition = condition; }
		bool					decMaxCondition(uint32 dec){int64 altered = static_cast<int64>(mMaxCondition-dec);if(altered <= 0) return false; else mMaxCondition = static_cast<uint32>(altered);return true; }


		bool					canRedeed();

		void					setRedeed(bool yesORno){mWillRedeed = yesORno;}
		bool					getRedeed(){return mWillRedeed;}

		// the code we need to enter to destroy a structure
		string					getCode(){return mCode;}
		void					setCode(){mCode = gStructureManager->getCode();}

		//we store the owners name here when read in from db for the status window
		string					getOwnersName(){return mOName;}
		void					setOwnersName(string name){mOName = name;}		

		// is called by the structuremanager after reading maintenance data from the db
		void					deleteStructureDBDataRead(uint64 playerId);

		// sends an UI List with the Admin list
		void					sendStructureAdminList(uint64 playerId);
		void					sendStructureHopperList(uint64 playerId);
		void					sendStructureBanList(uint64 playerId);
		void					sendStructureEntryList(uint64 playerId);

		void					handleUIEvent(uint32 action,int32 element,string inputStr,UIWindow* window);
		void					handleUIEvent(string strInventoryCash, string strBankCash, UIWindow* window);


		// thats the camps / structures lit of additionally created item like signs and stuff and fires and chairs
		void					addItem(TangibleObject* tO)	{ mItemList.push_back(tO); }
		ItemList*				getItemList()				{ return(&mItemList); }

		timerTodoStruct*		getTTS(){return &mTTS;}

		// thats the structures admin list
		BStringVector			getStrucureAdminList(){return mStructureAdminList;}
		void					addStructureAdminListEntry(string name){mStructureAdminList.push_back(name);}
		void					resetStructureAdminList(){mStructureAdminList.clear();}

		// thats the structures entry list
		BStringVector			getStrucureEntryList(){return mStructureEntryList;}
		void					addStructureEntryListEntry(string name){mStructureEntryList.push_back(name);}
		void					resetStructureEntryList(){mStructureEntryList.clear();}

		// thats the structures ban list
		BStringVector			getStrucureBanList(){return mStructureBanList;}
		void					addStructureBanListEntry(string name){mStructureBanList.push_back(name);}
		void					resetStructureBanList(){mStructureBanList.clear();}

		// thats the structures admin list
		BStringVector			getStrucureHopperList(){return mStructureHopperList;}
		void					addStructureHopperListEntry(string name){mStructureHopperList.push_back(name);}
		void					resetStructureHopperList(){mStructureHopperList.clear();}

		// thats the structures admin list
		ObjectIDList			getHousingList(){return mHousingAdminList;}
		void					addHousingAdminEntry(uint64 id){mHousingAdminList.push_back(id);}
		void					resetHousingAdminList(){mHousingAdminList.clear();}
		bool					hasAdminRights(uint64 id);

		

								 

	private:


		PlayerStructureFamily		mPlayerStructureFamily;

		uint32						mTotalLoadCount;

		float						mWidth;
		float						mHeight;

		//structures Owner
		uint64						mOwner;
		ItemList					mItemList;
		uint32						mCondition;
		uint32						mMaintenance;
		uint32						mMaxCondition;
		bool						mWillRedeed;
		uint8						mLotsUsed;

		string						mCode;
		timerTodoStruct				mTTS;
		string						mOName;

		ObjectIDList				mHousingAdminList;
		BStringVector				mStructureBanList;		
		BStringVector				mStructureEntryList;
		BStringVector				mStructureAdminList;
		BStringVector				mStructureHopperList;

		uint32						maint_cost_wk;
		uint32						mPowerUsed;
		uint32						mRepairCost;
};


//=============================================================================

#endif
