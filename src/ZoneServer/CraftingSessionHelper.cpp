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
#include "FactoryCrate.h"
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

#include "nonPersistantObjectFactory.h"

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
		mManufacturingSchematic->addObject(item);
		//the link will update the inventories item count
		gMessageLib->sendContainmentMessage(item->getId(),item->getParentId(),4,mOwner);

		//the db will only be updated if we really use the ´component!!
		//at this point we might still put it out again or cancel the crafting session!!!!
		//this is necessary in order to keep it consistent with a possible stack we might use

		return true;
	}

	return false;

}


//=============================================================================
//
// returns the serial of either a crate or component
// thats easy for stacks and a little more involved for factory crates
// 

string CraftingSession::ComponentGetSerial(Item* component)
{

	string componentSerial = "";

	FactoryCrate* fC  = dynamic_cast<FactoryCrate*>(component);
	if(fC)
	{
		if(fC->getLinkedObject()->hasAttribute("serial"))
			componentSerial = fC->getLinkedObject()->getAttribute<std::string>("serial").c_str();
		
		
		return	componentSerial;
	}

	if(component->hasAttribute("serial"))
		componentSerial = component->getAttribute<std::string>("serial").c_str();

	return	componentSerial;
		
}


//=============================================================================
//
// returns the amount of the item useable
// thats easy for stacks and a little more involved for factory crates
// under the presumption that crates can hold stackable items, too 

uint32 CraftingSession::getComponentOffer(Item* component, uint32 needed)
{

	uint32 crateSize = 0;
	uint32 stacksize = 1;
	FactoryCrate* fC  = dynamic_cast<FactoryCrate*>(component);
	if(fC)
	{
		if(!fC->hasAttribute("factory_count"))
		{
			gLogger->log(LogManager::DEBUG,"CraftingSession::prepareComponent crate without factory_count attribute");
			return 0;
		}
		
		crateSize = fC->getAttribute<uint32>("factory_count");
		stacksize = 1;
		
		if(!fC->getLinkedObject()->hasAttribute("stacksize"))
		{
			if(needed> crateSize)
				return crateSize;
			else
				return needed;
		}

		stacksize = fC->getLinkedObject()->getAttribute<uint32>("stacksize");

		if(needed> (stacksize*crateSize))
			return stacksize*crateSize;
		else
			return needed;
		
	}

	if(!component->hasAttribute("stacksize"))
	{
		return 1;
	}

	stacksize = component->getAttribute<uint32>("stacksize");

	if(needed> stacksize)
		return stacksize;
	else
		return stacksize;
		
}

//=============================================================================
//
// preparing the offer means linking the stack/ crate to the slot so we can access it later on
// delete it out of the inventory/continer if necessary
// otherwise adjust the stacksize

bool CraftingSession::prepareComponentOffer(Item* component, uint32 needed, ManufactureSlot* manSlot)
{
	Inventory* inventory = dynamic_cast<Inventory*>(mOwner->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));

	uint32 crateSize = 1;
	uint32 stackSize = 1;
	
	FactoryCrate* fC  = dynamic_cast<FactoryCrate*>(component);
	if(fC)
	{
		if(!fC->hasAttribute("factory_count"))					  
		{
			gLogger->log(LogManager::DEBUG,"CraftingSession::prepareComponentoffer crate without factory_count attribute");
			return false;
		}
		
		crateSize = fC->getAttribute<uint32>("factory_count");
		
		if(!fC->getLinkedObject()->hasAttribute("stacksize"))
		{
			stackSize = 1;
		}

		uint32 crateTaken = (uint32)needed/stackSize;
		
		if(crateTaken>crateSize)
		{
			gLogger->log(LogManager::DEBUG,"CraftingSession::prepareComponentOffer crate does not have enough content");
			return false;
		}

		//only take away whole stacks
		fC->setAttributeIncDB("factory_count",boost::lexical_cast<std::string>(crateSize-crateTaken));
		gMessageLib->sendUpdateCrateContent(fC,mOwner);

		//reference it in the slot so we dont loose it
		manSlot->mUsedComponentStacks.push_back(std::make_pair(component,crateTaken));

		//if its now empty remove it out of the inventory so we cant use it several times
		if(crateTaken == crateSize)
		{
			TangibleObject* tO = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById(component->getParentId()));
			if(!tO)
				tO = dynamic_cast<TangibleObject*>(inventory);

			tO->removeObject(component);
			
			//leave parent_id untouched - we might need to readd it to the container
			gMessageLib->sendContainmentMessage(fC->getId(),mManufacturingSchematic->getId(),0xffffffff,mOwner);
			
		}
		
		return true;
		
	}

	if(!component->hasAttribute("stacksize"))
	{
		manSlot->mUsedComponentStacks.push_back(std::make_pair(component,1));

		// remove it out of the inventory so we cant use it several times
		TangibleObject* tO = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById(component->getParentId()));
		if(!tO)
			tO = dynamic_cast<TangibleObject*>(inventory);

		tO->removeObject(component);
		
		//leave parent_id untouched - we might need to readd it to the container
		gMessageLib->sendContainmentMessage(component->getId(),mManufacturingSchematic->getId(),0xffffffff,mOwner);

		return true;
		
	}

	stackSize = component->getAttribute<uint32>("stacksize");

	uint32 stackTaken = needed;
	
	if(stackTaken>stackSize)
	{
		gLogger->log(LogManager::DEBUG,"CraftingSession::prepareComponentOffer stack does not have enough content");
		return false;
	}

	component->setAttributeIncDB("stacksize",boost::lexical_cast<std::string>(stackSize-stackTaken));

	//reference it in the slot so we dont loose it
	manSlot->mUsedComponentStacks.push_back(std::make_pair(component,stackTaken));

	//if its now empty remove it out of the inventory so we cant use it several times
	if(stackTaken == stackSize)
	{
		TangibleObject* tO = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById(component->getParentId()));
		if(!tO)
			tO = dynamic_cast<TangibleObject*>(inventory);

		tO->removeObject(component);
		//leave parent_id untouched - we might need to readd it to the container
		gMessageLib->sendContainmentMessage(component->getId(),mManufacturingSchematic->getId(),0xffffffff,mOwner);
	}
		
	return true;
		
}


//=============================================================================
//
// a component gets put into a manufacturing slot
//

void CraftingSession::handleFillSlotComponent(uint64 componentId,uint32 slotId,uint32 unknown,uint8 counter)
{

	ManufactureSlot*	manSlot			= mManufacturingSchematic->getManufactureSlots()->at(slotId);

	uint64 crateID = 0;
	uint64 crateTangID = 0;

	Item*		component	= dynamic_cast<Item*>(gWorldManager->getObjectById(componentId));

	FactoryCrate* fC  = dynamic_cast<FactoryCrate*>(component);
	if(fC)
	{
		crateTangID = fC->getLinkedObject()->getId();
	}

	Inventory* inventory = dynamic_cast<Inventory*>(mOwner->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));

	//remove the component out of the inventory and attach it to the man schematic


	//hardcoded to 1 until stacks are in
	uint32				availableAmount		= 1;
	uint32				existingAmount		= 0;
	uint32				totalNeededAmount	= manSlot->mDraftSlot->getNecessaryAmount();


	string				componentSerial	= "";
	string				filledSerial	= "";


	componentSerial = ComponentGetSerial(component);

	bool smallupdate = false;

	if((!component) || (!manSlot))
	{
		gMessageLib->sendCraftAcknowledge(opCraftFillSlot,CraftError_Ingredient_Not_In_Inventory,counter,mOwner);
		return;
	}

	// get the amount of already filled components
	FilledResources::iterator filledResIt = manSlot->mFilledResources.begin();
	while(filledResIt != manSlot->mFilledResources.end())
	{
		existingAmount += (*filledResIt).second;
		++filledResIt;
	}

	if(existingAmount)
		smallupdate = true;

	// update the needed amount
	totalNeededAmount -= existingAmount;

	// fail if its already complete
	if(!totalNeededAmount)
	{
		gMessageLib->sendCraftAcknowledge(opCraftFillSlot,CraftError_Slot_Already_Full,counter,mOwner);
		return;
	}

	//ok we probably have a deal here - if its a crate we need to get a stack out - or more ?

	// see how much this component stack /crate has to offer
	uint32 offer = getComponentOffer(component,totalNeededAmount); 
	if(!offer)
	{
		gMessageLib->sendCraftAcknowledge(opCraftFillSlot,CraftError_Ingredient_Not_In_Inventory,counter,mOwner);
		return;
	}


	
	// important question - are there stackable crafting components in crates - like stack in a stack ???? 
	// answer - not to my knowledge

	// add it to the slot get the update type
	uint64 selectedComponentId = componentId;

	if(crateTangID)
	{
		selectedComponentId = crateTangID;
	}

	// important is that when we use crates we need to keep track of the involved crates somehow
	//there is no smallupdate with components	 ????
	manSlot->addComponenttoSlot(selectedComponentId,offer,manSlot->mDraftSlot->getType());

	//the component is now referenced and the stack attributes (if any) are altered
	prepareComponentOffer(component, offer, manSlot);
	
	// update the slot total resource amount
	manSlot->setFilledAmount(manSlot->getFilledAmount()+offer);

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
			TangibleObject* tO = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById(resContainer->getParentId()));
			tO->deleteObject(resContainer);

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
		
		TangibleObject* tO = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById(resContainer->getParentId()));
		tO->deleteObject(resContainer);

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
	smallupdate = manSlot->addResourcetoSlot(containerResId,takeAmount,manSlot->mDraftSlot->getType());
	
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
	uint32 crateSize;
	manSlot->setFilledType(DST_Empty);

	Inventory* inventory = dynamic_cast<Inventory*>(mOwner->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));

	FilledComponent::iterator compIt = manSlot->mUsedComponentStacks.begin();
	while(compIt != manSlot->mUsedComponentStacks.end())
	{
		Item*	filledComponent	= dynamic_cast<Item*>((*compIt).first);

		if(!filledComponent)
		{
			gLogger->log(LogManager::DEBUG,"CraftingSession::bagComponents filledComponent not found");
			return;
		}

		uint32 amount = (*compIt).second;

		//add to original container
		TangibleObject* container = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById(filledComponent->getParentId()));
		if(!container)
			container = dynamic_cast<TangibleObject*>(inventory);

		if(!container)
		{
			gLogger->log(LogManager::DEBUG,"CraftingSession::bagComponents couldnt find components parent container");
			return;
		}
		
		FactoryCrate* fC = dynamic_cast<FactoryCrate*>(filledComponent);
		if(fC)
		{
			if(!fC->hasAttribute("factory_count"))					  
			{
				gLogger->log(LogManager::DEBUG,"CraftingSession::prepareComponentoffer crate without factory_count attribute");
				return;
			}
		
			crateSize = fC->getAttribute<uint32>("factory_count");
			//did we empty it??
			if(!crateSize)
			{
				container->addObject(fC);
				gMessageLib->sendContainmentMessage(fC->getId(),container->getId(),0xffffffff,mOwner);
			}	 		
			fC->setAttributeIncDB("factory_count",boost::lexical_cast<std::string>(amount+crateSize));
			gMessageLib->sendUpdateCrateContent(fC,mOwner);

			compIt = manSlot->mUsedComponentStacks.erase(compIt);
			continue;
		}
		
		if(!filledComponent->hasAttribute("stacksize"))					  
		{
			container->addObject(filledComponent);
			gMessageLib->sendContainmentMessage(filledComponent->getId(),container->getId(),0xffffffff,mOwner);

			compIt = manSlot->mUsedComponentStacks.erase(compIt);
			continue;
			
		}
		
		crateSize = filledComponent->getAttribute<uint32>("stacksize");
		filledComponent->setAttributeIncDB("stacksize",boost::lexical_cast<std::string>(crateSize+amount));
		
		//did we empty it??
		if(!crateSize)
		{
			container->addObject(filledComponent);
			gMessageLib->sendContainmentMessage(filledComponent->getId(),container->getId(),0xffffffff,mOwner);
		}	 		

		compIt = manSlot->mUsedComponentStacks.erase(compIt);

	}

	manSlot->mUsedComponentStacks.clear();
	manSlot->mFilledResources.clear();
}

void CraftingSession::destroyComponents()
{
	uint8 amount = mManufacturingSchematic->getManufactureSlots()->size();

	for (uint8 i = 0; i < amount; i++)
	{
		//iterate through our manslots which contain components
		ManufactureSlot* manSlot = mManufacturingSchematic->getManufactureSlots()->at(i);
		/*if((manSlot->getFilledType()!=DST_IdentComponent) && (manSlot->getFilledType()!=DST_SimiliarComponent))
		{
			continue;
		}*/
	
		//add the components back to the inventory
		uint32 crateSize;
		manSlot->setFilledType(DST_Empty);

		Inventory* inventory = dynamic_cast<Inventory*>(mOwner->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));

		FilledComponent::iterator compIt = manSlot->mUsedComponentStacks.begin();
		while(compIt != manSlot->mUsedComponentStacks.end())
		{
			Item*	filledComponent	= dynamic_cast<Item*>((*compIt).first);

			if(!filledComponent)
			{
				gLogger->log(LogManager::DEBUG,"CraftingSession::bagComponents filledComponent not found");
				return;
			}
			
			
			FactoryCrate* fC = dynamic_cast<FactoryCrate*>(filledComponent);
			if(fC)
			{
				if(!fC->hasAttribute("factory_count"))					  
				{
					gLogger->log(LogManager::DEBUG,"CraftingSession::prepareComponentoffer crate without factory_count attribute");
					return;
				}
			
				crateSize = fC->getAttribute<uint32>("factory_count");
				//did we empty it??
				if(!crateSize)
				{
					mManufacturingSchematic->removeObject((*compIt).first);
					gMessageLib->sendDestroyObject(fC->getId(),mOwner);
					gObjectFactory->deleteObjectFromDB(fC);
					gWorldManager->destroyObject(fC);
				}	 		

				compIt = manSlot->mUsedComponentStacks.erase(compIt);
				continue;
			}
			
			if(!filledComponent->hasAttribute("stacksize"))					  
			{
				mManufacturingSchematic->removeObject((*compIt).first);
				gObjectFactory->deleteObjectFromDB(filledComponent);
				gMessageLib->sendDestroyObject(filledComponent->getId(),mOwner);
				gWorldManager->destroyObject(filledComponent);

				compIt = manSlot->mUsedComponentStacks.erase(compIt);
				continue;
				
			}
			
			crateSize = filledComponent->getAttribute<uint32>("stacksize");
			
			//did we empty it??
			if(!crateSize)
			{
				mManufacturingSchematic->removeObject((*compIt).first);
				gObjectFactory->deleteObjectFromDB(filledComponent);
				gMessageLib->sendDestroyObject(filledComponent->getId(),mOwner);
				gWorldManager->destroyObject(filledComponent);
			}	 		

			compIt = manSlot->mUsedComponentStacks.erase(compIt);

		}

		manSlot->mUsedComponentStacks.clear();
		manSlot->mFilledResources.clear();
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
		ObjectIDList*			invObjects	= dynamic_cast<Inventory*>(mOwner->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory))->getObjects();
		ObjectIDList::iterator	listIt		= invObjects->begin();

		bool	foundSameType	= false;

		while(listIt != invObjects->end())
		{
			// we are looking for resource containers
			ResourceContainer* resCont = dynamic_cast<ResourceContainer*>(gWorldManager->getObjectById((*listIt)));
			if(resCont)
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
	if(manSlot->getFilledType() != DST_Empty)
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
	if(mOwnerExpSkillMod > 125)
	{
		mOwnerExpSkillMod = 125;
	}

	int32 assRoll;
	int32 riskRoll;

	float ma		= _calcAverageMalleability();

	//high rating means lesser risk!!
	float rating	= 50.0f + ((ma - 500.0f) / 40.0f) +  mOwnerExpSkillMod - (5.0f * expPoints);

	rating	-= (mManufacturingSchematic->getComplexity()/10);
	rating	+= (mToolEffectivity/10);

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

	gLogger->log(LogManager::DEBUG,"CraftingSession:: assembly Roll preMod %u",assRoll);

	int32 modRoll = static_cast<int32>(((assRoll - (rating * 0.4f)) / 15.0f) - (mToolEffectivity / 50.0f));

	++modRoll;

	//int32 modRoll = (gRandom->getRand() - (rating*0.2))/15;
	gLogger->log(LogManager::DEBUG,"CraftingSession:: assembly Roll postMod %i",modRoll);

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

	// make sure the values are valid and dont crash us cap it at 125
	if(mOwnerAssSkillMod > 125)
	{
		mOwnerAssSkillMod = 125;
	}

	float rating	= 50.0f + ((ma - 500.0f) / 40.0f) +  mOwnerAssSkillMod - 5.0f;

	rating	+= (mToolEffectivity/10);
	rating -= (mManufacturingSchematic->getComplexity()/10);

	float risk		= 100.0f - rating;

	mManufacturingSchematic->setExpFailureChance(risk);

	riskRoll		= (int32)(floor(((double)gRandom->getRand() / (RAND_MAX  + 1.0f) * (100.0f - 1.0f) + 1.0f)));

	//gLogger->logErrorF("Crafting","CraftingSession::_assembleRoll() relevant riskroll %u",riskRoll);


	// ensure that every critical makes the nect critical less likely
	// we dont want to have more than 3 criticals in a row

	//gLogger->logErrorF("Crafting","CraftingSession::_assembleRoll() relevant criticalCount %u",mCriticalCount);

	riskRoll += (mCriticalCount*5);
	//gLogger->logErrorF("Crafting","CraftingSession::_assembleRoll() modified riskroll %u",riskRoll);

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

	int32 modRoll = static_cast<uint32>(((assRoll - (rating * 0.4f)) / 15.0f) - (mToolEffectivity / 50.0f));

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
		//build these attributes by hand the attribute wont be found in the attributes table its custom made
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

	mCheckRes.clear();
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
			if(!(*manIt)->mFilledResources.size())
			{
				manIt++;
				continue;
			}
			//get component serial and amount
			//we only can enter one serial type in a slot so dont care about the other entries

			FilledResources::iterator filledResIt = (*manIt)->mFilledResources.begin();
			uint64 componentID	= (*filledResIt).first;
			
			TangibleObject* tO = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById(componentID));
			if(!tO)
			{
				//item not found??? wth
				assert(false && "CraftingSession::collectComponents No tangible object found in world manager");
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
		Item* tO = dynamic_cast<Item*>(gWorldManager->getObjectById((*checkResIt).first));
		if(!tO)
		{
			gLogger->log(LogManager::DEBUG,"CraftingSession::collectComponents() no component");
			continue;
		}
		string componentSerial = "";
		if(tO->hasAttribute("serial"))
				componentSerial = tO->getAttribute<std::string>("serial").c_str();

		name = tO->getName();
		sprintf(attr,"cat_manf_schem_ing_component.\"%s",name .getAnsi());
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
		mDatabase->ExecuteSqlAsync(0,0,"INSERT INTO manschemcomponents VALUES (NULL,%"PRIu64",%u,%s,%u)",mManufacturingSchematic->getId(),tO->getItemType(),componentSerial,(*checkResIt).second);

		checkResIt  ++;
	}

	mCheckRes.clear();
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
		
		TangibleObject* tO = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById(resContainer->getParentId()));
		tO->deleteObject(resContainer);
		
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

//===============================================================
// empties the slots of a man schem when a critical assembly error happens
// send

void CraftingSession::emptySlots(uint32 counter)
{
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
}

//===============================================================
// modifies an items attribute value
// 

void CraftingSession::modifyAttributeValue(CraftAttribute* att, float attValue)
{
	if(att->getType())
	{
		int32 intAtt = 0;
		//is there an attribute of a component that affects us??
		if(mManufacturingSchematic->hasPPAttribute(att->getAttributeKey()))
		{
			float attributeAddValue = mManufacturingSchematic->getPPAttribute<float>(att->getAttributeKey());
			intAtt = (int32)(ceil(attributeAddValue));
		}

		intAtt += (int32)(ceil(attValue));
		mItem->setAttributeIncDB(att->getAttributeKey(),boost::lexical_cast<std::string>(intAtt));
	}
	else
	{
		float f = rndFloat(attValue);

		//is there an attribute of a component that affects us??
		if(mManufacturingSchematic->hasPPAttribute(att->getAttributeKey()))
		{
			float attributeAddValue = mManufacturingSchematic->getPPAttribute<float>(att->getAttributeKey());
			f += rndFloat(attributeAddValue);

		}
		//mItem->setAttributeIncDB(att->getAttributeKey(),boost::lexical_cast<std::string>(f));
		mItem->setAttributeIncDB(att->getAttributeKey(),rndFloattoStr(f).getAnsi());
		
	}
}


float CraftingSession::getPercentage(uint8 roll)
{
	float percentage = 0.0;
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
	return percentage;
}

//========================================================================================
// gets the ExperimentationRoll and initializes the experimental properties
// meaning an exp property which exists several times (with different resourceweights) 
// gets the same roll assigned
							  
uint8 CraftingSession::getExperimentationRoll(ExperimentationProperty* expProperty, uint8 expPoints)
{
	ExperimentationProperties*			expAllProps = mManufacturingSchematic->getExperimentationProperties();
	ExperimentationProperties::iterator itAll		=	 expAllProps->begin();
	uint8 roll;

	if(expProperty->mRoll == -1)
	{
		gLogger->log(LogManager::DEBUG,"CraftingSession:: expProperty is a Virgin!");
		
		// get our Roll and take into account the relevant modifiers
		roll			= _experimentRoll(expPoints);

		// now go through all properties and mark them when its this one!
		// so we dont experiment two times on it!
		itAll =	 expAllProps->begin();
		while(itAll != expAllProps->end())
		{
			ExperimentationProperty* tempProperty = (*itAll);

			gLogger->log(LogManager::DEBUG,"CraftingSession:: now testing expProperty : %s",tempProperty->mExpAttributeName.getAnsi());
			if(expProperty->mExpAttributeName.getCrc() == tempProperty->mExpAttributeName.getCrc())
			{
				gLogger->log(LogManager::DEBUG,"CraftingSession:: assign it our roll : %u",roll);
				tempProperty->mRoll = roll;
			}

			itAll++;
		}

	}
	else
	{
		roll = static_cast<uint8>(expProperty->mRoll);
		gLogger->log(LogManager::DEBUG,"CraftingSession:: experiment expProperty isnt a virgin anymore ...(roll:%u)",roll);
	}

	return roll;

}
