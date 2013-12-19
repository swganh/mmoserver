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
#include "CraftingManager.h"
#include "BankTerminal.h"
#include "CraftingSessionFactory.h"
#include "CraftingTool.h"
#include "CraftingStation.h"
#include "CurrentResource.h"
#include "Item.h"
#include "Inventory.h"
#include "ManufacturingSchematic.h"
#include "CraftingSession.h"
#include "NPCObject.h"
#include "ObjectController.h"
#include "ObjectControllerCommandMap.h"
#include "ObjectControllerOpcodes.h"
#include "PlayerObject.h"
#include "SchematicManager.h"
#include "SurveyTool.h"
#include "TravelMapHandler.h"
#include "UIManager.h"
#include "Wearable.h"
#include "WorldConfig.h"
#include "WorldManager.h"
#include "MessageLib/MessageLib.h"

#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include "NetworkManager/Message.h"
#include "NetworkManager/MessageFactory.h"
#include "Utils/clock.h"

#include <boost/lexical_cast.hpp>

bool						CraftingManager::mInsFlag    = false;
CraftingManager*			CraftingManager::mSingleton  = NULL;

CraftingManager::CraftingManager(Database* database) : mDatabase(database){}


CraftingManager::~CraftingManager(void)
{
}

//=============================================================================
//
// request draftslots batch
//

bool CraftingManager::HandleRequestDraftslotsBatch(Object* object,Object* target,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    PlayerObject*	playerObject	= dynamic_cast<PlayerObject*>(object);
    BString			requestStr;
    BStringVector	dataElements;
    uint16			elementCount;

    message->getStringUnicode16(requestStr);
    requestStr.convert(BSTRType_ANSI);

    elementCount = requestStr.split(dataElements,' ');

    if(!elementCount)
    {
        return false;
    }

    for(uint16 i = 1; i < elementCount; i += 2)
    {
        // since we currently store everything in 1 schematic object, just look up by the crc
        // lookup of weights is done in requestresourceweightsbatch
        uint64 itemId = boost::lexical_cast<uint64>(dataElements[i].getAnsi());
        DraftSchematic* schematic = gSchematicManager->getSchematicBySlotId(static_cast<uint32>(itemId));

        if(schematic)
        {
            gMessageLib->sendDraftslotsResponse(schematic,playerObject);
        }
    }
    return true;
}

//======================================================================================================================
//
// request resource weights batch
//

bool CraftingManager::HandleRequestResourceWeightsBatch(Object* object,Object* target,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    PlayerObject*	playerObject	= dynamic_cast<PlayerObject*>(object);
    BString			requestStr;
    BStringVector	dataElements;
    uint16			elementCount;

    message->getStringUnicode16(requestStr);
    requestStr.convert(BSTRType_ANSI);

    elementCount = requestStr.split(dataElements,' ');

    if(!elementCount)
    {
        return false;
    }

    for(uint16 i = 0; i < elementCount; i++)
    {
        uint64 itemId = boost::lexical_cast<uint64>(dataElements[i].getAnsi());
        DraftSchematic* schematic = gSchematicManager->getSchematicByWeightId(static_cast<uint32>(itemId));

        if(schematic)
        {
            gMessageLib->sendDraftWeightsResponse(schematic,playerObject);
        }
    }
    return true;
}

//======================================================================================================================
//
// synchronized ui listen
//

bool CraftingManager::HandleSynchronizedUIListen(Object* object,Object* target,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    return true;
}
// get appropriate crafting tool from selected crafting station
// check inventory for same tool 'type' as crafting station
CraftingTool* CraftingManager::getCraftingStationTool(PlayerObject* playerObject, CraftingStation* station)
{
    CraftingTool*	tool	= NULL;
    int32 stationType = station->getItemType();
    Inventory* inventory = dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
    ObjectIDList::iterator It = inventory->getObjects()->begin();
    while(It != inventory->getObjects()->end())
    {
        Item* item = dynamic_cast<Item*>(gWorldManager->getObjectById((*It)));
        if(!item)
        {
            It++;
            continue;
        }
        int32 itemType = item->getItemType();
        switch (stationType)
        {
        case ItemType_ClothingStation:
        case ItemType_ClothingStationPublic:
        {
            if(itemType == ItemType_ClothingTool)
            {
                tool = dynamic_cast<CraftingTool*>(item);
            }
        }
        break;
        case ItemType_WeaponStation:
        case ItemType_WeaponStationPublic:
        {
            if(itemType == ItemType_WeaponTool)
            {
                tool = dynamic_cast<CraftingTool*>(item);
            }
        }
        break;
        case ItemType_FoodStation:
        case ItemType_FoodStationPublic:
        {
            if(itemType == ItemType_FoodTool)
            {
                tool = dynamic_cast<CraftingTool*>(item);
            }
        }
        break;
        case ItemType_StructureStation:
        case ItemType_StructureStationPublic:
        {
            if(itemType == ItemType_StructureTool)
            {
                tool = dynamic_cast<CraftingTool*>(item);
            }
        }
        break;
        case ItemType_SpaceStation:
        case ItemType_SpaceStationPublic:
        {
            if(itemType == ItemType_SpaceTool)
            {
                tool = dynamic_cast<CraftingTool*>(item);
            }
        }
        break;
        default:
        {
            break;
        }
        break;
        }
        if(tool)
        {
            // found it now jump out
            break;
        }
        ++It;
        continue;
    }
    return tool;
}
//======================================================================================================================
//
// request crafting session
//

bool CraftingManager::HandleRequestCraftingSession(Object* object,Object* target,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    PlayerObject*		playerObject	= dynamic_cast<PlayerObject*>(object);
    CraftingTool*		tool			= dynamic_cast<CraftingTool*>(target);
    CraftingStation*	station			= dynamic_cast<CraftingStation*>(target);
    uint32				expFlag			= 2;//needs to be >1 !!!!!

    message->setIndex(24);
    /*uint32				counter			= */
    message->getUint32();

    //get nearest crafting station
    ObjectSet			inRangeObjects;
    float				range = 25.0;

    // the player clicked directly on a station
    if(station)
    {
        tool = getCraftingStationTool(playerObject, station);
    }

    if(!tool)
    {
        gMessageLib->SendSystemMessage(common::OutOfBand("ui_craft","err_no_crafting_tool"),playerObject);
        gMessageLib->sendCraftAcknowledge(opCraftCancelResponse,0,0,playerObject);
        return false;
    }
    // if we haven't come in through a station
    if(!station)
    {
        // get the tangible objects in range
	    gSpatialIndexManager->getObjectsInRange(playerObject,&inRangeObjects,(ObjType_Tangible),range,true);
        //and see if a fitting crafting station is near
        station = playerObject->getCraftingStation(&inRangeObjects,(ItemType) tool->getItemType());
    }

    if(!station)
    {
        expFlag = false;
    }

    if(tool->getAttribute<std::string>("craft_tool_status") == "@crafting:tool_status_working")
    {
        if(tool->getCurrentItem())
            gMessageLib->SendSystemMessage(::common::OutOfBand("system_msg", "crafting_tool_creating_prototype"), playerObject);

        // TODO: put the right message for practice
        else
            gMessageLib->SendSystemMessage(::common::OutOfBand("system_msg", "crafting_tool_creating_prototype"), playerObject);

        gMessageLib->sendCraftAcknowledge(opCraftCancelResponse, 0, 0, playerObject);

        return false;
    }
	playerObject->setCraftingSession(gCraftingSessionFactory->createSession(Anh_Utils::Clock::getSingleton(), playerObject, tool, station, expFlag));
    return true;
}

//======================================================================================================================
//
// select draft schematic
//

bool CraftingManager::HandleSelectDraftSchematic(Object* object,Object* target,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    PlayerObject*		playerObject	= dynamic_cast<PlayerObject*>(object);
    CraftingSession*	session			= playerObject->getCraftingSession();
    //DraftSchematic*		schematic		= NULL;
    BString				dataStr;
    uint32				schematicIndex	= 0;

    message->getStringUnicode16(dataStr);

    if(session)
    {
        if(swscanf(dataStr.getUnicode16(),L"%u",&schematicIndex) != 1 || !session->selectDraftSchematic(schematicIndex))
        {
            gCraftingSessionFactory->destroySession(session);
        }
    }
    return true;
}

//======================================================================================================================
//
// cancel crafting session
//

bool CraftingManager::HandleCancelCraftingSession(Object* object,Object* target,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    PlayerObject*	playerObject	= dynamic_cast<PlayerObject*>(object);

    message->setIndex(24);

    /*uint32			counter			= */
    message->getUint32();

    gCraftingSessionFactory->destroySession(playerObject->getCraftingSession());
	//client complains over crafting tool already having an item when we go out of the slot screen!!!!!
    return true;
}

//=============================================================================================================================
//
// craft fill slot
//

void CraftingManager::handleCraftFillSlot(Object* object,Message* message)
{
    PlayerObject*		player		= dynamic_cast<PlayerObject*>(object);
    CraftingSession*	session		= player->getCraftingSession();

    uint64				resContainerId	= message->getUint64();
    uint32				slotId			= message->getUint32();
    uint32				unknownId		= message->getUint32();
    uint8				counter			= message->getUint8();

    // make sure we have a valid session and are in the assembly stage
    if(session && player->getCraftingStage() == 2)
    {
        session->handleFillSlot(resContainerId,slotId,unknownId,counter);
    }
    // it failed
    else
    {
        gMessageLib->sendCraftAcknowledge(opCraftFillSlot,CraftError_Not_In_Assembly_Stage,counter,player);
    }
}

//=============================================================================================================================
//
// craft empty slot
//

void CraftingManager::handleCraftEmptySlot(Object* object,Message* message)
{
    PlayerObject*		player		= dynamic_cast<PlayerObject*>(object);
    CraftingSession*	session		= player->getCraftingSession();
    uint32				slotId		= message->getUint32();
    uint64				containerId	= message->getUint64();
    uint8				counter		= message->getUint8();

    // make sure we have a valid session and are in the assembly stage
    if(session && player->getCraftingStage() == 2)
    {
        session->handleEmptySlot(slotId,containerId,counter);
    }
    // it failed
    else
    {
        gMessageLib->sendCraftAcknowledge(opCraftEmptySlot,CraftError_Not_In_Assembly_Stage,counter,player);
    }
}

//=============================================================================================================================
//
// craft experiment
//

void CraftingManager::handleCraftExperiment(Object* object, Message* message)
{
    PlayerObject*		player		= dynamic_cast<PlayerObject*>(object);
    CraftingSession*	session		= player->getCraftingSession();
    uint8				counter		= message->getUint8();
    uint32				propCount	= message->getUint32();
    std::vector<std::pair<uint32,uint32> >	properties;

    if(!session || player->getCraftingStage() != 3)
        return;

    for(uint32 i = 0; i < propCount; i++)
        properties.push_back(std::make_pair(message->getUint32(),message->getUint32()));

    session->experiment(counter,properties);
}

//=============================================================================================================================
//
// customization
//

void CraftingManager::handleCraftCustomization(Object* object,Message* message)
{
    PlayerObject*		player		= dynamic_cast<PlayerObject*>(object);
    CraftingSession*	session		= player->getCraftingSession();
    BString				itemName;
    uint8				hmmm1,hmmm2;
    uint32				amount,color;

    if(!session)
        return;
    player->setCraftingStage(4);

    message->getStringUnicode16(itemName);
    itemName.convert(BSTRType_ANSI);

    message->getUint8(hmmm1);

    message->getUint32(amount);
    message->getUint8(hmmm2);

    CustomizationList* cList;

    cList = session->getManufacturingSchematic()->getCustomizationList();
    CustomizationList::iterator	custIt = cList->begin();

    uint32 i = 0;
    while((custIt != cList->end())&&(i < hmmm2))
    {
        message->getUint32(color);
        message->getUint32(color);
        session->getItem()->setCustomization(static_cast<uint8>((*custIt)->cutomizationIndex),(uint16)color,3);

        i++;
        ++custIt;
    }

    int8 sql[550];
    sprintf(sql, "INSERT INTO %s.item_customization VALUES (%" PRIu64 ", %u, %u)",mDatabase->galaxy(),session->getItem()->getId(), session->getItem()->getCustomization(1), session->getItem()->getCustomization(2));
    mDatabase->executeAsyncSql(sql);

    session->setProductionAmount(amount);
    session->customize(itemName.getAnsi());
}

//=============================================================================================================================
//
// nextcraftingstage
//

bool CraftingManager::HandleNextCraftingStage(Object* object, Object* target,Message* message, ObjectControllerCmdProperties* cmdProperties)
{
    PlayerObject*		playerObject	= dynamic_cast<PlayerObject*>(object);
    CraftingSession*	session			= playerObject->getCraftingSession();
    BString				dataStr;
    uint32				counter			= 1;

    if(!session)
        return false;

    message->getStringUnicode16(dataStr);

    if(dataStr.getLength() == 0)
    {
        //Command Line Entry
        counter = session->getCounter();
    }
    else
    {
        uint32 resultCount = swscanf(dataStr.getUnicode16(),L"%u",&counter);
        if(resultCount != 1)
        {
            gCraftingSessionFactory->destroySession(session);
            return false;
        }
    }

    switch(session->getStage())
    {
    case 1:
    {
        //Player's Macro is wrong! :p
    }
    break;

    case 2:
    {
        session->assemble(counter);
    }
    break;

    case 3:
    {
        session->experimentationStage(counter);

    }
    break;

    case 4:
    {
        session->customizationStage(counter);
        //session->creationStage(counter);
    }
    break;

    case 5:
    {
        session->creationStage(counter);
    }
    break;

    default:
    {
    }
    break;
    }
    return true;
}

//=============================================================================================================================
//
// createprototype
//

bool CraftingManager::HandleCreatePrototype(Object* object, Object* target,Message* message, ObjectControllerCmdProperties* cmdProperties)
{
    PlayerObject*		player	= dynamic_cast<PlayerObject*>(object);
    CraftingSession*	session	= player->getCraftingSession();
    BString				dataStr;
    uint32				mode,counter;

    if(!session)
        return false;

    message->getStringUnicode16(dataStr);

    if(swscanf(dataStr.getUnicode16(),L"%u %u",&counter,&mode) != 2)
    {
        gCraftingSessionFactory->destroySession(player->getCraftingSession());
        return false;
    }

    session->createPrototype(mode,counter);
    return true;
}

//=============================================================================================================================
//
// create manufacture schematic
//

bool CraftingManager::HandleCreateManufactureSchematic(Object* object, Object* target,Message* message, ObjectControllerCmdProperties* cmdProperties)
{
    PlayerObject*		player	= dynamic_cast<PlayerObject*>(object);
    CraftingSession*	session	= player->getCraftingSession();
    BString				dataStr;
    uint32				counter;

    if(!session)
        return false;

    message->getStringUnicode16(dataStr);

    if(swscanf(dataStr.getUnicode16(),L"%u",&counter) != 1)
    {
        gCraftingSessionFactory->destroySession(player->getCraftingSession());
        return false;
    }

    //gLogger->hexDump(message->getData(),message->getSize());
    session->createManufactureSchematic(counter);
    return true;
}

//======================================================================================================================

