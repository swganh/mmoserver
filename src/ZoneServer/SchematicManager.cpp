/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "SchematicManager.h"
#include "CraftBatch.h"
#include "DraftSchematic.h"
#include "DraftSlot.h"
#include "DraftWeight.h"
#include "SchematicGroup.h"
#include "WeightsBatch.h"

#include "LogManager/LogManager.h"

#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"

//======================================================================================================================

bool SchematicManager::mInsFlag = false;
SchematicManager* SchematicManager::mSingleton = NULL;

//======================================================================================================================

SchematicManager::SchematicManager(Database* database) :
mDatabase(database),
mGroupCount(0),
mSchematicCount(0),
mGroupLoadCount(0),
mDBAsyncPool(sizeof(ScMAsyncContainer))
{
	mSchematicGroupList.reserve(350);

	// load skillschematicgroups
	mDatabase->ExecuteSqlAsync(this,new(mDBAsyncPool.ordered_malloc()) ScMAsyncContainer(ScMQuery_SchematicGroups),"SELECT * FROM schematic_groups ORDER BY id");

	// load experimentation groups
	mDatabase->ExecuteSqlAsync(this,new(mDBAsyncPool.ordered_malloc()) ScMAsyncContainer(ScMQuery_ExperimentationGroups),"SELECT * FROM draft_experiment_groups ORDER BY id");
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
			string expGroup;
			DataBinding* binding = mDatabase->CreateDataBinding(1);
			binding->addField(DFT_bstring,0,64,1);

			uint64 count = result->getRowCount();

			for(uint64 i = 0;i < count;i++)
			{
				result->GetNextRow(binding,&expGroup);
				mvExpGroups.push_back(expGroup.getAnsi());
			}

			mDatabase->DestroyDataBinding(binding);
		}
		break;

		case ScMQuery_SchematicGroups:
		{
			SchematicGroup* scGroup;
			DataBinding* binding = mDatabase->CreateDataBinding(2);
			binding->addField(DFT_uint32,offsetof(SchematicGroup,mId),4,0);
			binding->addField(DFT_bstring,offsetof(SchematicGroup,mName),64,1);

			uint64 count = mGroupCount = mGroupLoadCount = static_cast<uint32>(result->getRowCount());

			for(uint64 i = 0;i < count;i++)
			{
				scGroup = new SchematicGroup();

				result->GetNextRow(binding,scGroup);
				mSchematicGroupList.push_back(scGroup);

				ScMAsyncContainer* asContainer;

				asContainer = new(mDBAsyncPool.ordered_malloc()) ScMAsyncContainer(ScMQuery_GroupSchematics);
				asContainer->mGroupId = scGroup->mId - 1;
				mDatabase->ExecuteSqlAsync(this,asContainer,"SELECT object_string,weightsbatch_id,complexity,datasize,subCategory,craftEnabled FROM draft_schematics WHERE group_id=%u",scGroup->mId);
			}

			mDatabase->DestroyDataBinding(binding);
		}
		break;

		case ScMQuery_GroupSchematics:
		{
			DraftSchematic*	schematic;

			DataBinding* binding = mDatabase->CreateDataBinding(6);
			binding->addField(DFT_bstring,offsetof(DraftSchematic,mModel),128,0);
			binding->addField(DFT_uint32,offsetof(DraftSchematic,mWeightsBatchId),4,1);
			binding->addField(DFT_uint32,offsetof(DraftSchematic,mComplexity),4,2);
			binding->addField(DFT_uint32,offsetof(DraftSchematic,mDataSize),4,3);
			binding->addField(DFT_uint32,offsetof(DraftSchematic,mSubCategory),4,4);
			binding->addField(DFT_uint8,offsetof(DraftSchematic,mCraftEnabled),1,5);

			uint64 count = result->getRowCount();

			for(uint64 i = 0;i < count;i++)
			{
				schematic = new DraftSchematic();

				result->GetNextRow(binding,schematic);
						
				// gotta get shared_ into the name
				BStringVector splits;
				int elements = schematic->mModel.split(splits,'/');

				if(elements < 3)
				{
					gLogger->logMsg("SchematicManager: Error in Schematic String");
					break;
				}

				uint32 size =   schematic->mModel.getLength();
				schematic->mModel.setLength(256);
				schematic->mModel = splits[0].getAnsi();

				for(int i = 1;i < elements - 1;i++)
					schematic->mModel << "/" << splits[i].getAnsi(); 

				schematic->mModel << "/shared_" << splits[elements-1].getAnsi();
				
				// glue our ids and insert into maps
				uint64 schemId = schematic->mModel.getCrc();
				schematic->setId((schemId << 32) | (schematic->mWeightsBatchId));

				mSchematicGroupList[asyncContainer->mGroupId]->mSchematics.push_back(schematic);
				mSchematicSlotMap.insert(std::make_pair(schematic->mModel.getCrc(),schematic));
				mSchematicWeightMap.insert(std::make_pair((schematic->mWeightsBatchId ),schematic));

				// now query the draftslots
				ScMAsyncContainer* asContainer = new(mDBAsyncPool.ordered_malloc()) ScMAsyncContainer(ScMQuery_SchematicSlots);
				asContainer->mSchematic = schematic;
				int8 sql[2048];
				sprintf(sql,"SELECT draft_slots.component_file,draft_slots.component_name,draft_slots.resource_name,draft_slots.amount,draft_slots.optional,draft_slots.type"
							" FROM	draft_slots"
							" INNER JOIN draft_schematics_slots ON (draft_slots.id = draft_schematics_slots.draft_slot_id)"
							" WHERE"
							" (draft_schematics_slots.schematic_id = %lld)",schemId);

				mDatabase->ExecuteSqlAsync(this,asContainer,sql);


				// assemblybatches
				asContainer = new(mDBAsyncPool.ordered_malloc()) ScMAsyncContainer(ScMQuery_SchematicAssemblyBatches);
				asContainer->mSchematic = schematic;
				sprintf(sql,"SELECT draft_assembly_batches.id,draft_assembly_batches.list_id"
							" FROM"
							" draft_weights"
							" INNER JOIN draft_assembly_batches ON (draft_weights.assembly_batch_id = draft_assembly_batches.id)"
							" WHERE"
							" (draft_weights.id = %u) ORDER BY draft_assembly_batches.list_id",schematic->mWeightsBatchId);

				mDatabase->ExecuteSqlAsync(this,asContainer,sql);

				// experimentbatches
				asContainer = new(mDBAsyncPool.ordered_malloc()) ScMAsyncContainer(ScMQuery_SchematicExperimentBatches);
				asContainer->mSchematic = schematic;
				sprintf(sql,"SELECT draft_experiment_batches.id,draft_experiment_batches.list_id"
							" FROM"
							" draft_weights"
							" INNER JOIN draft_experiment_batches ON (draft_weights.experiment_batch_id = draft_experiment_batches.id)"
							" WHERE"
							" (draft_weights.id = %u) ORDER BY draft_experiment_batches.list_id",schematic->mWeightsBatchId);

				mDatabase->ExecuteSqlAsync(this,asContainer,sql);

				// craftingbatches
				asContainer = new(mDBAsyncPool.ordered_malloc()) ScMAsyncContainer(ScMQuery_SchematicCraftBatches);
				asContainer->mSchematic = schematic;
				sprintf(sql,"SELECT draft_craft_batches.id,draft_craft_batches.list_id,draft_craft_batches.expGroup"
							" FROM"
							" draft_weights"
							" INNER JOIN draft_craft_batches ON (draft_weights.craft_batch_id = draft_craft_batches.id)"
							" WHERE"

							" (draft_weights.id = %u) ORDER BY draft_craft_batches.list_id",schematic->mWeightsBatchId);

				/*
				// craftingbatches
				asContainer = new(mDBAsyncPool.ordered_malloc()) ScMAsyncContainer(ScMQuery_SchematicCraftBatches);
				asContainer->mSchematic = schematic;
				sprintf(sql,"SELECT draft_craft_batches.id,draft_craft_batches.list_id,draft_craft_batches.expGroup"
							" FROM"
							" draft_weights"
							" INNER JOIN draft_craft_batches ON (draft_weights.craft_batch_id = draft_craft_batches.id)"
							" WHERE"
							" (draft_weights.id = %u) ORDER BY draft_craft_batches.list_id",schematic->mWeightsBatchId);
				  */
				mDatabase->ExecuteSqlAsync(this,asContainer,sql);
			}

			mSchematicCount += static_cast<uint32>(count);

			if(!--mGroupLoadCount)
			{				
				gLogger->logMsgLoadSuccess("SchematicManager::Loaded %u Groups %u Schematics",MSG_NORMAL,mGroupCount,mSchematicCount);
			}

			mDatabase->DestroyDataBinding(binding);
		}
		break;

		case ScMQuery_SchematicSlots:
		{
			DraftSchematic*	schematic = asyncContainer->mSchematic;
			DraftSlot*		slot;

			DataBinding* binding = mDatabase->CreateDataBinding(6);
			binding->addField(DFT_bstring,offsetof(DraftSlot,mFile),64,0);
			binding->addField(DFT_bstring,offsetof(DraftSlot,mName),64,1);
			binding->addField(DFT_bstring,offsetof(DraftSlot,mResourceName),128,2);
			binding->addField(DFT_uint32,offsetof(DraftSlot,mAmount),4,3);
			binding->addField(DFT_uint8,offsetof(DraftSlot,mOptional),1,4);
			binding->addField(DFT_uint8,offsetof(DraftSlot,mType),1,5);

			uint64 count = result->getRowCount();

			for(uint64 i = 0;i < count;i++)
			{
				slot = new DraftSlot();

				result->GetNextRow(binding,slot);
				schematic->mDraftSlots.push_back(slot);
				slot->mResourceName.convert(BSTRType_Unicode16);
			}

			mDatabase->DestroyDataBinding(binding);
		}
		break;

		case ScMQuery_SchematicAssemblyBatches:
		{
			DraftSchematic*	schematic = asyncContainer->mSchematic;
			WeightsBatch*	batch;

			DataBinding* binding = mDatabase->CreateDataBinding(2);
			binding->addField(DFT_uint32,offsetof(WeightsBatch,mId),4,0);
			binding->addField(DFT_uint32,offsetof(WeightsBatch,mListId),4,1);

			uint64 count = result->getRowCount();

			for(uint64 i = 0;i < count;i++)
			{
				batch = new WeightsBatch();

				result->GetNextRow(binding,batch);

				schematic->mAssemblyBatch.push_back(batch);

				// query list items
				ScMAsyncContainer* asContainer = new(mDBAsyncPool.ordered_malloc()) ScMAsyncContainer(ScMQuery_SchematicAssemblyWeights);
				asContainer->mSchematic = schematic;
				asContainer->mBatchId = batch->getListId();

				mDatabase->ExecuteSqlAsync(this,asContainer,"SELECT datatype,distribution FROM draft_assembly_lists WHERE id=%u",asContainer->mBatchId);
			}

			mDatabase->DestroyDataBinding(binding);
		}
		break;

		case ScMQuery_SchematicExperimentBatches:
		{
			DraftSchematic*	schematic = asyncContainer->mSchematic;
			WeightsBatch*	batch;

			DataBinding* binding = mDatabase->CreateDataBinding(2);
			binding->addField(DFT_uint32,offsetof(WeightsBatch,mId),4,0);
			binding->addField(DFT_uint32,offsetof(WeightsBatch,mListId),4,1);

			uint64 count = result->getRowCount();

			for(uint64 i = 0;i < count;i++)
			{
				batch = new WeightsBatch();

				result->GetNextRow(binding,batch);

				schematic->mExperimentBatch.push_back(batch);

				// query list items
				ScMAsyncContainer* asContainer = new(mDBAsyncPool.ordered_malloc()) ScMAsyncContainer(ScMQuery_SchematicExperimentWeights);
				asContainer->mSchematic = schematic;
				asContainer->mBatchId = batch->getListId();

				mDatabase->ExecuteSqlAsync(this,asContainer,"SELECT datatype,distribution FROM draft_experiment_lists WHERE id=%u",asContainer->mBatchId);
			}

			mDatabase->DestroyDataBinding(binding);
		}
		break;

		case ScMQuery_SchematicCraftBatches:
		{
			DraftSchematic*	schematic = asyncContainer->mSchematic;
			CraftBatch*		batch;

			DataBinding* binding = mDatabase->CreateDataBinding(3);
			binding->addField(DFT_uint32,offsetof(CraftBatch,mId),4,0);
			binding->addField(DFT_uint32,offsetof(CraftBatch,mListId),4,1);
			binding->addField(DFT_uint32,offsetof(CraftBatch,mExpGroup),4,2);

			uint64 count = result->getRowCount();

			for(uint64 i = 0;i < count;i++)
			{
				batch = new CraftBatch();

				result->GetNextRow(binding,batch);

				schematic->mCraftBatch.push_back(batch);

				// query weight distribution
				ScMAsyncContainer* asContainer = new(mDBAsyncPool.ordered_malloc()) ScMAsyncContainer(ScMQuery_SchematicCraftWeights);
				asContainer->mSchematic = schematic;
				asContainer->mBatchId = batch->getListId();

				mDatabase->ExecuteSqlAsync(this,asContainer,"SELECT type,distribution FROM draft_craft_attribute_weights WHERE id=%u",asContainer->mBatchId);

				// query attribute links and ranges
				asContainer = new(mDBAsyncPool.ordered_malloc()) ScMAsyncContainer(ScMQuery_SchematicCraftAttributeLinks);
				asContainer->mSchematic = schematic;
				asContainer->mBatchId = batch->getListId();

				mDatabase->ExecuteSqlAsync(this,asContainer,"SELECT attributes.name,dcial.item_attribute,dcial.attribute_min,dcial.attribute_max,dcial.attribute_type"
															" FROM draft_craft_item_attribute_link as dcial"
															" INNER JOIN attributes ON (dcial.item_attribute = attributes.id)"
															" WHERE list_id=%u",asContainer->mBatchId);


				// query attribute weighting for component crafting
				asContainer = new(mDBAsyncPool.ordered_malloc()) ScMAsyncContainer(ScMQuery_SchematicCraftAttributeWeights);
				asContainer->mSchematic = schematic;
				asContainer->mBatchId = batch->getListId();

				mDatabase->ExecuteSqlAsync(this,asContainer,"SELECT dsam.Attribute, dsam.AffectedAttribute, dsam.Manipulation, a.name, b.name "
															" FROM draft_schematic_attribute_manipulation as dsam"
															" INNER JOIN attributes as a ON (dsam.attribute = a.id)"
															" INNER JOIN attributes as b ON (dsam.affectedattribute = b.id)"
															" WHERE Draft_Schematic=%u",schematic->getWeightsBatchId());
			}

			mDatabase->DestroyDataBinding(binding);
		}
		break;

		case ScMQuery_SchematicCraftAttributeWeights:
		{
			// "SELECT dsam.Attribute, dsam.AffectedAttribute, dsam.Manipulation, a.name, b.name "

			DraftSchematic*	schematic = asyncContainer->mSchematic;
			CraftAttributeWeight*	craftAttributeWeight;

			DataBinding* binding = mDatabase->CreateDataBinding(5);
			
			binding->addField(DFT_uint32,offsetof(CraftAttributeWeight,mAttributeId),4,0);
			binding->addField(DFT_uint32,offsetof(CraftAttributeWeight,mAffectedAttributeId),4,1);
			binding->addField(DFT_uint32,offsetof(CraftAttributeWeight,mManipulation),4,2);
			binding->addField(DFT_bstring,offsetof(CraftAttributeWeight,mAttributeKey),255,3);
			binding->addField(DFT_bstring,offsetof(CraftAttributeWeight,mAffectedAttributeKey),255,4);

			uint64 count = result->getRowCount();

			for(uint64 i = 0;i < count;i++)
			{
				craftAttributeWeight = new CraftAttributeWeight();

				result->GetNextRow(binding,craftAttributeWeight);

				//CraftBatch* batch = schematic->getCraftBatchByListId(asyncContainer->mBatchId);
				schematic->mAttributeWeights.push_back(craftAttributeWeight);
			}

			mDatabase->DestroyDataBinding(binding);
		
		}
		break;


		case ScMQuery_SchematicCraftAttributeLinks:
		{
			DraftSchematic*	schematic = asyncContainer->mSchematic;
			CraftAttribute*	craftAttribute;

			DataBinding* binding = mDatabase->CreateDataBinding(5);
			binding->addField(DFT_bstring,offsetof(CraftAttribute,mAttributeKey),255,0);
			binding->addField(DFT_uint32,offsetof(CraftAttribute,mAttributeId),4,1);
			binding->addField(DFT_float,offsetof(CraftAttribute,mMin),4,2);
			binding->addField(DFT_float,offsetof(CraftAttribute,mMax),4,3);
			binding->addField(DFT_uint8,offsetof(CraftAttribute,mType),1,4);

			uint64 count = result->getRowCount();

			for(uint64 i = 0;i < count;i++)
			{
				craftAttribute = new CraftAttribute();

				result->GetNextRow(binding,craftAttribute);

				CraftBatch* batch = schematic->getCraftBatchByListId(asyncContainer->mBatchId);
				batch->mAttributes.push_back(craftAttribute);
			}

			mDatabase->DestroyDataBinding(binding);
		}
		break;

		case ScMQuery_SchematicCraftWeights:
		{
			DraftSchematic*	schematic = asyncContainer->mSchematic;
			CraftWeight*	weight;

			DataBinding* binding = mDatabase->CreateDataBinding(2);
			binding->addField(DFT_uint8,offsetof(CraftWeight,mDataType),1,0);
			binding->addField(DFT_float,offsetof(CraftWeight,mDistribution),4,1);

			uint64 count = result->getRowCount();

			for(uint64 i = 0;i < count;i++)
			{
				weight = new CraftWeight();

				result->GetNextRow(binding,weight);

				CraftBatch* batch = schematic->getCraftBatchByListId(asyncContainer->mBatchId);
				batch->mWeights.push_back(weight);
			}

			mDatabase->DestroyDataBinding(binding);
		}
		break;

		case ScMQuery_SchematicAssemblyWeights:
		{
			DraftSchematic*	schematic = asyncContainer->mSchematic;
			DraftWeight*	weight;

			DataBinding* binding = mDatabase->CreateDataBinding(2);
			binding->addField(DFT_uint8,offsetof(DraftWeight,mDataType),1,0);
			binding->addField(DFT_uint8,offsetof(DraftWeight,mDistribution),1,1);

			uint64 count = result->getRowCount();

			for(uint64 i = 0;i < count;i++)
			{
				weight = new DraftWeight();

				result->GetNextRow(binding,weight);
				weight->prepareData();

				WeightsBatch* batch = schematic->getAssemblyWeightsBatchByListId(asyncContainer->mBatchId);
				batch->mWeights.push_back(weight);
			}

			mDatabase->DestroyDataBinding(binding);
		}
		break;

		case ScMQuery_SchematicExperimentWeights:
		{
			DraftSchematic*	schematic = asyncContainer->mSchematic;
			DraftWeight*	weight;

			DataBinding* binding = mDatabase->CreateDataBinding(2);
			binding->addField(DFT_uint8,offsetof(DraftWeight,mDataType),1,0);
			binding->addField(DFT_uint8,offsetof(DraftWeight,mDistribution),1,1);

			uint64 count = result->getRowCount();

			for(uint64 i = 0;i < count;i++)
			{
				weight = new DraftWeight();

				result->GetNextRow(binding,weight);
				weight->prepareData();

				WeightsBatch* batch = schematic->getExperimentWeightsBatchByListId(asyncContainer->mBatchId);
				batch->mWeights.push_back(weight);
			}

			mDatabase->DestroyDataBinding(binding);
		}
		break;

		default:break;
	}

	mDBAsyncPool.ordered_free(asyncContainer);
}

//======================================================================================================================

DraftSchematic* SchematicManager::getSchematicBySlotId(uint32 slotId)
{
	SchematicMap::iterator it = mSchematicSlotMap.find(slotId);

	if(it != mSchematicSlotMap.end())
		return((*it).second);

	return(NULL);
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

