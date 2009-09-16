/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_MANUFACTURING_SCHEMATIC_H
#define ANH_ZONESERVER_MANUFACTURING_SCHEMATIC_H

#include "Item.h"


//=============================================================================

class ManufactureSlot;
class ExperimentationProperty;
class CraftWeight;
class CraftAttribute;
class DraftSlot;

struct CustomizationOption
{
	string		attribute;
	uint16		cutomizationIndex;
	uint32		paletteSize;
	uint32		defaultValue;
};

typedef std::vector<CustomizationOption*>	CustomizationList;

typedef std::vector<ManufactureSlot*>			ManufactureSlots;
typedef std::vector<ExperimentationProperty*>	ExperimentationProperties; // stores all properties
typedef std::vector<std::pair<uint32,ExperimentationProperty*>>	ExperimentationPropertiesStore;	//stores the reference list of unique exp properties
typedef std::vector<CraftWeight*>				CraftWeights;
typedef std::vector<CraftAttribute*>			CraftAttributes;
typedef std::vector<std::pair<uint64,uint32> >	FilledResources;


//=============================================================================

class ManufacturingSchematic : public Item
{
	public:

		friend class ItemFactory;

		ManufacturingSchematic();
		~ManufacturingSchematic();

		uint8	getFilledSlotCount(){ return mSlotsFilled; }
		void	setFilledSlotCount(uint8 count){ mSlotsFilled = count; }

		uint8	getCounter(){ return mCounter += (mManufactureSlots).size(); }
		void	setCounter(uint8 count){ mCounter = count; }
		
		void	addFilledSlot(){ mSlotsFilled++; }
		void	removeFilledSlot(){ mSlotsFilled--; }

		string	getItemModel(){ return mItemModel; }
		void	setItemModel(const int8* model) { mItemModel = model; }

		Item*	getItem(){ return mItem; }
		void	setItem(Item* item) { mItem = item; }

		uint8	getUnknown(){ return mUnknown; }
		void	setUnknown(uint8 unknown){ mUnknown = unknown; }

		float	getExpFailureChance(){ return mExpFailureChance; }
		void	setExpFailureChance(float chance){ mExpFailureChance = chance; }

		void	prepareManufactureSlots();
		void	prepareCraftingAttributes();
		void	prepareAttributes();
		void	sendAttributes(PlayerObject* playerObject);

		bool	expPropStorefind(uint32 crc)
		{
			ExperimentationPropertiesStore::iterator	epStoreIt	= expPropStore.begin();
			while(epStoreIt != expPropStore.end())
			{
				if((*epStoreIt).first == crc)
					return true;
				epStoreIt++;
			}
			return false;
		}

		ManufactureSlots*			getManufactureSlots(){ return &mManufactureSlots; }
		ExperimentationProperties*	getExperimentationProperties(){ return &mExperimentationProperties; }
		ExperimentationPropertiesStore*	getExperimentationPropertiesStore(){ return &expPropStore; }

		CustomizationList*		getCustomizationList(){return &mCustomizationList;}

		uint32	mUpdateCounter[18];
		uint32	mAttributesUpdateCounter;

		bool	mExpAttributeValueChange;
		bool	mBlueBarSizeChange;
		bool	mMaxExpValueChange;
		uint64	mDataPadId;
		uint32	mPaletteSize;

		CustomizationList		mCustomizationList;

		ExperimentationPropertiesStore				expPropStore;

	private:

		string						mItemModel;
		uint8						mSlotsFilled;
		ManufactureSlots			mManufactureSlots;
		ExperimentationProperties	mExperimentationProperties;
		uint8						mUnknown;
		uint8						mCounter;
		float						mExpFailureChance;
		Item*						mItem;
		string						mSerial;
		
};

//=============================================================================

class ManufactureSlot
{
	public:

		ManufactureSlot(DraftSlot* draftSlot) :	mDraftSlot(draftSlot),mFilled(0),mUnknown1(0),mUnknown2(0xffffffff),mUnknown3(0),mFilledIndicator(0),mFilledIndicatorChange(false){}
			
		virtual ~ManufactureSlot(){}
		float	getmFilledIndicator(){ return mFilledIndicator; }
		void	setmFilledIndicator(uint32 indicator){ mFilledIndicatorChange = (mFilledIndicator != indicator);mFilledIndicator= indicator; }
		
		// slots
		DraftSlot*		mDraftSlot;
		uint32			mFilled;
		FilledResources	mFilledResources;
		
		// SlotContent filled
		// 0 nothing
		// 1 ??
		// 2 item
		// 4 resource
		uint32			mFilledIndicator;


		// informs us whether there have been changes to know whether to update the counter
		bool			mFilledIndicatorChange;

		uint32			mUnknown1;
		uint32			mUnknown2;
		uint32			mUnknown3;
};

//=============================================================================

// every experimentation property has a list of x attributes
// we experiment per exp property!!
// cave we might have several identical exp properties !!! in this case we treat them as one

class ExperimentationProperty//CraftingAttribute
{
	public:

		ExperimentationProperty(const int8* expAttName,CraftWeights* craftWeights,CraftAttributes* craftAttributes,float expAttValue,float assAttValue,float maxExpValue)
		: mExpAttributeValue(expAttValue),mWeights(craftWeights),mAttributes(craftAttributes),mExpUnknown(0),mBlueBarSize(assAttValue),mMaxExpValue(maxExpValue)
		{ 
			mExpAttributeName			=	expAttName;
			mBlueBarSize				=	1.0;
			
		}

		virtual ~ExperimentationProperty(){}

		CraftWeights*		mWeights;
		CraftAttributes*	mAttributes;

		string				mExpAttributeName;
		
		float				mExpAttributeValue;
		float				mExpAttributeValueOld;
		bool				mExpAttributeValueChange;

		uint32				mExpUnknown;//ExperimentationOffsetList

		float				mBlueBarSize;
		float				mBlueBarSizeOld;
		bool				mBlueBarSizeChange;

		float				mMaxExpValue;
		float				mMaxExpValueOld;
		bool				mMaxExpValueChange;

		int32				mRoll;
};

//=============================================================================

#endif

