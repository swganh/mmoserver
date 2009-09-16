/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "MessageLib.h"
#include "LogManager/LogManager.h"
#include "ZoneServer/ZoneOpcodes.h"
#include "ZoneServer/WorldManager.h"
#include "ZoneServer/UIOpcodes.h"
#include "ZoneServer/ObjectFactory.h"
#include "Common/MessageDispatch.h"
#include "Common/MessageFactory.h"
#include "Common/MessageOpcodes.h"
#include "Common/Message.h"
#include "Common/DispatchClient.h"
#include "ZoneServer/ObjectControllerOpcodes.h"
#include "ZoneServer/CurrentResource.h"
#include "ZoneServer/CharSheetManager.h"
#include "ZoneServer/Wearable.h"
#include "ZoneServer/Conversation.h"
#include "ZoneServer/NPCObject.h"
#include "ZoneServer/CraftingTool.h"
#include "ZoneServer/ManufacturingSchematic.h"
#include "ZoneServer/CraftingSession.h"
#include <boost/lexical_cast.hpp>
#include "Common/atMacroString.h"



//======================================================================================================================

bool MessageLib::sendBaselinesMSCO_3(ManufacturingSchematic* manSchem,PlayerObject* playerObject,bool sendAttributes)
{
	if(!(playerObject->isConnected()))
		return(false);

	Message*			message;
	Message*			part;
	DraftSchematic*		draftSchematic = gSchematicManager->getSchematicBySlotId(manSchem->getDynamicInt32());
	
	AttributeMap*			attributes		= manSchem->getAttributeMap();
	AttributeMap::iterator	it				= attributes->begin();
	uint32					attByteCount	= 0;

	if(sendAttributes)
	{
		while(it != attributes->end())
		{
			attByteCount += 21 + gWorldManager->getAttributeKey((*it).first).getLength();
			++it;
		}
	}

	if(!draftSchematic)
		return(false);

	uint32					namesByteCount	= manSchem->getNameFile().getLength() + manSchem->getName().getLength() + (manSchem->getCustomName().getLength()<< 1) + (playerObject->getFirstName().getLength() << 1);
	string					convPlayerName	= playerObject->getFirstName();
	string					convCustomName	= manSchem->getCustomName();

	convPlayerName.convert(BSTRType_Unicode16);
	convCustomName.convert(BSTRType_Unicode16);

	gMessageFactory->StartMessage();
	
	//object count
	gMessageFactory->addUint16(13);	
	
	gMessageFactory->addFloat(draftSchematic->getComplexity());
	gMessageFactory->addString(manSchem->getNameFile());
	gMessageFactory->addUint32(0);	
	gMessageFactory->addString(manSchem->getName());
	gMessageFactory->addString(convCustomName);
	gMessageFactory->addUint32(1);
	gMessageFactory->addUint32(1);

	// send attributes on baseline so that they are shown on assembly
	//cave review update counter
	if(sendAttributes)
	{
		manSchem->mAttributesUpdateCounter = attributes->size();
		gMessageFactory->addUint32(attributes->size());
		gMessageFactory->addUint32(manSchem->mAttributesUpdateCounter);

		it = attributes->begin();

		while(it != attributes->end())
		{
			gMessageFactory->addUint8(0);
			gMessageFactory->addString(BString("crafting"));
			gMessageFactory->addUint32(0);
			gMessageFactory->addString(gWorldManager->getAttributeKey((*it).first));
			
			gMessageFactory->addFloat(boost::lexical_cast<float,std::string>((*it).second));

			++it;
		}
	}
	else
	{
		gMessageFactory->addUint32(0);
		gMessageFactory->addUint32(0);
	}
	gMessageFactory->addString(convPlayerName);
	gMessageFactory->addUint32(manSchem->getComplexity());
	gMessageFactory->addUint32(1);
	
	part = gMessageFactory->EndMessage();


	gMessageFactory->StartMessage();

	gMessageFactory->addUint32(opBaselinesMessage);   
	gMessageFactory->addUint64(manSchem->getId()); 
	gMessageFactory->addUint32(opMSCO);
	gMessageFactory->addUint8(3);  
	gMessageFactory->addUint32(part->getSize());
	gMessageFactory->addData(part->getData(),part->getSize());

	message = gMessageFactory->EndMessage();
	part->setPendingDelete(true);


	(playerObject->getClient())->SendChannelA(message,playerObject->getAccountId(),CR_Client,5);

	return(true);
}

//======================================================================================================================

bool MessageLib::sendBaselinesMSCO_6(ManufacturingSchematic* manSchem,PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	Message*		message;
	DraftSchematic*	draftSchematic = gSchematicManager->getSchematicBySlotId(manSchem->getDynamicInt32());

	if(!draftSchematic)
		return(false);

	gMessageFactory->StartMessage();  
	gMessageFactory->addUint32(opBaselinesMessage);   
	gMessageFactory->addUint64(manSchem->getId()); 
	gMessageFactory->addUint32(opMSCO);
	gMessageFactory->addUint8(6);  

	gMessageFactory->addUint32(16 + manSchem->getItemModel().getLength());
	gMessageFactory->addUint16(6);
	gMessageFactory->addFloat(draftSchematic->getComplexity());
	//customization string
	gMessageFactory->addUint16(0);

	gMessageFactory->addString(manSchem->getItemModel());
	//prototype crc
	gMessageFactory->addUint32(manSchem->getDynamicInt32());
	gMessageFactory->addUint8(1);
	gMessageFactory->addUint8(manSchem->getFilledSlotCount());

	message = gMessageFactory->EndMessage();

	(playerObject->getClient())->SendChannelA(message,playerObject->getAccountId(),CR_Client,5);

	return(true);
}

//======================================================================================================================

bool MessageLib::sendBaselinesMSCO_7(ManufacturingSchematic* manSchem,PlayerObject* playerObject)
{

	if(!(playerObject->isConnected()))
		return(false);

	Message*							message;
	Message*							finalMessage;

	ManufactureSlots*					manSlots			= manSchem->getManufactureSlots();
	ManufactureSlots::iterator			manSlotIt			= manSlots->begin();
	
	//ExperimentationProperties*			exProp				= manSchem->getExperimentationProperties();
	//ExperimentationProperties::iterator	epIt				= exProp->begin();

	ExperimentationPropertiesStore::iterator	epStoreIt	= manSchem->expPropStore.begin();

	CustomizationList*					custList			= manSchem->getCustomizationList();
	CustomizationList::iterator			custIt				= custList->begin();
	
	//start with the messagebody

	gMessageFactory->StartMessage();   

	//object count
	gMessageFactory->addUint16(21);	

	// slot description
	//SlotNameList
	//		-1-
	gMessageFactory->addUint32(manSlots->size());
	gMessageFactory->addUint32(manSchem->mUpdateCounter[0]);

	manSlotIt = manSlots->begin();

	while(manSlotIt != manSlots->end())
	{
		gMessageFactory->addString((*manSlotIt)->mDraftSlot->getComponentFile());
		gMessageFactory->addUint32(0);
		gMessageFactory->addString((*manSlotIt)->mDraftSlot->getComponentName());
		
		++manSlotIt;
	}

	// slots filled
	//SlotContentsList
	//		-2-
	gMessageFactory->addUint32(manSlots->size());
	gMessageFactory->addUint32(manSchem->mUpdateCounter[1]);

	manSlotIt = manSlots->begin();

	while(manSlotIt != manSlots->end())
	{
		gMessageFactory->addUint32(0);

		++manSlotIt;
	}

	// resources filled, all lists initially 0
	// IngridientList
	//		-3-
	gMessageFactory->addUint32(manSlots->size());
	gMessageFactory->addUint32(manSchem->mUpdateCounter[2]);
	manSlotIt = manSlots->begin();

	while(manSlotIt != manSlots->end())
	{
		gMessageFactory->addUint32(0);

		++manSlotIt;
	}

	// resources filled amounts, all lists initially 0
	// QuantityList
	//		-4-
	gMessageFactory->addUint32(manSlots->size());
	gMessageFactory->addUint32(manSchem->mUpdateCounter[3]);

	manSlotIt = manSlots->begin();

	while(manSlotIt != manSlots->end())
	{
		gMessageFactory->addUint32(0);

		++manSlotIt;
	}

	// SlotQualityList 
	// is not used!!!!!!!!!!!!!! as quality is calculated clientside!
	//		-5-
	gMessageFactory->addUint32(manSlots->size());
	gMessageFactory->addUint32(manSchem->mUpdateCounter[4]);

	manSlotIt = manSlots->begin();

	while(manSlotIt != manSlots->end())
	{
		gMessageFactory->addUint32(0);

		++manSlotIt;
	}

	// CleanSlotList 
	//
	//		-6-
	gMessageFactory->addUint32(manSlots->size());
	gMessageFactory->addUint32(manSchem->mUpdateCounter[5]);

	manSlotIt = manSlots->begin();

	while(manSlotIt != manSlots->end())
	{
		gMessageFactory->addUint32((*manSlotIt)->mUnknown2);

		++manSlotIt;
	}

	// SlotIndexList
	//
	//		-7-
	manSchem->mUpdateCounter[6] *= 2;

	gMessageFactory->addUint32(manSlots->size());
	gMessageFactory->addUint32(manSchem->mUpdateCounter[6]);

	manSlotIt = manSlots->begin();

	uint32 index = 0;
	while(manSlotIt != manSlots->end())
	{
		gMessageFactory->addUint32(index);
		index++;

		++manSlotIt;
	}

	// IngredientsCounter
	//flowcontrol - 4 times number of slots for every update
	manSchem->setCounter(manSlots->size() * 4);
	gMessageFactory->addUint8(manSlots->size() * 4);

	// experimentation
	///8
	//we need to send them so that they are shown on assembly!!!
	gMessageFactory->addUint32(manSchem->expPropStore.size());
	gMessageFactory->addUint32(manSchem->mUpdateCounter[8]);

	epStoreIt			= manSchem->expPropStore.begin();

	while(epStoreIt != manSchem->expPropStore.end())
	{
		gMessageFactory->addString(BString("crafting"));
		gMessageFactory->addUint32(0);
		gMessageFactory->addString((*epStoreIt).second->mExpAttributeName);
		++epStoreIt;
	}

	//9
	//CurrentExperimentationValueList 

	gMessageFactory->addUint32(manSchem->expPropStore.size());

	//annoyingly the craftattributeslist is still zero at this time so we need to reinitialize them !!!!
	manSchem->mUpdateCounter[9] = manSchem->expPropStore.size();
	gMessageFactory->addUint32(manSchem->mUpdateCounter[9]);

	epStoreIt			= manSchem->expPropStore.begin();

	while(epStoreIt != manSchem->expPropStore.end())
	{
		gMessageFactory->addFloat(0);//(*epIt)->mExpAttributeValue);
		(*epStoreIt).second->mExpAttributeValue = 0;
		(*epStoreIt).second->mExpAttributeValueOld = 0;

		++epStoreIt;
	} 

	// 10
	//exp offset leave as 0

	gMessageFactory->addUint32(manSchem->expPropStore.size());
	//annoyingly the craftattributeslist is still zero at this time so we need to reinitialize them !!!!
	manSchem->mUpdateCounter[10] = manSchem->expPropStore.size();
	gMessageFactory->addUint32(manSchem->mUpdateCounter[10]);

	epStoreIt			= manSchem->expPropStore.begin();

	while(epStoreIt != manSchem->expPropStore.end())
	{

		gMessageFactory->addUint32(0);//(*epIt)->mExpUnknown);

		++epStoreIt;
	}

	//11
	// bluebar / exp bar value

	gMessageFactory->addUint32(manSchem->expPropStore.size());
	//annoyingly the craftattributeslist is still zero at this time so we need to reinitialize them !!!!
	manSchem->mUpdateCounter[11] = manSchem->expPropStore.size();
	gMessageFactory->addUint32(manSchem->mUpdateCounter[11]);

	epStoreIt			= manSchem->expPropStore.begin();

	while(epStoreIt != manSchem->expPropStore.end())
	{
		(*epStoreIt).second->mBlueBarSizeOld = 1.0;
		(*epStoreIt).second->mBlueBarSize = 1.0;
		gMessageFactory->addFloat((*epStoreIt).second->mBlueBarSize);//

		++epStoreIt;
	}

	//12 (c)
	// max experimentation
	gMessageFactory->addUint32(manSchem->expPropStore.size());
	//annoyingly the craftattributeslist is still zero at this time so we need to reinitialize them !!!!
	manSchem->mUpdateCounter[12] = manSchem->expPropStore.size();
	gMessageFactory->addUint32(manSchem->mUpdateCounter[12]);

	epStoreIt			= manSchem->expPropStore.begin();

	while(epStoreIt != manSchem->expPropStore.end())
	{

		(*epStoreIt).second->mMaxExpValue = 0;
		(*epStoreIt).second->mMaxExpValueOld = 0;
		gMessageFactory->addFloat(0);//(*epIt)->mMaxExpValue);

		++epStoreIt;
	}
	
	// 13 customization attribute name (d)	
	uint32 custSize = custList->size();
	gMessageFactory->addUint32(custSize);
	gMessageFactory->addUint32(custSize);
	
	custIt = custList->begin();
	while(custIt != custList->end())
	{
		gMessageFactory->addString((*custIt)->attribute);
		++custIt;
	}
	

	// 14 customization default color
	gMessageFactory->addUint32(custSize);
	gMessageFactory->addUint32(custSize);
	
	custIt = custList->begin();
	while(custIt != custList->end())
	{
		gMessageFactory->addUint32((*custIt)->defaultValue);
		++custIt;
	}

	// 15 palette start
	gMessageFactory->addUint32(custSize);
	gMessageFactory->addUint32(custSize);
	
	custIt = custList->begin();
	while(custIt != custList->end())
	{
		gMessageFactory->addUint32(0);
		++custIt;
	}


	// 16 palette end
	gMessageFactory->addUint32(custSize);
	gMessageFactory->addUint32(custSize);
	
	
	custIt = custList->begin();
	while(custIt != custList->end())
	{
		gMessageFactory->addUint32((*custIt)->paletteSize);
		++custIt;
	}

	// 17 customization counter
	gMessageFactory->addUint8(custSize);
	
	gMessageFactory->addUint64(0);
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint8(0);
	gMessageFactory->addUint8(1);

	message = gMessageFactory->EndMessage();


	gMessageFactory->StartMessage();         
	gMessageFactory->addUint32(opBaselinesMessage);   
	gMessageFactory->addUint64(manSchem->getId()); 
	gMessageFactory->addUint32(opMSCO);
	gMessageFactory->addUint8(7);  

	gMessageFactory->addUint32(message->getSize());
	gMessageFactory->addData(message->getData(),message->getSize());

	finalMessage = gMessageFactory->EndMessage();
	message->setPendingDelete(true);

	(playerObject->getClient())->SendChannelA(finalMessage,playerObject->getAccountId(),CR_Client,5);

	return(true);
}

//======================================================================================================================

bool MessageLib::sendDeltasMSCO_7(ManufacturingSchematic* manSchem,PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	Message*							message;
	ManufactureSlots*					manSlots			= manSchem->getManufactureSlots();
	ManufactureSlots::iterator			manSlotIt			= manSlots->begin();
	
	uint32								slotDataByteCount	= 0;
	//uint32							caByteCount			= 0;
	uint16								elementIndex		= 0;

	// get total size
	while(manSlotIt != manSlots->end())
	{
		slotDataByteCount += 57 + (*manSlotIt)->mFilledResources.size() * 12 + (*manSlotIt)->mDraftSlot->getComponentFile().getLength() + (*manSlotIt)->mDraftSlot->getComponentName().getLength();
		++manSlotIt;
	}
	slotDataByteCount += 3;

	//while(caIt != craftAtts->end())
	//{
	//	caByteCount += 8 ;
	//	++caIt;
	//}

	gMessageFactory->StartMessage();         
	gMessageFactory->addUint32(opDeltasMessage);   
	gMessageFactory->addUint64(manSchem->getId()); 
	gMessageFactory->addUint32(opMSCO);
	gMessageFactory->addUint8(7);  

	gMessageFactory->addUint32(103 + slotDataByteCount + 3);
	gMessageFactory->addUint16(11);	

	// slot description
	gMessageFactory->addUint16(0);
	gMessageFactory->addUint32(manSlots->size());

	gMessageFactory->addUint32(++manSchem->mUpdateCounter[0]);

	manSlotIt = manSlots->begin();

	gMessageFactory->addUint8(3);		//1 for add - 3 for redo
	//gMessageFactory->addUint16(elementIndex);
	gMessageFactory->addUint16(manSlots->size());

	while(manSlotIt != manSlots->end())
	{	
		gMessageFactory->addString((*manSlotIt)->mDraftSlot->getComponentFile());
		gMessageFactory->addUint32(0);	
		gMessageFactory->addString((*manSlotIt)->mDraftSlot->getComponentName());
	
		

		++elementIndex;
		++manSlotIt;
	}

	//
	// SlotContent filled
	// 0 nothing
	// 1 ??
	// 2 item
	// 4 resource
	gMessageFactory->addUint16(1);
	gMessageFactory->addUint32(manSlots->size() + 1);

	gMessageFactory->addUint32(++manSchem->mUpdateCounter[1]);

	manSlotIt		= manSlots->begin();
	elementIndex	= 0;

	while(manSlotIt != manSlots->end())
	{
		gMessageFactory->addUint8(1);
		gMessageFactory->addUint16(elementIndex);

		gMessageFactory->addUint32(0);//(*manSlotIt)->getmFilledIndicator());

		++elementIndex;
		++manSlotIt;
	}

	// the updated slot
	manSlotIt		= manSlots->begin();
	elementIndex	= 0;

	while(manSlotIt != manSlots->end())
	{
		if((*manSlotIt)->mFilled > 0)
			break;

		++elementIndex;
		++manSlotIt;
	}

	gMessageFactory->addUint8(2);
	gMessageFactory->addUint16(elementIndex);

	gMessageFactory->addUint32((*manSlotIt)->getmFilledIndicator());
	//printf("\n Filled type  : %u\n",(*manSlotIt)->getmFilledIndicator());

	//
	// resources filled
	//
	gMessageFactory->addUint16(2);
	gMessageFactory->addUint32(manSlots->size() + 1);

	gMessageFactory->addUint32(++manSchem->mUpdateCounter[2]);
	//printf("\nsendDeltasMSCO_7 counter 2 : %u\n",manSchem->mUpdateCounter[2]);

	manSlotIt		= manSlots->begin();
	elementIndex	= 0;
	
	while(manSlotIt != manSlots->end())
	{
		gMessageFactory->addUint8(1);
		gMessageFactory->addUint16(elementIndex);

		gMessageFactory->addUint32(0);

		++elementIndex;
		++manSlotIt;
	}

	// the updated slot
	manSlotIt		= manSlots->begin();
	elementIndex	= 0;
	FilledResources::iterator filledResIt;

	while(manSlotIt != manSlots->end())
	{
		filledResIt = (*manSlotIt)->mFilledResources.begin();

		if((*manSlotIt)->mFilledResources.size())
			break;

		++elementIndex;
		++manSlotIt;
	}

	gMessageFactory->addUint8(2);
	gMessageFactory->addUint16(elementIndex);

	gMessageFactory->addUint32((*manSlotIt)->mFilledResources.size());

	while(filledResIt != (*manSlotIt)->mFilledResources.end())
	{
		gMessageFactory->addUint64((*filledResIt).first);
		++filledResIt;
	}



	//
	// resources filled amounts, all lists initially 0
	//
	gMessageFactory->addUint16(3);
	gMessageFactory->addUint32(manSlots->size() + 1);


	gMessageFactory->addUint32(++manSchem->mUpdateCounter[3]);

	manSlotIt		= manSlots->begin();
	elementIndex	= 0;

	while(manSlotIt != manSlots->end())
	{
		gMessageFactory->addUint8(1);
		gMessageFactory->addUint16(elementIndex);

		gMessageFactory->addUint32(0);

		++elementIndex;
		++manSlotIt;
	}

	// the updated slot
	manSlotIt		= manSlots->begin();
	elementIndex	= 0;

	while(manSlotIt != manSlots->end())
	{
		filledResIt = (*manSlotIt)->mFilledResources.begin();

		if((*manSlotIt)->mFilled>0)
			break;

		++elementIndex;
		++manSlotIt;
	}

	gMessageFactory->addUint8(2);
	gMessageFactory->addUint16(elementIndex);

	gMessageFactory->addUint32((*manSlotIt)->mFilledResources.size());

	while(filledResIt != (*manSlotIt)->mFilledResources.end())
	{
		gMessageFactory->addUint32((*filledResIt).second);
		++filledResIt;
	}

	// slotqualitylist
	gMessageFactory->addUint16(4);
	gMessageFactory->addUint32(manSlots->size());

	gMessageFactory->addUint32(manSchem->mUpdateCounter[4]);

	manSlotIt		= manSlots->begin();
	elementIndex	= 0;

	while(manSlotIt != manSlots->end())
	{
		gMessageFactory->addUint8(1);
		gMessageFactory->addUint16(elementIndex);

		gMessageFactory->addUint32((*manSlotIt)->mUnknown1);

		++manSlotIt;
		++elementIndex;
	}

	// cleanslotlist
	gMessageFactory->addUint16(5);
	gMessageFactory->addUint32(manSlots->size() + 1);

	gMessageFactory->addUint32(++manSchem->mUpdateCounter[5]);

	manSlotIt		= manSlots->begin();
	elementIndex	= 0;

	while(manSlotIt != manSlots->end())
	{
		gMessageFactory->addUint8(1);
		gMessageFactory->addUint16(elementIndex);

		gMessageFactory->addUint32((*manSlotIt)->mUnknown2);

		++manSlotIt;
		++elementIndex;
	}

	// the updated slot
	manSlotIt		= manSlots->begin();
	elementIndex	= 0;

	while(manSlotIt != manSlots->end())
	{
		if((*manSlotIt)->mFilled > 0)
			break;

		++manSlotIt;
		++elementIndex;
	}

	gMessageFactory->addUint8(2);
	gMessageFactory->addUint16(elementIndex);

	gMessageFactory->addUint32(0);


	// slotindexlist
	gMessageFactory->addUint16(6);
	gMessageFactory->addUint32(manSlots->size() * 2);

	gMessageFactory->addUint32(manSchem->mUpdateCounter[6]);

	manSlotIt		= manSlots->begin();
	elementIndex	= 0;

	while(manSlotIt != manSlots->end())
	{
		gMessageFactory->addUint8(1);
		gMessageFactory->addUint16(elementIndex);

		gMessageFactory->addUint32((*manSlotIt)->mUnknown2);

		gMessageFactory->addUint8(2);
		gMessageFactory->addUint16(elementIndex);

		gMessageFactory->addUint32(elementIndex);

		++elementIndex;
		++manSlotIt;
	}

	// unknown flag
	gMessageFactory->addUint16(7);
	gMessageFactory->addUint8(manSchem->getCounter());

	// ready flag
	gMessageFactory->addUint16(20);
	gMessageFactory->addUint8(1);
	gMessageFactory->addUint32(0);

	/*gMessageFactory->addUint64(0);
	gMessageFactory->addUint64(0);
	gMessageFactory->addUint32(0);*/

	message = gMessageFactory->EndMessage();

	(playerObject->getClient())->SendChannelA(message,playerObject->getAccountId(),CR_Client,5);

	return(true);
}

//======================================================================================================================

bool MessageLib::sendBaselinesMSCO_8(ManufacturingSchematic* manSchem,PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	Message* message;

	gMessageFactory->StartMessage();       
	gMessageFactory->addUint32(opBaselinesMessage);   
	gMessageFactory->addUint64(manSchem->getId()); 
	gMessageFactory->addUint32(opMSCO);
	gMessageFactory->addUint8(8);  

	gMessageFactory->addUint32(2);
	gMessageFactory->addUint16(0);

	message = gMessageFactory->EndMessage();

	(playerObject->getClient())->SendChannelA(message,playerObject->getAccountId(),CR_Client,5);

	return(true);
}

//======================================================================================================================

bool MessageLib::sendBaselinesMSCO_9(ManufacturingSchematic* manSchem,PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	Message* message;

	gMessageFactory->StartMessage();         
	gMessageFactory->addUint32(opBaselinesMessage);   
	gMessageFactory->addUint64(manSchem->getId()); 
	gMessageFactory->addUint32(opMSCO);
	gMessageFactory->addUint8(9);  

	gMessageFactory->addUint32(2);
	gMessageFactory->addUint16(0);	

	message = gMessageFactory->EndMessage();

	(playerObject->getClient())->SendChannelA(message,playerObject->getAccountId(),CR_Client,5);

	return(true);
}

//======================================================================================================================

bool MessageLib::sendUpdateFilledManufactureSlots(ManufacturingSchematic* manSchem,PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	Message* newMessage;

	gMessageFactory->StartMessage();  
	gMessageFactory->addUint32(opDeltasMessage);
	gMessageFactory->addUint64(manSchem->getId());
	gMessageFactory->addUint32(opMSCO);
	gMessageFactory->addUint8(6);

	gMessageFactory->addUint32(5);
	gMessageFactory->addUint16(1);
	gMessageFactory->addUint16(5);
	gMessageFactory->addUint8(manSchem->getFilledSlotCount());

	newMessage = gMessageFactory->EndMessage();

	(playerObject->getClient())->SendChannelA(newMessage, playerObject->getAccountId(), CR_Client, 5);

	return(true);
}

//======================================================================================================================

bool MessageLib::sendManufactureSlotUpdateSmall(ManufacturingSchematic* manSchem,uint8 slotId,PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	ManufactureSlot*			manSlot	= manSchem->getManufactureSlots()->at(slotId);
	FilledResources::iterator	filledResIt = manSlot->mFilledResources.begin();
	Message*					newMessage;

	gMessageFactory->StartMessage();  
	gMessageFactory->addUint32(opDeltasMessage);
	gMessageFactory->addUint64(manSchem->getId());
	gMessageFactory->addUint32(opMSCO);
	gMessageFactory->addUint8(7);

	//the problem is that the update counters only are increased  clientside igf there HAS been a change
	//Thus, if we send an update, increase the update counter but the data is the same
	//the clientsided updatecounter will not increase and thus our objectdata will get unreliable
	//To design around this problem which likely can only be changed by mugly idea of an Objectbroker
	//which will pool objectchanges and then send the updates I implemented this short but elegant if/else construct
	//which will send (and thus increase the updatecounter) the ressourcedata only if necessary, but the amount always


	gMessageFactory->addUint32(39 + manSlot->mFilledResources.size() * 12);
	gMessageFactory->addUint16(3);

	
	// resources filled
	gMessageFactory->addUint16(2);

	gMessageFactory->addUint32(1);
	gMessageFactory->addUint32(++manSchem->mUpdateCounter[2]);

	gMessageFactory->addUint8(2);
	gMessageFactory->addUint16(slotId);

	gMessageFactory->addUint32(manSlot->mFilledResources.size());

	while(filledResIt != manSlot->mFilledResources.end())
	{
		gMessageFactory->addUint64((*filledResIt).first);
		++filledResIt;
	}
	
	
	// resource amounts filled
	gMessageFactory->addUint16(3);

	gMessageFactory->addUint32(1);
	gMessageFactory->addUint32(++manSchem->mUpdateCounter[3]);

	gMessageFactory->addUint8(2);
	gMessageFactory->addUint16(slotId);

	gMessageFactory->addUint32(manSlot->mFilledResources.size());

	filledResIt = manSlot->mFilledResources.begin();

	while(filledResIt != manSlot->mFilledResources.end())
	{
		gMessageFactory->addUint32((*filledResIt).second);
		++filledResIt;
	}

	
	// unknown
	gMessageFactory->addUint16(7);
	gMessageFactory->addUint8(manSchem->getCounter());

	newMessage = gMessageFactory->EndMessage();

	(playerObject->getClient())->SendChannelA(newMessage, playerObject->getAccountId(), CR_Client, 5);

	return(true);
}


bool MessageLib::sendManufactureSlotUpdate(ManufacturingSchematic* manSchem,uint8 slotId,PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	ManufactureSlot*			manSlot	= manSchem->getManufactureSlots()->at(slotId);
	FilledResources::iterator	filledResIt = manSlot->mFilledResources.begin();
	Message*					newMessage;

	gMessageFactory->StartMessage();  
	gMessageFactory->addUint32(opDeltasMessage);
	gMessageFactory->addUint64(manSchem->getId());
	gMessageFactory->addUint32(opMSCO);
	gMessageFactory->addUint8(7);

	//the problem is that the update counters only are increased  clientside if there HAS been a change
	//Thus, if we send an update, increase the update counter but the data is the same
	//the clientsided updatecounter will not increase and thus our objectdata will get unreliable
	//To design around this problem which likely can only be changed by mugly idea of an Objectbroker
	//which will pool objectchanges and then send the updates 


	gMessageFactory->addUint32(73 + manSlot->mFilledResources.size() * 12);
	gMessageFactory->addUint16(5);

	// filled indicator
	//find out wether there are changes
	//Only advance our updateCounter when there are changes!!!!!
	if(manSlot->mFilledIndicatorChange)
	{
		++manSchem->mUpdateCounter[1];
		manSlot->mFilledIndicatorChange = false;
	}
	
	gMessageFactory->addUint16(1);

	gMessageFactory->addUint32(1);
	gMessageFactory->addUint32(manSchem->mUpdateCounter[1]);

	gMessageFactory->addUint8(2);
	gMessageFactory->addUint16(slotId);

	gMessageFactory->addUint32(manSlot->getmFilledIndicator());
	

	// resources filled
	gMessageFactory->addUint16(2);

	gMessageFactory->addUint32(1);
	gMessageFactory->addUint32(++manSchem->mUpdateCounter[2]);

	gMessageFactory->addUint8(2);
	gMessageFactory->addUint16(slotId);

	gMessageFactory->addUint32(manSlot->mFilledResources.size());

	while(filledResIt != manSlot->mFilledResources.end())
	{
		gMessageFactory->addUint64((*filledResIt).first);

		++filledResIt;
	}
	
	
	// resource amounts filled
	gMessageFactory->addUint16(3);

	gMessageFactory->addUint32(1);
	gMessageFactory->addUint32(++manSchem->mUpdateCounter[3]);

	gMessageFactory->addUint8(2);
	gMessageFactory->addUint16(slotId);

	gMessageFactory->addUint32(manSlot->mFilledResources.size());

	filledResIt = manSlot->mFilledResources.begin();

	while(filledResIt != manSlot->mFilledResources.end())
	{
		gMessageFactory->addUint32((*filledResIt).second);
		++filledResIt;
	}

	// unknown
	gMessageFactory->addUint16(5);

	gMessageFactory->addUint32(1);
	gMessageFactory->addUint32(++manSchem->mUpdateCounter[5]);

	gMessageFactory->addUint8(2);
	gMessageFactory->addUint16(slotId);

	if (manSlot->mFilled >0)
	{
		gMessageFactory->addUint32(0);
	}
	else
	{
		//Fill again with 0xffffffff when its empty
		//this is important otherwise the client will get unstable and the slots show odd behaviour
		gMessageFactory->addUint32(0xFFFFFFFF);
	}

	// unknown
	gMessageFactory->addUint16(7);
	gMessageFactory->addUint8(manSchem->getCounter());

	newMessage = gMessageFactory->EndMessage();

	(playerObject->getClient())->SendChannelA(newMessage, playerObject->getAccountId(), CR_Client, 5);

	return(true);
}

//======================================================================================================================

bool MessageLib::sendDeltasMSCO_3(ManufacturingSchematic* manSchem,PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	Message*				newMessage;
	AttributeMap*			attributes		= manSchem->getAttributeMap();
	AttributeMap::iterator	it				= attributes->begin();
	uint32					attByteCount	= 0;

	// attributes we update here are the attrivutes the final object will have on completion
	while(it != attributes->end())
	{
		attByteCount += 21 + gWorldManager->getAttributeKey((*it).first).getLength();
		++it;
	}

	gMessageFactory->StartMessage();  
	gMessageFactory->addUint32(opDeltasMessage);
	gMessageFactory->addUint64(manSchem->getId());
	gMessageFactory->addUint32(opMSCO);
	gMessageFactory->addUint8(3);

	gMessageFactory->addUint32(12 + attByteCount);
	gMessageFactory->addUint16(1);

	gMessageFactory->addUint16(5);

	gMessageFactory->addUint32(attributes->size());
	gMessageFactory->addUint32(++manSchem->mAttributesUpdateCounter);

	it = attributes->begin();

	while(it != attributes->end())
	{
		gMessageFactory->addUint8(2);
		gMessageFactory->addString(BString("crafting"));
		gMessageFactory->addUint32(0);

		gMessageFactory->addString(gWorldManager->getAttributeKey((*it).first));

		gMessageFactory->addFloat(boost::lexical_cast<float,std::string>((*it).second));

		++it;
	}

	newMessage = gMessageFactory->EndMessage();

	(playerObject->getClient())->SendChannelA(newMessage,playerObject->getAccountId(),CR_Client,5);

	return(true);
}

//======================================================================================================================

bool MessageLib::sendAttributeDeltasMSCO_7(ManufacturingSchematic* manSchem,PlayerObject* playerObject)
{
	// this updates the attributes properties when they have changed on experimentation
	// updated are the attributes value, the bluebarsize or the attribute(indicator of the % of the max value reached)
	// as well as the maxvalue in theory reachable

	if(!(playerObject->isConnected()))
		return(false);

	Message*							newMessage;
	Message*							finalMessage ;

	ExperimentationProperties*			expProp				= manSchem->getExperimentationProperties();
	ExperimentationProperties::iterator	epIt				= expProp->begin();

	ExperimentationPropertiesStore::iterator	epStoreIt	= manSchem->expPropStore.begin();
	
	uint32								objectcount			= 1;

	manSchem->mExpAttributeValueChange = false;
	manSchem->mMaxExpValueChange = false;
	manSchem->mBlueBarSizeChange = false;

	//we MUST make sure that we only update values which have changed!!!
	//otherwise we will desynchronize our list and are stuck without being able to update the client

	epIt	= expProp->begin();
	while(epIt != expProp->end())
	{
		
		if ((*epIt)->mExpAttributeValueOld != (*epIt)->mExpAttributeValue)
		{
			manSchem->mExpAttributeValueChange = true;
			
		}
	
		if ((*epIt)->mBlueBarSizeOld != (*epIt)->mBlueBarSize)
			manSchem->mBlueBarSizeChange = true;
	
		if ((*epIt)->mMaxExpValueOld != (*epIt)->mMaxExpValue)
		{
			manSchem->mMaxExpValueChange = true;
			
		}
			
		++epIt;
	}

	//that is the attribute - not the experimental property!!!
	if(manSchem->mExpAttributeValueChange)
	{
		objectcount ++;
	}

	if(manSchem->mBlueBarSizeChange)
	{
		objectcount ++;
	}

	if(manSchem->mMaxExpValueChange)
	{
		objectcount ++;
	}


	//start writing the messages body
	gMessageFactory->StartMessage();  

	gMessageFactory->addUint16(objectcount);

	// experimentation/assembly percentage
	if(manSchem->mExpAttributeValueChange)
	{
		gMessageFactory->addUint16(9);

		manSchem->mUpdateCounter[9] += (expProp->size());
		gMessageFactory->addUint32(expProp->size());
		
		gMessageFactory->addUint32(manSchem->mUpdateCounter[9]);
		
		manSchem->mUpdateCounter[9] += 1;//(craftAtts->size());
		

		// ´send the *unique* exp properties
		// we send the values of the first of the propertie(s) that comes our way - so the experiment code needs to sort that

		gMessageFactory->addUint8(3);//3 as in write new 2 was change
		gMessageFactory->addUint16(manSchem->expPropStore.size());

		epStoreIt	= manSchem->expPropStore.begin();
		while(epStoreIt != manSchem->expPropStore.end())
		{

			(*epStoreIt).second->mExpAttributeValueOld = (*epStoreIt).second->mExpAttributeValue; 
			gMessageFactory->addFloat((*epStoreIt).second->mExpAttributeValue);
			++epStoreIt;
		}
	}

	//	BlueBarSizeList 
	if(manSchem->mBlueBarSizeChange)
	{
		gMessageFactory->addUint16(11);

		gMessageFactory->addUint32(expProp->size());
		
		manSchem->mUpdateCounter[11] += expProp->size();
		gMessageFactory->addUint32(manSchem->mUpdateCounter[11]);
		//manSchem->mUpdateCounter[11] += 1;//(exProp->size());
		

		gMessageFactory->addUint8(3);//3 as in write new 2 was change
		gMessageFactory->addUint16(manSchem->expPropStore.size());

		epStoreIt	= manSchem->expPropStore.begin();

		while(epStoreIt != manSchem->expPropStore.end())
		{
			gMessageFactory->addFloat((*epStoreIt).second->mBlueBarSize);
			(*epIt)->mBlueBarSizeOld = (*epStoreIt).second->mBlueBarSize;

			++epStoreIt;
		}
	}

	// max values MaxExperimentationList
	if(manSchem->mMaxExpValueChange)
	{
		gMessageFactory->addUint16(12);

		gMessageFactory->addUint32(expProp->size());

		manSchem->mUpdateCounter[12] += expProp->size();
		gMessageFactory->addUint32(manSchem->mUpdateCounter[12]);
		//manSchem->mUpdateCounter[12] += 1;
		

		gMessageFactory->addUint8(3);//3 as in write new; 2 was change
		gMessageFactory->addUint16(expProp->size());

		epStoreIt			= manSchem->expPropStore.begin();
		
		while(epStoreIt != manSchem->expPropStore.end())
		{

			(*epStoreIt).second->mMaxExpValueOld = (*epStoreIt).second->mMaxExpValue;
			gMessageFactory->addFloat((*epStoreIt).second->mMaxExpValue);
			++epStoreIt;
		}
	}

	// success chance
	gMessageFactory->addUint16(18);

	gMessageFactory->addFloat(manSchem->getExpFailureChance());

	newMessage = gMessageFactory->EndMessage();



	gMessageFactory->StartMessage();  
	gMessageFactory->addUint32(opDeltasMessage);
	gMessageFactory->addUint64(manSchem->getId());
	gMessageFactory->addUint32(opMSCO);
	gMessageFactory->addUint8(7);

	gMessageFactory->addUint32(newMessage->getSize());
	gMessageFactory->addData(newMessage->getData(),newMessage->getSize());
	
	finalMessage = gMessageFactory->EndMessage();

	newMessage->setPendingDelete(true);

	(playerObject->getClient())->SendChannelA(finalMessage ,playerObject->getAccountId(),CR_Client,5);

	return(true);
}



//======================================================================================================================

