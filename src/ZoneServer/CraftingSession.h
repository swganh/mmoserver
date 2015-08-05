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

#ifndef ANH_ZONESERVER_CRAFTINGSESSION_H
#define ANH_ZONESERVER_CRAFTINGSESSION_H

//#include "Utils/typedefs.h"
#include <map>
#include <vector>
#include "math.h"
#include "DatabaseManager/DatabaseCallback.h"
#include "Utils/bstring.h"
#include "Utils/rand.h"
#include "ObjectFactoryCallback.h"
#include "CraftingEnums.h"

#include <boost/lexical_cast.hpp>
#include <list>

//=============================================================================

class ExperimentationProperty;
class CraftAttribute;
class CraftingTool;
class CraftingStation;
class CraftSessionQueryContainer;
class CraftWeight;
class Database;
class DatbaseCallback;
class DispatchClient;
class DraftSchematic;
class FactoryCrate;
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

typedef std::list<uint64>					ObjectIDList;
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
    void					handleFillSlotResourceRewrite(uint64 resContainerId,uint32 slotId,uint32 unknown,uint8 counter);

    bool					prepareComponent(Item* component, uint32 needed, ManufactureSlot* manSlot);
    uint32					getComponentOffer(Item* component, uint32 needed);
    BString					ComponentGetSerial(Item* component);

    bool					AdjustComponentStack(Item* item, uint32 uses);
    uint32					AdjustFactoryCrate(FactoryCrate* crate, uint32 uses);
    void					destroyComponents();

    void					handleEmptySlot(uint32 slotId,uint64 containerId,uint8 counter);
    void					emptySlot(uint32 slotId,ManufactureSlot* manSlot,uint64 containerId);
    void					bagResource(ManufactureSlot* manSlot,uint64 containerId);
    void					bagComponents(ManufactureSlot* manSlot,uint64 containerId);

    void					updateResourceContainer(uint64 containerID, uint32 newAmount);

    CraftingTool*			getTool() {
        return mTool;
    }
    CraftingStation*		getStation() {
        return mStation;
    }
    uint32					getStage() {
        return mStage;
    }
    ManufacturingSchematic*	getManufacturingSchematic() {
        return mManufacturingSchematic;
    }
    Item*					getItem() {
        return mItem;
    }
    uint32					getOwnerExpSkillMod() {
        return mOwnerExpSkillMod;
    }
    uint32					getOwnerAssSkillMod() {
        return mOwnerAssSkillMod;
    }

    uint32					isExperimentationAllowed() {
        return mExpFlag;
    }

    void					setStage(uint32 stage) {
        mStage = stage;
    }
    void					setItem(Item* item) {
        mItem = item;
    }
    void					setManufactureSchematic(ManufacturingSchematic* manSchem) {
        mManufacturingSchematic = manSchem;
    }
    void					setOwnerExpSkillMod(uint32 skillMod) {
        mOwnerExpSkillMod = skillMod;
    }
    void					setOwnerAssSkillMod(uint32 skillMod) {
        mOwnerAssSkillMod = skillMod;
    }

    bool					selectDraftSchematic(uint32 schematicIndex);
    void					assemble(uint32 counter);

    void					experiment(uint8 counter,std::vector<std::pair<uint32,uint32> > properties);
    void					customizationStage(uint32 counter);
    void					creationStage(uint32 counter);
    void					experimentationStage(uint32 counter);
    void					customize(const int8* itemName);
    void					createPrototype(uint32 noPractice,uint32 counter);
    BString					getSerial();

    void					addComponentAttribute();

    //sets/retrieves the amounts a man schematic can produce
    void					setProductionAmount(uint32 amount) {
        mProductionAmount = amount;
    }
    uint32					getProductionAmount() {
        return mProductionAmount;
    }

    //sets/retrieves the customization value we have for the given schematic
    void					setCustomization(uint32 cust) {
        mCustomization = cust;
    }
    uint32					getCustomization() {
        return mCustomization;
    }

    void					createManufactureSchematic(uint32 counter);

    //collects a resourcelist for Manufacturing Schematics
    void					collectResources();
    void					collectComponents();

    //empties the slots of a manufacturing schematic when assembly failed
    void					emptySlots(uint32 counter);

    //an attributes value is updated depending on the attribute type (int/float)
    void					modifyAttributeValue(CraftAttribute* att, float attValue);

    //the percentage modifying our attribute based on the roll
    float					getPercentage(uint8 roll);

    // gets the ExperimentationRoll and initializes the experimental properties
    // meaning an exp property which exists several times (with different resourceweights)
    // gets the same roll assigned
    uint8					getExperimentationRoll(ExperimentationProperty* expProperty, uint8 expPoints);

    uint32					getCounter() {
        return mCounter;
    }

private:


    float rndFloat(float x)
    {
        int32 i = (int32)floorf(x * 100 + 0.5f);
        //	(float)(i / 100.00);

        int8 str[32];
        sprintf(str,"%.2f",(float)(i / 100.00));
        return boost::lexical_cast<float>(str);
    }

    BString rndFloattoStr(float x)
    {
        int32 i = (int32)floorf(x * 100 + 0.5f);
        //	(float)(i / 100.00);

        int8 str[32];
        sprintf(str,"%.2f",(float)(i / 100.00));
        return BString(str);
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
    Database*						mDatabase;
    DraftSchematic*					mDraftSchematic;
    Item*							mItem;
    ManufacturingSchematic*			mManufacturingSchematic;
    PlayerObject*					mOwner;
    CraftingStation*				mStation;
    CraftingTool*					mTool;
    CheckResources					mCheckRes;
    float							mToolEffectivity;
    uint32							mAssSkillModId;
    uint32							mCriticalCount;
    uint32							mCustomization;
    uint32							mExpFlag;
    uint32							mExpSkillModId;
    uint32							mOwnerAssSkillMod;
    uint32							mOwnerExpSkillMod;
    uint32							mProductionAmount;
    uint32							mStage;
    uint32							mSubCategory;
    uint32							mSchematicCRC;
    bool							mFirstFill;
    uint32							mCounter;

    // makes it that we know that the next item we load is a component to be attached to a slot
    bool							mItemLoaded;

    //dirty and ugly but we can do it as the crafting process stalls until the item is added to the slot
    //fact is we dont have asynccontainers with Objectcreates :(
    uint32							mAsyncComponentAmount;
    uint32							mAsyncStackSize;
    ManufactureSlot*				mAsyncManSlot;
    uint32							mAsyncSlotId;
    uint8							mAsyncCounter;
    bool							mAsyncSmallUpdate;
};

//=============================================================================

class CraftSessionQueryContainer
{
public:

    CraftSessionQueryContainer(CraftSessionQuery qType,uint8 counter) : mQType(qType),mCounter(counter) {}
    ~CraftSessionQueryContainer() {}

    CraftSessionQuery	mQType;
    uint8				mCounter;
};

//=============================================================================

#endif

