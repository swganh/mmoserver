/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2010 The SWG:ANH Team
---------------------------------------------------------------------------------------
Use of this source code is governed by the GPL v3 license that can be found
in the COPYING file or at http://www.gnu.org/licenses/gpl-3.0.html

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_MANUFACTURING_SCHEMATIC_H
#define ANH_ZONESERVER_MANUFACTURING_SCHEMATIC_H

#include "Item.h"
#include "DraftSlot.h"

//=============================================================================

class ManufactureSlot;
class ExperimentationProperty;
class CraftWeight;
class CraftAttribute;
class CraftAttributeWeight;
//class DraftSlot;

struct CustomizationOption
{
    BString	attribute;
    uint16		cutomizationIndex;
    uint32		paletteSize;
    uint32		defaultValue;
};

typedef std::vector<CustomizationOption*>	CustomizationList;

typedef std::vector<ManufactureSlot*>			ManufactureSlots;
typedef std::vector<ExperimentationProperty*>	ExperimentationProperties; // stores all properties
typedef std::vector<std::pair<uint32,ExperimentationProperty*> >	ExperimentationPropertiesStore;	//stores the reference list of unique exp properties
typedef std::vector<CraftWeight*>				CraftWeights;
typedef std::vector<CraftAttribute*>			CraftAttributes;
typedef std::vector<CraftAttributeWeight*>		CraftAttributeWeights;
typedef std::vector<std::pair<uint64,uint32> >	FilledResources;
typedef std::vector<std::pair<Item*,uint32> >	FilledComponent;


//=============================================================================

class ManufacturingSchematic : public Item
{
public:

    friend class ItemFactory;

    ManufacturingSchematic();
    ~ManufacturingSchematic();

    uint8	getFilledSlotCount() {
        return mSlotsFilled;
    }
    void	setFilledSlotCount(uint8 count) {
        mSlotsFilled = count;
    }

    uint8	getCounter() {
        return mCounter += (mManufactureSlots).size();
    }
    void	setCounter(uint8 count) {
        mCounter = count;
    }

    void	addFilledSlot() {
        mSlotsFilled++;
    }
    void	removeFilledSlot() {
        mSlotsFilled--;
    }

    BString	getItemModel() {
        return mItemModel;
    }
    void	setItemModel(const int8* model) {
        mItemModel = model;
    }

    Item*	getItem() {
        return mItem;
    }
    void	setItem(Item* item) {
        mItem = item;
    }

    uint8	getUnknown() {
        return mUnknown;
    }
    void	setUnknown(uint8 unknown) {
        mUnknown = unknown;
    }

    float	getExpFailureChance() {
        return mExpFailureChance;
    }
    void	setExpFailureChance(float chance) {
        mExpFailureChance = chance;
    }

    void	prepareManufactureSlots();
    void	prepareCraftingAttributes();
    void	prepareAttributes();
    void	sendAttributes(PlayerObject* playerObject);

    void	ModifyBlueBars(float mod);

    ManufactureSlots*			getManufactureSlots() {
        return &mManufactureSlots;
    }


    //===============================================================================0
    // necessary to keep track on the same experimental propertie added to a schematic several times
    // because its attributes have differing resource weights
    // they need to still be send in the MSCOs as one experimental propertie
    bool							expPropStorefind(uint32 crc);
    ExperimentationProperties*		getExperimentationProperties() {
        return &mExperimentationProperties;
    }
    ExperimentationPropertiesStore*	getExperimentationPropertiesStore() {
        return &expPropStore;
    }


    //===============================================================================0
    // the mPPAttributeMap stores relevant additions to be added to the final attributes from components AFTER experimentation
    // see implementation for details
    AttributeMap*				getPPAttributeMap() {
        return &mPPAttributeMap;
    }
    template<typename T> T		getPPAttribute(BString key) const;
    template<typename T> T		getPPAttribute(uint32 keyCrc) const;
    void						setPPAttribute(BString key,std::string value);
    void						addPPAttribute(BString key,std::string value);
    bool						hasPPAttribute(BString key) const;
    void						removePPAttribute(BString key);

    CustomizationList*			getCustomizationList() {
        return &mCustomizationList;
    }

    void						incComplexity() {
        mComplexity++;
    }

    uint32					mUpdateCounter[19];
    uint32					mAttributesUpdateCounter;

    bool					mExpAttributeValueChange;
    bool					mBlueBarSizeChange;
    bool					mMaxExpValueChange;

    uint64					mDataPadId;
    uint32					mPaletteSize;

    CustomizationList		mCustomizationList;

    ExperimentationPropertiesStore				expPropStore;

private:

    BString					mItemModel;
    uint8						mSlotsFilled;
    ManufactureSlots			mManufactureSlots;
    ExperimentationProperties	mExperimentationProperties;
    uint8						mUnknown;
    uint8						mCounter;
    float						mExpFailureChance;
    Item*						mItem;
    BString     				mSerial;

    AttributeMap				mPPAttributeMap;
};

//=============================================================================

class ManufactureSlot
{
public:

    ManufactureSlot(DraftSlot* draftSlot)
        : mDraftSlot(draftSlot)
        , mResourceId(0)
        , mFilled(0)
        , mFilledType(DST_Empty)
        , mUnknown1(0)
        , mUnknown2(0xffffffff)
        , mUnknown3(0)
        , mFilledIndicatorChange(false)
    {
        mSerial = "";
    }

    virtual ~ManufactureSlot() {}

    bool	addResourcetoSlot(uint64 resID, uint32 amount, uint8 type);
    void	addComponenttoSlot(uint64 resID, uint32 amount, uint8 type);

    DSType	getFilledType() {
        return mFilledType;
    }
    void	setFilledType(DSType indicator) {
        mFilledIndicatorChange = (mFilledType != indicator);
        mFilledType= indicator;
    }

    uint64	getResourceId() {
        return mResourceId;
    }
    void	setResourceId(uint64 id) {
        mResourceId = id;
    }

    BString	getSerial() {
        return mSerial;
    }
    void	setSerial(BString s) {
        mSerial = s;
    }

    uint32	getFilledAmount() {
        return mFilled;
    }
    void	setFilledAmount(uint32 amount) {
        mFilled = amount;
    }

    FilledResources	mFilledResources;

    FilledComponent	mUsedComponentStacks;

    // slots
    DraftSlot*		mDraftSlot;

    //id of the resource filled or the serial
    uint64	mResourceId;
    BString  mSerial;

    //amount we have filled in the slot
    uint32			mFilled;

    //type of stuff filled see draftslot.h
    DSType			mFilledType;

    uint32			mUnknown1;
    uint32			mUnknown2;
    uint32			mUnknown3;

    // informs us whether there have been changes to know whether to update the counter
    bool			mFilledIndicatorChange;

};

//=============================================================================

// every experimentation property has a list of x attributes
// we experiment per exp property!!
// cave we might have several identical exp properties !!! in this case we treat them as one

class ExperimentationProperty//CraftingAttribute
{
public:

    ExperimentationProperty(const int8* expAttName,CraftWeights* craftWeights,CraftAttributes* craftAttributes,float expAttValue,float assAttValue,float maxExpValue)
        : mExpAttributeName(expAttName)
        , mWeights(craftWeights)
        , mAttributes(craftAttributes)
        , mBlueBarSize(assAttValue)
        , mExpAttributeValue(expAttValue)
        , mMaxExpValue(maxExpValue)
        , mExpUnknown(0)
    {}

    virtual ~ExperimentationProperty() {}

    BString					mExpAttributeName;
    CraftWeights*			mWeights;
    CraftAttributes*	mAttributes;
    float							mBlueBarSize;
    float							mBlueBarSizeOld;
    float							mExpAttributeValue;
    float							mExpAttributeValueOld;
    float							mMaxExpValue;
    float							mMaxExpValueOld;
    uint32							mExpUnknown;//ExperimentationOffsetList
    int32							mRoll;
    bool							mBlueBarSizeChange;
    bool							mExpAttributeValueChange;
    bool							mMaxExpValueChange;
};

//=============================================================================

//=============================================================================

template<typename T>
T	ManufacturingSchematic::getPPAttribute(BString key) const
{
    AttributeMap::const_iterator it = mPPAttributeMap.find(key.getCrc());

    if(it != mPPAttributeMap.end())
    {
        try
        {
            return(boost::lexical_cast<T>((*it).second));
        }
        catch(boost::bad_lexical_cast &)
        {
            DLOG(INFO) << "ManufacturingSchematic::getPPAttribute: cast failed " << key.getAnsi();
        }
    }
    else
        DLOG(INFO) << "ManufacturingSchematic::getPPAttribute: could not find " << key.getAnsi();

    return(T());
}
//=============================================================================

template<typename T>
T	ManufacturingSchematic::getPPAttribute(uint32 keyCrc) const
{
    AttributeMap::iterator it = mPPAttributeMap.find(keyCrc);

    if(it != mPPAttributeMap.end())
    {
        try
        {
            return(boost::lexical_cast<T>((*it).second));
        }
        catch(boost::bad_lexical_cast &)
        {
            DLOG(INFO) << "ManufacturingSchematic::getPPAttribute: cast failed " << keyCrc;
        }
    }
    else
        DLOG(INFO) << "ManufacturingSchematic::getPPAttribute: could not find " << keyCrc;

    return(T());
}


//=============================================================================


#endif

