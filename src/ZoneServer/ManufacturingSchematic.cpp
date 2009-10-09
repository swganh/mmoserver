/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "ManufacturingSchematic.h"
#include "SchematicManager.h"
#include "PlayerObject.h"
#include "WorldManager.h"
#include "ZoneOpcodes.h"
#include "CraftBatch.h"
#include "DraftSchematic.h"
#include "Common/Message.h"
#include "Common/MessageFactory.h"

//=============================================================================

ManufacturingSchematic::ManufacturingSchematic() : Item(),
mSlotsFilled(0),mCounter(0)
{
	mExpAttributeValueChange	=	false;
	mBlueBarSizeChange			=	false;
	mMaxExpValueChange			=	false;
	mDataPadId					=	0;
	mItem						=	NULL;
	mSerial						=	"";

}

//=============================================================================

ManufacturingSchematic::~ManufacturingSchematic()
{
	ManufactureSlots::iterator manIt = mManufactureSlots.begin();

	while(manIt != mManufactureSlots.end())
	{
		delete(*manIt);
		mManufactureSlots.erase(manIt);
		manIt = mManufactureSlots.begin();
	}

	ExperimentationProperties::iterator caIt = mExperimentationProperties.begin();

	while(caIt != mExperimentationProperties.end())
	{
		delete(*caIt);
		mExperimentationProperties.erase(caIt);
		caIt = mExperimentationProperties.begin();
	}

	CustomizationList::iterator	custIt = mCustomizationList.begin();
	while(custIt != mCustomizationList.end())
	{
		delete((*custIt));
		++custIt;
	}

	SAFE_DELETE(mItem);
}

//=============================================================================

void ManufacturingSchematic::prepareManufactureSlots()
{
	DraftSchematic*			draftSchematic	= gSchematicManager->getSchematicBySlotId(mDynamicInt32);
	DraftSlots*				draftSlots		= draftSchematic->getDraftSlots();
	DraftSlots::iterator	draftSlotIt		= draftSlots->begin();

	while(draftSlotIt != draftSlots->end())
	{
		mManufactureSlots.push_back(new ManufactureSlot(*draftSlotIt));

		++draftSlotIt;
	}

	for(uint32 i = 0;i < 8;i++)
		mUpdateCounter[i] = mManufactureSlots.size();

	//annoyingly the craftattributeslist is still zero at this time so we need to reinitialize them !!!!
	mUpdateCounter[8]	=	mExperimentationProperties.size();
	mUpdateCounter[9]	=	mExperimentationProperties.size();
	mUpdateCounter[10]	=	mExperimentationProperties.size();
	mUpdateCounter[11]	=	mExperimentationProperties.size();
	mUpdateCounter[12]	=	mExperimentationProperties.size();
	mUpdateCounter[13]	=	0;
	mUpdateCounter[14]	=	0;
	mUpdateCounter[15]	=	0;
	mUpdateCounter[16]	=	0;
	mUpdateCounter[17]	=	0;
	mUpdateCounter[18]	=	0;

	mUnknown = mManufactureSlots.size();
}

//=============================================================================

void ManufacturingSchematic::prepareCraftingAttributes()
{
	DraftSchematic*				draftSchematic		= gSchematicManager->getSchematicBySlotId(mDynamicInt32);
	CraftBatches*				expCraftBatches		= draftSchematic->getCraftBatches();
	CraftBatches::iterator		expCraftBatchIt		= expCraftBatches->begin();

	while(expCraftBatchIt != expCraftBatches->end())
	{
		string						attName		= gSchematicManager->getExpGroup((*expCraftBatchIt)->getExpGroup());
		ExperimentationProperty*	expProperty	= new ExperimentationProperty(attName.getAnsi(),(*expCraftBatchIt)->getCraftWeights(),(*expCraftBatchIt)->getCraftAttributes(),0.0f,0.0f,0.0f);

		mExperimentationProperties.push_back(expProperty);

		++expCraftBatchIt;
	}

	//for(uint32 i = 8;i < 15;i++)
	//	mUpdateCounter[i] = mExperimentationProperties.size();

	mExpFailureChance = 90.0f;

	// ----------------------------------------------------------------
	// collect the *unique* exp properties in a list for easy reference
	// so we can bundle them on sending the deltas / baseline

	ExperimentationProperties*			expProp				= getExperimentationProperties();
	ExperimentationProperties::iterator	epIt				= expProp->begin();

	epIt	= expProp->begin();
	while(epIt != expProp->end())
	{
		// do we have this exp property already??

		if(!expPropStorefind((*epIt)->mExpAttributeName.getCrc()))
		{
			// add it
			expPropStore.push_back(std::make_pair((*epIt)->mExpAttributeName.getCrc(),(*epIt)));
		}

		++epIt;
	}
}

//=============================================================================

void ManufacturingSchematic::prepareAttributes()
{
	mAttributesUpdateCounter = mAttributeMap.size();
}

//=============================================================================


void ManufacturingSchematic::sendAttributes(PlayerObject* playerObject)
{

	if(playerObject->getConnectionState() != PlayerConnState_Connected)
		return;

	TangibleObject* tItem = this->getItem();
	if(!tItem)
		return;

	AttributeMap::iterator		mapIt;
	AttributeMap*				iAttributeMap		= tItem->getAttributeMap();
	AttributeOrderList*			iAttributeOrderList	= tItem->getAttributeOrder();

	AttributeMap*				rAttributeMap		= getAttributeMap();
	AttributeOrderList*			rAttributeOrderList	= getAttributeOrder();

	DraftSchematic*				draftSchematic		= gSchematicManager->getSchematicBySlotId(mDynamicInt32);
	DraftSlots*					draftSlots			= draftSchematic->getDraftSlots();

	Message*					newMessage;
	string						value,aStr;
	BStringVector				dataElements;

	uint32	amountSlots		= draftSlots->size();
	uint8	i				= 0;

	gMessageFactory->StartMessage();   
	gMessageFactory->addUint32(opAttributeListMessage);
	gMessageFactory->addUint64(mId);
	
	//add slots and resource/item requirements
	
	gMessageFactory->addUint32(iAttributeMap->size()+ rAttributeMap->size()+1); 
	
	AttributeOrderList::iterator	orderIt = rAttributeOrderList->begin();

	while(orderIt != rAttributeOrderList->end())
	{
		mapIt = rAttributeMap->find(*orderIt);		

		gMessageFactory->addString(gWorldManager->getAttributeKey((*mapIt).first));

		value = (*mapIt).second.c_str();
		value.convert(BSTRType_Unicode16);

		gMessageFactory->addString(value);

		++orderIt;
	}

	//attributes ....
	gMessageFactory->addString(BString("manf_attribs"));
	aStr = "\\#ff0000 --------------";
	aStr.convert(BSTRType_Unicode16);
	gMessageFactory->addString(aStr);

	
	orderIt = iAttributeOrderList->begin();

	while(orderIt != iAttributeOrderList->end())
	{
		mapIt = iAttributeMap->find(*orderIt);		

		gMessageFactory->addString(gWorldManager->getAttributeKey((*mapIt).first));

		value = (*mapIt).second.c_str();
		value.convert(BSTRType_Unicode16);

		gMessageFactory->addString(value);

		++orderIt;
	}

	//gMessageFactory->addUint32(0xffffffff);

	newMessage = gMessageFactory->EndMessage();
		
	(playerObject->getClient())->SendChannelAUnreliable(newMessage, playerObject->getAccountId(),CR_Client,9);
	
}