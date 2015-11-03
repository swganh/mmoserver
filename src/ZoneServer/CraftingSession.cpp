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


#include "CraftingSession.h"

#include "CraftBatch.h"
#include "CraftingSessionFactory.h"
#include "CraftingStation.h"
#include "CraftingTool.h"
#include "Datapad.h"
#include "DraftSchematic.h"
#include "DraftSlot.h"
#include "Inventory.h"
#include "Item.h"
#include "ManufacturingSchematic.h"
#include "ObjectControllerOpcodes.h"
#include "ObjectFactory.h"
#include "PlayerObject.h"
#include "ResourceContainer.h"
#include "ResourceManager.h"
#include "StateManager.h"
#include "SchematicManager.h"
#include "WorldManager.h"

#include "MessageLib/MessageLib.h"

#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseCallback.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"

#include "Utils/clock.h"

#include <boost/lexical_cast.hpp>

//=============================================================================
//
// initiates a crafting session, we usually start at stage 1
//
CraftingSession::CraftingSession(Anh_Utils::Clock* clock,Database* database,PlayerObject* player,CraftingTool* tool,CraftingStation* station,uint32 expFlag)
    : mClock(clock)
    , mDatabase(database)
    , mDraftSchematic(NULL)
    , mItem(NULL)
    , mManufacturingSchematic(NULL)
    , mOwner(player)
    , mStation(station)
    , mTool(tool)
    , mCriticalCount(0)
    , mExpFlag(expFlag)
    , mStage(1)
	, mFirstFill(false)
    , mCounter(2)
    , mItemLoaded(false)
{
	// update player variables
	mOwner->setCraftingStage(mStage);
	mOwner->setExperimentationFlag(mExpFlag);
	mOwner->setExperimentationPoints(0);


	// the station given is the crafting station compatible with our tool in a set radius
	//
	if(station)
		mOwner->setNearestCraftingStation(station->getId());
	else
		mOwner->setNearestCraftingStation(0);

	gStateManager.setCurrentActionState(mOwner,CreatureState_Crafting);

	// send the updates
	gMessageLib->sendStateUpdate(mOwner);
	gMessageLib->sendUpdateCraftingStage(mOwner);
	gMessageLib->sendUpdateExperimentationFlag(mOwner);
	gMessageLib->sendUpdateNearestCraftingStation(mOwner);
	gMessageLib->sendUpdateExperimentationPoints(mOwner);
	gMessageLib->sendDraftSchematicsList(mTool,mOwner);
	mToolEffectivity = mTool->getAttribute<float>("craft_tool_effectiveness");
}

//=============================================================================
//
// cancels a crafting session, should be only called through the factory
//
CraftingSession::~CraftingSession()
{
	_cleanUp();

	// reset player variables
	mOwner->setCraftingSession(NULL);
    gStateManager.removeActionState(mOwner, CreatureState_Crafting);
	mOwner->setCraftingStage(0);
	mOwner->setExperimentationFlag(1);
	mOwner->setExperimentationPoints(10);
	mOwner->setNearestCraftingStation(0);

	// send cancel session
	gMessageLib->sendSharedNetworkMessage(mOwner,0,1);
    gMessageLib->SendSystemMessage(::common::OutOfBand("ui_craft", "session_ended"), mOwner);

    // send player updates
    gMessageLib->sendStateUpdate(mOwner);
    gMessageLib->sendUpdateCraftingStage(mOwner);
    gMessageLib->sendUpdateExperimentationFlag(mOwner);
    gMessageLib->sendUpdateNearestCraftingStation(mOwner);
    gMessageLib->sendUpdateExperimentationPoints(mOwner);
}

//=============================================================================

void CraftingSession::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
    CraftSessionQueryContainer* qContainer = reinterpret_cast<CraftSessionQueryContainer*>(ref);

    switch(qContainer->mQType)
    {
        //get the skillmod for experimentation
    case CraftSessionQuery_SkillmodExp:
    {
        uint32			modId;
        DataBinding*	binding = mDatabase->createDataBinding(1);
        binding->addField(DFT_uint32,0,4);

        uint32 count = static_cast<uint32>(result->getRowCount());

        mOwnerExpSkillMod = 0;
        if(count)
        {
            // please note that our schematic might be granted by several skills
            // thus we should now find the skillmod to use (ie the highest ?)

            // alternatively a separate db entry needs to be made
            for(uint32 i = 0; i<count; i++)
            {
                result->getNextRow(binding,&modId);
                mExpSkillModId = modId;
                int32 resultInt = mOwner->getSkillModValue(mExpSkillModId);
                if ((resultInt > 0)&& (resultInt > (int32) mOwnerExpSkillMod))
                {
                    mOwnerExpSkillMod = resultInt;
                }
            }

            mOwner->setExperimentationPoints(mOwnerExpSkillMod / 10);
            gMessageLib->sendUpdateExperimentationPoints(mOwner);
        }


        mDatabase->destroyDataBinding(binding);

        CraftSessionQueryContainer* container = new CraftSessionQueryContainer(CraftSessionQuery_SkillmodAss,0);
        uint32 groupId = mDraftSchematic->getWeightsBatchId();

        int8 sql[550];
        sprintf(sql,"SELECT DISTINCT skills_skillmods.skillmod_id FROM %s.draft_schematics INNER JOIN %s.skills_schematicsgranted "
                    "ON draft_schematics.group_id = skills_schematicsgranted.schem_group_id INNER JOIN %s.skills_skillmods "
                    "ON skills_schematicsgranted.skill_id = skills_skillmods.skill_id INNER JOIN %s.skillmods "
                    "ON skills_skillmods.skillmod_id = skillmods.skillmod_id WHERE draft_schematics.weightsbatch_id = %u "
                    "AND skillmods.skillmod_name LIKE %s",
                    mDatabase->galaxy(),mDatabase->galaxy(),
                    mDatabase->galaxy(),mDatabase->galaxy(),
                    groupId,"'%%asse%%'");
        mDatabase->executeSqlAsyncNoArguments(this,container,sql);

    }
    break;

    case CraftSessionQuery_SkillmodAss:
    {
        uint32			resultId;
        DataBinding*	binding = mDatabase->createDataBinding(1);
        binding->addField(DFT_uint32,0,4);

        uint32 count = static_cast<uint32>(result->getRowCount());

        mOwnerAssSkillMod = 0;
        if(count)
        {

            // please note that our schematic might be granted by several skills
            // thus we should now find the skillmod to use (ie the highest ?)

            // alternatively a separate db entry needs to be made
            for(uint32 i = 0; i<count; i++)
            {
                result->getNextRow(binding,&resultId);
                mAssSkillModId = resultId;
                int32 resultInt = mOwner->getSkillModValue(mAssSkillModId);
                if ((resultInt > 0)&& (resultInt > (int32) mOwnerAssSkillMod))
                {
                    mOwnerAssSkillMod = resultInt;
                }
            }

        }


        mDatabase->destroyDataBinding(binding);

        //lets get the customization data from db
        CraftSessionQueryContainer* container = new CraftSessionQueryContainer(CraftSessionQuery_CustomizationData,0);
        uint32 groupId = mDraftSchematic->getWeightsBatchId();

        int8 sql[550];
        sprintf(sql,"SELECT dsc.attribute, dsc.cust_attribute, dsc.palette_size, dsc.default_value FROM "
                    "%s.draft_schematic_customization dsc WHERE dsc.batchId = %u",mDatabase->galaxy(),groupId);
        mDatabase->executeSqlAsync(this,container,sql);


    }
    break;

    case CraftSessionQuery_CustomizationData:
    {
        DataBinding*	binding = mDatabase->createDataBinding(4);
        binding->addField(DFT_bstring,offsetof(CustomizationOption,attribute),32,0);
        binding->addField(DFT_uint16,offsetof(CustomizationOption,cutomizationIndex),2,1);
        binding->addField(DFT_uint32,offsetof(CustomizationOption,paletteSize),4,2);
        binding->addField(DFT_uint32,offsetof(CustomizationOption,defaultValue),4,3);

        uint32 count = static_cast<uint32>(result->getRowCount());

        for(uint32 i = 0; i<count; i++)
        {
            CustomizationOption* cO = new(CustomizationOption);
            result->getNextRow(binding,cO);

            cO->paletteSize =  (uint32)(cO->paletteSize /200)* getCustomization();

            mManufacturingSchematic->mCustomizationList.push_back(cO);
        }
        mDatabase->destroyDataBinding(binding);

        // lets move on to stage 2, send the updates


        gMessageLib->sendCreateManufacturingSchematic(mManufacturingSchematic,mOwner);
        gMessageLib->sendCreateTano(mItem,mOwner);
        gMessageLib->sendManufactureSlots(mManufacturingSchematic,mTool,mItem,mOwner);
        gMessageLib->sendUpdateCraftingStage(mOwner);
        gMessageLib->sendBaselinesMSCO_7(mManufacturingSchematic,mOwner);

    }
    break;

    // item creation complete
    case CraftSessionQuery_Prototype:
    {
        uint32			resultId;
        DataBinding*	binding = mDatabase->createDataBinding(1);
        binding->addField(DFT_uint32,0,4);

        result->getNextRow(binding,&resultId);

        // all went well
        if(!resultId)
        {
            // ack and create it for the player
            gMessageLib->sendCraftAcknowledge(opCreatePrototypeResponse,CraftCreate_Success,qContainer->mCounter,mOwner);

            // schedule item for creation and update the tool timer
            mTool->initTimer((int32)(mManufacturingSchematic->getComplexity() * 3.0f),3000,mClock->getStoredTime());

            mTool->setAttributeIncDB("craft_tool_status","@crafting:tool_status_working");


            gMessageLib->sendUpdateTimer(mTool,mOwner);
            gWorldManager->addBusyCraftTool(mTool);

            // make sure we don't delete it on session destroy
            mTool->setCurrentItem(mItem);
            mItem = NULL;
            mManufacturingSchematic->setItem(NULL);

            // grant xp
            float xp		= 0.0f;
            float xpType	= 22.0f;

            if(mManufacturingSchematic->hasAttribute("xp"))
                xp = mManufacturingSchematic->getAttribute<float>("xp");

            if(mManufacturingSchematic->hasInternalAttribute("xp_type"))
                xpType = mManufacturingSchematic->getInternalAttribute<float>("xp_type");

            if(xp > 0.0f)
                gSkillManager->addExperience((uint32)xpType,(int32)xp,mOwner);

        }
        // something failed
        else
        {
            gMessageLib->sendCraftAcknowledge(opCreatePrototypeResponse,CraftCreate_Failure,qContainer->mCounter,mOwner);

            // end the session
            mDatabase->destroyDataBinding(binding);
            delete(qContainer);

            gCraftingSessionFactory->destroySession(this);

            return;
        }

        mDatabase->destroyDataBinding(binding);
    }
    break;

    default:
    {
        
    }
    break;
    }

    delete(qContainer);
}

//=============================================================================

void CraftingSession::handleObjectReady(Object* object,DispatchClient* client)
{

    Item* item = dynamic_cast<Item*>(object);
    if(!item)
    {   //Manufacturingschematic couldnt be created!!! Crashbug patch for: http://paste.swganh.org/viewp.php?id=20100627064849-3d026388be3dc63f7d9706f737e6d510
        LOG(WARNING) << "CraftingSession::handleObjectReady: Couldnt Cast item.";
        mManufacturingSchematic = NULL;

        gMessageLib->sendCraftAcknowledge(opCreatePrototypeResponse,CraftCreate_Failure,this->getCounter(),mOwner);
        // end the session
        gCraftingSessionFactory->destroySession(this);
        return;
    }
    // its the manufacturing schematic
    if(item->getItemFamily() == ItemFamily_ManufacturingSchematic)
    {
        //reset - we just started from scratch
        mItemLoaded = false;

        mManufacturingSchematic = dynamic_cast<ManufacturingSchematic*>(item);
        if(!mManufacturingSchematic)
        {
            LOG(WARNING) << "CraftingSession::handleObjectReady: Couldnt Cast ManufacturingSchematic.";

            if(mDraftSchematic)
            {
                LOG(WARNING) << "CraftingSession::handleObjectReady: DraftSchematic : " << mDraftSchematic->getModel().getAnsi() << " Object " << object->getId();
            }
            mManufacturingSchematic = NULL;
            gMessageLib->sendCraftAcknowledge(opCreatePrototypeResponse,CraftCreate_Failure,this->getCounter(),mOwner);
            // end the session
            gCraftingSessionFactory->destroySession(this);
            return;

        }
        mManufacturingSchematic->setComplexity((float)mDraftSchematic->getComplexity());

        // now request the (temporary) item, based on the draft schematic defaults
        gObjectFactory->requestNewDefaultItem(this, (mDraftSchematic->getId() >> 32), mTool->getId(), 99, glm::vec3());
    }
    // its the item we load
    else if(!mItemLoaded)
    {
        //mark it as loaded the next item we might receive will be a component
        mItemLoaded = true;
        mItem = item;

        // link item data
        mManufacturingSchematic->setName(mItem->getName().getAnsi());
        mManufacturingSchematic->setItemModel(mItem->getModelString().getAnsi());
        //mManufacturingSchematic->setModelString(mItem->getModelString());

        // set up initial configuration
        mManufacturingSchematic->prepareManufactureSlots();
        mManufacturingSchematic->prepareCraftingAttributes();
        mManufacturingSchematic->prepareAttributes();

        //get the man schematics skillmod Id  for experimentation
        CraftSessionQueryContainer* container = new CraftSessionQueryContainer(CraftSessionQuery_SkillmodExp,0);
        uint32 groupId = mDraftSchematic->getWeightsBatchId();


        int8 sql[550];
        sprintf(sql,"SELECT DISTINCT skills_skillmods.skillmod_id FROM %s.draft_schematics "
                    "INNER JOIN %s.skills_schematicsgranted ON draft_schematics.group_id = skills_schematicsgranted.schem_group_id "
                    "INNER JOIN %s.skills_skillmods ON skills_schematicsgranted.skill_id = skills_skillmods.skill_id "
                    "INNER JOIN %s.skillmods ON skills_skillmods.skillmod_id = skillmods.skillmod_id WHERE "
                    "draft_schematics.weightsbatch_id = %u AND skillmods.skillmod_name LIKE %s",
                    mDatabase->galaxy(),mDatabase->galaxy(),mDatabase->galaxy(),mDatabase->galaxy(),groupId,"'%%exper%%'");

        //% just upsets the standard query
        mDatabase->executeSqlAsyncNoArguments(this,container,sql);
        
        
        return;
    }
    //as the main item has been loaded we can now only receive components when we fill slots with stack-/crate- content
    else if(mItemLoaded)
    {
        uint32 maxsize = mAsyncStackSize;

        //make sure we have the proper stacksizes
        if(item->hasAttribute("stacksize"))
        {

            if(maxsize > mAsyncComponentAmount)
            {
                item->setAttribute("stacksize",boost::lexical_cast<std::string>(mAsyncComponentAmount));
                maxsize = mAsyncComponentAmount;
            }
            else
            {
                item->setAttribute("stacksize",boost::lexical_cast<std::string>(maxsize));
                mAsyncComponentAmount -= maxsize;
            }
        }
        else
        {
            // when we use a crate with components that are not stacks we add every component individually
            // only when the last component is added we want to continue execution
            mAsyncComponentAmount--;
        }

        if(!mAsyncManSlot)
        {
            LOG(WARNING) << "CraftingSession::handleObjectReady: Couldnt find Slot!!!.";

            mManufacturingSchematic = NULL;
            gMessageLib->sendCraftAcknowledge(opCreatePrototypeResponse,CraftCreate_Failure,this->getCounter(),mOwner);
            // end the session
            gCraftingSessionFactory->destroySession(this);
            return;

        }

        //add the necessary information to the slot
        mAsyncManSlot->mUsedComponentStacks.push_back(std::make_pair(item,maxsize));
        mAsyncManSlot->addComponenttoSlot(item->getId(),maxsize,mAsyncManSlot->mDraftSlot->getType());

        // update the slot total resource amount
        mAsyncManSlot->setFilledAmount(mAsyncManSlot->getFilledAmount()+maxsize);


        if(mAsyncComponentAmount == 0)
        {
            if(mAsyncManSlot->getFilledAmount() == mAsyncManSlot->mDraftSlot->getNecessaryAmount())
            {
                // update the total count of filled slots
                mManufacturingSchematic->addFilledSlot();
                gMessageLib->sendUpdateFilledManufactureSlots(mManufacturingSchematic,mOwner);
            }

            // update the slot contents, send all slots on first fill
            // we need to make sure we only update lists with changes, so the lists dont desynchronize!
            if(!mFirstFill)
            {
                mFirstFill = true;
                gMessageLib->sendDeltasMSCO_7(mManufacturingSchematic,mOwner);
            }
            else if(mAsyncSmallUpdate == true)
            {
                gMessageLib->sendManufactureSlotUpdateSmall(mManufacturingSchematic,static_cast<uint8>(mAsyncSlotId),mOwner);
            }
            else
            {
                gMessageLib->sendManufactureSlotUpdate(mManufacturingSchematic,static_cast<uint8>(mAsyncSlotId),mOwner);

                //after the first big update we want to send small ones to not desynchronize
                mAsyncSmallUpdate = true;
            }

            // done
            gMessageLib->sendCraftAcknowledge(opCraftFillSlot,CraftError_None,mAsyncCounter,mOwner);
        }


    }
}

//=============================================================================
//
// returns : true on success, false on error
//

bool CraftingSession::selectDraftSchematic(uint32 schematicIndex)
{
    // get the selected draft schematic
    SchematicsIdList*	filteredPlayerSchematics = mOwner->getFilteredSchematicsIdList();

    // invalid index
    if(filteredPlayerSchematics->empty() || filteredPlayerSchematics->size() < schematicIndex)
    {
        return(false);
    }

    // get the schematic from the filtered list
    uint32 schemCrc = ((filteredPlayerSchematics->at(schematicIndex))>>32);

    mSchematicCRC = schemCrc;

    mDraftSchematic = gSchematicManager->getSchematicBySlotId(schemCrc);

    if(!mDraftSchematic)
    {
        LOG(INFO) << "CraftingSession::selectDraftSchematic: not found crc:" << schemCrc;
        return(false);
    }

    // temporary check until all items are craftable
    if(!mDraftSchematic->isCraftEnabled())
    {
        LOG(INFO) << "CraftingSession::selectDraftSchematic: schematic not craftable crc:" <<schemCrc;
        gMessageLib->SendSystemMessage(L"This item is currently not craftable.", mOwner);
        return(true);
    }

    // advance the crafting stage
    mStage = 2;
    mOwner->setCraftingStage(mStage);

    // create the (temporary) manufacturing schematic
    gObjectFactory->requestNewDefaultManufactureSchematic(this,schemCrc,mTool->getId());

    mSubCategory = mDraftSchematic->getSubCategory();

    if(mSubCategory == 2)
    {
        setCustomization(mOwner->getSkillModValue(SMod_armor_customization));
    }
    else
        setCustomization(mOwner->getSkillModValue(SMod_clothing_customization));

    return(true);
}

//=============================================================================
//
// set object pointers to 0 previously, if we want to keep it
//
void CraftingSession::_cleanUp()
{
    // if we are in stage 2, recreate the resources that have been filled
    if(mStage == 2 && mOwner->isConnected() && mManufacturingSchematic)
    {
        FilledResources::iterator	resIt;
        ManufactureSlots::iterator	manSlotIt = mManufacturingSchematic->getManufactureSlots()->begin();

        while(manSlotIt != mManufacturingSchematic->getManufactureSlots()->end())
        {
            if((*manSlotIt)->getFilledType()== DST_Resource)
                bagResource((*manSlotIt),mOwner->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory)->getId());
            else if(((*manSlotIt)->getFilledType()== DST_SimiliarComponent)||((*manSlotIt)->getFilledType()== DST_IdentComponent))
                bagComponents((*manSlotIt),mOwner->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory)->getId());

            (*manSlotIt)->setFilledType(DST_Empty);

            ++manSlotIt;
        }
    }

    // delete the manufacture schematic
    // hark!!! only delete if we didnt create a man schematic!!!!
    if(mManufacturingSchematic)
    {
        if(!mManufacturingSchematic->mDataPadId)
        {
            gObjectFactory->deleteObjectFromDB(mManufacturingSchematic);
            gMessageLib->sendDestroyObject(mManufacturingSchematic->getId(),mOwner);
            delete(mManufacturingSchematic);
        }
    }

    // delete the item
    // please note that we need to retain the item in case we have created a manufacturing schematic!!!
    if(mItem)
    {
        if(!mManufacturingSchematic->mDataPadId)
        {
            gObjectFactory->deleteObjectFromDB(mItem);
            gMessageLib->sendDestroyObject(mItem->getId(),mOwner);
            delete(mItem);
        }
    }
}

//=============================================================================
//
// a resource or component gets put into a manufacturing slot
//

void CraftingSession::handleFillSlot(uint64 resContainerId,uint32 slotId,uint32 unknown,uint8 counter)
{
    //check whether we are dealing with a resource or a component slot
    ManufactureSlot*	manSlot			= mManufacturingSchematic->getManufactureSlots()->at(slotId);

    if (manSlot->mDraftSlot->getType() == DST_Resource)
    {
        handleFillSlotResourceRewrite(resContainerId, slotId, unknown, counter);
        //handleFillSlotResource(resContainerId, slotId, unknown, counter);
    }
    else
        handleFillSlotComponent(resContainerId, slotId, unknown, counter);
}


//=============================================================================

void CraftingSession::handleEmptySlot(uint32 slotId,uint64 containerId,uint8 counter)
{
    ManufactureSlot* manSlot = mManufacturingSchematic->getManufactureSlots()->at(slotId);

    if(manSlot)
    {
        emptySlot(slotId,manSlot,containerId);

        // done
        gMessageLib->sendCraftAcknowledge(opCraftEmptySlot,CraftError_None,counter,mOwner);
    }
    else
    {
        gMessageLib->sendCraftAcknowledge(opCraftEmptySlot,CraftError_Invalid_Slot,counter,mOwner);
    }
}


//=============================================================================
//
// initialize attribue post Processing on Assembly
// that means we possibly want to add certain attributes to our final product
// and we want to have some fixed additions to other attributes values
// please note though, that we annot experiment on these additions and that added values do not count
// towards the upper attributes cap

void CraftingSession::addComponentAttribute()
{
    // attributemanipulation through a component might be
    // 1) the manipulation of an attributes value
    // 2) the addition of an attribute


    //iterate through all Slots - in case we have several components in ONE slot only the attributes of one component apply
    uint8 amount = mManufacturingSchematic->getManufactureSlots()->size();

    for (uint8 i = 0; i < amount; i++)
    {
        //iterate through our manslots which contain components
        ManufactureSlot* manSlot = mManufacturingSchematic->getManufactureSlots()->at(i);
        if(manSlot && ((manSlot->getFilledType() == DST_IdentComponent) || (manSlot->getFilledType() == DST_SimiliarComponent)))
        {
            //in case a slot is not obligate it might be empty
            if(manSlot->mFilledResources.empty())
                continue;

            //in case a slot has several components in it were just interested in the first
            //that at least one item is in it we already established
            FilledResources::iterator resIt = manSlot->mFilledResources.begin();

            Item*	filledComponent	= dynamic_cast<Item*>(gWorldManager->getObjectById((*resIt).first));

            if(!filledComponent)
            {
                continue;
            }

            // now that we have our component we need to check for the relevant attributes
            CraftAttributeWeights* cAPP = mDraftSchematic->getCraftAttributeWeights();
            CraftAttributeWeights::iterator cAPPiT = cAPP->begin();

            while(cAPPiT != cAPP->end())
            {
                // see whether our filled component has the relevant attribute
                if(!filledComponent->hasAttribute( (*cAPPiT)->getAttributeKey().getAnsi() ))
                {
					cAPPiT++;
                    continue;
                }
                // now that we have the attribute lets check how we want to manipulate what attribute of our final item
                // here we are during the items assembly so we want to initialize value additions or add new attributes to the final item

                if((*cAPPiT)->getManipulation() == AttributePPME_AddValue)
                {
                    // just add our value to the items attribute in case the attribute on the item exists
                    // do NOT add the attribute in case it wont exist
                    if(mItem->hasAttribute( (*cAPPiT)->getAffectedAttributeKey().getAnsi() ))
                    {
                        // add the attribute (to the schematic) if it doesnt exist already to the relevant list for storage
                        // on sending the msco deltas respective producing the final items the values will be added to the attributes
                        if(mManufacturingSchematic->hasPPAttribute((*cAPPiT)->getAffectedAttributeKey()))
                        {
                            float attributeValue = filledComponent->getAttribute<float>((*cAPPiT)->getAttributeKey());
                            float attributeAddValue = mManufacturingSchematic->getPPAttribute<float>((*cAPPiT)->getAffectedAttributeKey());

                            mManufacturingSchematic->setPPAttribute((*cAPPiT)->getAffectedAttributeKey(),boost::lexical_cast<std::string>(attributeValue+attributeAddValue));
                        }

                        if(!mManufacturingSchematic->hasPPAttribute((*cAPPiT)->getAffectedAttributeKey()))
                        {
                            std::string attributeValue = filledComponent->getAttribute<std::string>((*cAPPiT)->getAttributeKey());
                            mManufacturingSchematic->addPPAttribute((*cAPPiT)->getAffectedAttributeKey(),attributeValue);
                        }

                    }
                    else
                    {
                        DLOG(INFO) << "CraftingSession::addComponentAttribute  : Attribute " << (*cAPPiT)->getAffectedAttributeKey().getAnsi()  << " is not part of the item";
                    }

                }

                if((*cAPPiT)->getManipulation() == AttributePPME_AddAttribute)
                {
                    // just add our Attribute to the item and take over the value
                    // In case the attribute exists it might already have been added by another component

                    if(!mItem->hasAttribute( (*cAPPiT)->getAffectedAttributeKey() ))
                    {
                        std::string attributeValue = filledComponent->getAttribute<std::string>((*cAPPiT)->getAttributeKey());
                        mItem->addAttribute((*cAPPiT)->getAffectedAttributeKey(),attributeValue);
                    }

                    if(mItem->hasAttribute( (*cAPPiT)->getAffectedAttributeKey() ))
                    {

                        float attributeValue = mItem->getAttribute<float>((*cAPPiT)->getAffectedAttributeKey());
                        float attributeAddValue = filledComponent->getAttribute<float>((*cAPPiT)->getAttributeKey());

                        mItem->setAttribute((*cAPPiT)->getAffectedAttributeKey(),boost::lexical_cast<std::string>(attributeValue+attributeAddValue));

                    }

                }


                cAPPiT++;
            }

        }
    }

}

//=============================================================================
//
// assemble the item(s)
//

void CraftingSession::assemble(uint32 counter)
{

    ExperimentationProperties*			expPropertiesList	= mManufacturingSchematic->getExperimentationProperties();
    ExperimentationProperties::iterator	expIt				= expPropertiesList->begin();
    mCounter = counter;

    // get the items serial
    BString serial;
    serial = getSerial();
    mItem->addAttributeIncDB("serial_number",serial.getAnsi());

    //add creator
    mItem->addAttributeIncDB("crafter",mOwner->getFirstName().getAnsi());

    int8 assRoll = _assembleRoll();

    // move to experimentation stage
    if(mOwner->getExperimentationFlag())
        mStage = 3;
    else
        mStage = 4;

    // --------------------------------------------------
    // --------------------------------------------------
    //this is a critical error
    if(assRoll == 8)
    {
        //the client forces us to stay in stage 2!!!
        mStage = 2;

        mManufacturingSchematic->ModifyBlueBars(static_cast<float>(0.9));

        //now empty the slots
        emptySlots(counter);

        gMessageLib->sendGenericIntResponse(assRoll,static_cast<uint8>(counter),mOwner);
        return;
    }

    // ---------------------------------------------
    // it wasnt a critical so go on

    // ---------------------------------------------
    // if we have components which add attributes these should now be added to the item

    addComponentAttribute();

    mOwner->setCraftingStage(mStage);
    gMessageLib->sendUpdateCraftingStage(mOwner);


    // update item complexity
    mItem->setComplexity(static_cast<float>(mManufacturingSchematic->getComplexity()));
    gMessageLib->sendUpdateComplexity(mItem,mOwner);

    // calc initial assembly percentages

    float	wrv;

    expIt	= expPropertiesList->begin();

    // ------------------------------------------------
    // to do it properly we would have to roll for every experimentation property  ???
    // in assembly, too or only for experimentation ??

    while(expIt != expPropertiesList->end())
    {
        ExperimentationProperty*	expProperty = (*expIt);

        wrv = _calcWeightedResourceValue(expProperty->mWeights);

        // max reachable percentage
        expProperty->mMaxExpValue = wrv * 0.001f;
        // we need to mark changed values so we now what list to update when we send our deltas
        mManufacturingSchematic->mMaxExpValueChange = true;

        // initial assembly percentage
        expProperty->mExpAttributeValue = ((0.00000015f * (wrv*wrv)) + (0.00015f * wrv));

        // update the items attributes
        CraftAttributes::iterator caIt = expProperty->mAttributes->begin();

        // one exp attribute can have several item attributes!!!
        // however if the itemattributes have different weights we will introduce an additional
        // exp attribute - the msco code will sort that out so that the exp attribute is only send once!
        while(caIt != expProperty->mAttributes->end())
        {
            CraftAttribute* att = (*caIt);
            float attValue	= att->getMin() + ((att->getMax() - att->getMin()) * expProperty->mExpAttributeValue);

            //modify the attribute for the item - in memory AND db
            modifyAttributeValue((*caIt),attValue);

            ++caIt;
        }

        ++expIt;
    }

    // send assembly attributes and results

    gMessageLib->sendDeltasMSCO_3(mManufacturingSchematic,mOwner);
    gMessageLib->sendAttributeDeltasMSCO_7(mManufacturingSchematic,mOwner);

    gMessageLib->sendGenericIntResponse(assRoll,static_cast<uint8>(counter),mOwner);
}

//=============================================================================

void CraftingSession::customizationStage(uint32 counter)
{
    mStage = 5;
    mCounter = counter;
    mOwner->setCraftingStage(mStage);
    gMessageLib->sendUpdateCraftingStage(mOwner);

    gMessageLib->sendGenericIntResponse(4,static_cast<uint8>(counter),mOwner);
}

//=============================================================================

void CraftingSession::creationStage(uint32 counter)
{
    mStage = 5;
    mCounter = counter;
    mOwner->setCraftingStage(mStage);
    gMessageLib->sendUpdateCraftingStage(mOwner);

    gMessageLib->sendGenericIntResponse(4,static_cast<uint8>(counter),mOwner);
}

//=============================================================================

void CraftingSession::experimentationStage(uint32 counter)
{
    mStage = 4;
    mCounter = counter;

    mOwner->setCraftingStage(mStage);
    gMessageLib->sendUpdateCraftingStage(mOwner);

    gMessageLib->sendGenericIntResponse(4,static_cast<uint8>(counter),mOwner);
}

//=============================================================================

void CraftingSession::customize(const int8* itemName)
{
    mItem->setCustomName(itemName);

    gMessageLib->sendCraftAcknowledge(opCraftCustomization,CraftError_None,0,mOwner);
}



//=============================================================================

void CraftingSession::createPrototype(uint32 noPractice,uint32 counter)
{
    if(noPractice)
    {
        int8 sql[1024],restStr[128],*sqlPointer;

        Transaction* t = mDatabase->startTransaction(this,new CraftSessionQueryContainer(CraftSessionQuery_Prototype,static_cast<uint8>(counter)));

        // we need to alter / add attributes affected by attributes of components


        // update the custom name and parent
        sprintf(sql,"UPDATE %s.items SET parent_id=%" PRIu64 ", customName='",mDatabase->galaxy(),mOwner->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory)->getId());
        sqlPointer = sql + strlen(sql);
        sqlPointer += mDatabase->escapeString(sqlPointer,mItem->getCustomName().getAnsi(),mItem->getCustomName().getLength());
        sprintf(restStr,"' WHERE id=%" PRIu64 " ",mItem->getId());
        strcat(sql,restStr);

        t->addQuery(sql);
        t->execute();
        

        // add the crafter name attribute
        // adds automatically when necessary
        mItem->setAttributeIncDB("crafter",mOwner->getFirstName().getAnsi());

        // now the serial
        BString serial = getSerial();

        // adds automatically when necessary
        mItem->setAttributeIncDB("serial_number",serial.getAnsi());

    }
    else
    {
        float xp = 0.0f;
        float xpType = 22.0f;

        if(mManufacturingSchematic->hasAttribute("xp"))
            xp = mManufacturingSchematic->getAttribute<float>("xp");

        if(mManufacturingSchematic->hasInternalAttribute("xp_type"))
            xpType = mManufacturingSchematic->getInternalAttribute<float>("xp_type");

        // add some cooldown time
        mTool->initTimer((int32)(mManufacturingSchematic->getComplexity() * 2.0f),3000,mClock->getLocalTime());

        mTool->setAttributeIncDB("craft_tool_status","@crafting:tool_status_working");

        gMessageLib->sendUpdateTimer(mTool,mOwner);
        gWorldManager->addBusyCraftTool(mTool);

        // just ack
        gMessageLib->sendCraftAcknowledge(opCreatePrototypeResponse,CraftCreate_Success,static_cast<uint8>(counter),mOwner);

        // grant some additional xp
        xp *= 1.1f;

        // update xp
        if(xp > 0.0f)
            gSkillManager->addExperience((uint32)xpType,(int32)xp,mOwner);
    }

    destroyComponents();
}



//=============================================================================

void CraftingSession::experiment(uint8 counter,std::vector<std::pair<uint32,uint32> > properties)
{
    // a list containing every exp property ONCE (the first if one is represented several times)
    ExperimentationPropertiesStore*		expPropList = mManufacturingSchematic->getExperimentationPropertiesStore();

    // a list containing ALL exp properties - including the ones appearing more often than once
    ExperimentationProperties*			expAllProps = mManufacturingSchematic->getExperimentationProperties();
    ExperimentationProperties::iterator itAll		=	 expAllProps->begin();


    uint32				accumulatedRoll = 0;//here we accumulate the roll results
    uint8				roll = 0;				//thats the experimentation roll
    uint8				rollCount = 0;		//thats the amount of rolls made
    uint32				expPoints = 0;

    //this is the list containing the assigned experimentation points
    std::vector<std::pair<uint32,uint32> >::iterator it = properties.begin();

    // collect the total of spend exp points
    while(it != properties.end())
    {
        expPoints +=(*it).first;

        ++it;
    }
    // update left exp points
    mOwner->setExperimentationPoints(mOwner->getExperimentationPoints() - expPoints);

    //items complexity increases per experimentation
    mManufacturingSchematic->incComplexity();
    mItem->setComplexity(static_cast<float>(mManufacturingSchematic->getComplexity()));
    gMessageLib->sendUpdateComplexity(mItem,mOwner);

    // initialize our storage value so we know what we already experimented on
    // use the list containing ALL properties
    itAll =	 expAllProps->begin();
    while(itAll != expAllProps->end())
    {
        ExperimentationProperty* expProperty = (*itAll);
        expProperty->mRoll = -1;
        itAll++;
    }


    // update expAttributes
    it			= properties.begin();
    expPoints	= 0;


    while(it != properties.end())
    {
        expPoints = (*it).first;
        ExperimentationProperty* expProperty = expPropList->at((*it).second).second;

        // make sure that we only experiment once for exp properties that might be entered twice in our list !!!!
        roll = getExperimentationRoll(expProperty,expPoints);

        //get the mean roll for our status message
        rollCount++;
        accumulatedRoll		+= (uint32)roll;

        //the percentage gets added to / deducted from our attributes value
        float modifier	= getPercentage(roll);

        //the bluebar gives the max amount of experimentation we can do
        float malus = 1.0f + modifier;
        expProperty->mBlueBarSize = (expProperty->mBlueBarSize * malus);

        expProperty->mExpAttributeValue += modifier * (*it).first;

        // update item attributes
        CraftAttributes::iterator caIt = expProperty->mAttributes->begin();

        //one exp attribute can have several item attributes!!!
        while(caIt != expProperty->mAttributes->end())
        {
            CraftAttribute* att = (*caIt);

            float attValue;
            if(att->getMin() > att->getMax())
            {
                attValue	= att->getMin() - ((att->getMin() - att->getMax()) * expProperty->mExpAttributeValue);

                if(attValue > att->getMin())
                    attValue = att->getMin();

            }
            else
            {
                attValue	= att->getMin() + ((att->getMax() - att->getMin()) * expProperty->mExpAttributeValue);

                if(attValue > att->getMax())
                    attValue = att->getMax();
            }

            //modify the attributes value in memory and db depending on float or integer value
            modifyAttributeValue((*caIt),attValue);
            ++caIt;
        }

        ++it;
    }


    // get the gross of our exp rolls for the message
    roll = (uint8)   accumulatedRoll/rollCount;

    // send updates
    gMessageLib->sendAttributeDeltasMSCO_7(mManufacturingSchematic,mOwner);

    //update the schematics complexity
    gMessageLib->sendMSCO_3_ComplexityUpdate(mManufacturingSchematic,mOwner);
    //gMessageLib->sendDeltasMSCO_3(mManufacturingSchematic,mOwner);
    gMessageLib->sendUpdateExperimentationPoints(mOwner);

    // ack
    gMessageLib->sendCraftExperimentResponse(opCraftExperiment,roll,counter,mOwner);
}


//=============================================================================
//
// calculate the maximum reachable percentage through assembly with the filled resources
//

float CraftingSession::_calcWeightedResourceValue(CraftWeights* weights)
{
    FilledResources::iterator	filledResIt;
    ManufactureSlots*			manSlots	= mManufacturingSchematic->getManufactureSlots();
    ManufactureSlots::iterator	manIt		= manSlots->begin();
    CraftWeights::iterator		weightIt;
    float						wrv			= 0.0f;
    CraftWeight*				craftWeight;
    ManufactureSlot*			manSlot;
    Resource*					resource;
    uint16						resAtt;
    float						totalQuantity	= 0.0f;
    //float						subQuantity		= 0.0f;
    float						totalResStat	= 0.0f;
    //float						validCount		= 0.0f;
    float						wrvResult		= 0.0f;
    bool						slotCounted;
    float						slotCount		= 0.0f;

    while(manIt != manSlots->end())
    {
        manSlot		= (*manIt);
        weightIt	= weights->begin();
        slotCounted	= false;

        // skip if its a sub component slot
        if(manSlot->mDraftSlot->getType() != DST_Resource)
        {
            ++manIt;
            continue;
        }

        // we limit it so that only the same resource can go into one slot, so grab only the first entry
        filledResIt		= manSlot->mFilledResources.begin();

        if(manSlot->mFilledResources.size() == 0)
        {
            //PANICK - theres no resource filled !!!!!!!!!!!!!!!!!!!!!!!!!!
            DLOG(INFO) << "CraftingSession::_calcWeightedResourceValue: NO REOURCE IN RESOURCE SLOT :";

            if(manSlot->mDraftSlot->getOptional())
            {
                DLOG(INFO) << "CraftingSession::_calcWeightedResourceValue: SLOT WAS OPTIONAL:";
            }
            assert(false&&"CraftingSession::_calcWeightedResourceValue: NO RESSOURCE IN RESOURCE SLOT ");
            ++manIt;
            continue;
        }

        resource		= gResourceManager->getResourceById((*filledResIt).first);
        totalResStat	= 0.0f;
        totalQuantity	= 0.0f;

        while(weightIt != weights->end())
        {
            craftWeight = (*weightIt);
            resAtt		= resource->getAttribute(craftWeight->getDataType());
            //printf("\ndatatype : %u\n",craftWeight->getDataType());

            if(resAtt)
            {
                if(!slotCounted)
                {
                    slotCount	+= 1.0f;
                    slotCounted = true;
                }

                totalResStat	+= (float)((manSlot->mDraftSlot->getNecessaryAmount() * resAtt) * craftWeight->getDistribution());
                totalQuantity	+= (float)(manSlot->mDraftSlot->getNecessaryAmount() * craftWeight->getDistribution());
                //printf("distribution : %f\n",craftWeight->getDistribution());
            }

            ++weightIt;
        }

        if(totalResStat && totalQuantity)
            wrvResult += (totalResStat / totalQuantity);

        ++manIt;
    }

    if(wrvResult && slotCount)
        wrv = wrvResult / slotCount;

    return(wrv);
}

//=============================================================================

void CraftingSession::createManufactureSchematic(uint32 counter)
{
    //Man schematic attributes
    //The manufacture schematic attributes are not needed anymore when the crafting process is dealt with
    //as the item attributes will then have been properly created
    //this means we can safely delete them and put the datapad associated attributes there
    //as long as we make sure that the message lib will not send those attributes in the MSCO3 when
    //we have a datapad associated man schematic

    //set the datapad Id of the man schematic so it wont get deleted
    Datapad* datapad = mOwner->getDataPad();
    mManufacturingSchematic->setParentId(datapad->getId());
    mManufacturingSchematic->mDataPadId = datapad->getId();

    if(!datapad->addManufacturingSchematic(mManufacturingSchematic))
    {
        //TODO
        //delete the man schem from the objectlist and the db
        gObjectFactory->deleteObjectFromDB(mItem);
        gObjectFactory->deleteObjectFromDB(mManufacturingSchematic);

        gWorldManager->destroyObject(mManufacturingSchematic);
        SAFE_DELETE(mManufacturingSchematic);
        mManufacturingSchematic = NULL;

        gWorldManager->destroyObject(mItem);
        SAFE_DELETE(mItem);
        mItem= NULL;

        return;
    }

    mManufacturingSchematic->setCustomName(mItem->getCustomName().getAnsi());

    //now delete the old object client side and create it new
    gMessageLib->sendDestroyObject(mManufacturingSchematic->getId(),mOwner);
    gMessageLib->sendCreateManufacturingSchematic(mManufacturingSchematic,mOwner,false);

    //delete the old attributes db side and object side
    AttributeMap* map = mManufacturingSchematic->getAttributeMap();
    map->clear();

    map = mManufacturingSchematic->getInternalAttributeMap();
    map->clear();

    AttributeOrderList*	list = 	mManufacturingSchematic->getAttributeOrder();
    list->clear();

    mDatabase->executeSqlAsync(0,0,"DELETE FROM %s.item_attributes WHERE item_id=%" PRIu64 "",mDatabase->galaxy(),mManufacturingSchematic->getId());


    //save the datapad as the Owner Id in the db
    mManufacturingSchematic->setParentIdIncDB(datapad->getId());

    //set the custom name of the object
    mManufacturingSchematic->setCustomNameIncDB(mItem->getCustomName().getAnsi());

    //set the schematic as parent id for our item - we need it as dummy!!!
    mItem->setParentIdIncDB(mManufacturingSchematic->getId());

    mManufacturingSchematic->setItem(mItem);

    //Now enter the relevant information into the Manufactureschematic table
    std::string serial = mItem->getAttribute<std::string>("serial_number");

    mDatabase->executeSqlAsync(0, 0, "INSERT INTO %s.manufactureschematic VALUES (%" PRIu64 ",%u,%u,%" PRIu64 ",'%s',%f)",mDatabase->galaxy(),mManufacturingSchematic->getId(),this->getProductionAmount(),this->mSchematicCRC,mItem->getId(),serial.c_str(),mManufacturingSchematic->getComplexity());


    //save the customization - thats part of the item!!!!

    //add datapadsize
    //data size - hardcode to 1 for now not sure whether there are different sizes
    mManufacturingSchematic->addAttributeIncDB("data_volume","1");

    //save the resource Information as atributes
    // please note that the attribute 173 cat_manf_schem_ing_ressource or _component cannot be entered several times in our table
    // as we cannot store several entries with the same key in our hashmaps
    //alternatively we have to create attributes with the res name in them and check whether it already exists or not

    //establishes a resourcelist and adds it to the schematic
    collectResources();

    //establishes a componentlist and adds it to the schematic
    collectComponents();

    //manufacturing limit
    BString limit = boost::lexical_cast<std::string>(this->getProductionAmount()).c_str();

    mManufacturingSchematic->addAttributeIncDB("manf_limit",limit.getAnsi());


    BString mask = boost::lexical_cast<std::string>(mDraftSchematic->getSubCategory()).c_str();
    mManufacturingSchematic->addInternalAttributeIncDB("craft_tool_typemask",mask.getAnsi());

    gMessageLib->sendCraftAcknowledge(opCreatePrototypeResponse,CraftCreate_2,static_cast<uint8>(counter),mOwner);

    destroyComponents();
}
