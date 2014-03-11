/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2014 The SWG:ANH Team
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

#include "OCCommonHandlers.h"
#include "Zoneserver/Objects/Bank.h"
#include "ZoneServer/Objects/BankTerminal.h"
#include "Zoneserver/Objects/Datapad.h"
#include "Zoneserver/objects/IntangibleObject.h"
#include "Zoneserver/Objects/Inventory.h"
#include "Zoneserver/Objects/Item.h"
#include "ZoneServer/Objects/CraftingTool.h"
#include "ZoneServer/Objects/Object/ObjectFactory.h"
#include "ZoneServer/Objects/Player Object/PlayerObject.h"
#include "ZoneServer/Objects/Shuttle.h"
#include "ZoneServer/Objects/SurveyTool.h"

#include "ZoneServer\Services\equipment\equipment_service.h"

#include "ZoneServer/GameSystemManagers/Structure Manager/CellObject.h"
#include "ZoneServer/GameSystemManagers/Structure Manager/BuildingObject.h"

//#include "ZoneServer/GameSystemManagers/Container Manager/ContainerObjectFactory.h"
#include "ZoneServer/GameSystemManagers/Container Manager/ContainerManager.h"

#include "ZoneServer/GameSystemManagers/Resource Manager/CurrentResource.h"
#include "ZoneServer/GameSystemManagers/Crafting Manager/CraftingSession.h"
#include "ZoneServer/GameSystemManagers/Crafting Manager/ManufacturingSchematic.h"
#include "ZoneServer/GameSystemManagers/Mission Manager/MissionObject.h"
#include "ZoneServer/GameSystemManagers/NPC Manager/NPCObject.h"
#include "Zoneserver/ObjectController/ObjectController.h"
#include "ZoneServer/ObjectController/ObjectControllerCommandMap.h"
#include "ZoneServer/ObjectController/ObjectControllerOpcodes.h"
#include "ZoneServer/GameSystemManagers/Resource Manager/ResourceContainer.h"
#include "ZoneServer/GameSystemManagers/Resource Manager/ResourceManager.h"

//#include "ZoneServer/ProfessionManagers/Artisan Manager/SurveyEvent.h"
//#include "ZoneServer/ProfessionManagers/Artisan Manager/SampleEvent.h"
#include "ZoneServer/GameSystemManagers/Event Manager/LogOutEvent.h"
//#include "ZoneServer/Objects/ItemDeleteEvent.h"
//#include "ZoneServer/ProfessionManagers/Medic Manager/InjuryTreatmentEvent.h"
//#include "ZoneServer/ProfessionManagers/Medic Manager/QuickHealInjuryTreatmentEvent.h"
//#include "ZoneServer/ProfessionManagers/Medic Manager/WoundTreatmentEvent.h"


#include "ZoneServer/GameSystemManagers/Travel Manager/TravelMapHandler.h"
#include "ZoneServer/GameSystemManagers/Travel Manager/TravelTerminal.h"
#include "ZoneServer/GameSystemManagers/Treasury Manager/TreasuryManager.h"
#include "ZoneServer/Tutorial.h"
#include "ZoneServer/GameSystemManagers/UI Manager/UIManager.h"
//#include "ZoneServer/Objects/wearable.h"
#include "ZoneServer/WorldConfig.h"
#include "ZoneServer/WorldManager.h"
#include "ZoneServer/ZoneOpcodes.h"
#include "anh/Utils/clock.h"

#include "MessageLib/MessageLib.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/DatabaseResult.h"
#include "Common/EventDispatcher.h"
#include "NetworkManager/Message.h"
#include "NetworkManager/MessageFactory.h"

#include "ZoneServer/GameSystemManagers/State Manager/StateManager.h"
#include "ZoneServer/GameSystemManagers/State Manager/LocomotionState.h"
#include "SwgProtocol/BurstRunEvents.h"
#include "SwgProtocol/ObjectControllerEvents.h"

#include <cassert>

using ::common::IEventPtr;
using ::common::OutOfBand;
using ::swg_protocol::BurstRunEndEvent;
using ::swg_protocol::BurstRunCooldownEndEvent;
using ::swg_protocol::object_controller::PreCommandExecuteEvent;

//=============================================================================

void ObjectController::_handleBoardTransport(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    CreatureObject* creature  = dynamic_cast<CreatureObject*>(mObject);
	PlayerObject* playerObject = creature->GetGhost();

    ObjectSet		inRangeObjects;
    float			boardingRange	= 25.0;

    if(playerObject->GetCreature()->GetPosture() == CreaturePosture_SkillAnimating)
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("error_message", "wrong_state"), playerObject);
        return;
    }

    BString str;
    message->getStringUnicode16(str);
    str.convert(BSTRType_ANSI);
    str.toLower();

    if((str.getCrc() != BString("transport").getCrc()))
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("travel", "boarding_what_shuttle"), playerObject);
        return;
    }

    gSpatialIndexManager->getObjectsInRange(playerObject,&inRangeObjects,ObjType_Creature | ObjType_NPC, boardingRange, true);

	// iterate through the results
	ObjectSet::iterator it = inRangeObjects.begin();

	while(it != inRangeObjects.end())
	{
        if(Shuttle* shuttle = dynamic_cast<Shuttle*>(*it))
        {
            // in range check
            if(playerObject->getParentId() !=  shuttle->getParentId())
            {
                gMessageLib->SendSystemMessage(::common::OutOfBand("travel", "boarding_too_far"), playerObject);
                return;
            }

            if (!shuttle->availableInPort())
            {
                gMessageLib->SendSystemMessage(::common::OutOfBand("travel", "shuttle_not_available"), playerObject);
                return;
            }

            shuttle->useShuttle(playerObject);

            return;
        }

        ++it;
    }

    gMessageLib->SendSystemMessage(::common::OutOfBand("structure/structure_messages", "boarding_what_shuttle"), playerObject);
}

//=============================================================================
//
// open container
//
void ObjectController::_handleOpenContainer(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{

	CreatureObject* creature  = dynamic_cast<CreatureObject*>(mObject);
	PlayerObject* playerObject = creature->GetGhost();

	Object*			itemObject		= gWorldManager->getObjectById(targetId);

	if (itemObject)
	{
		if(glm::distance(playerObject->getWorldPosition(), itemObject->getWorldPosition()) > 10)
		{
			gMessageLib->SendSystemMessage(std::u16string(), playerObject, "system_msg", "out_of_range");
			return;
		}

		bool aContainer = false;

		if (gWorldConfig->isTutorial())
		{
			playerObject->getTutorial()->containerOpen(targetId);
		}

		if (itemObject->getType() == ObjType_Tangible)
		{
			TangibleObject* tangObj = dynamic_cast<TangibleObject*>(itemObject);
			
			//this might be a backpack
			//or a chest - it needs to have a capacity to be a container!
			if (tangObj->getCapacity())
			{
				//checkContainingContainer checks the permission
				auto permission = tangObj->GetPermissions();
				
				/*
				if(gContainerManager->checkContainingContainer(tangObj->getId(),playerObject->getId()))
				{
					aContainer = true;
					
					//register the player to the container and create the content
					gContainerManager->registerPlayerToContainer(tangObj,playerObject);
				}
				*/
			}
		}
		//its not a Container* Object however in theory it still can be a backpack for example
		if (!aContainer)
		{
			// STF: container_error_message Key: container8 does not seem to be working, using this custom string temperary.
			gMessageLib->SendSystemMessage(L"You do not have permission to access that container.", playerObject);
		}
		else
		{
            gMessageLib->sendOpenedContainer(targetId, playerObject);
        }
    }
    else
    {
        DLOG(info) <<  "ObjectController::_handleOpenContainer: INVALID Object id " << targetId;
    }
}

void ObjectController::_handleCloseContainer(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    CreatureObject* creature  = dynamic_cast<CreatureObject*>(mObject);
	PlayerObject* playerObject = creature->GetGhost();

    if (gWorldConfig->isTutorial())
    {
        playerObject->getTutorial()->containerClose(targetId);
    }
}


//=============================================================================
//
// transfer item
//

void ObjectController::_handleTransferItem(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	_handleTransferItemMisc(targetId, message, cmdProperties);
}



//	Transfer items between player inventories, containers and cells. Also handles transfer from  creature inventories (looting).
//	Also handles trandfer between player inventory and player (equipping items).
//	We don't handle transfer to the world outside.
//

void ObjectController::_handleTransferItemMisc(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	//we need to make sure that ONLY equipped items are contained by the player
	//all other items are contained by the inventory!!!!!!!!

	CreatureObject* creature  = dynamic_cast<CreatureObject*>(mObject);
	PlayerObject* playerObject = creature->GetGhost();

	Object*			itemObject		=	gWorldManager->getObjectById(targetId);
	//Inventory*		inventory		=	dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));

	BString			dataStr;
	uint64			targetContainerId;
	int32			linkType;
	float			x,y,z;

	message->getStringUnicode16(dataStr);



	if(swscanf(dataStr.getUnicode16(),L"%"WidePRIu64 L" %u %f %f %f",&targetContainerId,&linkType,&x,&y,&z) != 5)
	{
		DLOG(info) << "ObjController::_handleTransferItemMisc: Error in parameters";
		return;
	}

	if (!itemObject)
	{
		DLOG(warning) << "ObjController::_handleTransferItemMisc: No Object to transfer :(";
		return;
	}

	TangibleObject* tangible = dynamic_cast<TangibleObject*>(itemObject);
	if(!tangible)
	{
		//no tagible - get out of here
		DLOG(warning) << "ObjController::_handleTransferItemMisc: No tangible to transfer :(";
		return;
	}

	
	DLOG(info) << "ObjController::_handleTransferItemMisc: parameters";
	DLOG(info) << "ObjController::_handleTransferItemMisc: newcontainer : " << targetContainerId;
	DLOG(info) << "ObjController::_handleTransferItemMisc: oldcontainer : " << tangible->getParentId();
	DLOG(info) << "ObjController::_handleTransferItemMisc: linktype : " << linkType;

	// We may want to transfer other things than items...basically tangibleObjects!
	// resourcecontainers / factory crates
	
	// first check whether its an instrument with persistant copy   - thats a special case!
	
	Item* item = dynamic_cast<Item*>(itemObject);
	if (item)
	{
		//check if its only temporarily placed
		if(item->getItemFamily() == ItemFamily_Instrument)
		{
			if(item->getPersistantCopy())
			{
				// gMessageLib->sendSystemMessage(playerObject,L"you cannot pick this up");
				// You bet, I can! Remove the temp instrument from the world.

				// Do I have access to this instrument?
				if (item->getOwner() == playerObject->getId())
				{

					playerObject->getController()->destroyObject(targetId);
				}
				return;
			}
		}
	}
	

	// A FYI: When we drop items, we use player pos.
    itemObject->mPosition = glm::vec3(x,y,z);

	if (!targetContainerId)
	{
		DLOG(info) << "ObjController::_handleTransferItemMisc  TargetContainer is NULL :(";
		//return;

	}
	
	//ok how to tackle this ... :
	//basically I want to use ObjectContainer as standard access point for item handling!
	//so far we have different accesses for Objects on the player and for the inventory	and for ContainerObjects and for cells ...

	//
	

	
	gContainerManager->transferItem(targetContainerId, targetId, playerObject, linkType);
}


//=============================================================================
//
// purchase ticket
//

void ObjectController::_handlePurchaseTicket(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    CreatureObject* creature  = dynamic_cast<CreatureObject*>(mObject);
	PlayerObject* playerObject = creature->GetGhost();

    BString			dataStr;
    BStringVector	dataElements;
    uint16			elements;

	auto equip_service = gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::equipment::EquipmentService>("EquipmentService");
	auto inventory = dynamic_cast<Inventory*>(equip_service->GetEquippedObject(playerObject->GetCreature(), "inventory"));

    float		purchaseRange = gWorldConfig->getConfiguration<float>("Player_TicketTerminalAccess_Distance",(float)10.0);

    if(playerObject->GetCreature()->GetPosture() == CreaturePosture_SkillAnimating)
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("error_message", "wrong_state"), playerObject);
        return;
    }


    //however we are able to use the purchaseticket command in starports
    //without having to use a ticketvendor by just giving commandline parameters
    //when we are *near* a ticket vendor

    TravelTerminal* terminal = dynamic_cast<TravelTerminal*> (gWorldManager->getNearestTerminal(playerObject,TanType_TravelTerminal));
    // iterate through the results

    if((!terminal)|| (glm::distance(terminal->mPosition, playerObject->mPosition) > purchaseRange))
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("travel", "too_far"), playerObject);
        return;
    }

    playerObject->setTravelPoint(terminal);

    message->getStringUnicode16(dataStr);

    // Have to convert BEFORE using split, since the conversion done there is removed It will assert().. evil grin...
    // Either do the conversion HERE, or better fix the split so it handles unicoe also.
    dataStr.convert(BSTRType_ANSI);

    elements = dataStr.split(dataElements,' ');

    if(elements < 4)
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("travel", "route_not_available"), playerObject);
        return;
    }

    // get price and planet ids
    TicketProperties ticketProperties;
    gTravelMapHandler->getTicketInformation(dataElements,&ticketProperties);

    if(!ticketProperties.dstPoint)
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("travel", "route_not_available"), playerObject);
        return;
    }

    uint8 roundTrip = 0;

    if(elements > 4)
        roundTrip = atoi(dataElements[4].getAnsi());

    if(dataElements[4].getCrc() == BString("single").getCrc())
        roundTrip = 0;


    //how many tickets will it be?
    uint32 amount = 1;
    if(roundTrip)
        amount = 2;


    if(!inventory->checkSlots(static_cast<uint8>(amount)))
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("error_message", "inv_full"), playerObject);
        return;
    }

    if(roundTrip == 1)
    {
        ticketProperties.price *= 2;
    }

    // update bank or inventory credits
	if(!playerObject->updateCredits(-ticketProperties.price))    {
        gUIManager->createNewMessageBox(NULL,"ticketPurchaseFailed","The Galactic Travel Commission","You do not have enough money to complete the ticket purchase.",playerObject);
        return;
    }
    
    if(playerObject->isConnected())
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("base_player", "prose_pay_acct_success", "", "", "", "", "money/acct_n", "travelsystem", ticketProperties.price), playerObject);

        gObjectFactory->requestNewTravelTicket(inventory,ticketProperties,inventory->getId(),99);

        if(roundTrip == 1)
        {
            uint32 tmpId = ticketProperties.srcPlanetId;
            TravelPoint* tmpPoint = ticketProperties.srcPoint;

            ticketProperties.srcPlanetId = ticketProperties.dstPlanetId;
            ticketProperties.srcPoint = ticketProperties.dstPoint;
            ticketProperties.dstPlanetId = static_cast<uint16>(tmpId);
            ticketProperties.dstPoint = tmpPoint;

            gObjectFactory->requestNewTravelTicket(inventory,ticketProperties,inventory->getId(),99);
        }

        gUIManager->createNewMessageBox(NULL,"handleSUI","The Galactic Travel Commission","@travel:ticket_purchase_complete",playerObject);
    }
}

//======================================================================================================================
//
// get attributes batch
//

void ObjectController::_handleGetAttributesBatch(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	CreatureObject* creature  = dynamic_cast<CreatureObject*>(mObject);
	PlayerObject* playerObject = creature->GetGhost();

    BString			requestStr;
    BStringVector	dataElements;
    BStringVector	dataElements2;
    uint16			elementCount;

	auto equip_service = gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::equipment::EquipmentService>("EquipmentService");
	auto inventory = dynamic_cast<Inventory*>(equip_service->GetEquippedObject(playerObject->GetCreature(), "inventory"));

	if(!inventory)	{
		LOG(error) << "ObjectController::_handleGetAttributesBatch couldnt cast inventory";
	}

    message->getStringUnicode16(requestStr);
    requestStr.convert(BSTRType_ANSI);
    requestStr.getRawData()[requestStr.getLength()] = 0;

    elementCount = requestStr.split(dataElements,' ');

    if(!elementCount)
    {
        return;
    }

    Message* newMessage;

    for(uint16 i = 0; i < elementCount; i++)
    {

        uint64 itemId	= boost::lexical_cast<uint64>(dataElements[i].getAnsi());
        Object* object	= gWorldManager->getObjectById(itemId);

        if(object == NULL)
        {
            // could be a resource
            Resource* resource = gResourceManager->getResourceById(itemId);

            if(resource != NULL)
            {
                resource->sendAttributes(playerObject);
                continue;
            }

            //could be a schematic!
            Datapad* datapad			= playerObject->getDataPad();
            ManufacturingSchematic* schem	= datapad->getManufacturingSchematicById(itemId);

            if(schem != NULL)
            {
                schem->sendAttributes(playerObject);
                continue;
            }

            MissionObject* mission			= datapad->getMissionById(itemId);
            if(mission != NULL)
            {
                mission->sendAttributes(playerObject);
                continue;
            }

            IntangibleObject* data = datapad->getDataById(itemId);
            if(data != NULL)
            {
                data->sendAttributes(playerObject);
                continue;
            }

            // TODO: check our datapad items
            if(playerObject->isConnected())
            {
                // default reply for schematics
                gMessageFactory->StartMessage();
                gMessageFactory->addUint32(opAttributeListMessage);
                gMessageFactory->addUint64(itemId);
                gMessageFactory->addUint32(0);
                //gMessageFactory->addUint16(0);
                //gMessageFactory->addUint32(40);

                newMessage = gMessageFactory->EndMessage();

                (playerObject->getClient())->SendChannelAUnreliable(newMessage, playerObject->getAccountId(),  CR_Client, 8);
            }

            //finally, when we are crafting this could be the new item, not yet added to the worldmanager??
            if(playerObject->getCraftingSession())
            {
                if(playerObject->getCraftingSession()->getItem()&&playerObject->getCraftingSession()->getItem()->getId() == itemId)
                {
                    playerObject->getCraftingSession()->getItem()->sendAttributes(playerObject);
                }
            }
        }
        else
        {
            // Tutorial: I (Eru) have to do some hacks here, since I don't know how to get the info of what object the client has selected (by single click) in the Inventory.
            if (gWorldConfig->isTutorial())
            {
                // Let's see if the actual object is the food item "Melon" in our inventory.
                if (inventory->getId() == object->getParentId())
                {
                    //uint64 id = object->getId();

                    // Is it an Item?
                    Item* item = dynamic_cast<Item*>(object);

                    // Check if this item is a food item.
                    if (item)
                    {
                        if (item->getItemFamily() == ItemFamily_Foods)
                        {
                            //playerObject->getTutorial()->tutorialResponse("foodSelected");
                        }
                    }
                }
            }

            object->sendAttributes(playerObject);
        }
    }
}

//=============================================================================
//
// quests
//

void ObjectController::_handleRequestQuestTimersAndCounters(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{

}

//=============================================================================
//
// target
//

void ObjectController::_handleTarget(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    //PlayerObject* playerObject = (PlayerObject*)mObject;
}
//======================================================================================================================
//
// end burst run
//

void ObjectController::_endBurstRun(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    // set locomotion
    PlayerObject* playerObject = (PlayerObject*)mObject;
    playerObject->GetCreature()->states.setLocomotion(CreatureLocomotion_Standing);
}

//======================================================================================================================
//
// surrender skill
//

void ObjectController::_handleSurrenderSkill(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    CreatureObject* creature  = dynamic_cast<CreatureObject*>(mObject);
	PlayerObject* player = creature->GetGhost();

    BString			skillStr;

    message->getStringUnicode16(skillStr);
    skillStr.convert(BSTRType_ANSI);

    if(!(skillStr.getLength()))
    {
        DLOG(info) << "ObjectController::handleSurrenderSkill: no skillname";
        return;
    }

    Skill* skill = gSkillManager->getSkillByName(skillStr.getAnsi());

    if(skill == NULL)
    {
        DLOG(info)<<"ObjectController::handleSurrenderSkill: could not find skill " << skillStr.getAnsi();
        return;
    }

    gSkillManager->dropSkill(skill->mId,player->GetCreature());
}

//======================================================================================================================
//
// qualified for skill
//

void ObjectController::_handleClientQualifiedForSkill(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    //gLogger->hexDump(message->getData(),message->getSize());
}


//=============================================================================================================================
//
// radial request
//

void ObjectController::handleObjectMenuRequest(Message* message)
{
    //for every item where we request a radial the client starts by displaying a radial on his own and additionally sends a
    //objectMenuRequest to the server
    //The server then either just resends the radial as send by the client or adds / modifies options on his own
    //this is why sometimes when lag is involved it takes some time for all options to display


    CreatureObject* creature  = dynamic_cast<CreatureObject*>(mObject);
	PlayerObject* playerObject = creature->GetGhost();

    message->getUint32(); // unknown
    uint64 requestedObjectId = message->getUint64();
    message->getUint64(); // player id again ?

    Object* requestedObject = gWorldManager->getObjectById(requestedObjectId);

    uint32 itemCount = message->getUint32();

    BString extendedDescription;
    MenuItemList menuItemList;

    MenuItem* menuItem;
    for(uint32 i = 0; i < itemCount; i++)
    {
        menuItem = new(MenuItem);

        menuItem->sItem			= message->getUint8();   // item nr
        menuItem->sSubMenu		= message->getUint8();   // submenu flag
        menuItem->sIdentifier	= message->getUint8();   // item identifier
        menuItem->sOption		= message->getUint8();   // extended option
        message->getStringUnicode16(extendedDescription);
        menuItemList.push_back(menuItem);
    }

    uint8 responseNr = message->getUint8();

	//actually we should never get here
    if(!requestedObject)
    {
        if(playerObject->isConnected())	{
            gMessageLib->sendEmptyObjectMenuResponse(requestedObjectId,playerObject,responseNr,menuItemList);
			
			MenuItemList::iterator it = menuItemList.begin();
			while(it != menuItemList.end())
			{
				delete (*it);
				it++;
			}
		}

        
        return;
    }

	//this is complete bullshit menuItemList is gone once this functions destructor is called
    requestedObject->setMenuList(&menuItemList);



    //are we an item dropped in a structure awaiting to be moved or picked u`p?
    //just implement this virtual function for items as we need just one central point instead
    //of the same code over and over for all items

    CellObject* itemCell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(requestedObject->getParentId()));

    Item* item = dynamic_cast<Item*>(requestedObject);
    ResourceContainer* rC = dynamic_cast<ResourceContainer*>(requestedObject);
    TangibleObject* tO = dynamic_cast<TangibleObject*>(requestedObject);

    //only display that menu when *we* and the item are in the same structure
    if((rC || item) && itemCell && (!tO->getStatic()))
    {
        CellObject* playerCell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(playerObject->getParentId()));
        if(playerCell && (playerCell->getParentId() == itemCell->getParentId()))
        {
            PlayerStructure* pS = dynamic_cast<PlayerStructure*>(gWorldManager->getObjectById(playerCell->getParentId()));
            if(pS)
                requestedObject->prepareCustomRadialMenuInCell(playerObject->GetCreature(),static_cast<uint8>(itemCount));
        }
    }
    /*
    if(rc && requestedObject->getParentId())
    {
        if(CellObject* cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(requestedObject->getParentId())))
        {
            requestedObject->prepareCustomRadialMenuInCell(playerObject,static_cast<uint8>(itemCount));
        }
    }
    */
    //delete the radials after every use or provide every object with set rules when to delete it ?

    if(!requestedObject->getRadialMenu())
        requestedObject->prepareCustomRadialMenu(playerObject->GetCreature(),static_cast<uint8>(itemCount));

    if (requestedObject->getRadialMenu())
    {
        if(playerObject->isConnected())
        {
            gMessageLib->sendObjectMenuResponse(requestedObject,playerObject,responseNr);
        }
        //they only reset if the objects virtual function does it
        //by default it stays
        requestedObject->ResetRadialMenu();
        //the radial menu is supposed to be an intelligent pointer deleting itself when no reference is left
        //however during runtime the item always references the radialmenu that was generated for it on the first call.
        //when the circumstances of the item change we need to delete the pointer and thus force it to generate a new radial
    }
    else
    {
        // putting this for static objects/objects that are not known by the server yet
        // send a default menu,so client stops flooding us with requests

        //empty might just mean that the clients radial is sufficient

        if(playerObject->isConnected())
            gMessageLib->sendEmptyObjectMenuResponse(requestedObjectId,playerObject,responseNr,menuItemList);

        //the list is cleared and items are destroyes in the message lib
        //for the default response
    }


    //we need to clear that if the messagelib wont clear it
    //still want to use it for the player radials at some point
    for(MenuItemList::iterator it=menuItemList.begin(); it != menuItemList.end(); it++)
        delete (*it);

    menuItemList.clear();

}

//=============================================================================================================================
//this code doesnt make any sense ... a container* Object is the container like we see in the tutorial. It inherits tangibleObject
//so by definition it cannot be a resourcecontainer or any other tangible despite a container!

// This is called when trying to retrieve a resource from a hopper
void ObjectController::handleObjectReady(Object* object,DispatchClient* client)
{
	PlayerObject* player = gWorldManager->getPlayerByAccId(client->getAccountId());
	

	gSpatialIndexManager->createInWorld(object);
}

//======================================================================================================================

void ObjectController::_handleNewbieSelectStartingLocation(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    CreatureObject* creature  = dynamic_cast<CreatureObject*>(mObject);
	PlayerObject* player = creature->GetGhost();

    // gLogger->hexDump(message->getData(),message->getSize());

    // Find the planet and position.
    if (gWorldConfig->isTutorial())
    {
        BString name;
        message->getStringUnicode16(name);

        if (!(name.getLength()))
        {
            return;
        }
        name.convert(BSTRType_ANSI);
        player->getTutorial()->warpToStartingLocation(name);
    }
}




//======================================================================================================================
//
//handles the logout via /logout command
//
void ObjectController::_handleClientLogout(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    
    CreatureObject* creature  = dynamic_cast<CreatureObject*>(mObject);
	PlayerObject* player = creature->GetGhost();

    //make sure we cannot use the /logout multiple times
    //as this will invalidate our disconnect lists
    if(player->checkPlayerCustomFlag(PlayerCustomFlag_LogOut))
    {

        return;
    }

    player->togglePlayerCustomFlagOn(PlayerCustomFlag_LogOut);

    uint32 logout		= gWorldConfig->getConfiguration<uint32>("Player_LogOut_Time",(uint32)30);
    uint32 logoutSpacer = gWorldConfig->getConfiguration<uint32>("Player_LogOut_Spacer",(uint32)5);

    if(logoutSpacer > logout)
        logoutSpacer = logout;

    if(logoutSpacer < 1)
        logoutSpacer = 1;

    if(logout < logoutSpacer)
        logout = logoutSpacer;

    if(logout > 300)
        logout = 300;

    // schedule execution
    addEvent(new LogOutEvent(Anh_Utils::Clock::getSingleton()->getLocalTime()+((logout-logoutSpacer)*1000),logoutSpacer*1000),logoutSpacer*1000);
    gMessageLib->SendSystemMessage(::common::OutOfBand("logout", "time_left", 0, 0, 0, logout), player);

}


//======================================================================================================================
//
// start burst run
//

bool HandleBurstRun(Object* object, Object* target, Message* message, ObjectControllerCmdProperties* cmd_properties) {
    PlayerObject* player = dynamic_cast<PlayerObject*>(object);
    if (!player) {
        return false;
    }

    //can we burst run right now ??
    if(player->checkPlayerCustomFlag(PlayerCustomFlag_BurstRun)) {
        gMessageLib->SendSystemMessage(L"You are already running as hard as you can.", player);
        return false;
    }

    if(player->checkPlayerCustomFlag(PlayerCustomFlag_BurstRunCD)) {
        gMessageLib->SendSystemMessage(OutOfBand("combat_effects", "burst_run_wait"), player);
        return false;
    }

    // Create a pre-command processing event.
    auto pre_execute_event = std::make_shared<PreCommandExecuteEvent>(object->getId());
    pre_execute_event->target_id(0); // This command never has a target.
    pre_execute_event->command_crc(cmd_properties->mCmdCrc);

    // Trigger a pre-command execute event and get the result. This allows
    // any listeners a last chance to veto the processing of the command.
    if (!gEventDispatcher.Deliver(pre_execute_event).get()) {
        return false;
    }



    // Update the player's speed modifier.
    player->GetCreature()->setCurrentSpeedModifier(player->GetCreature()->getCurrentSpeedModifier()*2);
    gMessageLib->sendUpdateMovementProperties(player);

    // Update the player's locomotion to a running state.
    player->GetCreature()->states.setLocomotion(CreatureLocomotion_Running);
    // Toggle the flags for the burst run effect and the cool-down timer on the corresponding command.
    player->togglePlayerCustomFlagOn(PlayerCustomFlag_BurstRunCD);
    player->togglePlayerCustomFlagOn(PlayerCustomFlag_BurstRun);

    //Send the burst run system message to the player
    gMessageLib->SendSystemMessage(L"You run as hard as you can!", player);

    //Now send the burst run combat spam message to InRange
    gMessageLib->sendCombatSpam(player, player, 0, "cbt_spam", "burstrun_start");

    // Duration of the burst run effect in seconds.
    uint32_t effect_duration_sec = gWorldConfig->getConfiguration<uint32_t>("Player_BurstRun_Time", 60);

    // Create a delayed event for the end of the burst run and attach a custom
    // callback to be executed 60 seconds after being triggered.
    auto burst_end_event = std::make_shared<BurstRunEndEvent>(object->getId(), effect_duration_sec * 1000, [player] {
        // Make sure the target for the event is still valid and that their burst run flag is still set.
        if(player && player->checkPlayerCustomFlag(PlayerCustomFlag_BurstRun)) {
            // Return the player to normal movement.
            player->GetCreature()->setCurrentSpeedModifier(player->GetCreature()->getBaseSpeedModifier());
            gMessageLib->sendUpdateMovementProperties(player);

            // Update the player's locomotion to a walking state.
            player->GetCreature()->states.setLocomotion(CreatureLocomotion_Walking);
            
            // Remove the burst run flag.
            player->togglePlayerCustomFlagOff(PlayerCustomFlag_BurstRun);

            // Alert the player the burst run has ended and that they are now tired.
            gMessageLib->SendSystemMessage(OutOfBand("cbt_spam", "burstrun_stop_single"), player);
            gMessageLib->sendCombatSpam(player, player, 0, "cbt_spam", "burstrun_stop");
            gMessageLib->SendSystemMessage(OutOfBand("combat_effects", "burst_run_tired"), player);
        }
    });

    // Create a delayed event for the end of the burst run cool-down timer and attach
    // a custom callback to be executed 6 minutes after being triggered.
    auto burst_cooldown_end_event = std::make_shared<BurstRunCooldownEndEvent>(object->getId(), static_cast<uint64_t>(cmd_properties->mDelayMultiplier) * 1000, [player] {
        // Make sure the target for the event is still valid and that their burst run cool-down flag is still set.
        if(player && player->checkPlayerCustomFlag(PlayerCustomFlag_BurstRunCD)) {
            // Turn off the burst run cool-down flag and alert the player.
            player->togglePlayerCustomFlagOff(PlayerCustomFlag_BurstRunCD);
            gMessageLib->SendSystemMessage(OutOfBand("combat_effects", "burst_run_not_tired"), player);
        }
    });

    // Chain the two events together so that they are called in the appropriate order.
    burst_end_event->next(burst_cooldown_end_event);

    // Notify any curious listeners of the event.
    gEventDispatcher.Notify(burst_end_event);

    return true;
}
