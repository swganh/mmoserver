/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_CRAFTINGSESSION_H
#define ANH_ZONESERVER_CRAFTINGSESSION_H

//#include "Utils/typedefs.h"
#include <map>
#include <vector>
#include "math.h"
#include "DatabaseManager/DatabaseCallback.h"
#include "Utils/rand.h"
#include "ObjectFactoryCallback.h"
#include "CraftingEnums.h"

//=============================================================================

class CraftingTool;
class CraftingStation;
class CraftSessionQueryContainer;
class CraftWeight;
class Database;
class DatbaseCallback;
class DispatchClient;
class DraftSchematic;
class Inventory;
class Item;
class ManufacturingSchematic;
class ManufactureSlot;
class ObjectFactoryCallback;
class PlayerObject;

namespace Anh_Utils
{
    class Clock;
}

typedef std::vector<CraftWeight*>			CraftWeights;
typedef std::map<uint64,uint32>				CheckResources;

//=============================================================================

class CraftingSession : public DatabaseCallback, public ObjectFactoryCallback
{
	public:

		CraftingSession(Anh_Utils::Clock* clock,Database* database,PlayerObject* player,CraftingTool* tool,CraftingStation* station,uint32 expFlag);
		~CraftingSession();

		virtual void			handleObjectReady(Object* object,DispatchClient* client);
		virtual void			handleDatabaseJobComplete(void* ref,DatabaseResult* result);

		void					handleFillSlot(uint64 resContainerId,uint32 slotId,uint32 unknown,uint8 counter);
		void					handleFillSlotResource(uint64 resContainerId,uint32 slotId,uint32 unknown,uint8 counter);
		void					handleFillSlotComponent(uint64 componentId,uint32 slotId,uint32 unknown,uint8 counter);
		uint32					getComponentSerial(ManufactureSlot*	manSlot, Inventory* inventory);
		bool					AdjustComponentStack(Item* item, Inventory* inventory, uint32 uses);

		void					handleEmptySlot(uint32 slotId,uint64 containerId,uint8 counter);
		void					emptySlot(uint32 slotId,ManufactureSlot* manSlot,uint64 containerId);
		void					bagResource(ManufactureSlot* manSlot,uint64 containerId);
		void					bagComponents(ManufactureSlot* manSlot,uint64 containerId);

		CraftingTool*			getTool(){ return mTool; }
		CraftingStation*		getStation(){ return mStation; }
		uint32					getStage(){ return mStage; }
		ManufacturingSchematic*	getManufacturingSchematic(){ return mManufacturingSchematic; }
		Item*					getItem(){ return mItem; }
		uint32					getOwnerExpSkillMod(){ return mOwnerExpSkillMod; }
		uint32					getOwnerAssSkillMod(){ return mOwnerAssSkillMod; }

		uint32					isExperimentationAllowed(){ return mExpFlag; }

		void					setStage(uint32 stage){ mStage = stage; }
		void					setItem(Item* item){ mItem = item; }
		void					setManufactureSchematic(ManufacturingSchematic* manSchem){ mManufacturingSchematic = manSchem; }
		void					setOwnerExpSkillMod(uint32 skillMod){ mOwnerExpSkillMod = skillMod; }
		void					setOwnerAssSkillMod(uint32 skillMod){ mOwnerAssSkillMod = skillMod; }

		bool					selectDraftSchematic(uint32 schematicIndex);
		void					assemble(uint32 counter);

		void					experiment(uint8 counter,std::vector<std::pair<uint32,uint32> > properties);
		void					customizationStage(uint32 counter);
		void					creationStage(uint32 counter);
		void					experimentationStage(uint32 counter);
		void					customize(const int8* itemName);
		void					createPrototype(uint32 noPractice,uint32 counter);
		string					getSerial();

		void					addComponentAttribute();

		//sets/retrieves the amounts a man schematic can produce
		void					setProductionAmount(uint32 amount){mProductionAmount = amount;}
		uint32					getProductionAmount(){return mProductionAmount;}

		//sets/retrieves the customization value we have for the given schematic
		void					setCustomization(uint32 cust){mCustomization = cust;}
		uint32					getCustomization(){return mCustomization;}

		void					createManufactureSchematic(uint32 counter);

	private:


		float roundF(float x,const int places)
		{
			const float shift = pow(10.0f,places);

			return floorf(x * shift + 0.5f) / shift;
		}

		float					_calcWeightedResourceValue(CraftWeights* weights);
		void					_cleanUp();
		//===========================
		//gets the type of success failur for experimentation
		uint8					_experimentRoll(uint32 expPoints);

		//===========================
		//gets the type of success failur assembly
		uint8					_assembleRoll();

		//===========================
		//gets the average Malleability
		float					_calcAverageMalleability();

		Anh_Utils::Clock*				mClock;
		Database*								mDatabase;
		DraftSchematic*					mDraftSchematic;
		Item*										mItem;
		ManufacturingSchematic*	mManufacturingSchematic;
		PlayerObject*						mOwner;
		CraftingStation*				mStation;
		CraftingTool*						mTool;
		float										mToolEffectivity;
		uint32									mAssSkillModId;
		uint32									mCriticalCount;
		uint32									mCustomization;
		uint32									mExpFlag;
		uint32									mExpSkillModId;
		uint32									mOwnerAssSkillMod;
		uint32									mOwnerExpSkillMod;
		uint32									mProductionAmount;
		uint32									mStage;
		uint32									mSubCategory;
		bool										mFirstFill;
};

//=============================================================================

class CraftSessionQueryContainer
{
	public:

		CraftSessionQueryContainer(CraftSessionQuery qType,uint8 counter) : mQType(qType),mCounter(counter){}
		~CraftSessionQueryContainer(){}

		CraftSessionQuery	mQType;
		uint8				mCounter;
};

//=============================================================================

#endif

