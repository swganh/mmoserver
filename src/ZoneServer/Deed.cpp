/*
 This file is part of MMOServer. For more information, visit http://swganh.com
 
 Copyright (c) 2006 - 2011 The SWG:ANH Team

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "Deed.h"
#include "Datapad.h"
#include "Inventory.h"
#include "Item_Enums.h"
#include "nonPersistantObjectFactory.h"
//#include "ObjectFactory.h"
#include "PlayerObject.h"
#include "ScoutManager.h"
#include "StructureManager.h"
#include "VehicleControllerFactory.h"
#include "WorldManager.h"
#include "ContainerManager.h"
#include "ZoneOpcodes.h"

#include "MessageLib/MessageLib.h"

#include "NetworkManager/Message.h"
#include "NetworkManager/MessageFactory.h"

//=============================================================================

Deed::Deed() : Item()
{
}

//=============================================================================

Deed::~Deed()
{
}

//=============================================================================
//handles the radial selection

void Deed::handleObjectMenuSelect(uint8 messageType,Object* srcObject)
{
    if(PlayerObject* player = dynamic_cast<PlayerObject*>(srcObject))
    {
        switch(messageType)
        {
        case radId_itemUse:
        {

            if(this->getItemType() >= ItemType_Deed_X34 && this->getItemType() <= ItemType_Deed_Swoop) //landspeeder x34, speederbike, swoop
            {
                // create the vehicle and put in datapad
                Datapad*		datapad = player->getDataPad();

                if(datapad->getCapacity())
                {
                    	gVehicleControllerFactory->createVehicle(this->getItemType(),player);

						Inventory* inventory = player->getInventory();
						gContainerManager->deleteObject(this,inventory);						
                }
                else
                {
                    gMessageLib->SendSystemMessage(L"Error datapad at max capacity. Couldn't create the vehicle.", player);
                }
            }
            else
            {
                unsigned int itemType = this->getItemType();

                //Is it a city hall?
                if(itemType ==  1566 ||itemType == 1567 || itemType == 1568)
                {
                    if(PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(this->getOwner())))
                    {
                        if(!player->checkSkill(623)) // Must be a novice Politician
                        {
                            gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "place_cityhall"), player);
                            return;
                        }
                    }
                }

                //check the region whether were allowed to build
                if(gStructureManager->checkNoBuildRegion(player))
                {
                    return;
                }

                //enter deed placement mode
                StructureDeedLink* data = gStructureManager->getDeedData(this->getItemType());
                if(!data)
                {
                    return;
                }
                if(player->getParentId())
                {
                    gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "not_inside"), player);
                    return;
                }

                //check available Lots and remove ... grml
                if(!player->useLots(data->requiredLots))
                {
                    gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "not_enough_lots", 0, 0, 0, data->requiredLots, 0.0f), player);
                    return;
                }

                //TODO
                //check for city boundaries

                //check if were allowed to build that structure on this planet

                uint64 zoneId = (uint64)pow(2.0,(int)gWorldManager->getZoneId());
                uint64 mask = data->placementMask;
                if((mask&zoneId) == zoneId)
                {
                    //sadly the client wont inform us when the player hit escape
                    gMessageLib->sendEnterStructurePlacement(this,data->structureObjectString,player);
                }
                else
                {
                    //we cannot differ whether its a no build planet
                    //or just the house type isnt permitted here
                    //wrong_planet
                    //not_permitted
                    gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "wrong_planet", 0, 0, 0, data->requiredLots, 0.0f), player);
                    return;
                }

            }
        }
        break;
        default:
            break;
        }


    }
}

//=============================================================================

void Deed::sendAttributes(PlayerObject* playerObject)
{
    if(!(playerObject->isConnected()))
        return;

    Message* newMessage;

    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opAttributeListMessage);
    gMessageFactory->addUint64(mId);

    gMessageFactory->addUint32(1 + mAttributeMap.size());

    BString	value;

	wchar_t temp[64];
    swprintf(temp,50,L"%u/%u",mMaxCondition - mDamage,mMaxCondition);

    gMessageFactory->addString(BString("condition"));
    gMessageFactory->addString(temp);

    AttributeMap::iterator			mapIt;
    AttributeOrderList::iterator	orderIt = mAttributeOrderList.begin();

    while(orderIt != mAttributeOrderList.end())
    {
        mapIt = mAttributeMap.find(*orderIt);

        gMessageFactory->addString(gWorldManager->getAttributeKey((*mapIt).first));

        value = (*mapIt).second.c_str();
        value.convert(BSTRType_Unicode16);

        gMessageFactory->addString(value);

        ++orderIt;
    }

    newMessage = gMessageFactory->EndMessage();

    (playerObject->getClient())->SendChannelAUnreliable(newMessage, playerObject->getAccountId(), CR_Client, 9);
}

//=============================================================================

void Deed::prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount)
{

    RadialMenu* radial	= new RadialMenu();

    uint8 radId = 1;

    //if we have a prefab Menu (we will have one as a deed)  iterate through it and add it to our response
    //this way we will have our menu item numbering done right

    MenuItemList* menuItemList = 		getMenuList();
    if(menuItemList)
    {
        MenuItemList::iterator it	=	menuItemList->begin();

        while(it != menuItemList->end())
        {
            radId++;

            if((*it)->sIdentifier == 7)
            {
                radial->addItem((*it)->sItem,(*it)->sSubMenu,(RadialIdentifier)(*it)->sIdentifier,radAction_ObjCallback,"");
            }
            else
                radial->addItem((*it)->sItem,(*it)->sSubMenu,(RadialIdentifier)(*it)->sIdentifier,radAction_Default,"");
            it++;
        }
    }


    radial->addItem(radId++,0,radId_itemUse,radAction_ObjCallback,"");
    radial->addItem(radId++,0,radId_examine,radAction_ObjCallback,"");
    radial->addItem(radId++,0,radId_itemDestroy,radAction_ObjCallback,"");
    RadialMenuPtr radialPtr(radial);
    mRadialMenu = radialPtr;
}

//=============================================================================
