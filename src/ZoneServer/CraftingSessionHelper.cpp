/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

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
	uint32				totalNeededAmount	= manSlot->mDraftSlot->getNecessaryAmount();


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
			//important!!! if it is an individual component add the items id with stacksize
			//dont loose items through stacking already present items
			manSlot->mFilledResources.push_back(std::make_pair(component->getId(),totalNeededAmount));
			filledResIt				= manSlot->mFilledResources.begin();
			manSlot->setFilledType((DSType)manSlot->mDraftSlot->getType());
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
			manSlot->setFilledType((DSType)manSlot->mDraftSlot->getType());
			manSlot->mFilledIndicatorChange = true;
			//link it to the schematic ?

		}

		if(!AdjustComponentStack(component,inventory,totalNeededAmount))
		{
			gMessageLib->sendCraftAcknowledge(opCraftFillSlot,CraftError_Internal_Invalid_Ingredient_Size,counter,mOwner);
			return;
		}


		// update the slot total resource amount
		manSlot->mFilled = manSlot->mDraftSlot->getNecessaryAmount();

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
			manSlot->setFilledType((DSType)manSlot->mDraftSlot->getType());
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
			manSlot->setFilledType((DSType)manSlot->mDraftSlot->getType());
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
	ResourceContainer*			resContainer	= dynamic_cast<ResourceContainer*>(gWorldManager->getObjectById(resContainerId));
	ManufactureSlot*			manSlot			= mManufacturingSchematic->getManufactureSlots()->at(slotId);

	FilledResources::iterator	filledResIt		= manSlot->mFilledResources.begin();

	//bool resourceBool = false;
	bool smallupdate = false;

	if(!resContainer)
	{
		gMessageLib->sendCraftAcknowledge(opCraftFillSlot,CraftError_Ingredient_Not_In_Inventory,counter,mOwner);
	}

	if(!manSlot)
	{
		gMessageLib->sendCraftAcknowledge(opCraftFillSlot,CraftError_Invalid_Slot,counter,mOwner);
	}

	
	uint32	availableAmount		= resContainer->getAmount();
	uint32	totalNeededAmount	= manSlot->mDraftSlot->getNecessaryAmount();
	uint32	existingAmount		= 0;
	uint64	containerResId		= resContainer->getResourceId();

	// see if something is filled already
	existingAmount = manSlot->getFilledAmount();
	
	// update the needed amount
	totalNeededAmount -= existingAmount;

	// fail if its already complete
	if(!totalNeededAmount)
	{
		gMessageLib->sendCraftAcknowledge(opCraftFillSlot,CraftError_Slot_Already_Full,counter,mOwner);
		return;
	}

	//check whether we have the same resource - no go if its different - check for the slot being empty though
	if(manSlot->getResourceId() && (containerResId != manSlot->getResourceId()))
	{
		gMessageLib->sendCraftAcknowledge(opCraftFillSlot,CraftError_Internal_Invalid_Ingredient,counter,mOwner);
		return;
	}

	// see how much this container has to offer
	// slot completely filled
	if(availableAmount >= totalNeededAmount)
	{
		// add to the filled resources
		filledResIt				= manSlot->mFilledResources.begin();

		while(filledResIt != manSlot->mFilledResources.end())
		{
			// already got something of that type filled
			if(containerResId == (*filledResIt).first)
			{
				//hark in live the same resource gets added to a second slot
				manSlot->mFilledResources.push_back(std::make_pair(containerResId,totalNeededAmount));
				filledResIt				= manSlot->mFilledResources.begin();
				manSlot->setFilledType(DST_Resource);//4  resource has been filled
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
				manSlot->setFilledType(DST_Resource);
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
			mDatabase->ExecuteSqlAsync(NULL,NULL,"UPDATE resource_containers SET amount=%u WHERE id=%"PRIu64"",newContainerAmount,resContainer->getId());
		}

		// update the slot total resource amount
		manSlot->setFilledAmount(manSlot->mDraftSlot->getNecessaryAmount());

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
				manSlot->setFilledType(DST_Resource);
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
				manSlot->setFilledType(DST_Resource);
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


//=============================================================================
//
// an amount of resource gets put into a manufacturing slot
//

void CraftingSession::handleFillSlotResourceRewrite(uint64 resContainerId,uint32 slotId,uint32 unknown,uint8 counter)
{
	// update resource container
	ResourceContainer*			resContainer	= dynamic_cast<ResourceContainer*>(gWorldManager->getObjectById(resContainerId));
	ManufactureSlot*			manSlot			= mManufacturingSchematic->getManufactureSlots()->at(slotId);

	FilledResources::iterator	filledResIt		= manSlot->mFilledResources.begin();

	//bool resourceBool = false;
	bool smallupdate = false;

	if(!resContainer)
	{
		gMessageLib->sendCraftAcknowledge(opCraftFillSlot,CraftError_Ingredient_Not_In_Inventory,counter,mOwner);
	}

	if(!manSlot)
	{
		gMessageLib->sendCraftAcknowledge(opCraftFillSlot,CraftError_Invalid_Slot,counter,mOwner);
	}

	
	uint32	availableAmount		= resContainer->getAmount();
	uint32	totalNeededAmount	= manSlot->mDraftSlot->getNecessaryAmount();
	uint32	existingAmount		= 0;
	uint64	containerResId		= resContainer->getResourceId();

	// see if something is filled already
	existingAmount = manSlot->getFilledAmount();
	
	// update the needed amount
	totalNeededAmount -= existingAmount;

	// fail if its already complete
	if(!totalNeededAmount)
	{
		gMessageLib->sendCraftAcknowledge(opCraftFillSlot,CraftError_Slot_Already_Full,counter,mOwner);
		return;
	}

	//check whether we have the same resource - no go if its different - check for the slot being empty though
	if(manSlot->getResourceId() && (containerResId != manSlot->getResourceId()))
	{
		gMessageLib->sendCraftAcknowledge(opCraftFillSlot,CraftError_Internal_Invalid_Ingredient,counter,mOwner);
		return;
	}
									
	uint32 takeAmount = 0;
	if(availableAmount >= totalNeededAmount)
	{
		takeAmount = totalNeededAmount;
	}
	else
	{
		takeAmount = availableAmount;
	}
	
	// add it to the slot get the update type
	smallupdate = manSlot->addResourcetoSlot(containerResId,takeAmount);
	
	// update the container amount
	uint32 newContainerAmount = availableAmount - takeAmount;

	updateResourceContainer(resContainer->getId(),newContainerAmount);

	// update the slot total resource amount
	manSlot->setFilledAmount(manSlot->getFilledAmount()+takeAmount);

	if(manSlot->getFilledAmount() == manSlot->mDraftSlot->getNecessaryAmount())
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
// filled components get returned to the inventory
//

void CraftingSession::bagComponents(ManufactureSlot* manSlot,uint64 containerId)
{
	//add the components back to the inventory
	manSlot->setFilledType(DST_Empty);

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

	manSlot->setFilledType(DST_Empty);

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

	if(manSlot->getFilledType() == DST_Resource)
		bagResource(manSlot,containerId);
	else
		bagComponents(manSlot,containerId);

	// update the slot
	manSlot->mFilledResources.clear();
	manSlot->setFilledType(DST_Empty);

	// if it was completely filled, update the total amount of filled slots
	if(manSlot->getFilledAmount() == manSlot->mDraftSlot->getNecessaryAmount())
	{
		mManufacturingSchematic->removeFilledSlot();
		gMessageLib->sendUpdateFilledManufactureSlots(mManufacturingSchematic,mOwner);
		//update the amount clientside, too ?
	}

	if(manSlot->getFilledAmount())
	{
		manSlot->setFilledAmount(0);
		//only send when changes !!!!!
		gMessageLib->sendManufactureSlotUpdate(mManufacturingSchematic,static_cast<uint8>(slotId),mOwner);
	}

	manSlot->setResourceId(0);
	manSlot->setSerial("");
}

//=============================================================================
 //this creates the serial for a crafted item
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
			if((u >57)&&(u <97))
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
//this determines our success in the assembly of the item

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

	if((mCriticalCount = 3))
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

//===============================================================================
//collects a resourcelist to give to a manufacturing schematic
//
void CraftingSession::collectResources()
{
	ManufactureSlots::iterator manIt = mManufacturingSchematic->getManufactureSlots()->begin();
	int8			str[64];
	int8			attr[64];
	
	CheckResources::iterator checkResIt = mCheckRes.begin();
	string			name;
	while(manIt != mManufacturingSchematic->getManufactureSlots()->end())
	{
		
		//is it a resource??
		if((*manIt)->mDraftSlot->getType() == DST_Resource)
		{
			//get resource name and amount
			//we only can enter one res type in a slot so dont care about the other entries

			FilledResources::iterator filledResIt = (*manIt)->mFilledResources.begin();
			uint64 resID	= (*filledResIt).first;


			checkResIt = mCheckRes.find(resID);
			if(checkResIt == mCheckRes.end())
			{
				mCheckRes.insert(std::make_pair(resID,(*filledResIt).second));
			}
			else
			{
				//uint32 amount	= ((*checkResIt).second + (*filledResIt).second);
				(*checkResIt).second += (*filledResIt).second;
				//uint64 id		= (*checkResIt).first;
				//mCheckRes.erase(checkResIt);
				//mCheckRes.insert(std::make_pair(id,amount));

			}

		}

		manIt++;
	}

	checkResIt = mCheckRes.begin();
	while(checkResIt  != mCheckRes.end())
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
		mDatabase->ExecuteSqlAsync(0,0,"INSERT INTO item_attributes VALUES (%"PRIu64",173,'%s',1,0)",mManufacturingSchematic->getId(),str);
		
		//now enter it in the relevant manschem table so we can use it in factories
		mDatabase->ExecuteSqlAsync(0,0,"INSERT INTO manschemresources VALUES (NULL,%"PRIu64",%"PRIu64",%u)",mManufacturingSchematic->getId(),(*checkResIt).first,(*checkResIt).second);

		checkResIt  ++;
	}

}

//===============================================================================
//collects a resourcelist to give to a manufacturing schematic
//
void CraftingSession::collectComponents()
{
	ManufactureSlots::iterator manIt = mManufacturingSchematic->getManufactureSlots()->begin();
	int8			str[64];
	int8			attr[64];
	
	CheckResources::iterator checkResIt = mCheckRes.begin();
	string			name;
	while(manIt != mManufacturingSchematic->getManufactureSlots()->end())
	{
		
		//is it a resource??
		if(((*manIt)->mDraftSlot->getType() == DST_IdentComponent)||((*manIt)->mDraftSlot->getType() == DST_SimiliarComponent))
		{
			//get component serial and amount
			//we only can enter one serial type in a slot so dont care about the other entries

			FilledResources::iterator filledResIt = (*manIt)->mFilledResources.begin();
			uint64 componentID	= (*filledResIt).first;
			
			TangibleObject* tO = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById(componentID));
			if(!tO)
			{
				//item not found??? wth
				assert(false);
			}

			string componentSerial = "";
			
			if(tO->hasAttribute("serial"))
				componentSerial = tO->getAttribute<std::string>("serial").c_str();
			
			uint32 serialCRC = componentSerial.getCrc();

			checkResIt = mCheckRes.find(componentID);
			if(checkResIt == mCheckRes.end())
			{
				mCheckRes.insert(std::make_pair(componentID,(*filledResIt).second));
			}
			else
			{
				//uint32 amount	= ((*checkResIt).second + (*filledResIt).second);
				(*checkResIt).second += (*filledResIt).second;
				//uint64 id		= (*checkResIt).first;
				//mCheckRes.erase(checkResIt);
				//mCheckRes.insert(std::make_pair(id,amount));

			}

		}

		manIt++;
	}

	checkResIt = mCheckRes.begin();
	while(checkResIt  != mCheckRes.end())
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
		mDatabase->ExecuteSqlAsync(0,0,"INSERT INTO item_attributes VALUES (%"PRIu64",173,'%s',1,0)",mManufacturingSchematic->getId(),str);
		
		//now enter it in the relevant manschem table so we can use it in factories
		mDatabase->ExecuteSqlAsync(0,0,"INSERT INTO manschemresources VALUES (NULL,%"PRIu64",%"PRIu64",%u)",mManufacturingSchematic->getId(),(*checkResIt).first,(*checkResIt).second);

		checkResIt  ++;
	}

}


//===============================================================================
//collects a resourcelist to give to a manufacturing schematic
//
void CraftingSession::updateResourceContainer(uint64 containerID, uint32 newAmount)
{
// destroy if its empty
	if(!newAmount)
	{
		//now destroy it client side
		gMessageLib->sendDestroyObject(containerID,mOwner);


		gObjectFactory->deleteObjectFromDB(containerID);
		ResourceContainer*			resContainer	= dynamic_cast<ResourceContainer*>(gWorldManager->getObjectById(containerID));
		dynamic_cast<Inventory*>(mOwner->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory))->deleteObject(resContainer);
	}
	// update it
	else
	{
		ResourceContainer*			resContainer	= dynamic_cast<ResourceContainer*>(gWorldManager->getObjectById(containerID));

		resContainer->setAmount(newAmount);
		gMessageLib->sendResourceContainerUpdateAmount(resContainer,mOwner);
		mDatabase->ExecuteSqlAsync(NULL,NULL,"UPDATE resource_containers SET amount=%u WHERE id=%"PRIu64"",newAmount,resContainer->getId());
	}

}