/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

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

SchematicManager::SchematicManager(Database* database)
: mDBAsyncPool(sizeof(ScMAsyncContainer))
, mDatabase(database)
, mGroupCount(0)
, mGroupLoadCount(0)
, mSchematicCount(0)
{
	// load skillschematicgroups
	gLogger->log(LogManager::DEBUG,"Started Loading Schematic Groups.");
	mDatabase->ExecuteSqlAsync(this,new(mDBAsyncPool.ordered_malloc()) ScMAsyncContainer(ScMQuery_SchematicGroups),"SELECT * FROM schematic_groups ORDER BY id");

	// load experimentation groups
	gLogger->log(LogManager::DEBUG,"Finished Loading Experimentation Groups.");
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
			mvExpGroups.reserve((uint32)count);
			for(uint64 i = 0;i < count;i++)
			{
				result->GetNextRow(binding,&expGroup);
				mvExpGroups.push_back(expGroup.getAnsi());
			}

			mDatabase->DestroyDataBinding(binding);
			gLogger->log(LogManager::DEBUG,"Finished Loading Experimentation Groups.");
		}
		break;

		case ScMQuery_SchematicGroups:
		{
			SchematicGroup* scGroup;
			DataBinding* binding = mDatabase->CreateDataBinding(2);
			binding->addField(DFT_uint32,offsetof(SchematicGroup,mId),4,0);
			binding->addField(DFT_bstring,offsetof(SchematicGroup,mName),64,1);

			uint64 count = mGroupCount = mGroupLoadCount = static_cast<uint32>(result->getRowCount());
			mSchematicGroupList.reserve((uint32)count);
			for(uint64 i = 0;i < count;i++)
			{
				scGroup = new SchematicGroup();

				result->GetNextRow(binding,scGroup);
				mSchematicGroupList.push_back(scGroup);
			}

			mDatabase->DestroyDataBinding(binding);
			ScMAsyncContainer* asContainer;

			gLogger->log(LogManager::DEBUG,"Started Loading Schematics");
			asContainer = new(mDBAsyncPool.ordered_malloc()) ScMAsyncContainer(ScMQuery_GroupSchematics);
			mDatabase->ExecuteSqlAsync(this,asContainer,"SELECT object_string,weightsbatch_id,complexity,datasize,subCategory,craftEnabled,group_id FROM draft_schematics");
			
			gLogger->log(LogManager::DEBUG,"Finished Loading Schematic Groups.");
		}
		break;

		case ScMQuery_GroupSchematics:
		{
			DraftSchematic*	schematic = NULL;

			DataBinding* binding = mDatabase->CreateDataBinding(7);
			binding->addField(DFT_bstring,offsetof(DraftSchematic,mModel),128,0);
			binding->addField(DFT_uint32,offsetof(DraftSchematic,mWeightsBatchId),4,1);
			binding->addField(DFT_uint32,offsetof(DraftSchematic,mComplexity),4,2);
			binding->addField(DFT_uint32,offsetof(DraftSchematic,mDataSize),4,3);
			binding->addField(DFT_uint32,offsetof(DraftSchematic,mSubCategory),4,4);
			binding->addField(DFT_uint8,offsetof(DraftSchematic,mCraftEnabled),1,5);
			binding->addField(DFT_uint32,offsetof(DraftSchematic,mGroupId),4,6);

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
					gLogger->log(LogManager::DEBUG,"SchematicManager: Error in Schematic String");
					break;
				}

				//uint32 size =   schematic->mModel.getLength();
				schematic->mModel.setLength(256);
				schematic->mModel = splits[0].getAnsi();

				for(int j = 1;j < elements - 1;j++)
					schematic->mModel << "/" << splits[j].getAnsi();

				schematic->mModel << "/shared_" << splits[elements-1].getAnsi();

				// glue our ids and insert into maps
				uint64 schemId = schematic->mModel.getCrc();
				schematic->setId((schemId << 32) | (schematic->mWeightsBatchId));
				//added temporary log in order to dump schematic ids for DB insertion
				//gLogger->log(LogManager::DEBUG,"Schematic,%u,%u",schematic->mId,schematic->mWeightsBatchId);
				mSchematicMap.insert(std::make_pair(schematic->getId(), schematic));
				mSchematicGroupList[schematic->getGroupId()]->mSchematics.push_back(schematic);
				mSchematicSlotMap.insert(std::make_pair(schematic->mModel.getCrc(),schematic));
				mSchematicWeightMap.insert(std::make_pair((schematic->mWeightsBatchId ),schematic));
			}

			mSchematicCount += static_cast<uint32>(count);

			gLogger->log(LogManager::DEBUG,"Started Loading Schematic Draft Slots.");
			// now query the draftslots
			ScMAsyncContainer* asContainer = new(mDBAsyncPool.ordered_malloc()) ScMAsyncContainer(ScMQuery_SchematicSlots);
			int8 sql[2048];
			sprintf(sql,"SELECT draft_slots.component_file,draft_slots.component_name,draft_slots.resource_name,draft_slots.amount,draft_slots.optional,draft_slots.type,draft_schematics_slots.schematic_id,draft_schematics.group_id"
						" FROM draft_slots "
						" INNER JOIN draft_schematics_slots ON (draft_slots.id = draft_schematics_slots.draft_slot_id) "
						" INNER JOIN draft_schematics ON(draft_schematics_slots.schematic_id = draft_schematics.schematic_id) ");
			mDatabase->ExecuteSqlAsync(this,asContainer,sql);


			gLogger->log(LogManager::DEBUG,"Started Loading Schematic Assembly Batches.");
			// assemblybatches
			asContainer = new(mDBAsyncPool.ordered_malloc()) ScMAsyncContainer(ScMQuery_SchematicAssemblyBatches);
			sprintf(sql,"SELECT datatype,distribution,draft_slots.id,draft_schematics_slots.schematic_id,draft_schematics.group_id"
						" FROM draft_assembly_lists "
						" INNER JOIN draft_slots ON (draft_assembly_lists.id = draft_slots.id) "
						" INNER JOIN draft_schematics_slots ON (draft_slots.id = draft_schematics_slots.draft_slot_id) "
						" INNER JOIN draft_schematics ON(draft_schematics_slots.schematic_id = draft_schematics.schematic_id) ");
			mDatabase->ExecuteSqlAsync(this,asContainer,sql);

			gLogger->log(LogManager::DEBUG,"Started Loading Schematic Experimentation Batches.");
			// experimentbatches
			asContainer = new(mDBAsyncPool.ordered_malloc()) ScMAsyncContainer(ScMQuery_SchematicExperimentBatches);
			sprintf(sql,"SELECT draft_experiment_batches.id,draft_experiment_batches.list_id,draft_schematics.schematic_id,draft_schematics.group_id"
						" FROM draft_weights "
						" INNER JOIN draft_experiment_batches ON (draft_weights.experiment_batch_id = draft_experiment_batches.id) "
						" INNER JOIN draft_schematics ON(draft_weights.id = draft_schematics.weightsbatch_id) "
						" ORDER BY draft_experiment_batches.list_id ");

			mDatabase->ExecuteSqlAsync(this,asContainer,sql);

			gLogger->log(LogManager::DEBUG,"Started Loading Schematic Crafting Batches.");
			// craftingbatches
			asContainer = new(mDBAsyncPool.ordered_malloc()) ScMAsyncContainer(ScMQuery_SchematicCraftBatches);
			sprintf(sql,"SELECT draft_craft_batches.id,draft_craft_batches.list_id,draft_craft_batches.expGroup,draft_schematics.schematic_id,draft_schematics.group_id"
						" FROM draft_weights "
						" INNER JOIN draft_craft_batches ON (draft_weights.craft_batch_id = draft_craft_batches.id) "
						" INNER JOIN draft_schematics ON(draft_weights.id = draft_schematics.weightsbatch_id) "
						" ORDER BY draft_craft_batches.list_id ");
			mDatabase->ExecuteSqlAsync(this,asContainer,sql);

			if(!--mGroupLoadCount)
			{
				gLogger->log(LogManager::NOTICE,"Finished Loading Groups and Schematics.");

		
			}

			mDatabase->DestroyDataBinding(binding);
			gLogger->log(LogManager::DEBUG,"Finished Loading Schematics");
		}
		break;

		case ScMQuery_SchematicSlots:
		{
			DraftSchematic*	schematic = NULL;//asyncContainer->mSchematic;
			DraftSlot*		slot;

			DataBinding* binding = mDatabase->CreateDataBinding(8);
			binding->addField(DFT_bstring,offsetof(DraftSlot,mFile),64,0);
			binding->addField(DFT_bstring,offsetof(DraftSlot,mName),64,1);
			binding->addField(DFT_bstring,offsetof(DraftSlot,mResourceName),128,2);
			binding->addField(DFT_uint32,offsetof(DraftSlot,mAmount),4,3);
			binding->addField(DFT_uint8,offsetof(DraftSlot,mOptional),1,4);
			binding->addField(DFT_uint8,offsetof(DraftSlot,mType),1,5);
			binding->addField(DFT_uint64,offsetof(DraftSlot,mSchematicId),8,6);
			binding->addField(DFT_uint32,offsetof(DraftSlot,mSchemGroupId),4,7);

			uint64 count = result->getRowCount();

			for(uint64 i = 0;i < count;i++)
			{
				slot = new DraftSlot();
				result->GetNextRow(binding,slot);

				if((schematic == NULL) || (schematic->getId() != slot->getSchemId()))
				{
					this->getSchematicByID(slot->getSchemId(), slot->getSchemGroupId());
				}
				if(schematic != NULL)
					schematic->mDraftSlots.push_back(slot);
				slot->mResourceName.convert(BSTRType_Unicode16);
			}

			mDatabase->DestroyDataBinding(binding);
			gLogger->log(LogManager::DEBUG,"Finished Loading Schematic Draft Slots.");
		}
		break;

		case ScMQuery_SchematicAssemblyBatches:
		{
			DraftSchematic*	schematic = NULL;//asyncContainer->mSchematic;
			WeightsBatch*	batch;

			DataBinding* binding = mDatabase->CreateDataBinding(4);
			binding->addField(DFT_uint32,offsetof(WeightsBatch,mId),4,0);
			binding->addField(DFT_uint32,offsetof(WeightsBatch,mListId),4,1);
			binding->addField(DFT_uint64,offsetof(WeightsBatch,mSchematicId),8,3);
			binding->addField(DFT_uint32,offsetof(WeightsBatch,mSchemGroupId),4,4);

			uint64 count = result->getRowCount();

			for(uint64 i = 0;i < count;i++)
			{
				batch = new WeightsBatch();

				result->GetNextRow(binding,batch);
				if((schematic == NULL) || (schematic->getId() != batch->getSchemId()))
				{
					this->getSchematicByID(batch->getSchemId(), batch->getSchemGroupId());
				}
				if(schematic != NULL)
					schematic->mAssemblyBatch.push_back(batch);

				
			}
			// query list items
			ScMAsyncContainer* asContainer = new(mDBAsyncPool.ordered_malloc()) ScMAsyncContainer(ScMQuery_SchematicAssemblyWeights);
			//asContainer->mSchematic = schematic;
			//asContainer->mBatchId = batch->getListId();
			gLogger->log(LogManager::DEBUG,"Started Loading Schematic Assembly Weights.");
			mDatabase->ExecuteSqlAsync(this,asContainer,"SELECT datatype,distribution,draft_schematics.schematic_id,draft_schematics.group_id"
														" FROM draft_assembly_lists"
														" INNER JOIN draft_schematics ON(id = draft_schematics.weightsbatch_id)");
			mDatabase->DestroyDataBinding(binding);
			gLogger->log(LogManager::DEBUG,"Finished Loading Schematic Assembly Batches.");
		}
		break;

		case ScMQuery_SchematicExperimentBatches:
		{
			DraftSchematic*	schematic = NULL;//asyncContainer->mSchematic;
			WeightsBatch*	batch;

			DataBinding* binding = mDatabase->CreateDataBinding(4);
			binding->addField(DFT_uint32,offsetof(WeightsBatch,mId),4,0);
			binding->addField(DFT_uint32,offsetof(WeightsBatch,mListId),4,1);
			binding->addField(DFT_uint32,offsetof(WeightsBatch,mSchematicId),8,2);
			binding->addField(DFT_uint32,offsetof(WeightsBatch,mSchemGroupId),4,3);

			uint64 count = result->getRowCount();

			for(uint64 i = 0;i < count;i++)
			{
				batch = new WeightsBatch();

				result->GetNextRow(binding,batch);
				if((schematic == NULL) || (schematic->getId() != batch->getSchemId()))
				{
					this->getSchematicByID(batch->getSchemId(), batch->getSchemGroupId());
				}
				if(schematic != NULL)
					schematic->mExperimentBatch.push_back(batch);

				
			}
			// query list items
			gLogger->log(LogManager::DEBUG,"Started Loading Schematic Experimentation Weights.");
			ScMAsyncContainer* asContainer = new(mDBAsyncPool.ordered_malloc()) ScMAsyncContainer(ScMQuery_SchematicExperimentWeights);
			mDatabase->ExecuteSqlAsync(this,asContainer,"SELECT datatype,distribution,draft_schematics.schematic_id,draft_schematics.group_id "
														" FROM draft_experiment_lists "
														" INNER JOIN draft_schematics ON(id = draft_schematics.weightsbatch_id)");
			mDatabase->DestroyDataBinding(binding);
			gLogger->log(LogManager::DEBUG,"Finished Loading Schematic Experimentation Batches.");
		}
		break;

		case ScMQuery_SchematicCraftBatches:
		{
			DraftSchematic*	schematic = NULL;//asyncContainer->mSchematic;
			CraftBatch*		batch;

			DataBinding* binding = mDatabase->CreateDataBinding(5);
			binding->addField(DFT_uint32,offsetof(CraftBatch,mId),4,0);
			binding->addField(DFT_uint32,offsetof(CraftBatch,mListId),4,1);
			binding->addField(DFT_uint32,offsetof(CraftBatch,mExpGroup),4,2);
			binding->addField(DFT_uint32,offsetof(CraftBatch,mSchematicId),8,3);
			binding->addField(DFT_uint32,offsetof(CraftBatch,mSchemGroupId),4,4);

			uint64 count = result->getRowCount();

			for(uint64 i = 0;i < count;i++)
			{
				batch = new CraftBatch();

				result->GetNextRow(binding,batch);
				if((schematic == NULL) || (schematic->getId() != batch->getSchemId()))
				{
					this->getSchematicByID(batch->getSchemId(), batch->getSchemGroupId());
				}
				if(schematic != NULL)
					schematic->mCraftBatch.push_back(batch);
			}

			// query weight distribution
			ScMAsyncContainer* asContainer = new(mDBAsyncPool.ordered_malloc()) ScMAsyncContainer(ScMQuery_SchematicCraftWeights);
			gLogger->log(LogManager::DEBUG,"Started Loading Schematic Craft Weights.");
			mDatabase->ExecuteSqlAsync(this,asContainer,"SELECT type,distribution,draft_schematics.schematic_id,draft_schematics.group_id "
				" FROM draft_craft_attribute_weights"
				" INNER JOIN draft_schematics ON(id = draft_schematics.weightsbatch_id)");

			// query attribute links and ranges
			asContainer = new(mDBAsyncPool.ordered_malloc()) ScMAsyncContainer(ScMQuery_SchematicCraftAttributeLinks);
			gLogger->log(LogManager::DEBUG,"Started Loading Schematic Craft Attribute Links.");
			mDatabase->ExecuteSqlAsync(this,asContainer,
			"SELECT attributes.name,dcial.item_attribute,dcial.attribute_min,dcial.attribute_max,dcial.attribute_type,draft_schematics.schematic_id,draft_schematics.group_id "
				" FROM draft_craft_item_attribute_link as dcial"
				" INNER JOIN attributes ON (dcial.item_attribute = attributes.id)"
				" INNER JOIN draft_schematics ON(list_id = draft_schematics.weightsbatch_id)");

			// query attribute weighting for component crafting
			asContainer = new(mDBAsyncPool.ordered_malloc()) ScMAsyncContainer(ScMQuery_SchematicCraftAttributeWeights);
			gLogger->log(LogManager::DEBUG,"Started Loading Schematic Craft Attribute Weights.");
			mDatabase->ExecuteSqlAsync(this,asContainer,
				"SELECT dsam.Attribute, dsam.AffectedAttribute, dsam.Manipulation, a.name, b.name,draft_schematics.schematic_id,draft_schematics.group_id "
				" FROM draft_schematic_attribute_manipulation as dsam"
				" INNER JOIN attributes as a ON (dsam.attribute = a.id)"
				" INNER JOIN attributes as b ON (dsam.affectedattribute = b.id)"
				" INNER JOIN draft_schematics ON(Draft_Schematic = draft_schematics.weightsbatch_id)");
			mDatabase->DestroyDataBinding(binding);
			gLogger->log(LogManager::DEBUG,"Finished Loading Schematic Crafting Batches.");
		}
		break;

		case ScMQuery_SchematicCraftAttributeWeights:
		{
			// "SELECT dsam.Attribute, dsam.AffectedAttribute, dsam.Manipulation, a.name, b.name "

			DraftSchematic*	schematic = NULL;//asyncContainer->mSchematic;
			CraftAttributeWeight*	craftAttributeWeight;

			DataBinding* binding = mDatabase->CreateDataBinding(7);

			binding->addField(DFT_uint32,offsetof(CraftAttributeWeight,mAttributeId),4,0);
			binding->addField(DFT_uint32,offsetof(CraftAttributeWeight,mAffectedAttributeId),4,1);
			binding->addField(DFT_uint32,offsetof(CraftAttributeWeight,mManipulation),4,2);
			binding->addField(DFT_bstring,offsetof(CraftAttributeWeight,mAttributeKey),255,3);
			binding->addField(DFT_bstring,offsetof(CraftAttributeWeight,mAffectedAttributeKey),255,4);
			binding->addField(DFT_uint64,offsetof(CraftAttributeWeight,mSchematicId),8,5);
			binding->addField(DFT_uint32,offsetof(CraftAttributeWeight,mSchemGroupId),4,6);

			uint64 count = result->getRowCount();

			for(uint64 i = 0;i < count;i++)
			{
				craftAttributeWeight = new CraftAttributeWeight();

				result->GetNextRow(binding,craftAttributeWeight);
				if((schematic == NULL) || (schematic->getId() != craftAttributeWeight->getSchemId()))
				{
					this->getSchematicByID(craftAttributeWeight->getSchemId(), craftAttributeWeight->getSchemGroupId());
				}
				if(schematic != NULL)
				//CraftBatch* batch = schematic->getCraftBatchByListId(asyncContainer->mBatchId);
					schematic->mAttributeWeights.push_back(craftAttributeWeight);
			}

			mDatabase->DestroyDataBinding(binding);
			gLogger->log(LogManager::DEBUG,"Finished Loading Schematic Craft Attribute Weights.");
		}
		break;


		case ScMQuery_SchematicCraftAttributeLinks:
		{
			DraftSchematic*	schematic = NULL;//asyncContainer->mSchematic;
			CraftAttribute*	craftAttribute;

			DataBinding* binding = mDatabase->CreateDataBinding(7);
			binding->addField(DFT_bstring,offsetof(CraftAttribute,mAttributeKey),255,0);
			binding->addField(DFT_uint32,offsetof(CraftAttribute,mAttributeId),4,1);
			binding->addField(DFT_float,offsetof(CraftAttribute,mMin),4,2);
			binding->addField(DFT_float,offsetof(CraftAttribute,mMax),4,3);
			binding->addField(DFT_uint8,offsetof(CraftAttribute,mType),1,4);
			binding->addField(DFT_uint64,offsetof(CraftAttribute,mSchematicId),8,5);
			binding->addField(DFT_uint32,offsetof(CraftAttribute,mSchemGroupId),4,6);

			uint64 count = result->getRowCount();

			for(uint64 i = 0;i < count;i++)
			{
				craftAttribute = new CraftAttribute();

				result->GetNextRow(binding,craftAttribute);
				if((schematic == NULL) || (schematic->getId() != craftAttribute->getSchemId()))
				{
					this->getSchematicByID(craftAttribute->getSchemId(), craftAttribute->getSchemGroupId());
				}
				if(schematic != NULL)
				{
					CraftBatch* batch = schematic->getCraftBatchByListId(asyncContainer->mBatchId);
					batch->mAttributes.push_back(craftAttribute);
				}
			}

			mDatabase->DestroyDataBinding(binding);
			gLogger->log(LogManager::DEBUG,"Finished Loading Schematic Craft Attribute Links.");
		}
		break;

		case ScMQuery_SchematicCraftWeights:
		{
			DraftSchematic*	schematic = NULL;//asyncContainer->mSchematic;
			CraftWeight*	weight;

			DataBinding* binding = mDatabase->CreateDataBinding(4);
			binding->addField(DFT_uint8,offsetof(CraftWeight,mDataType),1,0);
			binding->addField(DFT_float,offsetof(CraftWeight,mDistribution),4,1);
			binding->addField(DFT_uint64,offsetof(CraftWeight,mSchematicId),8,2);
			binding->addField(DFT_uint32,offsetof(CraftWeight,mSchemGroupId),4,3);

			uint64 count = result->getRowCount();

			for(uint64 i = 0;i < count;i++)
			{
				weight = new CraftWeight();

				result->GetNextRow(binding,weight);
				if((schematic == NULL) || (schematic->getId() != weight->getSchemId()))
				{
					this->getSchematicByID(weight->getSchemId(), weight->getSchemGroupId());
				}
				if(schematic != NULL)
				{
					CraftBatch* batch = schematic->getCraftBatchByListId(asyncContainer->mBatchId);
					batch->mWeights.push_back(weight);
				}
			}

			mDatabase->DestroyDataBinding(binding);
			gLogger->log(LogManager::DEBUG,"Finished Loading Schematic Craft Weights.");
		}
		break;

		case ScMQuery_SchematicAssemblyWeights:
		{
			DraftSchematic*	schematic = NULL;//asyncContainer->mSchematic;
			DraftWeight*	weight;

			DataBinding* binding = mDatabase->CreateDataBinding(4);
			binding->addField(DFT_uint8,offsetof(DraftWeight,mDataType),1,0);
			binding->addField(DFT_uint8,offsetof(DraftWeight,mDistribution),1,1);
			binding->addField(DFT_uint64,offsetof(DraftWeight,mSchematicId),8,2);
			binding->addField(DFT_uint32,offsetof(DraftWeight,mSchemGroupId),4,3);

			uint64 count = result->getRowCount();

			for(uint64 i = 0;i < count;i++)
			{
				weight = new DraftWeight();

				result->GetNextRow(binding,weight);
				weight->prepareData();
				if((schematic == NULL) || (schematic->getId() != weight->getSchemId()))
				{
					this->getSchematicByID(weight->getSchemId(), weight->getSchemGroupId());
				}
				if(schematic != NULL)
				{
					WeightsBatch* batch = schematic->getAssemblyWeightsBatchByListId(asyncContainer->mBatchId);
					batch->mWeights.push_back(weight);
				}
			}

			mDatabase->DestroyDataBinding(binding);
			gLogger->log(LogManager::DEBUG,"Finished Loading Schematic Assembly Weights.");
		}
		break;

		case ScMQuery_SchematicExperimentWeights:
		{
			DraftSchematic*	schematic = NULL;//asyncContainer->mSchematic;
			DraftWeight*	weight;

			DataBinding* binding = mDatabase->CreateDataBinding(4);
			binding->addField(DFT_uint8,offsetof(DraftWeight,mDataType),1,0);
			binding->addField(DFT_uint8,offsetof(DraftWeight,mDistribution),1,1);
			binding->addField(DFT_uint64,offsetof(DraftWeight,mSchematicId),8,2);
			binding->addField(DFT_uint32,offsetof(DraftWeight,mSchemGroupId),4,3);

			uint64 count = result->getRowCount();

			for(uint64 i = 0;i < count;i++)
			{
				weight = new DraftWeight();

				result->GetNextRow(binding,weight);
				weight->prepareData();
				if((schematic == NULL) || (schematic->getId() != weight->getSchemId()))
				{
					this->getSchematicByID(weight->getSchemId(), weight->getSchemGroupId());
				}
				if(schematic != NULL)
				{
					WeightsBatch* batch = schematic->getExperimentWeightsBatchByListId(asyncContainer->mBatchId);
					batch->mWeights.push_back(weight);
				}
			}
			mDatabase->DestroyDataBinding(binding);
			gLogger->log(LogManager::DEBUG,"Finished Loading Schematic Experimentation Weights.");
		}
		break;

		default:break;
	}

	mDBAsyncPool.ordered_free(asyncContainer);
}
//======================================================================================================================
DraftSchematic* SchematicManager::getSchematicByID(uint64 schematicId)
{
	SchematicMap64::iterator it = mSchematicMap.find(schematicId);

	if(it != mSchematicMap.end())
		return((*it).second);

	return NULL;
	////return mSchematicMap.find();
	//for(SchematicGroupList::iterator it = mSchematicGroupList.begin(); it != mSchematicGroupList.end(); it++)
	//{
	//	for(SchematicsList::iterator jt = (*it)->mSchematics.begin(); jt != (*it)->mSchematics.end(); jt++)
	//	{
	//		if((*jt)->getId() == schematicId)
	//			return *jt;
	//	}
	//}
	//return NULL;
}
DraftSchematic* SchematicManager::getSchematicByID(uint64 schematicId, uint32 groupid)
{
	return getSchematicByID(schematicId);
	/*for(SchematicsList::iterator jt = mSchematicGroupList[groupid]->mSchematics.begin(); jt != mSchematicGroupList[groupid]->mSchematics.end(); jt++)
	{
		if((*jt)->getId() == schematicId)
			return *jt;
	}
	return NULL;*/
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

