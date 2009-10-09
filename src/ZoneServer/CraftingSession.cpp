/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/


#include "CraftingSession.h"
#include "CraftingSessionFactory.h"
#include "PlayerObject.h"
#include "DraftSchematic.h"
#include "ResourceContainer.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/DatabaseCallback.h"
#include "WorldManager.h"
#include "MessageLib/MessageLib.h"
#include "Utils/clock.h"
#include "ObjectControllerOpcodes.h"
#include "SchematicManager.h"
#include "ObjectFactory.h"
#include "ManufacturingSchematic.h"
#include "ResourceManager.h"
#include "Inventory.h"
#include "Item.h"
#include "CraftingTool.h"
#include "CraftingStation.h"
#include "DraftSlot.h"
#include "CraftBatch.h"

//=============================================================================
//
// initiates a crafting session, we usually start at stage 1
//
CraftingSession::CraftingSession(Anh_Utils::Clock* clock,Database* database,PlayerObject* player,CraftingTool* tool,CraftingStation* station,uint32 expFlag) :
mTool(tool),
mStation(station),
mExpFlag(expFlag),
mStage(1),
mDatabase(database),
mOwner(player),
mManufacturingSchematic(NULL),
mDraftSchematic(NULL),
mItem(NULL),
mFirstFill(false),
mClock(clock),
mCriticalCount(0)
{

	// update player variables
	mOwner->setCraftingStage(mStage);
	mOwner->setExperimentationFlag(mExpFlag);
	mOwner->setExperimentationPoints(0);

	// we do not properly check for crafting stations at this point !!!
	// we need to check for crafting stations compatible with our crafting tools!!!!!
	if(station)
		mOwner->setNearestCraftingStation(station->getId());
	else
		mOwner->setNearestCraftingStation(0);

	mOwner->toggleStateOn(CreatureState_Crafting);

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
	mOwner->toggleStateOff(CreatureState_Crafting);
	mOwner->setCraftingStage(0);
	mOwner->setExperimentationFlag(1);
	mOwner->setExperimentationPoints(10);
	mOwner->setNearestCraftingStation(0);

	// send cancel session
	gMessageLib->sendSharedNetworkMessage(mOwner,0,1);
	gMessageLib->sendSystemMessage(mOwner,L"","ui_craft","session_ended");

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
			DataBinding*	binding = mDatabase->CreateDataBinding(1);
			binding->addField(DFT_uint32,0,4);

			uint32 count = static_cast<uint32>(result->getRowCount());
			
			mOwnerExpSkillMod = 0;
			if(count)
			{
				// please note that our schematic might be granted by several skills
				// thus we should now find the skillmod to use (ie the highest ?)

				// alternatively a separate db entry needs to be made
				for(uint32 i = 0;i<count;i++)
				{
					result->GetNextRow(binding,&modId);
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


			mDatabase->DestroyDataBinding(binding);

			CraftSessionQueryContainer* container = new CraftSessionQueryContainer(CraftSessionQuery_SkillmodAss,0);
			uint32 groupId = mDraftSchematic->getWeightsBatchId();

			int8 sql[550];
			sprintf(sql,"SELECT DISTINCT skills_skillmods.skillmod_id FROM draft_schematics INNER JOIN skills_schematicsgranted ON draft_schematics.group_id = skills_schematicsgranted.schem_group_id INNER JOIN skills_skillmods ON skills_schematicsgranted.skill_id = skills_skillmods.skill_id INNER JOIN skillmods ON skills_skillmods.skillmod_id = skillmods.skillmod_id WHERE draft_schematics.weightsbatch_id = %u AND skillmods.skillmod_name LIKE %s",groupId,"'%%asse%%'");
		
			mDatabase->ExecuteSqlAsyncNoArguments(this,container,sql);
			//mDatabase->ExecuteSqlAsync(this,container,sql);
		

		}
		break;

		case CraftSessionQuery_SkillmodAss:
		{
			uint32			resultId;
			DataBinding*	binding = mDatabase->CreateDataBinding(1);
			binding->addField(DFT_uint32,0,4);

			uint32 count = static_cast<uint32>(result->getRowCount());
			
			mOwnerAssSkillMod = 0;
			if(count)
			{

				// please note that our schematic might be granted by several skills
				// thus we should now find the skillmod to use (ie the highest ?)

				// alternatively a separate db entry needs to be made
				for(uint32 i = 0;i<count;i++)
				{
					result->GetNextRow(binding,&resultId);
					mAssSkillModId = resultId;
					int32 resultInt = mOwner->getSkillModValue(mAssSkillModId);
					if ((resultInt > 0)&& (resultInt > (int32) mOwnerAssSkillMod))
					{
						mOwnerAssSkillMod = resultInt;
					}
				}
	
			}
			

			mDatabase->DestroyDataBinding(binding);

			//lets get the customization data from db
			CraftSessionQueryContainer* container = new CraftSessionQueryContainer(CraftSessionQuery_CustomizationData,0);
			uint32 groupId = mDraftSchematic->getWeightsBatchId();

			int8 sql[550];
			sprintf(sql,"SELECT dsc.attribute, dsc.cust_attribute, dsc.palette_size, dsc.default_value FROM draft_schematic_customization dsc WHERE dsc.batchId = %u",groupId);
			mDatabase->ExecuteSqlAsync(this,container,sql);
			

		}
		break;

		case CraftSessionQuery_CustomizationData:
		{
			DataBinding*	binding = mDatabase->CreateDataBinding(4);
			binding->addField(DFT_bstring,offsetof(CustomizationOption,attribute),32,0);
			binding->addField(DFT_uint16,offsetof(CustomizationOption,cutomizationIndex),2,1);
			binding->addField(DFT_uint32,offsetof(CustomizationOption,paletteSize),4,2);
			binding->addField(DFT_uint32,offsetof(CustomizationOption,defaultValue),4,3);

			uint32 count = static_cast<uint32>(result->getRowCount());
			
			for(uint32 i = 0;i<count;i++)
			{
				CustomizationOption* cO = new(CustomizationOption);
				result->GetNextRow(binding,cO);
			
				cO->paletteSize =  (uint32)(cO->paletteSize /200)* getCustomization();
		
				mManufacturingSchematic->mCustomizationList.push_back(cO);
			}
			mDatabase->DestroyDataBinding(binding);

			// lets move on to stage 2, send the updates
		

			gMessageLib->sendCreateManufacturingSchematic(mManufacturingSchematic,mOwner);
			gMessageLib->sendCreateTangible(mItem,mOwner);
			gMessageLib->sendManufactureSlots(mManufacturingSchematic,mTool,mItem,mOwner);
			gMessageLib->sendUpdateCraftingStage(mOwner);
			gMessageLib->sendBaselinesMSCO_7(mManufacturingSchematic,mOwner);
		
		
		}
		break;

		// item creation complete
		case CraftSessionQuery_Prototype:
		{
			uint32			resultId;
			DataBinding*	binding = mDatabase->CreateDataBinding(1);
			binding->addField(DFT_uint32,0,4);

			result->GetNextRow(binding,&resultId);

			// all went well
			if(!resultId)
			{
				// ack and create it for the player
				gMessageLib->sendCraftAcknowledge(opCreatePrototypeResponse,CraftCreate_Success,qContainer->mCounter,mOwner);

				// schedule item for creation and update the tool timer
				mTool->initTimer((int32)(mDraftSchematic->getComplexity() * 3.0f),3000,mClock->getLocalTime());

				mTool->setAttribute("craft_tool_status","@crafting:tool_status_working");
				mDatabase->ExecuteSqlAsync(0,0,"UPDATE item_attributes SET value='@crafting:tool_status_working' WHERE item_id=%lld AND attribute_id=18",mTool->getId());
				
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

				if(mManufacturingSchematic->hasInternalAttribute("xpType"))
					xpType = mManufacturingSchematic->getInternalAttribute<float>("xpType");

				if(xp > 0.0f)
					gSkillManager->addExperience((uint32)xpType,(int32)xp,mOwner);

			}
			// something failed
			else
			{
				gMessageLib->sendCraftAcknowledge(opCreatePrototypeResponse,CraftCreate_Failure,qContainer->mCounter,mOwner);

				// end the session
				mDatabase->DestroyDataBinding(binding);
				delete(qContainer);

				gCraftingSessionFactory->destroySession(this);

				return;
			}

			mDatabase->DestroyDataBinding(binding);
		}
		break;

		default: 
		{
			//gLogger->logMsg("CraftSession: unhandled DatabaseQuery");
			gLogger->logErrorF("Crafting","CraftSession: unhandled DatabaseQuery",MSG_NORMAL);
		}
		break;
	}

	delete(qContainer);
}

//=============================================================================

void CraftingSession::handleObjectReady(Object* object,DispatchClient* client)
{
	
	Item* item = dynamic_cast<Item*>(object);

	// its the manufacturing schematic
	if(item->getItemFamily() == ItemFamily_ManufacturingSchematic)
	{
		mManufacturingSchematic = dynamic_cast<ManufacturingSchematic*>(item);

		// now request the (temporary) item, based on the draft schematic defaults
		gObjectFactory->requestNewDefaultItem(this,(mDraftSchematic->getId() >> 32),mTool->getId(),99,Anh_Math::Vector3());
	}
	// its the item
	else
	{
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
		sprintf(sql,"SELECT DISTINCT skills_skillmods.skillmod_id FROM draft_schematics INNER JOIN skills_schematicsgranted ON draft_schematics.group_id = skills_schematicsgranted.schem_group_id INNER JOIN skills_skillmods ON skills_schematicsgranted.skill_id = skills_skillmods.skill_id INNER JOIN skillmods ON skills_skillmods.skillmod_id = skillmods.skillmod_id WHERE draft_schematics.weightsbatch_id = %u AND skillmods.skillmod_name LIKE %s",groupId,"'%%exper%%'");
		
		//% just upsets the standard query
		mDatabase->ExecuteSqlAsyncNoArguments(this,container,sql);
		//mDatabase->ExecuteSqlAsync(this,container,sql);
		
		
		
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
		return(false);

	// get the schematic from the filtered list
	uint32 schemCrc = ((filteredPlayerSchematics->at(schematicIndex))>>32);

	mDraftSchematic = gSchematicManager->getSchematicBySlotId(schemCrc);

	if(!mDraftSchematic)
	{
		//gLogger->logMsgF("CraftingSession::selectDraftSchematic: not found %u",MSG_NORMAL,schemCrc);
		gLogger->logErrorF("Crafting","CraftingSession::selectDraftSchematic: not found crc:%u",MSG_NORMAL,schemCrc);
		return(false);
	}

	// temporary check until all items are craftable
	if(!mDraftSchematic->isCraftEnabled())
	{
		gLogger->logErrorF("Crafting","CraftingSession::selectDraftSchematic: schematic not craftable crc:%u",MSG_NORMAL,schemCrc);
		gMessageLib->sendSystemMessage(mOwner,L"This item is currently not craftable.");
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
			if((*manSlotIt)->getmFilledIndicator()==4)
				bagResource((*manSlotIt),mOwner->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory)->getId());
			else
				bagComponents((*manSlotIt),mOwner->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory)->getId());

			(*manSlotIt)->setmFilledIndicator(0);

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

	//4 is resource
	//2 is identical component 
	//3 is like component

	if (manSlot->mDraftSlot->getType() == 4)
	{
		handleFillSlotResource(resContainerId, slotId, unknown, counter);
	}
	else
		handleFillSlotComponent(resContainerId, slotId, unknown, counter);
}

//=============================================================================
//
// get the serials crc for a (filled)crafting slot
//

uint32 CraftingSession::getComponentSerial(ManufactureSlot*	manSlot, Inventory* inventory)
{

	Item*		filledComponent;
	FilledResources::iterator filledResIt = manSlot->mFilledResources.begin();
	while(filledResIt != manSlot->mFilledResources.end())
	{
		uint64		itemId = (*filledResIt).first;
		string filledSerial;

		filledComponent = dynamic_cast<Item*>(inventory->getObjectById(itemId));
		if(filledComponent->hasAttribute("serial"))
			filledSerial = filledComponent->getAttribute<std::string>("serial").c_str();
		else
			filledSerial ="";
		
		return(filledSerial.getCrc());
		++filledResIt;
	
	}
	return(0);
}

//=============================================================================
//
// Adjust an Items amount / delete it
//

bool CraftingSession::AdjustComponentStack(Item* item, Inventory* inventory, uint32 uses)
{
	//is this a stack ???
	if(item->hasAttribute("stacksize"))
	{
		//alter stacksize, delete if necessary

		uint32 stackSize;
		stackSize = item->getAttribute<uint32>("stacksize");

		if(stackSize > uses)
		{
			//just adjust the stacks size
		}
		return true;
	}

	//no stack, just a singular item
	if(uses == 1)
	{
		//remove the item out of the inventory
		inventory->removeObject(item);

		//and add it to the manufacturing schematic
		item->setParentId(mManufacturingSchematic->getId());
		mManufacturingSchematic->addData(item);
		//the link will update the inventories item count
		gMessageLib->sendContainmentMessage(item->getId(),item->getParentId(),4,mOwner);

		//the db will only be updated if we really use the ´component!!
		//at this point we might still put it out again or cancel the crafting session!!!!

		return true;
	}

	return false;

}



//=============================================================================
//
// a component gets put into a manufacturing slot
//

void CraftingSession::handleFillSlotComponent(uint64 componentId,uint32 slotId,uint32 unknown,uint8 counter)
{

	ManufactureSlot*	manSlot			= mManufacturingSchematic->getManufactureSlots()->at(slotId);	

	Inventory* inventory = dynamic_cast<Inventory*>(mOwner->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));

	//remove the component out of the inventory and attach it to the man schematic


	//hardcoded to 1 until stacks are in
	uint32				availableAmount		= 1;
	uint32				existingAmount		= 0;
	uint32				totalNeededAmount	= manSlot->mDraftSlot->getAmount();


	string				componentSerial	= "";
	string				filledSerial	= "";

	Item*		component	= dynamic_cast<Item*>(inventory->getObjectById(componentId));
	
	if(component->hasAttribute("serial"))
		componentSerial = component->getAttribute<std::string>("serial").c_str();

						 	
//	bool resourceBool = false;
	bool smallupdate = false;
	
	if((!component) || (!manSlot))
	{
		gMessageLib->sendCraftAcknowledge(opCraftFillSlot,CraftError_Ingredient_Not_In_Inventory,counter,mOwner);
		return;
	}

	// components need the same serial
	// see if something is filled already
	FilledResources::iterator filledResIt = manSlot->mFilledResources.begin();
	
	//get the amount of already filled components
	while(filledResIt != manSlot->mFilledResources.end())
	{
		
		existingAmount += (*filledResIt).second;
		++filledResIt;
	}

	// update the needed amount
	totalNeededAmount -= existingAmount;

	// fail if its already complete
	if(!totalNeededAmount)
	{
		gMessageLib->sendCraftAcknowledge(opCraftFillSlot,CraftError_Slot_Already_Full,counter,mOwner);
		return;
	}

	// see how much this component stack has to offer
	// slot completely filled
	if(availableAmount >= totalNeededAmount)
	{
		// add to the already filled components

		filledResIt				= manSlot->mFilledResources.begin();
		
		if(manSlot->mFilledResources.size()&&(getComponentSerial(manSlot, inventory) == componentSerial.getCrc()))
		{
			manSlot->mFilledResources.push_back(std::make_pair(component->getId(),totalNeededAmount));
			filledResIt				= manSlot->mFilledResources.begin();
			manSlot->setmFilledIndicator(manSlot->mDraftSlot->getType());
			manSlot->mFilledIndicatorChange = true;
			smallupdate = true;
		}
		else
		if(manSlot->mFilledResources.size())
		{
			gMessageLib->sendCraftAcknowledge(opCraftFillSlot,CraftError_Internal_Invalid_Ingredient,counter,mOwner);
			return;
		}
		else
		if(manSlot->mFilledResources.empty())
		{
			//resourceBool = true;
			// only allow one unique type
			
			manSlot->mFilledResources.push_back(std::make_pair(component->getId(),totalNeededAmount));
			manSlot->setmFilledIndicator(manSlot->mDraftSlot->getType());
			manSlot->mFilledIndicatorChange = true;
			//link it to the schematic ?
	
		}
	
		if(!AdjustComponentStack(component,inventory,totalNeededAmount))
		{
			gMessageLib->sendCraftAcknowledge(opCraftFillSlot,CraftError_Internal_Invalid_Ingredient_Size,counter,mOwner);
			return;
		}
		

		// update the slot total resource amount
		manSlot->mFilled = manSlot->mDraftSlot->getAmount();

		// update the total count of filled slots
		mManufacturingSchematic->addFilledSlot();
		gMessageLib->sendUpdateFilledManufactureSlots(mManufacturingSchematic,mOwner);
	}
	// got only a bit of the required amount
	else//if(availableAmount >= totalNeededAmount)
	{
		// add a new entry
		filledResIt				= manSlot->mFilledResources.begin();

		if((getComponentSerial(manSlot, inventory) == componentSerial.getCrc())&&manSlot->mFilledResources.size())
		{
			//(*filledResIt).second += availableAmount;
			manSlot->mFilledResources.push_back(std::make_pair(componentId,availableAmount));
			filledResIt				= manSlot->mFilledResources.begin();
			manSlot->setmFilledIndicator(manSlot->mDraftSlot->getType());
			manSlot->mFilledIndicatorChange = true;
			smallupdate = true;
		}

		else
		if(!manSlot->mFilledResources.empty())
		{
			gMessageLib->sendCraftAcknowledge(opCraftFillSlot,CraftError_Bad_Resource_Chosen,counter,mOwner);
			return;
		}
		// nothing of that resource filled, add a new entry
		else
		if(manSlot->mFilledResources.empty())
		{
			
			manSlot->mFilledResources.push_back(std::make_pair(componentId,availableAmount));
			manSlot->setmFilledIndicator(manSlot->mDraftSlot->getType());
			manSlot->mFilledIndicatorChange = true;
			
		}

		if(!AdjustComponentStack(component,inventory,totalNeededAmount))
		{
			gMessageLib->sendCraftAcknowledge(opCraftFillSlot,CraftError_Internal_Invalid_Ingredient_Size,counter,mOwner);
			return;
		}

		// update the slot total resource amount
		manSlot->mFilled += availableAmount;
		
	}

	// update the slot contents, send all slots on first fill
	if(!mFirstFill)
	{
		mFirstFill = true;
		gMessageLib->sendDeltasMSCO_7(mManufacturingSchematic,mOwner);
	}
	else if(smallupdate == true)
	{
		gMessageLib->sendManufactureSlotUpdateSmall(mManufacturingSchematic,static_cast<uint8>(slotId),mOwner);
	}
	else
	{
		gMessageLib->sendManufactureSlotUpdate(mManufacturingSchematic,static_cast<uint8>(slotId),mOwner);
	}		

	// done
	gMessageLib->sendCraftAcknowledge(opCraftFillSlot,CraftError_None,counter,mOwner);
	
}

//=============================================================================
//
// an amount of resource gets put into a manufacturing slot
//

void CraftingSession::handleFillSlotResource(uint64 resContainerId,uint32 slotId,uint32 unknown,uint8 counter)
{
	// update resource container
	ResourceContainer*	resContainer	= dynamic_cast<ResourceContainer*>(gWorldManager->getObjectById(resContainerId));
	ManufactureSlot*	manSlot			= mManufacturingSchematic->getManufactureSlots()->at(slotId);	

	//bool resourceBool = false;
	bool smallupdate = false;
	if(resContainer && manSlot)
	{
		uint32	availableAmount		= resContainer->getAmount();
		uint32	totalNeededAmount	= manSlot->mDraftSlot->getAmount();
		uint32	existingAmount		= 0;

		// see if something is filled already
		FilledResources::iterator filledResIt = manSlot->mFilledResources.begin();

		while(filledResIt != manSlot->mFilledResources.end())
		{
			
			existingAmount += (*filledResIt).second;
			++filledResIt;
		}

		// update the needed amount
		totalNeededAmount -= existingAmount;

		// fail if its already complete
		if(!totalNeededAmount)
		{
			gMessageLib->sendCraftAcknowledge(opCraftFillSlot,CraftError_Slot_Already_Full,counter,mOwner);
			return;
		}

		// see how much this container has to offer
		// slot completely filled
		if(availableAmount >= totalNeededAmount)
		{
			// add to the filled resources
			uint64 containerResId	= resContainer->getResourceId();
			filledResIt				= manSlot->mFilledResources.begin();
			
			while(filledResIt != manSlot->mFilledResources.end())
			{
				// already got something of that type filled
				if(containerResId == (*filledResIt).first)
				{
					//hark in live the same resource gets added to a second slot
					manSlot->mFilledResources.push_back(std::make_pair(containerResId,totalNeededAmount));
					filledResIt				= manSlot->mFilledResources.begin();
					manSlot->setmFilledIndicator(4);//4  resource has been filled
					smallupdate = true;
					break;
				}

				++filledResIt;
			}

			// nothing of that resource filled, add a new entry
			if(filledResIt == manSlot->mFilledResources.end())
			{
				//resourceBool = true;
				// only allow one unique type
				if(manSlot->mFilledResources.empty())
				{
					manSlot->mFilledResources.push_back(std::make_pair(containerResId,totalNeededAmount));
					manSlot->setmFilledIndicator(4);
					manSlot->mFilledIndicatorChange = true;
				}
				else
				{
					gMessageLib->sendCraftAcknowledge(opCraftFillSlot,CraftError_Bad_Resource_Chosen,counter,mOwner);
					return;
				}
			}
		
			// update the container amount
			uint32 newContainerAmount = availableAmount - totalNeededAmount;

			// destroy if its empty
			if(!newContainerAmount)
			{
				//now destroy it client side
				gMessageLib->sendDestroyObject(resContainerId,mOwner);
				
				
				gObjectFactory->deleteObjectFromDB(resContainer);
				dynamic_cast<Inventory*>(mOwner->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory))->deleteObject(resContainer);
			}
			// update it
			else
			{
				resContainer->setAmount(newContainerAmount);
				gMessageLib->sendResourceContainerUpdateAmount(resContainer,mOwner);
				mDatabase->ExecuteSqlAsync(NULL,NULL,"UPDATE resource_containers SET amount=%u WHERE id=%lld",newContainerAmount,resContainer->getId());
			}

			// update the slot total resource amount
			manSlot->mFilled = manSlot->mDraftSlot->getAmount();

			// update the total count of filled slots
			mManufacturingSchematic->addFilledSlot();
			gMessageLib->sendUpdateFilledManufactureSlots(mManufacturingSchematic,mOwner);
		}
		// got only a bit
		else
		{
			// add to the filled resources
			uint64 containerResId	= resContainer->getResourceId();
			filledResIt				= manSlot->mFilledResources.begin();

			while(filledResIt != manSlot->mFilledResources.end())
			{
				// already got something of that type filled
				if(containerResId == (*filledResIt).first)
				{
					//(*filledResIt).second += availableAmount;
					manSlot->mFilledResources.push_back(std::make_pair(containerResId,availableAmount));
					filledResIt				= manSlot->mFilledResources.begin();
					manSlot->setmFilledIndicator(4);
					smallupdate = true;
					break;
				}

				++filledResIt;
			}

			// nothing of that resource filled, add a new entry
			if(filledResIt == manSlot->mFilledResources.end())
			{
				// only allow one unique type
				if(manSlot->mFilledResources.empty())
				{
					manSlot->mFilledResources.push_back(std::make_pair(containerResId,availableAmount));
					manSlot->setmFilledIndicator(4);
				}
				else
				{
					gMessageLib->sendCraftAcknowledge(opCraftFillSlot,CraftError_Bad_Resource_Chosen,counter,mOwner);
					return;
				}
			}

			// destroy the container as its empty now
			gMessageLib->sendDestroyObject(resContainerId,mOwner);
			gObjectFactory->deleteObjectFromDB(resContainer);
			dynamic_cast<Inventory*>(mOwner->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory))->deleteObject(resContainer);

			// update the slot total resource amount
			manSlot->mFilled += availableAmount;
		}

		// update the slot contents, send all slots on first fill
		// we need to make sure we only update lists with changes, so the lists dont desynchronize!
		if(!mFirstFill)
		{
			mFirstFill = true;
			gMessageLib->sendDeltasMSCO_7(mManufacturingSchematic,mOwner);
		}
		else if(smallupdate == true)
		{
			gMessageLib->sendManufactureSlotUpdateSmall(mManufacturingSchematic,static_cast<uint8>(slotId),mOwner);
		}
		else
		{
			gMessageLib->sendManufactureSlotUpdate(mManufacturingSchematic,static_cast<uint8>(slotId),mOwner);
		}		

		// done
		gMessageLib->sendCraftAcknowledge(opCraftFillSlot,CraftError_None,counter,mOwner);
	}
	// failed locating the container
	else
	{
		gMessageLib->sendCraftAcknowledge(opCraftFillSlot,CraftError_Ingredient_Not_In_Inventory,counter,mOwner);
	}
}



//=============================================================================
//
// filled components get returned to the inventory
//

void CraftingSession::bagComponents(ManufactureSlot* manSlot,uint64 containerId)
{
	//add the components back to the inventory
	manSlot->setmFilledIndicator(0);

	Inventory* inventory = dynamic_cast<Inventory*>(mOwner->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));

	FilledResources::iterator resIt = manSlot->mFilledResources.begin();
	while(resIt != manSlot->mFilledResources.end())
	{
		Item*	filledComponent	= dynamic_cast<Item*>(mManufacturingSchematic->getDataById((*resIt).first));

		if(!filledComponent)
		{
			gLogger->logMsgF("CraftingSession::bagComponents filledComponent not found",MSG_HIGH);
			return;
		}
		mManufacturingSchematic->removeData((*resIt).first);
		
		//add to inventory
		inventory->addObject(filledComponent);

		filledComponent->setParentId(inventory->getId());
		//do we have to uncontain them off the schematic?
		gMessageLib->sendContainmentMessage(filledComponent->getId(),filledComponent->getParentId(),4,mOwner);
		resIt++;

	}
	
}

//=============================================================================
//
// filled resources get returned to the inventory
//


void CraftingSession::bagResource(ManufactureSlot* manSlot,uint64 containerId)
{
	//iterates through the slots filled resources
	//respectively create a new one if necessary

	//TODO : what happens if the container is full ? 
	
	FilledResources::iterator resIt = manSlot->mFilledResources.begin();
	
	manSlot->setmFilledIndicator(0);
	
	while(resIt != manSlot->mFilledResources.end())
	{
		uint32 amount = (*resIt).second;

		// see if we can add it to an existing container
		ObjectList*	invObjects = dynamic_cast<Inventory*>(mOwner->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory))->getObjects();
		ObjectList::iterator listIt = invObjects->begin();

		bool	foundSameType	= false;

		while(listIt != invObjects->end())
		{
			// we are looking for resource containers
			if(ResourceContainer* resCont = dynamic_cast<ResourceContainer*>(*listIt))
			{
				uint32 targetAmount	= resCont->getAmount();
				uint32 maxAmount	= resCont->getMaxAmount();
				uint32 newAmount;

				if((resCont->getResourceId() == (*resIt).first) && (targetAmount < maxAmount))
				{
					foundSameType = true;

					newAmount = targetAmount + amount;

					if(newAmount  <= maxAmount)
					{
						// update target container
						resCont->setAmount(newAmount);

						gMessageLib->sendResourceContainerUpdateAmount(resCont,mOwner);

						gWorldManager->getDatabase()->ExecuteSqlAsync(NULL,NULL,"UPDATE resource_containers SET amount=%u WHERE id=%I64u",newAmount,resCont->getId());
					}
					// target container full, put in what fits, create a new one
					else if(newAmount > maxAmount)
					{
						uint32 selectedNewAmount = newAmount - maxAmount;

						resCont->setAmount(maxAmount);

						gMessageLib->sendResourceContainerUpdateAmount(resCont,mOwner);
						gWorldManager->getDatabase()->ExecuteSqlAsync(NULL,NULL,"UPDATE resource_containers SET amount=%u WHERE id=%I64u",maxAmount,resCont->getId());

						gObjectFactory->requestNewResourceContainer(dynamic_cast<Inventory*>(mOwner->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory)),(*resIt).first,mOwner->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory)->getId(),99,selectedNewAmount);
					}

					break;
				}
			}

			++listIt;
		}

		// or need to create a new one
		if(!foundSameType)
		{
			gObjectFactory->requestNewResourceContainer(dynamic_cast<Inventory*>(mOwner->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory)),(*resIt).first,mOwner->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory)->getId(),99,amount);
		}

		++resIt;
	}
	
}

//=============================================================================
//
// a manufacture slot is emptied
//

void CraftingSession::emptySlot(uint32 slotId,ManufactureSlot* manSlot,uint64 containerId)
{

	//get ressources back in their stack
	//or components back in the inventory

	if(manSlot->getmFilledIndicator() == 4)
		bagResource(manSlot,containerId);
	else
		bagComponents(manSlot,containerId);
	
	// update the slot
	manSlot->mFilledResources.clear();
	manSlot->setmFilledIndicator(0);

	// if it was completely filled, update the total amount of filled slots
	if(manSlot->mFilled == manSlot->mDraftSlot->getAmount())
	{
		mManufacturingSchematic->removeFilledSlot();
		gMessageLib->sendUpdateFilledManufactureSlots(mManufacturingSchematic,mOwner);
	}

	if(manSlot->mFilled > 0)
	{
		manSlot->mFilled = 0;
		//only send when changes !!!!!
		gMessageLib->sendManufactureSlotUpdate(mManufacturingSchematic,static_cast<uint8>(slotId),mOwner);
	}
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

uint8 CraftingSession::_assembleRoll()
{
	// assembly roll, needs to be improved, maybe make a pool to draw results from, which is populated based on the skillmod
	//int32 assRoll = (int32)(floor((float)(gRandom->getRand()%9) - ((float)assMod / 20.0f)));

	int32 assRoll;
	int32 riskRoll;
	
	float ma		= _calcAverageMalleability();

	// make sure the values are valid and dont crash us
	if((mOwnerAssSkillMod > 100)||(mOwnerAssSkillMod < 0))
	{
		mOwnerAssSkillMod = 0;
	}

	
	float rating	= 50.0f + (ma - 500.0f) / 40.0f +  mOwnerAssSkillMod - 5.0f + (mToolEffectivity/10);
	//gLogger->logMsgF("CraftingSession:: relevant rating %f",MSG_NORMAL,rating);
	gLogger->logErrorF("Crafting","CraftingSession::_assembleRoll() relevant rating %f",MSG_NORMAL,rating);
	
	rating	+= (mToolEffectivity/10);
	
	gLogger->logErrorF("Crafting","CraftingSession::_assembleRoll() relevant rating modified with tool %f",MSG_NORMAL,rating);

	float risk		= 100.0f - rating;
	
	gLogger->logErrorF("Crafting","CraftingSession::_assembleRoll() relevant Skill Mod %u",MSG_NORMAL,mOwnerAssSkillMod);
	gLogger->logErrorF("Crafting","CraftingSession::_assembleRoll() relevant risk %f",MSG_NORMAL,risk);
	

	mManufacturingSchematic->setExpFailureChance(risk);

	riskRoll		= (int32)(floor(((double)gRandom->getRand() / (RAND_MAX  + 1.0f) * (100.0f - 1.0f) + 1.0f)));

	gLogger->logErrorF("Crafting","CraftingSession::_assembleRoll() relevant riskroll %u",MSG_NORMAL,riskRoll);


	// ensure that every critical makes the nect critical less likely
	// we dont want to have more than 3 criticals in a row

	gLogger->logErrorF("Crafting","CraftingSession::_assembleRoll() relevant criticalCount %u",MSG_NORMAL,mCriticalCount);
	
	riskRoll += (mCriticalCount*5);
	gLogger->logErrorF("Crafting","CraftingSession::_assembleRoll() modified riskroll %u",MSG_NORMAL,riskRoll);
	
	if(mCriticalCount = 3)
		riskRoll = static_cast<uint32>(risk+1);

	if(riskRoll <= risk)
	{
		//ok critical failure time !
		mCriticalCount++;
		return(8);
	}
	else
		mCriticalCount = 0;

	//ok we have some sort of success
	assRoll = (int32)floor((double)gRandom->getRand() / (RAND_MAX  + 1.0f) * (100.0f - 1.0f) + 1.0f) ;

	//gLogger->logMsgF("CraftingSession:: assembly Roll preMod %u",MSG_NORMAL,assRoll);

	int32 modRoll = static_cast<uint32>(((assRoll - (rating * 0.4f)) / 15.0f) - (mToolEffectivity / 50.0f));
	
	//gLogger->logMsgF("CraftingSession:: assembly Roll postMod %u",MSG_NORMAL,modRoll);

	//0 is amazing success
	//1 is great success
	//2 is good success
	//3 success
	//4 is moderate success
	//5 marginally successful
	//6 ok
	//7 barely succesfull
	//8 critical failure
	
	// make sure we are in valid range
	if(modRoll < 0)
		modRoll = 0;

	else if(modRoll > 7)
		modRoll = 7;

	return static_cast<uint8>(modRoll);
}

//=============================================================================
//
// assemble the item(s)
//

void CraftingSession::assemble(uint32 counter)
{
	ExperimentationProperties*			expPropertiesList	= mManufacturingSchematic->getExperimentationProperties();
	ExperimentationProperties::iterator	expIt				= expPropertiesList->begin();

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

		while(expIt!= expPropertiesList->end())
		{
			(*expIt)->mBlueBarSize = ((*expIt)->mBlueBarSize * 0.9f);
			
			++expIt;
		}

		//now empty the slots
		uint8 amount = mManufacturingSchematic->getManufactureSlots()->size();

	    for (uint8 i = 0; i < amount; i++)
		{

			ManufactureSlot* manSlot = mManufacturingSchematic->getManufactureSlots()->at(i);

			if(manSlot)
			{
				emptySlot(i,manSlot,mOwner->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory)->getId());

				gMessageLib->sendCraftAcknowledge(opCraftEmptySlot,CraftError_None,static_cast<uint8>(counter),mOwner);

			}
		}
		// done
		

		gMessageLib->sendGenericIntResponse(assRoll,static_cast<uint8>(counter),mOwner);
		return;
	}

	// ---------------------------------------------
	// it wasnt a critical so go on
		
	mOwner->setCraftingStage(mStage);
	gMessageLib->sendUpdateCraftingStage(mOwner);
	
	
	// update item complexity
	mItem->setComplexity(static_cast<float>(mDraftSchematic->getComplexity()));
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
			// ceil and cut off, when it needs to be an integer
			if(att->getType())
			{
				int32 intAtt = (int32)(ceil(attValue));

				mItem->setAttribute(att->getAttributeKey(),boost::lexical_cast<std::string>(intAtt));
				mDatabase->ExecuteSqlAsync(0,0,"UPDATE item_attributes SET value='%i' WHERE item_id=%lld AND attribute_id=%u",intAtt,mItem->getId(),att->getAttributeId());
			}
			else
			{
				//attValue = Anh_Math::roundF(attValue,2);

				mItem->setAttribute(att->getAttributeKey(),boost::lexical_cast<std::string>(attValue));
				mDatabase->ExecuteSqlAsync(0,0,"UPDATE item_attributes SET value='%.2f' WHERE item_id=%lld AND attribute_id=%u",attValue,mItem->getId(),att->getAttributeId());
			}

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

	mOwner->setCraftingStage(mStage);
	gMessageLib->sendUpdateCraftingStage(mOwner);

	gMessageLib->sendGenericIntResponse(4,static_cast<uint8>(counter),mOwner);
}

//=============================================================================

void CraftingSession::creationStage(uint32 counter)
{
	mStage = 5;

	mOwner->setCraftingStage(mStage);
	gMessageLib->sendUpdateCraftingStage(mOwner);

	gMessageLib->sendGenericIntResponse(4,static_cast<uint8>(counter),mOwner);	
}

//=============================================================================

void CraftingSession::experimentationStage(uint32 counter)
{
	mStage = 4;

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

string CraftingSession::getSerial()
{
	int8	serial[12],chance[9];
	bool	found = false;
	uint8	u;

	for(uint8 i = 0; i < 8; i++)
	{
		while(!found)
		{
			found = true;
			u = static_cast<uint8>(static_cast<double>(gRandom->getRand()) / (RAND_MAX + 1.0f) * (122.0f - 48.0f) + 48.0f);

			//only 1 to 9 or a to z
			if((u >55)&&(u <97))
				found = false;

			if((u < 48)||(u >122))
				found = false;
			
		}
		chance[i] = u;
		found = false;
	}
	chance[8] = 0;

	sprintf(serial,"(%s)",chance);

	return(BString(serial));
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
		sprintf(sql,"UPDATE items SET parent_id=%lld, customName='",mOwner->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory)->getId());
		sqlPointer = sql + strlen(sql);
		sqlPointer += mDatabase->Escape_String(sqlPointer,mItem->getCustomName().getAnsi(),mItem->getCustomName().getLength());
		sprintf(restStr,"' WHERE id=%lld ",mItem->getId());
		strcat(sql,restStr);

		t->addQuery(sql);

		// add the crafter name attribute
		if(mItem->hasAttribute("crafter"))
		{
			int8 s[64];
			mDatabase->Escape_String(s,mOwner->getFirstName().getAnsi(),mOwner->getFirstName().getLength());
			mItem->setAttribute("crafter",mOwner->getFirstName().getAnsi());
			sprintf(sql,"UPDATE item_attributes SET value='%s' WHERE item_id=%lld AND attribute_id=%u",s,mItem->getId(),AttrType_crafter);
		}
		else
		{
			mItem->addAttribute("crafter",mOwner->getFirstName().getAnsi());
		
			sprintf(sql,"INSERT INTO item_attributes VALUES(%I64u,%u,'",mItem->getId(),AttrType_crafter);
			sqlPointer = sql + strlen(sql);
			sqlPointer += mDatabase->Escape_String(sqlPointer,mOwner->getFirstName().getAnsi(),mOwner->getFirstName().getLength());
			sprintf(restStr,"',%u,0)",mItem->getAttributeMap()->size());
			strcat(sql,restStr);

		}
		t->addQuery(sql);

		// now the serial
		string serial;
		serial = getSerial();

		if(mItem->hasAttribute("serial_number"))
		{
			mItem->setAttribute("serial_number",serial.getAnsi());
			sprintf(sql,"UPDATE item_attributes SET value='%s' WHERE item_id=%lld AND attribute_id=%u",serial.getAnsi(),mItem->getId(),AttrType_serial_number);
		}
		else		
		{

			sprintf(sql,"INSERT INTO item_attributes VALUES(%I64u,%u,'",mItem->getId(),AttrType_serial_number);
			sqlPointer = sql + strlen(sql);
			sqlPointer += mDatabase->Escape_String(sqlPointer,serial.getAnsi(),serial.getLength());
			sprintf(restStr,"',%u,0)",mItem->getAttributeMap()->size());
			strcat(sql,restStr);
			mItem->addAttribute("serial_number",serial.getAnsi());
		}
		
		t->addQuery(sql);
		t->execute();

	}
	else
	{
		float xp = 0.0f;
		float xpType = 22.0f;

		if(mManufacturingSchematic->hasAttribute("xp"))
			xp = mManufacturingSchematic->getAttribute<float>("xp");

		if(mManufacturingSchematic->hasInternalAttribute("xpType"))
			xpType = mManufacturingSchematic->getInternalAttribute<float>("xpType");

		// add some cooldown time
		mTool->initTimer((int32)(mDraftSchematic->getComplexity() * 2.0f),3000,mClock->getLocalTime());

		mTool->setAttribute("craft_tool_status","@crafting:tool_status_working");
		mDatabase->ExecuteSqlAsync(0,0,"UPDATE item_attributes SET value='@crafting:tool_status_working' WHERE item_id=%lld AND attribute_id=%u",mTool->getId(),AttrType_CraftToolStatus);

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
}

//=============================================================================
//gets the type of success / failure for experimentation

uint8 CraftingSession::_experimentRoll(uint32 expPoints)
{

	// get the assembly mod, TODO: match to schematic groups
	int32 assMod = mOwner->getSkillModValue(SMod_general_assembly);

	int32 assRoll;
	int32 riskRoll;
	
	float ma		= _calcAverageMalleability();

	float rating	= 50.0f + (ma - 500.0f) / 40.0f +  assMod - (5.0f * expPoints);

	float risk		= 100.0f - rating;

	riskRoll		= (int32)(floor(((double)gRandom->getRand() / (RAND_MAX  + 1.0f) * (100.0f - 1.0f) + 1.0f)));

	if(riskRoll <= risk)
	{
		//ok we have some sort of failure
		assRoll = (int32)(floor((double)gRandom->getRand() / (RAND_MAX  + 1.0f) * (100.0f - 1.0f) + 1.0f));
		
		int32 modRoll = (int32)(floor((double)(assRoll / 25)) + 4);
		
		if(modRoll < 4)
			modRoll = 4;

		else if(modRoll > 8)
			modRoll = 8;
		
		return static_cast<uint8>(modRoll);
	}

	mManufacturingSchematic->setExpFailureChance(risk);
	
	//ok we have some sort of success
	assRoll = (int32) floor( (double)gRandom->getRand() / (RAND_MAX  + 1.0f) * (100.0f - 1.0f) + 1.0f) ;

	gLogger->logErrorF("crafting","CraftingSession:: assembly Roll preMod %u",MSG_NORMAL,assRoll);

	int32 modRoll = static_cast<int32>(((assRoll - (rating * 0.4f)) / 15.0f) - (mToolEffectivity / 50.0f));

	++modRoll;

	//int32 modRoll = (gRandom->getRand() - (rating*0.2))/15;
	gLogger->logErrorF("crafting","CraftingSession:: assembly Roll postMod %u",MSG_NORMAL,modRoll);

	//0 is amazing success
	//1 is great success
	//2 is good success
	//3 moderate success
	//4 success
	//5 failure
	//6 moderate failure
	//7 big failure
	//8 critical failure
	
	
	// make sure we are in valid range
	if(modRoll < 0)
		modRoll = 0;

	else if(modRoll > 4)
		modRoll = 4;

	return static_cast<uint8>(modRoll);
}

//=============================================================================

void CraftingSession::experiment(uint8 counter,std::vector<std::pair<uint32,uint32> > properties)
{
	// a list containing every exp property ONCE (the first if one is represented several times)
	ExperimentationPropertiesStore*		expPropList = mManufacturingSchematic->getExperimentationPropertiesStore();

	// a list containing ALL exp properties - including the ones appearing more often than once
	ExperimentationProperties*			expAllProps = mManufacturingSchematic->getExperimentationProperties();
	ExperimentationProperties::iterator itAll =	 expAllProps->begin();

	
	
	uint32				expPoints = 0;

	//this is the list containing the assigned experimentation points
	std::vector<std::pair<uint32,uint32> >::iterator it = properties.begin();
	
	// collect the total of spend exp points
	while(it != properties.end())
	{
		expPoints +=(*it).first; 

		++it;
	}

	// here we accumulate our experimentation rolls
	uint16	accumulatedRoll = 0;
	uint8	rollCount = 0;

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

	uint8 roll;
	while(it != properties.end())
	{
		ExperimentationProperty* expProperty = expPropList->at((*it).second).second;
		gLogger->logMsgF("CraftingSession:: experiment expProperty : %s",MSG_NORMAL,expProperty->mExpAttributeName.getAnsi());

		// make sure that we only experiment once for exp properties that might be entered twice in our list !!!!
		if(expProperty->mRoll == -1)
		{
			gLogger->logMsgF("CraftingSession:: expProperty is a Virgin!",MSG_NORMAL);
			// get our Roll and take into account the relevant modifiers
			roll			= _experimentRoll(expPoints);
			
			// now go through all properties and mark them when its this one!
			// so we dont experiment two times on it!
			itAll =	 expAllProps->begin();
			while(itAll != expAllProps->end())
			{
				ExperimentationProperty* tempProperty = (*itAll);
				
				gLogger->logMsgF("CraftingSession:: now testing expProperty : %s",MSG_NORMAL,tempProperty->mExpAttributeName.getAnsi());
				if(expProperty->mExpAttributeName.getCrc() == tempProperty->mExpAttributeName.getCrc())
				{
					gLogger->logMsgF("CraftingSession:: yay :) lets assign it our roll : %u",MSG_NORMAL,roll);
					tempProperty->mRoll = roll;
				}
				
				itAll++;
			}
			
		}
		else
		{
			roll = static_cast<uint8>(expProperty->mRoll);
			gLogger->logMsgF("CraftingSession:: experiment expProperty isnt a virgin anymore ...(roll:%u)",MSG_NORMAL,roll);
		}

		
		accumulatedRoll += roll;
		rollCount++;
		float percentage	= 0.0f;

		switch(roll)
		{
			case 0 :	percentage = 0.08f;	break;
			case 1 :	percentage = 0.07f;	break;
			case 2 :	percentage = 0.06f;	break;
			case 3 :	percentage = 0.02f;	break;
			case 4 :	percentage = 0.01f;	break;
			case 5 :	percentage = -0.0175f;	break; //failure
			case 6 :	percentage = -0.035f;	break;//moderate failure
			case 7 :	percentage = -0.07f;	break;//big failure
			case 8 :	percentage = -0.14f;	break;//critical failure
		}

		if((roll < 1) || (roll >4))
		{
			float malus = 1.0f + percentage;
			expProperty->mBlueBarSize = (expProperty->mBlueBarSize * malus);
		}

		float add = percentage;// attribute->mExpAttributeValue * percentage;

		expProperty->mExpAttributeValue += add * (*it).first;

		expPoints += (*it).first;

		// update item attributes
		CraftAttributes::iterator caIt = expProperty->mAttributes->begin();

		//one exp attribute can have several item attributes!!!
		while(caIt != expProperty->mAttributes->end())
		{
			CraftAttribute* att = (*caIt);
			
			//gLogger->logMsgF("CraftingSession:: experiment attribute ID %u",MSG_NORMAL,att->getAttributeId());

			float attValue	= att->getMin() + ((att->getMax() - att->getMin()) * expProperty->mExpAttributeValue);
			
			if(attValue > att->getMax())
				attValue = att->getMax();

			//gLogger->logMsgF("CraftingSession:: experiment attribute value %f",MSG_NORMAL,attValue);
			// ceil and cut off, when it needs to be an integer
			if(att->getType())
			{
				int32 intAtt = (int32)(ceil(attValue));

				mItem->setAttribute(att->getAttributeKey(),boost::lexical_cast<std::string>(intAtt));
				mDatabase->ExecuteSqlAsync(0,0,"UPDATE item_attributes SET value='%i' WHERE item_id=%lld AND attribute_id=%u",intAtt,mItem->getId(),att->getAttributeId());
			}
			else
			{
				//attValue = Anh_Math::roundF(attValue,2);

				mItem->setAttribute(att->getAttributeKey(),boost::lexical_cast<std::string>(attValue));
				mDatabase->ExecuteSqlAsync(0,0,"UPDATE item_attributes SET value='%.2f' WHERE item_id=%lld AND attribute_id=%u",attValue,mItem->getId(),att->getAttributeId());
			}

			++caIt;
		}

		++it;
	}

	// get the gross of our exp rolls for the message
	roll = (uint8)   accumulatedRoll/rollCount;
	
	// update left exp points
	mOwner->setExperimentationPoints(mOwner->getExperimentationPoints() - expPoints);

	// send updates
	gMessageLib->sendAttributeDeltasMSCO_7(mManufacturingSchematic,mOwner);
	//gMessageLib->sendDeltasMSCO_3(mManufacturingSchematic,mOwner);
	gMessageLib->sendUpdateExperimentationPoints(mOwner);

	// ack
	gMessageLib->sendCraftExperimentResponse(opCraftExperiment,roll,counter,mOwner);
}

//=============================================================================
//
// calculate the maximum reachable percentage through assembly with the filled resources
//
float CraftingSession::_calcAverageMalleability()
{
	FilledResources::iterator	filledResIt;
	ManufactureSlots*			manSlots	= mManufacturingSchematic->getManufactureSlots();
	ManufactureSlots::iterator	manIt		= manSlots->begin();
	CraftWeights::iterator		weightIt;

	ManufactureSlot*			manSlot;
	Resource*					resource;
	uint16						resAtt		= 0;
	uint8						slotCount	= 0;

	while(manIt != manSlots->end())
	{
		manSlot		= (*manIt);

		// skip if its a sub component slot
		if(manSlot->mDraftSlot->getType() != 4)
		{
			++manIt;
			continue;
		}

		// we limit it so that only the same resource can go into one slot, so grab only the first entry
		filledResIt		= manSlot->mFilledResources.begin();
		
		if(manSlot->mFilledResources.empty())
		{
			//in case we can leave resource slots optionally emptymanSlot->mFilledResources
			++manIt;
			continue;
		}

		resource		= gResourceManager->getResourceById((*filledResIt).first);
		
		resAtt			+= resource->getAttribute(ResAttr_MA);

		++slotCount;
		++manIt;
	}

	return static_cast<float>(resAtt/slotCount);
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
	float						subQuantity		= 0.0f;
	float						totalResStat	= 0.0f;
	float						validCount		= 0.0f;
	float						wrvResult		= 0.0f;
	bool						slotCounted;
	float						slotCount		= 0.0f;

	while(manIt != manSlots->end())
	{
		manSlot		= (*manIt);
		weightIt	= weights->begin();
		slotCounted	= false;

		// skip if its a sub component slot
		if(manSlot->mDraftSlot->getType() != 4)
		{
			++manIt;
			continue;
		}

		// we limit it so that only the same resource can go into one slot, so grab only the first entry
		filledResIt		= manSlot->mFilledResources.begin();
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

				totalResStat	+= (float)((manSlot->mDraftSlot->getAmount() * resAtt) * craftWeight->getDistribution());
				totalQuantity	+= (float)(manSlot->mDraftSlot->getAmount() * craftWeight->getDistribution());
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
	Datapad* datapad = dynamic_cast<Datapad*>(mOwner->getEquipManager()->getEquippedObject(CreatureEquipSlot_Datapad));
	mManufacturingSchematic->setParentId(datapad->getId());
	mManufacturingSchematic->mDataPadId = datapad->getId();

	if(!datapad->addManufacturingSchematic(mManufacturingSchematic))
	{
		//TODO
		//delete the man schem from the objectlist and the db
		return;
	}

	mManufacturingSchematic->setCustomName(mItem->getCustomName().getAnsi());

	//now delete the old object client side and create it new
	gMessageLib->sendDestroyObject(mManufacturingSchematic->getId(),mOwner);
	gMessageLib->sendCreateManufacturingSchematic(mManufacturingSchematic,mOwner,false);

	//delete the old attributes db side and object side
	AttributeMap* map = mManufacturingSchematic->getAttributeMap();
	map->empty();

	map = mManufacturingSchematic->getInternalAttributeMap();
	map->empty();

	AttributeOrderList*	list = 	mManufacturingSchematic->getAttributeOrder(); 
	list->empty();

	mDatabase->ExecuteSqlAsync(0,0,"DELETE FROM item_attributes WHERE item_id=%I64u",mManufacturingSchematic->getId());
	
	
	//save the datapad as the Owner Id in the db
	mDatabase->ExecuteSqlAsync(0,0,"UPDATE items SET parent_id=%I64u WHERE id=%lld",datapad->getId(),mManufacturingSchematic->getId());

	//set the custom name of the object
	int8 sql[1024],restStr[128],*sqlPointer;
	sprintf(sql,"UPDATE items SET customName='");
		sqlPointer = sql + strlen(sql);
		sqlPointer += mDatabase->Escape_String(sqlPointer,mItem->getCustomName().getAnsi(),mItem->getCustomName().getLength());
		sprintf(restStr,"' WHERE id=%lld ",mManufacturingSchematic->getId());
		
	strcat(sql,restStr);

	mDatabase->ExecuteSqlAsync(0,0,sql);

	//set the schematic as parent id for our item - we need it as dummy!!!
	mDatabase->ExecuteSqlAsync(0,0,"UPDATE items SET parent_id=%I64u WHERE id=%lld ",mManufacturingSchematic->getId(),mItem->getId());
	mItem->setParentId(mManufacturingSchematic->getId());
	mManufacturingSchematic->setItem(mItem);

	//save the customization - thats part of the item!!!!

	//add datapadsize
	//data size - hardcode to 1 for now
	//244 is id of attribute data_volume!!!
	mManufacturingSchematic->addAttribute("data_volume","1");	
	sprintf(sql,"INSERT INTO item_attributes VALUES(%I64u,244,'1',0,0)",mManufacturingSchematic->getId());
	mDatabase->ExecuteSqlAsync(0,0,sql);

	//add creator
	mItem->addAttribute("crafter",mOwner->getFirstName().getAnsi());	
	sprintf(sql,"INSERT INTO item_attributes VALUES(%I64u,17,'%s',0,0)",mItem->getId(),mOwner->getFirstName().getAnsi());
	mDatabase->ExecuteSqlAsync(0,0,sql);

	//save the resource Information as atributes
	// please note that the attribute 173 cat_manf_schem_ing_ressource or _component cannot be entered several times in our table
	// as we cannot store several entries with the same key in our hashmaps
	//alternatively we have to create attributes with the res name in them and check whether it already exists or not

	ManufactureSlots::iterator manIt = mManufacturingSchematic->getManufactureSlots()->begin();
	int8			str[64];
	int8			attr[64];
	CheckResources	checkRes;
	CheckResources::iterator checkResIt = checkRes.begin();
	string			name;
	while(manIt != mManufacturingSchematic->getManufactureSlots()->end())
	{		
		//is it a resource??
	
		if((*manIt)->mDraftSlot->getType() == 4)
		{
			//get resource name and amount
			//we only can enter one res type in a slot so dont care about the res list

			FilledResources::iterator filledResIt = (*manIt)->mFilledResources.begin();
			uint64 resID	= (*filledResIt).first;
			

			checkResIt = checkRes.find(resID);
			if(checkResIt == checkRes.end())
			{
				checkRes.insert(std::make_pair(resID,(*filledResIt).second));
				//checkRes .push_back(resID);
			}
			else
			{
				uint32 amount	= ((*checkResIt).second + (*filledResIt).second);
				uint64 id		= (*checkResIt).first;
				checkRes.erase(checkResIt);
				checkRes.insert(std::make_pair(id,amount));
				
			}

		}
		
		manIt++;
	}

	//thats the ressource list still need to add component list
	checkResIt = checkRes.begin();
	while(checkResIt  != checkRes.end())
	{
		name = gResourceManager->getResourceById((*checkResIt).first)->getName();
		sprintf(attr,"cat_manf_schem_ing_resource.\"%s",name .getAnsi());
		string attrName = BString(attr);

		sprintf(str,"%u",(*checkResIt).second);
				
		//add to the public attribute list
		mManufacturingSchematic->addAttribute(attrName.getAnsi(),str);
		
		//now add to the db		
		sprintf(str,"%s %u",name.getAnsi(),(*checkResIt).second);
		
		//update db
		//enter it slotdependent as we dont want to clot our attributes table with resources
		//173  is cat_manf_schem_resource
		mDatabase->ExecuteSqlAsync(0,0,"INSERT INTO item_attributes VALUES (%I64u,173,'%s',1,0)",mManufacturingSchematic->getId(),str);

		//enter attribute in list
		
		checkResIt  ++;
	}

	

	// now the serial
	string serial;
	serial = getSerial();

	mItem->addAttribute("serial_number",serial.getAnsi());
	
	//16 is id of attribute serial number!!!
	sprintf(sql,"INSERT INTO item_attributes VALUES(%I64u,16,'",mItem->getId());
	sqlPointer = sql + strlen(sql);
	sqlPointer += mDatabase->Escape_String(sqlPointer,serial.getAnsi(),serial.getLength());
	sprintf(restStr,"',%u,0)",mItem->getAttributeMap()->size());
	strcat(sql,restStr);
	mDatabase->ExecuteSqlAsync(0,0,sql);

	//manufacturing limit
	int8 buf[8];
	string limit = _itoa(this->getProductionAmount(),buf,10);
	mManufacturingSchematic->addAttribute("manf_limit",limit.getAnsi());
	
	//504 is id of manf_limit number!!!
	sprintf(sql,"INSERT INTO item_attributes VALUES(%I64u,504,'%s',2,0)",mManufacturingSchematic->getId(),limit.getAnsi());


	gMessageLib->sendCraftAcknowledge(opCreatePrototypeResponse,CraftCreate_2,static_cast<uint8>(counter),mOwner);
}