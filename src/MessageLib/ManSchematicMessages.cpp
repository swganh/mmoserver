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

#include "MessageLib.h"

#include "ZoneServer/CharSheetManager.h"
#include "ZoneServer/Conversation.h"
#include "ZoneServer/CraftingSession.h"
#include "ZoneServer/CraftingTool.h"
#include "ZoneServer/CurrentResource.h"
#include "ZoneServer/DraftSchematic.h"
#include "ZoneServer/DraftSlot.h"
#include "ZoneServer/ManufacturingSchematic.h"
#include "ZoneServer/NPCObject.h"
#include "ZoneServer/ObjectControllerOpcodes.h"
#include "ZoneServer/ObjectFactory.h"
#include "ZoneServer/PlayerObject.h"
#include "ZoneServer/SchematicManager.h"
#include "ZoneServer/UIOpcodes.h"
#include "ZoneServer/Wearable.h"
#include "ZoneServer/WorldManager.h"
#include "ZoneServer/ZoneOpcodes.h"


#ifdef ERROR
#undef ERROR
#endif

#include "Utils/logger.h"

#include "Common/atMacroString.h"
#include "NetworkManager/DispatchClient.h"
#include "NetworkManager/Message.h"
#include "NetworkManager/MessageDispatch.h"
#include "NetworkManager/MessageFactory.h"
#include "NetworkManager/MessageOpcodes.h"

#include <boost/lexical_cast.hpp>

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

    BString					convPlayerName	= playerObject->getFirstName();
    BString					convCustomName	= manSchem->getCustomName();

    convPlayerName.convert(BSTRType_Unicode16);
    convCustomName.convert(BSTRType_Unicode16);

    mMessageFactory->StartMessage();

    //object count
    mMessageFactory->addUint16(13);

    //0
    mMessageFactory->addFloat(static_cast<float>(draftSchematic->getComplexity()));

    //1
    mMessageFactory->addString(manSchem->getNameFile());
    mMessageFactory->addUint32(0);
    mMessageFactory->addString(manSchem->getName());

    //2
    mMessageFactory->addString(convCustomName);

    //3 = volume
    mMessageFactory->addUint32(1);

    //4 = schematic quantity used with schematics with limited uses
    mMessageFactory->addUint32(1);

    //5
    // send attributes on baseline so that they are shown on assembly
    //cave review update counter
    if(sendAttributes)
    {
        manSchem->mAttributesUpdateCounter = attributes->size();
        mMessageFactory->addUint32(attributes->size());
        mMessageFactory->addUint32(manSchem->mAttributesUpdateCounter);

        it = attributes->begin();

        while(it != attributes->end())
        {
            mMessageFactory->addUint8(0);
            mMessageFactory->addString(BString("crafting"));
            mMessageFactory->addUint32(0);
            mMessageFactory->addString(gWorldManager->getAttributeKey((*it).first));

            //=============================0
            // see whether the attribute has any component values which need adding in the preview

            if(manSchem->hasPPAttribute(gWorldManager->getAttributeKey((*it).first)))
            {
                float attributeValue = boost::lexical_cast<float,std::string>((*it).second);
                float attributeAddValue = manSchem->getPPAttribute<float>(gWorldManager->getAttributeKey((*it).first));
                DLOG(INFO) << "MessageLib::sendBaselinesMSCO_3 Attribute Add Value";
                DLOG(INFO) << "MessageLib::sendBaselinesMSCO_3 we will add " << attributeAddValue << " to " << gWorldManager->getAttributeKey((*it).first).getAnsi();
                mMessageFactory->addFloat(attributeValue+attributeAddValue);
            }
            else
                mMessageFactory->addFloat(boost::lexical_cast<float,std::string>((*it).second));

            ++it;
        }
    }
    else
    {
        mMessageFactory->addUint32(0);
        mMessageFactory->addUint32(0);
    }
    //6 creators name
    mMessageFactory->addString(convPlayerName);

    //7 complexity
    mMessageFactory->addUint32(static_cast<uint32>(manSchem->getComplexity()));

    // schematic data size
    mMessageFactory->addUint32(1);

    part = mMessageFactory->EndMessage();


    mMessageFactory->StartMessage();

    mMessageFactory->addUint32(opBaselinesMessage);
    mMessageFactory->addUint64(manSchem->getId());
    mMessageFactory->addUint32(opMSCO);
    mMessageFactory->addUint8(3);
    mMessageFactory->addUint32(part->getSize());
    mMessageFactory->addData(part->getData(),part->getSize());

    message = mMessageFactory->EndMessage();
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

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opBaselinesMessage);
    mMessageFactory->addUint64(manSchem->getId());
    mMessageFactory->addUint32(opMSCO);
    mMessageFactory->addUint8(6);

    mMessageFactory->addUint32(16 + manSchem->getItemModel().getLength());
    mMessageFactory->addUint16(6);
    mMessageFactory->addFloat(static_cast<float>(draftSchematic->getComplexity()));
    //customization string
    mMessageFactory->addUint16(0);

    mMessageFactory->addString(manSchem->getItemModel());
    //prototype crc
    mMessageFactory->addUint32(manSchem->getDynamicInt32());
    mMessageFactory->addUint8(1);
    mMessageFactory->addUint8(manSchem->getFilledSlotCount());

    message = mMessageFactory->EndMessage();

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

    mMessageFactory->StartMessage();

    //object count
    mMessageFactory->addUint16(21);

    // slot description
    //SlotNameList
    //		-1-
    mMessageFactory->addUint32(manSlots->size());
    mMessageFactory->addUint32(manSchem->mUpdateCounter[0]);

    manSlotIt = manSlots->begin();

    while(manSlotIt != manSlots->end())
    {
        mMessageFactory->addString((*manSlotIt)->mDraftSlot->getComponentFile());
        mMessageFactory->addUint32(0);
        mMessageFactory->addString((*manSlotIt)->mDraftSlot->getComponentName());

        ++manSlotIt;
    }

    // slots filled
    //SlotContentsList
    //		-2-
    mMessageFactory->addUint32(manSlots->size());
    mMessageFactory->addUint32(manSchem->mUpdateCounter[1]);

    manSlotIt = manSlots->begin();

    while(manSlotIt != manSlots->end())
    {
        mMessageFactory->addUint32(0);

        ++manSlotIt;
    }

    // resources filled, all lists initially 0
    // IngridientList
    //		-3-
    mMessageFactory->addUint32(manSlots->size());
    mMessageFactory->addUint32(manSchem->mUpdateCounter[2]);
    manSlotIt = manSlots->begin();

    while(manSlotIt != manSlots->end())
    {
        mMessageFactory->addUint32(0);

        ++manSlotIt;
    }

    // resources filled amounts, all lists initially 0
    // QuantityList
    //		-4-
    mMessageFactory->addUint32(manSlots->size());
    mMessageFactory->addUint32(manSchem->mUpdateCounter[3]);

    manSlotIt = manSlots->begin();

    while(manSlotIt != manSlots->end())
    {
        mMessageFactory->addUint32(0);

        ++manSlotIt;
    }

    // SlotQualityList
    // is not used!!!!!!!!!!!!!! as quality is calculated clientside!
    //		-5-
    mMessageFactory->addUint32(manSlots->size());
    mMessageFactory->addUint32(manSchem->mUpdateCounter[4]);

    manSlotIt = manSlots->begin();

    while(manSlotIt != manSlots->end())
    {
        mMessageFactory->addUint32(0);

        ++manSlotIt;
    }

    // CleanSlotList
    //
    //		-6-
    mMessageFactory->addUint32(manSlots->size());
    mMessageFactory->addUint32(manSchem->mUpdateCounter[5]);

    manSlotIt = manSlots->begin();

    while(manSlotIt != manSlots->end())
    {
        mMessageFactory->addUint32((*manSlotIt)->mUnknown2);

        ++manSlotIt;
    }

    // SlotIndexList
    //
    //		-7-
    manSchem->mUpdateCounter[6] *= 2;

    mMessageFactory->addUint32(manSlots->size());
    mMessageFactory->addUint32(manSchem->mUpdateCounter[6]);

    manSlotIt = manSlots->begin();

    uint32 index = 0;
    while(manSlotIt != manSlots->end())
    {
        mMessageFactory->addUint32(index);
        index++;

        ++manSlotIt;
    }

    // IngredientsCounter
    //flowcontrol - 4 times number of slots for every update
    manSchem->setCounter(manSlots->size() * 4);
    mMessageFactory->addUint8(manSlots->size() * 4);

    // experimentation
    ///8
    //we need to send them so that they are shown on assembly!!!
    mMessageFactory->addUint32(manSchem->expPropStore.size());
    mMessageFactory->addUint32(manSchem->mUpdateCounter[8]);

    epStoreIt			= manSchem->expPropStore.begin();

    while(epStoreIt != manSchem->expPropStore.end())
    {
        mMessageFactory->addString(BString("crafting"));
        mMessageFactory->addUint32(0);
        mMessageFactory->addString((*epStoreIt).second->mExpAttributeName);
        ++epStoreIt;
    }

    //9
    //CurrentExperimentationValueList

    mMessageFactory->addUint32(manSchem->expPropStore.size());

    //annoyingly the craftattributeslist is still zero at this time so we need to reinitialize them !!!!
    manSchem->mUpdateCounter[9] = manSchem->expPropStore.size();
    mMessageFactory->addUint32(manSchem->mUpdateCounter[9]);

    epStoreIt			= manSchem->expPropStore.begin();

    while(epStoreIt != manSchem->expPropStore.end())
    {
        mMessageFactory->addFloat(0);//(*epStoreIt)->mExpAttributeValue);
        (*epStoreIt).second->mExpAttributeValue = 0;
        (*epStoreIt).second->mExpAttributeValueOld = 0;

        ++epStoreIt;
    }

    // 10
    //exp offset leave as 0

    mMessageFactory->addUint32(manSchem->expPropStore.size());
    //annoyingly the craftattributeslist is still zero at this time so we need to reinitialize them !!!!
    manSchem->mUpdateCounter[10] = manSchem->expPropStore.size();
    mMessageFactory->addUint32(manSchem->mUpdateCounter[10]);

    epStoreIt			= manSchem->expPropStore.begin();

    while(epStoreIt != manSchem->expPropStore.end())
    {

        mMessageFactory->addUint32(0);//(*epStoreIt)->mExpUnknown);

        ++epStoreIt;
    }

    //11
    // bluebar / exp bar value

    mMessageFactory->addUint32(manSchem->expPropStore.size());
    //annoyingly the craftattributeslist is still zero at this time so we need to reinitialize them !!!!
    manSchem->mUpdateCounter[11] = manSchem->expPropStore.size();
    mMessageFactory->addUint32(manSchem->mUpdateCounter[11]);

    epStoreIt			= manSchem->expPropStore.begin();

    while(epStoreIt != manSchem->expPropStore.end())
    {
        (*epStoreIt).second->mBlueBarSizeOld = 1.0;
        (*epStoreIt).second->mBlueBarSize = 1.0;
        mMessageFactory->addFloat((*epStoreIt).second->mBlueBarSize);//

        ++epStoreIt;
    }

    //12 (c)
    // max experimentation
    mMessageFactory->addUint32(manSchem->expPropStore.size());
    //annoyingly the craftattributeslist is still zero at this time so we need to reinitialize them !!!!
    manSchem->mUpdateCounter[12] = manSchem->expPropStore.size();
    mMessageFactory->addUint32(manSchem->mUpdateCounter[12]);

    epStoreIt			= manSchem->expPropStore.begin();

    while(epStoreIt != manSchem->expPropStore.end())
    {

        (*epStoreIt).second->mMaxExpValue = 0;
        (*epStoreIt).second->mMaxExpValueOld = 0;
        mMessageFactory->addFloat(0);//(*epStoreIt)->mMaxExpValue);

        ++epStoreIt;
    }

    // 13 customization attribute name (d)
    uint32 custSize = custList->size();
    mMessageFactory->addUint32(custSize);
    mMessageFactory->addUint32(custSize);

    custIt = custList->begin();
    while(custIt != custList->end())
    {
        mMessageFactory->addString((*custIt)->attribute);
        ++custIt;
    }


    // 14 customization default color
    mMessageFactory->addUint32(custSize);
    mMessageFactory->addUint32(custSize);

    custIt = custList->begin();
    while(custIt != custList->end())
    {
        mMessageFactory->addUint32((*custIt)->defaultValue);
        ++custIt;
    }

    // 15 palette start
    mMessageFactory->addUint32(custSize);
    mMessageFactory->addUint32(custSize);

    custIt = custList->begin();
    while(custIt != custList->end())
    {
        mMessageFactory->addUint32(0);
        ++custIt;
    }


    // 16 palette end
    mMessageFactory->addUint32(custSize);
    mMessageFactory->addUint32(custSize);


    custIt = custList->begin();
    while(custIt != custList->end())
    {
        mMessageFactory->addUint32((*custIt)->paletteSize);
        ++custIt;
    }

    // 17 customization counter
    mMessageFactory->addUint8(static_cast<uint8>(custSize));

    mMessageFactory->addUint64(0);
    mMessageFactory->addUint32(0);
    mMessageFactory->addUint8(0);
    mMessageFactory->addUint8(1);

    message = mMessageFactory->EndMessage();


    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opBaselinesMessage);
    mMessageFactory->addUint64(manSchem->getId());
    mMessageFactory->addUint32(opMSCO);
    mMessageFactory->addUint8(7);

    mMessageFactory->addUint32(message->getSize());
    mMessageFactory->addData(message->getData(),message->getSize());

    finalMessage = mMessageFactory->EndMessage();
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

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opDeltasMessage);
    mMessageFactory->addUint64(manSchem->getId());
    mMessageFactory->addUint32(opMSCO);
    mMessageFactory->addUint8(7);

    mMessageFactory->addUint32(103 + slotDataByteCount + 3);
    mMessageFactory->addUint16(11);

    // slot description
    mMessageFactory->addUint16(0);
    mMessageFactory->addUint32(manSlots->size());

    mMessageFactory->addUint32(++manSchem->mUpdateCounter[0]);

    manSlotIt = manSlots->begin();

    mMessageFactory->addUint8(3);		//1 for add - 3 for redo
    //mMessageFactory->addUint16(elementIndex);
    mMessageFactory->addUint16(manSlots->size());

    while(manSlotIt != manSlots->end())
    {
        mMessageFactory->addString((*manSlotIt)->mDraftSlot->getComponentFile());
        mMessageFactory->addUint32(0);
        mMessageFactory->addString((*manSlotIt)->mDraftSlot->getComponentName());



        ++elementIndex;
        ++manSlotIt;
    }

    //
    // SlotContent filled
    // 0 nothing
    // 1 ??
    // 2 item
    // 4 resource
    mMessageFactory->addUint16(1);
    mMessageFactory->addUint32(manSlots->size() + 1);

    mMessageFactory->addUint32(++manSchem->mUpdateCounter[1]);

    manSlotIt		= manSlots->begin();
    elementIndex	= 0;

    while(manSlotIt != manSlots->end())
    {
        mMessageFactory->addUint8(1);
        mMessageFactory->addUint16(elementIndex);

        mMessageFactory->addUint32(0);//(*manSlotIt)->getmFilledIndicator());

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

    mMessageFactory->addUint8(2);
    mMessageFactory->addUint16(elementIndex);

    mMessageFactory->addUint32(static_cast<uint32>((*manSlotIt)->getFilledType()));


    //
    // resources filled
    //
    mMessageFactory->addUint16(2);
    mMessageFactory->addUint32(manSlots->size() + 1);

    mMessageFactory->addUint32(++manSchem->mUpdateCounter[2]);
    //printf("\nsendDeltasMSCO_7 counter 2 : %u",ManSchem->mUpdateCounter[2]);

    manSlotIt		= manSlots->begin();
    elementIndex	= 0;

    while(manSlotIt != manSlots->end())
    {
        mMessageFactory->addUint8(1);
        mMessageFactory->addUint16(elementIndex);

        mMessageFactory->addUint32(0);

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

    mMessageFactory->addUint8(2);
    mMessageFactory->addUint16(elementIndex);

    mMessageFactory->addUint32((*manSlotIt)->mFilledResources.size());

    while(filledResIt != (*manSlotIt)->mFilledResources.end())
    {
        mMessageFactory->addUint64((*filledResIt).first);
        ++filledResIt;
    }



    //
    // resources filled amounts, all lists initially 0
    //
    mMessageFactory->addUint16(3);
    mMessageFactory->addUint32(manSlots->size() + 1);


    mMessageFactory->addUint32(++manSchem->mUpdateCounter[3]);

    manSlotIt		= manSlots->begin();
    elementIndex	= 0;

    while(manSlotIt != manSlots->end())
    {
        mMessageFactory->addUint8(1);
        mMessageFactory->addUint16(elementIndex);

        mMessageFactory->addUint32(0);

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

    mMessageFactory->addUint8(2);
    mMessageFactory->addUint16(elementIndex);

    mMessageFactory->addUint32((*manSlotIt)->mFilledResources.size());

    filledResIt = (*manSlotIt)->mFilledResources.begin();
    while(filledResIt != (*manSlotIt)->mFilledResources.end())
    {
        mMessageFactory->addUint32((*filledResIt).second);
        ++filledResIt;
    }

    // slotqualitylist
    mMessageFactory->addUint16(4);
    mMessageFactory->addUint32(manSlots->size());

    mMessageFactory->addUint32(manSchem->mUpdateCounter[4]);

    manSlotIt		= manSlots->begin();
    elementIndex	= 0;

    while(manSlotIt != manSlots->end())
    {
        mMessageFactory->addUint8(1);
        mMessageFactory->addUint16(elementIndex);

        mMessageFactory->addUint32((*manSlotIt)->mUnknown1);

        ++manSlotIt;
        ++elementIndex;
    }

    // cleanslotlist
    mMessageFactory->addUint16(5);
    mMessageFactory->addUint32(manSlots->size() + 1);

    mMessageFactory->addUint32(++manSchem->mUpdateCounter[5]);

    manSlotIt		= manSlots->begin();
    elementIndex	= 0;

    while(manSlotIt != manSlots->end())
    {
        mMessageFactory->addUint8(1);
        mMessageFactory->addUint16(elementIndex);

        mMessageFactory->addUint32((*manSlotIt)->mUnknown2);

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

    mMessageFactory->addUint8(2);
    mMessageFactory->addUint16(elementIndex);

    mMessageFactory->addUint32(0);


    // slotindexlist
    mMessageFactory->addUint16(6);
    mMessageFactory->addUint32(manSlots->size() * 2);

    mMessageFactory->addUint32(manSchem->mUpdateCounter[6]);

    manSlotIt		= manSlots->begin();
    elementIndex	= 0;

    while(manSlotIt != manSlots->end())
    {
        mMessageFactory->addUint8(1);
        mMessageFactory->addUint16(elementIndex);

        mMessageFactory->addUint32((*manSlotIt)->mUnknown2);

        mMessageFactory->addUint8(2);
        mMessageFactory->addUint16(elementIndex);

        mMessageFactory->addUint32(elementIndex);

        ++elementIndex;
        ++manSlotIt;
    }

    // unknown flag
    mMessageFactory->addUint16(7);
    mMessageFactory->addUint8(manSchem->getCounter());

    // ready flag
    mMessageFactory->addUint16(20);
    mMessageFactory->addUint8(1);
    mMessageFactory->addUint32(0);

    /*mMessageFactory->addUint64(0);
    mMessageFactory->addUint64(0);
    mMessageFactory->addUint32(0);*/

    message = mMessageFactory->EndMessage();

    (playerObject->getClient())->SendChannelA(message,playerObject->getAccountId(),CR_Client,5);

    return(true);
}

//======================================================================================================================

bool MessageLib::sendBaselinesMSCO_8(ManufacturingSchematic* manSchem,PlayerObject* playerObject)
{
    if(!(playerObject->isConnected()))
        return(false);

    Message* message;

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opBaselinesMessage);
    mMessageFactory->addUint64(manSchem->getId());
    mMessageFactory->addUint32(opMSCO);
    mMessageFactory->addUint8(8);

    mMessageFactory->addUint32(2);
    mMessageFactory->addUint16(0);

    message = mMessageFactory->EndMessage();

    (playerObject->getClient())->SendChannelA(message,playerObject->getAccountId(),CR_Client,5);

    return(true);
}

//======================================================================================================================

bool MessageLib::sendBaselinesMSCO_9(ManufacturingSchematic* manSchem,PlayerObject* playerObject)
{
    if(!(playerObject->isConnected()))
        return(false);

    Message* message;

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opBaselinesMessage);
    mMessageFactory->addUint64(manSchem->getId());
    mMessageFactory->addUint32(opMSCO);
    mMessageFactory->addUint8(9);

    mMessageFactory->addUint32(2);
    mMessageFactory->addUint16(0);

    message = mMessageFactory->EndMessage();

    (playerObject->getClient())->SendChannelA(message,playerObject->getAccountId(),CR_Client,5);

    return(true);
}

//======================================================================================================================

bool MessageLib::sendUpdateFilledManufactureSlots(ManufacturingSchematic* manSchem,PlayerObject* playerObject)
{
    if(!(playerObject->isConnected()))
        return(false);

    Message* newMessage;

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opDeltasMessage);
    mMessageFactory->addUint64(manSchem->getId());
    mMessageFactory->addUint32(opMSCO);
    mMessageFactory->addUint8(6);

    mMessageFactory->addUint32(5);
    mMessageFactory->addUint16(1);
    mMessageFactory->addUint16(5);
    mMessageFactory->addUint8(manSchem->getFilledSlotCount());

    newMessage = mMessageFactory->EndMessage();

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

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opDeltasMessage);
    mMessageFactory->addUint64(manSchem->getId());
    mMessageFactory->addUint32(opMSCO);
    mMessageFactory->addUint8(7);

    //the problem is that the update counters only are increased  clientside igf there HAS been a change
    //Thus, if we send an update, increase the update counter but the data is the same
    //the clientsided updatecounter will not increase and thus our objectdata will get unreliable
    //To design around this problem which likely can only be changed by mugly idea of an Objectbroker
    //which will pool objectchanges and then send the updates I implemented this short but elegant if/else construct
    //which will send (and thus increase the updatecounter) the ressourcedata only if necessary, but the amount always


    mMessageFactory->addUint32(39 + manSlot->mFilledResources.size() * 12);
    mMessageFactory->addUint16(3);


    // resources filled
    mMessageFactory->addUint16(2);

    mMessageFactory->addUint32(1);
    mMessageFactory->addUint32(++manSchem->mUpdateCounter[2]);

    mMessageFactory->addUint8(2);
    mMessageFactory->addUint16(slotId);

    mMessageFactory->addUint32(manSlot->mFilledResources.size());

    filledResIt = manSlot->mFilledResources.begin();
    while(filledResIt != manSlot->mFilledResources.end())
    {
        mMessageFactory->addUint64((*filledResIt).first);
        ++filledResIt;
    }


    // resource amounts filled
    mMessageFactory->addUint16(3);

    mMessageFactory->addUint32(1);
    mMessageFactory->addUint32(++manSchem->mUpdateCounter[3]);

    mMessageFactory->addUint8(2);
    mMessageFactory->addUint16(slotId);

    mMessageFactory->addUint32(manSlot->mFilledResources.size());

    filledResIt = manSlot->mFilledResources.begin();
    while(filledResIt != manSlot->mFilledResources.end())
    {
        mMessageFactory->addUint32((*filledResIt).second);
        ++filledResIt;
    }


    // unknown
    mMessageFactory->addUint16(7);
    mMessageFactory->addUint8(manSchem->getCounter());

    newMessage = mMessageFactory->EndMessage();

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

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opDeltasMessage);
    mMessageFactory->addUint64(manSchem->getId());
    mMessageFactory->addUint32(opMSCO);
    mMessageFactory->addUint8(7);

    //the problem is that the update counters only are increased  clientside if there HAS been a change
    //Thus, if we send an update, increase the update counter but the data is the same
    //the clientsided updatecounter will not increase and thus our objectdata will get unreliable
    //To design around this problem which likely can only be changed by mugly idea of an Objectbroker
    //which will pool objectchanges and then send the updates


    mMessageFactory->addUint32(73 + manSlot->mFilledResources.size() * 12);
    mMessageFactory->addUint16(5);

    // filled Type is eithe 0 for not filled or the types enum

    // find out wether there are changes
    // Only advance our updateCounter when there are changes!!!!!
    if(manSlot->mFilledIndicatorChange)
    {
        ++manSchem->mUpdateCounter[1];
        manSlot->mFilledIndicatorChange = false;
    }

    mMessageFactory->addUint16(1);

    mMessageFactory->addUint32(1);
    mMessageFactory->addUint32(manSchem->mUpdateCounter[1]);

    mMessageFactory->addUint8(2);
    mMessageFactory->addUint16(slotId);

    mMessageFactory->addUint32(static_cast<uint32>(manSlot->getFilledType()));


    // resources filled
    mMessageFactory->addUint16(2);

    mMessageFactory->addUint32(1);
    mMessageFactory->addUint32(++manSchem->mUpdateCounter[2]);

    mMessageFactory->addUint8(2);
    mMessageFactory->addUint16(slotId);

    mMessageFactory->addUint32(manSlot->mFilledResources.size());

    filledResIt = manSlot->mFilledResources.begin();
    while(filledResIt != manSlot->mFilledResources.end())
    {
        mMessageFactory->addUint64((*filledResIt).first);

        ++filledResIt;
    }


    // resource amounts filled
    mMessageFactory->addUint16(3);

    mMessageFactory->addUint32(1);
    mMessageFactory->addUint32(++manSchem->mUpdateCounter[3]);

    mMessageFactory->addUint8(2);
    mMessageFactory->addUint16(slotId);

    mMessageFactory->addUint32(manSlot->mFilledResources.size());

    filledResIt = manSlot->mFilledResources.begin();

    while(filledResIt != manSlot->mFilledResources.end())
    {
        mMessageFactory->addUint32((*filledResIt).second);
        ++filledResIt;
    }

    // unknown
    mMessageFactory->addUint16(5);

    mMessageFactory->addUint32(1);
    mMessageFactory->addUint32(++manSchem->mUpdateCounter[5]);

    mMessageFactory->addUint8(2);
    mMessageFactory->addUint16(slotId);

    if (manSlot->mFilled >0)
    {
        mMessageFactory->addUint32(0);
    }
    else
    {
        //Fill again with 0xffffffff when its empty
        //this is important otherwise the client will get unstable and the slots show odd behaviour
        mMessageFactory->addUint32(0xFFFFFFFF);
    }

    // unknown
    mMessageFactory->addUint16(7);
    mMessageFactory->addUint8(manSchem->getCounter());

    newMessage = mMessageFactory->EndMessage();

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

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opDeltasMessage);
    mMessageFactory->addUint64(manSchem->getId());
    mMessageFactory->addUint32(opMSCO);
    mMessageFactory->addUint8(3);

    mMessageFactory->addUint32(12 + attByteCount);
    mMessageFactory->addUint16(1);

    mMessageFactory->addUint16(5);

    mMessageFactory->addUint32(attributes->size());
    mMessageFactory->addUint32(++manSchem->mAttributesUpdateCounter);

    it = attributes->begin();

    while(it != attributes->end())
    {
        mMessageFactory->addUint8(2);
        mMessageFactory->addString(BString("crafting"));
        mMessageFactory->addUint32(0);

        mMessageFactory->addString(gWorldManager->getAttributeKey((*it).first));

        if(manSchem->hasPPAttribute(gWorldManager->getAttributeKey((*it).first)))
        {
            float attributeValue = boost::lexical_cast<float,std::string>((*it).second);
            float attributeAddValue = manSchem->getPPAttribute<float>(gWorldManager->getAttributeKey((*it).first));
            mMessageFactory->addFloat(attributeValue+attributeAddValue);
        }
        else
            mMessageFactory->addFloat(boost::lexical_cast<float,std::string>((*it).second));

        //mMessageFactory->addFloat(boost::lexical_cast<float,std::string>((*it).second));

        ++it;
    }

    newMessage = mMessageFactory->EndMessage();

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
    mMessageFactory->StartMessage();

    mMessageFactory->addUint16(static_cast<uint16>(objectcount));

    // experimentation/assembly percentage
    if(manSchem->mExpAttributeValueChange)
    {
        mMessageFactory->addUint16(9);

        manSchem->mUpdateCounter[9] += (manSchem->expPropStore.size());
        mMessageFactory->addUint32(manSchem->expPropStore.size());

        mMessageFactory->addUint32(manSchem->mUpdateCounter[9]);

        manSchem->mUpdateCounter[9] += 1;//(craftAtts->size());


        // ´send the *unique* exp properties
        // we send the values of the first of the propertie(s) that comes our way - so the experiment code needs to sort that
        // the reason we do this is that we might have exp properties with attributes affected by differing resource weights

        mMessageFactory->addUint8(3);//3 as in write new 2 was change
        mMessageFactory->addUint16(manSchem->expPropStore.size());

        epStoreIt	= manSchem->expPropStore.begin();
        while(epStoreIt != manSchem->expPropStore.end())
        {

            (*epStoreIt).second->mExpAttributeValueOld = (*epStoreIt).second->mExpAttributeValue;

            if(manSchem->hasPPAttribute(gWorldManager->getAttributeKey((*epStoreIt).first)))
            {
                float attributeValue = (*epStoreIt).second->mExpAttributeValue;
                float attributeAddValue = manSchem->getPPAttribute<float>(gWorldManager->getAttributeKey((*epStoreIt).first));
                mMessageFactory->addFloat(attributeValue+attributeAddValue);
            }
            else
                mMessageFactory->addFloat((*epStoreIt).second->mExpAttributeValue);

            //mMessageFactory->addFloat((*epStoreIt).second->mExpAttributeValue);
            ++epStoreIt;
        }
    }

    //	BlueBarSizeList
    if(manSchem->mBlueBarSizeChange)
    {
        mMessageFactory->addUint16(11);

        mMessageFactory->addUint32(manSchem->expPropStore.size());

        manSchem->mUpdateCounter[11] += manSchem->expPropStore.size();
        mMessageFactory->addUint32(manSchem->mUpdateCounter[11]);
        //manSchem->mUpdateCounter[11] += 1;//(exProp->size());


        mMessageFactory->addUint8(3);//3 as in write new 2 was change
        mMessageFactory->addUint16(manSchem->expPropStore.size());

        epStoreIt	= manSchem->expPropStore.begin();

        while(epStoreIt != manSchem->expPropStore.end())
        {
            mMessageFactory->addFloat((*epStoreIt).second->mBlueBarSize);
            (*epStoreIt).second->mBlueBarSizeOld = (*epStoreIt).second->mBlueBarSize;

            ++epStoreIt;
        }
    }

    // max values MaxExperimentationList
    if(manSchem->mMaxExpValueChange)
    {
        mMessageFactory->addUint16(12);

        mMessageFactory->addUint32(manSchem->expPropStore.size());

        manSchem->mUpdateCounter[12] += manSchem->expPropStore.size();
        mMessageFactory->addUint32(manSchem->mUpdateCounter[12]);
        //manSchem->mUpdateCounter[12] += 1;


        mMessageFactory->addUint8(3);//3 as in write new; 2 was change
        mMessageFactory->addUint16(manSchem->expPropStore.size());

        epStoreIt			= manSchem->expPropStore.begin();

        while(epStoreIt != manSchem->expPropStore.end())
        {

            (*epStoreIt).second->mMaxExpValueOld = (*epStoreIt).second->mMaxExpValue;
            mMessageFactory->addFloat((*epStoreIt).second->mMaxExpValue);
            ++epStoreIt;
        }
    }

    // success chance
    mMessageFactory->addUint16(18);

    mMessageFactory->addFloat(manSchem->getExpFailureChance());

    newMessage = mMessageFactory->EndMessage();



    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opDeltasMessage);
    mMessageFactory->addUint64(manSchem->getId());
    mMessageFactory->addUint32(opMSCO);
    mMessageFactory->addUint8(7);

    mMessageFactory->addUint32(newMessage->getSize());
    mMessageFactory->addData(newMessage->getData(),newMessage->getSize());

    finalMessage = mMessageFactory->EndMessage();

    newMessage->setPendingDelete(true);

    (playerObject->getClient())->SendChannelA(finalMessage ,playerObject->getAccountId(),CR_Client,5);

    return(true);
}



//======================================================================================================================


//======================================================================================================================

bool MessageLib::sendMSCO_3_ComplexityUpdate(ManufacturingSchematic* manSchem,PlayerObject* playerObject)
{

    Message*							newMessage;

    if(!(playerObject->isConnected()))
        return(false);

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opDeltasMessage);
    mMessageFactory->addUint64(manSchem->getId());
    mMessageFactory->addUint32(opMSCO);
    mMessageFactory->addUint8(3);

    mMessageFactory->addUint32(8);
    mMessageFactory->addUint16(1);

    mMessageFactory->addUint16(7);

    mMessageFactory->addUint32(static_cast<uint32>(manSchem->getComplexity()));

    newMessage = mMessageFactory->EndMessage();

    (playerObject->getClient())->SendChannelA(newMessage,playerObject->getAccountId(),CR_Client,5);

    return(true);
}
