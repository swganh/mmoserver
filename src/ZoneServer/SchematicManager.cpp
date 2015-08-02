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

#include "SchematicManager.h"


#include "Utils/logger.h"

#include "CraftBatch.h"
#include "DraftSchematic.h"
#include "DraftSlot.h"
#include "DraftWeight.h"
#include "SchematicGroup.h"
#include "WeightsBatch.h"

#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"

//======================================================================================================================

bool SchematicManager::mInsFlag = false;
SchematicManager* SchematicManager::mSingleton = NULL;

//======================================================================================================================

SchematicManager::SchematicManager(Database* database)
    : mDBAsyncPool(sizeof(ScMAsyncContainer))
    , mDatabase(database)
    , mGroupCount(0)
    , mGroupLoadCount(0)
    , mSchematicCount(0)
{
    // load skillschematicgroups
    //gLogger->log(LogManager::DEBUG,"Started Loading Schematic Groups.");
    mDatabase->executeSqlAsync(this,new(mDBAsyncPool.ordered_malloc()) ScMAsyncContainer(ScMQuery_SchematicGroups),"SELECT * FROM %s.schematic_groups ORDER BY id",mDatabase->galaxy());
    

    // load experimentation groups
    //gLogger->log(LogManager::DEBUG,"Finished Loading Experimentation Groups.");
    mDatabase->executeSqlAsync(this,new(mDBAsyncPool.ordered_malloc()) ScMAsyncContainer(ScMQuery_ExperimentationGroups),"SELECT * FROM %s.draft_experiment_groups ORDER BY id",mDatabase->galaxy());
    
}

//======================================================================================================================

SchematicManager* SchematicManager::Init(Database* database)
{
    if(mInsFlag == false)
    {
        mSingleton = new SchematicManager(database);
        mInsFlag = true;
        return mSingleton;
    }
    else
        return mSingleton;
}

//======================================================================================================================

SchematicManager::~SchematicManager()
{
    SchematicGroupList::iterator it = mSchematicGroupList.begin();
    while(it != mSchematicGroupList.end())
    {
        delete(*it);
        mSchematicGroupList.erase(it);
        it = mSchematicGroupList.begin();
    }

    mInsFlag = false;
    delete(mSingleton);
}

//======================================================================================================================

void SchematicManager::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
    ScMAsyncContainer* asyncContainer = reinterpret_cast<ScMAsyncContainer*>(ref);

    switch(asyncContainer->mQueryType)
    {
        // index table for experimentation properties
    case ScMQuery_ExperimentationGroups:
    {
        BString expGroup;
        DataBinding* binding = mDatabase->createDataBinding(1);
        binding->addField(DFT_bstring,0,64,1);

        uint64 count = result->getRowCount();
        mvExpGroups.reserve((uint32)count);

        for(uint64 i = 0; i < count; i++)
        {
            result->getNextRow(binding,&expGroup);
            mvExpGroups.push_back(expGroup.getAnsi());
        }

        mDatabase->destroyDataBinding(binding);
        //gLogger->log(LogManager::DEBUG,"Finished Loading Experimentation Groups.");
    }
    break;

    case ScMQuery_SchematicGroups:
    {
        SchematicGroup* scGroup;
        DataBinding* binding = mDatabase->createDataBinding(2);
        binding->addField(DFT_uint32,offsetof(SchematicGroup,mId),4,0);
        binding->addField(DFT_bstring,offsetof(SchematicGroup,mName),64,1);

        uint64 count = mGroupCount = mGroupLoadCount = static_cast<uint32>(result->getRowCount());
        mSchematicGroupList.reserve((uint32)count);

        for(uint64 i = 0; i < count; i++)
        {
            scGroup = new SchematicGroup();

            result->getNextRow(binding,scGroup);
            mSchematicGroupList.push_back(scGroup);
        }

        mDatabase->destroyDataBinding(binding);
        ScMAsyncContainer* asContainer;

        DLOG(INFO) <<  "Started Loading Schematics";
        asContainer = new(mDBAsyncPool.ordered_malloc()) ScMAsyncContainer(ScMQuery_GroupSchematics);
        mDatabase->executeSqlAsync(this,asContainer,"SELECT object_string,weightsbatch_id,complexity,datasize,subCategory,craftEnabled,group_id FROM %s.draft_schematics",mDatabase->galaxy());
    }
    break;

    case ScMQuery_GroupSchematics:
    {
        DraftSchematic*	schematic = NULL;

        DataBinding* binding = mDatabase->createDataBinding(7);
        binding->addField(DFT_bstring,offsetof(DraftSchematic,mModel),128,0);
        binding->addField(DFT_uint32,offsetof(DraftSchematic,mWeightsBatchId),4,1);
        binding->addField(DFT_uint32,offsetof(DraftSchematic,mComplexity),4,2);
        binding->addField(DFT_uint32,offsetof(DraftSchematic,mDataSize),4,3);
        binding->addField(DFT_uint32,offsetof(DraftSchematic,mSubCategory),4,4);
        binding->addField(DFT_uint8,offsetof(DraftSchematic,mCraftEnabled),1,5);
        binding->addField(DFT_uint32,offsetof(DraftSchematic,mGroupId),4,6);

        uint64 count = result->getRowCount();
        //mSchematicList.reserve((uint32)count);
        uint32 num = 0;
        for(uint64 i = 0; i < count; i++)
        {
            schematic = new DraftSchematic();

            result->getNextRow(binding,schematic);

            // gotta get shared_ into the name
            BStringVector splits;
            int elements = schematic->mModel.split(splits,'/');

            if(elements < 3)
            {
            	LOG(ERR) << "Error in Schematic String";
                break;
            }

            //uint32 size =   schematic->mModel.getLength();
            schematic->mModel.setLength(256);
            schematic->mModel = splits[0].getAnsi();

            for(int j = 1; j < elements - 1; j++)
                schematic->mModel << "/" << splits[j].getAnsi();

            schematic->mModel << "/shared_" << splits[elements-1].getAnsi();

            // glue our ids and insert into maps
            uint64 schemId = schematic->mModel.getCrc();
            schematic->setId((schemId << 32) | (schematic->mWeightsBatchId));
            //added temporary log in order to dump schematic ids for DB insertion
            //gLogger->log(LogManager::DEBUG,"Schematic,%" PRIu64 ",%u",schematic->mId,schematic->mWeightsBatchId);
            //mSchematicList.push_back(schematic);

            mSchematicList.insert(std::make_pair(schematic->getWeightsBatchId(), schematic));
            mSchematicGroupList[schematic->getGroupId()-1]->mSchematics.push_back(schematic);
            mSchematicSlotMap.insert(std::make_pair(schematic->mModel.getCrc(),schematic));
            mSchematicWeightMap.insert(std::make_pair((schematic->mWeightsBatchId ),schematic));
            num++;
        }

        mSchematicCount += static_cast<uint32>(count);

        DLOG(INFO) << "Started Loading Schematic Draft Slots.";
        // now query the draftslots
        ScMAsyncContainer* asContainer = new(mDBAsyncPool.ordered_malloc()) ScMAsyncContainer(ScMQuery_SchematicSlots);
        int8 sql[2048];
        sprintf(sql,"SELECT draft_slots.component_file, draft_slots.component_name, draft_slots.resource_name, draft_slots.amount, draft_slots.optional, draft_slots.`type`, draft_schematics.weightsbatch_id"
                " FROM %s.draft_slots"
                " INNER JOIN %s.draft_schematics_slots ON (draft_slots.id = draft_schematics_slots.draft_slot_id)"
                " INNER JOIN %s.schem_crc ON (draft_schematics_slots.schematic_id = schem_crc.crc)"
                " INNER JOIN %s.draft_schematics ON (schem_crc.object_string = draft_schematics.object_string)",
                mDatabase->galaxy(),mDatabase->galaxy(),mDatabase->galaxy(),mDatabase->galaxy());
        mDatabase->executeSqlAsync(this,asContainer,sql);
        


        DLOG(INFO) << "Started Loading Schematic Assembly Batches.";
        // assemblybatches
        asContainer = new(mDBAsyncPool.ordered_malloc()) ScMAsyncContainer(ScMQuery_SchematicAssemblyBatches);
        sprintf(sql,"SELECT draft_assembly_batches.id,draft_assembly_batches.list_id,draft_weights.id"
                " FROM %s.draft_weights"
                " INNER JOIN %s.draft_assembly_batches ON (draft_weights.assembly_batch_id = draft_assembly_batches.id)"
                " ORDER BY draft_weights.id",
                mDatabase->galaxy(),mDatabase->galaxy());
        mDatabase->executeSqlAsync(this,asContainer,sql);
        

        DLOG(INFO) << "Started Loading Schematic Experimentation Batches.";
        // experimentbatches
        asContainer = new(mDBAsyncPool.ordered_malloc()) ScMAsyncContainer(ScMQuery_SchematicExperimentBatches);
        sprintf(sql,"SELECT draft_experiment_batches.id,draft_experiment_batches.list_id,draft_schematics.weightsbatch_id"
                " FROM %s.draft_weights "
                " INNER JOIN %s.draft_experiment_batches ON (draft_weights.experiment_batch_id = draft_experiment_batches.id) "
                " INNER JOIN %s.draft_schematics ON(draft_weights.id = draft_schematics.weightsbatch_id) "
                " ORDER BY draft_experiment_batches.list_id ",
                mDatabase->galaxy(),mDatabase->galaxy(),mDatabase->galaxy());

        mDatabase->executeSqlAsync(this,asContainer,sql);
        

        DLOG(INFO) << "Started Loading Schematic Crafting Batches.";
        // craftingbatches
        asContainer = new(mDBAsyncPool.ordered_malloc()) ScMAsyncContainer(ScMQuery_SchematicCraftBatches);
        sprintf(sql,"SELECT draft_craft_batches.id,draft_craft_batches.list_id,draft_craft_batches.expGroup,draft_schematics.weightsbatch_id"
                " FROM %s.draft_weights "
                " INNER JOIN %s.draft_craft_batches ON (draft_weights.craft_batch_id = draft_craft_batches.id) "
                " INNER JOIN %s.draft_schematics ON(draft_weights.id = draft_schematics.weightsbatch_id) "
                " ORDER BY draft_craft_batches.list_id ",
                mDatabase->galaxy(),mDatabase->galaxy(),mDatabase->galaxy());
        mDatabase->executeSqlAsync(this,asContainer,sql);
        

        if(!--mGroupLoadCount)
        {
            DLOG(INFO) << "Finished Loading " << num << " Groups and Schematics out of " << count;


        }

        mDatabase->destroyDataBinding(binding);
    }
    break;

    case ScMQuery_SchematicSlots:
    {
        DraftSchematic*	schematic = NULL;//asyncContainer->mSchematic;
        DraftSlot*		slot;

        DataBinding* binding = mDatabase->createDataBinding(7);
        binding->addField(DFT_bstring,offsetof(DraftSlot,mFile),64,0);
        binding->addField(DFT_bstring,offsetof(DraftSlot,mName),64,1);
        binding->addField(DFT_bstring,offsetof(DraftSlot,mResourceName),128,2);
        binding->addField(DFT_uint32,offsetof(DraftSlot,mAmount),4,3);
        binding->addField(DFT_uint8,offsetof(DraftSlot,mOptional),1,4);
        binding->addField(DFT_uint8,offsetof(DraftSlot,mType),1,5);
        binding->addField(DFT_uint32,offsetof(DraftSlot,mSchemWeightBatch),4,6);

        uint64 count = result->getRowCount();
        uint32 num=0;
        for(uint64 i = 0; i < count; i++)
        {
            slot = new DraftSlot();
            result->getNextRow(binding,slot);

            if((schematic == NULL) || (schematic->getWeightsBatchId() != slot->getSchemWeightBatch()))
            {
                schematic = getSchematicByWeightID(slot->getSchemWeightBatch());
            }
            if(schematic != NULL)
            {
                schematic->mDraftSlots.push_back(slot);
                num++;
            }

            slot->mResourceName.convert(BSTRType_Unicode16);
        }

        mDatabase->destroyDataBinding(binding);
        //gLogger->log(LogManager::DEBUG,"Finished Loading %u Schematic Draft Slots out of %u.",num,count);
    }
    break;

    case ScMQuery_SchematicAssemblyBatches:
    {
        DraftSchematic*	schematic = NULL;//asyncContainer->mSchematic;
        WeightsBatch*	batch;

        DataBinding* binding = mDatabase->createDataBinding(3);
        binding->addField(DFT_uint32,offsetof(WeightsBatch,mId),4,0);
        binding->addField(DFT_uint32,offsetof(WeightsBatch,mListId),4,1);
        binding->addField(DFT_uint32,offsetof(WeightsBatch,mSchemWeightBatch),4,2);

        uint64 count = result->getRowCount();
        uint32 num=0;
        for(uint64 i = 0; i < count; i++)
        {
            batch = new WeightsBatch();

            result->getNextRow(binding,batch);
            if((schematic == NULL) || (schematic->getId() != batch->getSchemWeightBatch()))
            {
                schematic = getSchematicByWeightID(batch->getSchemWeightBatch());
            }
            if(schematic != NULL)
            {
                schematic->mAssemblyBatch.push_back(batch);
                num++;
            }


        }
        // query list items
        ScMAsyncContainer* asContainer = new(mDBAsyncPool.ordered_malloc()) ScMAsyncContainer(ScMQuery_SchematicAssemblyWeights);
        
        mDatabase->executeSqlAsync(this,asContainer,"SELECT datatype,distribution,draft_weights.id,draft_assembly_batches.list_id"
                                   " FROM %s.draft_assembly_lists"
                                   " INNER JOIN %s.draft_assembly_batches ON(draft_assembly_batches.list_id = draft_assembly_lists.id)"
                                   " INNER JOIN %s.draft_weights ON(draft_weights.assembly_batch_id = draft_assembly_batches.id)"
                                   " ORDER BY draft_weights.id",
                                   mDatabase->galaxy(),mDatabase->galaxy(),mDatabase->galaxy());
        mDatabase->destroyDataBinding(binding);
        }
    break;

    case ScMQuery_SchematicExperimentBatches:
    {
        DraftSchematic*	schematic = NULL;//asyncContainer->mSchematic;
        WeightsBatch*	batch;

        DataBinding* binding = mDatabase->createDataBinding(3);
        binding->addField(DFT_uint32,offsetof(WeightsBatch,mId),4,0);
        binding->addField(DFT_uint32,offsetof(WeightsBatch,mListId),4,1);
        binding->addField(DFT_uint32,offsetof(WeightsBatch,mSchemWeightBatch),4,2);

        uint64 count = result->getRowCount();
        uint32 num=0;
        for(uint64 i = 0; i < count; i++)
        {
            batch = new WeightsBatch();

            result->getNextRow(binding,batch);
            if((schematic == NULL) || (schematic->getId() != batch->getSchemWeightBatch()))
            {
                schematic = getSchematicByWeightID(batch->getSchemWeightBatch());
            }
            if(schematic != NULL)
            {
                schematic->mExperimentBatch.push_back(batch);
                num++;
            }


        }
        // query list items
        ScMAsyncContainer* asContainer = new(mDBAsyncPool.ordered_malloc()) ScMAsyncContainer(ScMQuery_SchematicExperimentWeights);
        mDatabase->executeSqlAsync(this,asContainer,"SELECT datatype,distribution,draft_weights.id,draft_experiment_batches.list_id "
                                   " FROM %s.draft_experiment_lists "
                                   " INNER JOIN %s.draft_experiment_batches ON(draft_experiment_batches.list_id = draft_experiment_lists.id)"
                                   " INNER JOIN %s.draft_weights ON (draft_weights.experiment_batch_id = draft_experiment_batches.id)"
                                   " ORDER BY draft_weights.id",
                                   mDatabase->galaxy(),mDatabase->galaxy(),mDatabase->galaxy());
       
        mDatabase->destroyDataBinding(binding);
    }
    break;

    case ScMQuery_SchematicCraftBatches:
    {
        DraftSchematic*	schematic = NULL;//asyncContainer->mSchematic;
        CraftBatch*		batch;

        DataBinding* binding = mDatabase->createDataBinding(4);
        binding->addField(DFT_uint32,offsetof(CraftBatch,mId),4,0);
        binding->addField(DFT_uint32,offsetof(CraftBatch,mListId),4,1);
        binding->addField(DFT_uint32,offsetof(CraftBatch,mExpGroup),4,2);
        binding->addField(DFT_uint32,offsetof(CraftBatch,mSchemWeightBatch),4,3);

        uint64 count = result->getRowCount();
        uint32 num=0;
        for(uint64 i = 0; i < count; i++)
        {
            batch = new CraftBatch();

            result->getNextRow(binding,batch);
            if((schematic == NULL) || (schematic->getId() != batch->getSchemWeightBatch()))
            {
                schematic = getSchematicByWeightID(batch->getSchemWeightBatch());
            }
            if(schematic != NULL)
            {
                schematic->mCraftBatch.push_back(batch);
                num++;
            }
        }

        // query weight distribution
        ScMAsyncContainer* asContainer = new(mDBAsyncPool.ordered_malloc()) ScMAsyncContainer(ScMQuery_SchematicCraftWeights);
        mDatabase->executeSqlAsync(this,asContainer,"SELECT type,distribution,draft_weights.id,draft_craft_batches.list_id "
                                   " FROM %s.draft_craft_attribute_weights"
                                   " INNER JOIN %s.draft_craft_batches ON(draft_craft_attribute_weights.id = draft_craft_batches.list_id)"
                                   " INNER JOIN %s.draft_weights ON(draft_weights.craft_batch_id = draft_craft_batches.id)"
                                   " ORDER BY draft_weights.id",
                                   mDatabase->galaxy(),mDatabase->galaxy(),mDatabase->galaxy());
       



        // query attribute links and ranges
        asContainer = new(mDBAsyncPool.ordered_malloc()) ScMAsyncContainer(ScMQuery_SchematicCraftAttributeLinks);
        mDatabase->executeSqlAsync(this,asContainer,
                                   "SELECT attributes.name,dcial.item_attribute,dcial.attribute_min,dcial.attribute_max,dcial.attribute_type,draft_craft_batches.id,dcial.list_id "
                                   " FROM %s.draft_craft_item_attribute_link as dcial"
                                   " INNER JOIN %s.attributes ON (dcial.item_attribute = attributes.id)"
                                   " INNER JOIN %s.draft_craft_batches ON(dcial.list_id = draft_craft_batches.list_id)"
                                   " ORDER BY draft_craft_batches.id",
                                   mDatabase->galaxy(),mDatabase->galaxy(),mDatabase->galaxy());
        

        // query attribute weighting for component crafting
        asContainer = new(mDBAsyncPool.ordered_malloc()) ScMAsyncContainer(ScMQuery_SchematicCraftAttributeWeights);
        //gLogger->log(LogManager::DEBUG,"Started Loading Schematic Craft Attribute Weights.");
        mDatabase->executeSqlAsync(this,asContainer,
                                   "SELECT dsam.Attribute, dsam.AffectedAttribute, dsam.Manipulation, a.name, b.name,draft_schematics.weightsbatch_id "
                                   " FROM %s.draft_schematic_attribute_manipulation as dsam"
                                   " INNER JOIN %s.attributes as a ON (dsam.attribute = a.id)"
                                   " INNER JOIN %s.attributes as b ON (dsam.affectedattribute = b.id)"
                                   " INNER JOIN %s.draft_schematics ON(dsam.Draft_Schematic = draft_schematics.weightsbatch_id)",
                                   mDatabase->galaxy(),mDatabase->galaxy(),mDatabase->galaxy(),mDatabase->galaxy());
        mDatabase->destroyDataBinding(binding);
       
        //gLogger->log(LogManager::DEBUG,"Finished Loading %u Schematic Crafting Batches out of %u.",num,count);
    }
    break;

    case ScMQuery_SchematicCraftAttributeWeights:
    {
        // "SELECT dsam.Attribute, dsam.AffectedAttribute, dsam.Manipulation, a.name, b.name "

        DraftSchematic*	schematic = NULL;//asyncContainer->mSchematic;
        CraftAttributeWeight*	craftAttributeWeight;

        DataBinding* binding = mDatabase->createDataBinding(6);

        binding->addField(DFT_uint32,offsetof(CraftAttributeWeight,mAttributeId),4,0);
        binding->addField(DFT_uint32,offsetof(CraftAttributeWeight,mAffectedAttributeId),4,1);
        binding->addField(DFT_uint32,offsetof(CraftAttributeWeight,mManipulation),4,2);
        binding->addField(DFT_bstring,offsetof(CraftAttributeWeight,mAttributeKey),255,3);
        binding->addField(DFT_bstring,offsetof(CraftAttributeWeight,mAffectedAttributeKey),255,4);
        binding->addField(DFT_uint32,offsetof(CraftAttributeWeight,mSchemWeightBatch),4,5);

        uint64 count = result->getRowCount();
        uint32 num = 0;
        for(uint64 i = 0; i < count; i++)
        {
            craftAttributeWeight = new CraftAttributeWeight();

            result->getNextRow(binding,craftAttributeWeight);
            if((schematic == NULL) || (schematic->getId() != craftAttributeWeight->getSchemWeightBatch()))
            {
                schematic = getSchematicByWeightID(craftAttributeWeight->getSchemWeightBatch());
            }
            if(schematic != NULL)
            {
                //CraftBatch* batch = schematic->getCraftBatchByListId(asyncContainer->mBatchId);
                schematic->mAttributeWeights.push_back(craftAttributeWeight);
                num++;
            }
        }

        mDatabase->destroyDataBinding(binding);
        //gLogger->log(LogManager::DEBUG,"Finished Loading %u Schematic Craft Attribute Weights out of %u",num,count);
    }
    break;


    case ScMQuery_SchematicCraftAttributeLinks:
    {
        DraftSchematic*	schematic = NULL;//asyncContainer->mSchematic;
        CraftAttribute*	craftAttribute;

        DataBinding* binding = mDatabase->createDataBinding(7);
        binding->addField(DFT_bstring,offsetof(CraftAttribute,mAttributeKey),255,0);
        binding->addField(DFT_uint32,offsetof(CraftAttribute,mAttributeId),4,1);
        binding->addField(DFT_float,offsetof(CraftAttribute,mMin),4,2);
        binding->addField(DFT_float,offsetof(CraftAttribute,mMax),4,3);
        binding->addField(DFT_uint8,offsetof(CraftAttribute,mType),1,4);
        binding->addField(DFT_uint32,offsetof(CraftAttribute,mSchemWeightBatch),4,5);
        binding->addField(DFT_uint32,offsetof(CraftAttribute,mListId),4,6);

        uint64 count = result->getRowCount();
        uint32 num = 0;
        for(uint64 i = 0; i < count; i++)
        {
            craftAttribute = new CraftAttribute();

            result->getNextRow(binding,craftAttribute);
            if((schematic == NULL) || (schematic->getId() != craftAttribute->getSchemWeightBatch()))
            {
                schematic = getSchematicByWeightID(craftAttribute->getSchemWeightBatch());
            }
            if(schematic != NULL)
            {
                CraftBatch* batch = schematic->getCraftBatchByListId(craftAttribute->getListId());
                if(batch != NULL)
                {
                    batch->mAttributes.push_back(craftAttribute);
                    num++;
                }
            }
        }

        mDatabase->destroyDataBinding(binding);
        //gLogger->log(LogManager::DEBUG,"Finished Loading %u Schematic Craft Attribute Links out of %u",num,count);
    }
    break;

    case ScMQuery_SchematicCraftWeights:
    {
        DraftSchematic*	schematic = NULL;//asyncContainer->mSchematic;
        CraftWeight*	weight;

        DataBinding* binding = mDatabase->createDataBinding(4);
        binding->addField(DFT_uint8,offsetof(CraftWeight,mDataType),1,0);
        binding->addField(DFT_float,offsetof(CraftWeight,mDistribution),4,1);
        binding->addField(DFT_uint32,offsetof(CraftWeight,mSchemWeightBatch),4,2);
        binding->addField(DFT_uint32,offsetof(CraftWeight,mBatchListId),4,3);

        uint64 count = result->getRowCount();
        uint32 num = 0;
        for(uint64 i = 0; i < count; i++)
        {
            weight = new CraftWeight();

            result->getNextRow(binding,weight);
            if((schematic == NULL) || (schematic->getId() != weight->getSchemWeightBatch()))
            {
                schematic = getSchematicByWeightID(weight->getSchemWeightBatch());
            }
            if(schematic != NULL)
            {
                CraftBatch* batch = schematic->getCraftBatchByListId(weight->getBatchListId());
                if(batch != NULL)
                {
                    batch->mWeights.push_back(weight);
                    num++;
                }
            }
        }

        mDatabase->destroyDataBinding(binding);
        //gLogger->log(LogManager::DEBUG,"Finished Loading %u Schematic Craft Weights out of %u.",num,count);
    }
    break;

    case ScMQuery_SchematicAssemblyWeights:
    {
        DraftSchematic*	schematic = NULL;
        DraftWeight*	weight;

        DataBinding* binding = mDatabase->createDataBinding(4);
        binding->addField(DFT_uint8,offsetof(DraftWeight,mDataType),1,0);
        binding->addField(DFT_uint8,offsetof(DraftWeight,mDistribution),1,1);
        binding->addField(DFT_uint32,offsetof(DraftWeight,mSchemWeightBatch),4,2);
        binding->addField(DFT_uint32,offsetof(DraftWeight,mBatchListId),4,3);

        uint64 count = result->getRowCount();
        uint32 num = 0;
        for(uint64 i = 0; i < count; i++)
        {
            weight = new DraftWeight();

            result->getNextRow(binding,weight);
            weight->prepareData();
            if((schematic == NULL) || (schematic->getId() != weight->getSchemWeightBatch()))
            {
                schematic = getSchematicByWeightID(weight->getSchemWeightBatch());
            }
            if(schematic != NULL)
            {
                WeightsBatch* batch = schematic->getAssemblyWeightsBatchByListId(weight->getBatchListId());
                if(batch != NULL)
                {
                    batch->mWeights.push_back(weight);
                    num++;
                }
            }
        }

        mDatabase->destroyDataBinding(binding);
        //gLogger->log(LogManager::DEBUG,"Finished Loading %u Schematic Assembly Weights out of %u",num,count);
    }
    break;

    case ScMQuery_SchematicExperimentWeights:
    {
        DraftSchematic*	schematic = NULL;
        DraftWeight*	weight;

        DataBinding* binding = mDatabase->createDataBinding(4);
        binding->addField(DFT_uint8,offsetof(DraftWeight,mDataType),1,0);
        binding->addField(DFT_uint8,offsetof(DraftWeight,mDistribution),1,1);
        binding->addField(DFT_uint32,offsetof(DraftWeight,mSchemWeightBatch),4,2);
        binding->addField(DFT_uint32,offsetof(DraftWeight,mBatchListId),4,3);

        uint64 count = result->getRowCount();
        uint32 num = 0;
        for(uint64 i = 0; i < count; i++)
        {
            weight = new DraftWeight();

            result->getNextRow(binding,weight);
            weight->prepareData();
            if((schematic == NULL) || (schematic->getWeightsBatchId() != weight->getSchemWeightBatch()))
            {
                schematic = getSchematicByWeightID(weight->getSchemWeightBatch());
            }
            if(schematic != NULL)
            {
                WeightsBatch* batch = schematic->getExperimentWeightsBatchByListId(weight->getBatchListId());
                if(batch != NULL)
                {
                    batch->mWeights.push_back(weight);
                    num++;
                }
            }
        }
        mDatabase->destroyDataBinding(binding);
        //gLogger->log(LogManager::DEBUG,"Finished Loading %u Schematic Experimentation Weights out of %u",num,count);
    }
    break;

    default:
        break;
    }

    mDBAsyncPool.ordered_free(asyncContainer);
}
//======================================================================================================================
DraftSchematic* SchematicManager::getSchematicByWeightID(uint32 weightsbatch_Id)
{
    SchematicList::iterator it = mSchematicList.find(weightsbatch_Id);

    if(it != mSchematicList.end())
        return((*it).second);

    return NULL;
}
//======================================================================================================================

DraftSchematic* SchematicManager::getSchematicBySlotId(uint32 slotId)
{
    SchematicMap::iterator it = mSchematicSlotMap.find(slotId);

    if(it != mSchematicSlotMap.end())
        return((*it).second);

    return NULL;
}

//======================================================================================================================

DraftSchematic* SchematicManager::getSchematicByWeightId(uint32 weightId)
{
    SchematicMap::iterator it = mSchematicWeightMap.find(weightId);

    if(it != mSchematicWeightMap.end())
        return((*it).second);

    return(NULL);
}

//======================================================================================================================

