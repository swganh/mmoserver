/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "Deed.h"
#include "Datapad.h"
#include "Inventory.h"
#include "Item_Enums.h"
#include "nonPersistantObjectFactory.h"
#include "ObjectFactory.h"
#include "PlayerObject.h"
#include "ScoutManager.h"
#include "StructureManager.h"
#include "VehicleFactory.h"
#include "WorldManager.h"
#include "ZoneOpcodes.h"

#include "MessageLib/MessageLib.h"

#include "Common/Message.h"
#include "Common/MessageFactory.h"

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
					// create the VehicleController and put in datapad
					Datapad*		datapad = dynamic_cast<Datapad*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Datapad));

					if(datapad->getCapacity())
					{
						gVehicleControllerFactory->createVehicleController(this->getItemType(),player);

						Inventory* inventory = dynamic_cast<Inventory*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
						inventory->removeObject(this);
						gMessageLib->sendDestroyObject(this->getId(),player);
						gObjectFactory->deleteObjectFromDB(this);
						gWorldManager->destroyObject(this);
					}
					else
					{
						gMessageLib->sendSystemMessage(player,L"Error datapad at max capacity. Couldn't create the VehicleController.");
					}
				}
				else
				{
					//todo : check if the type of building is allowed on the planet

					//check the region whether were allowed to build
					if(!gStructureManager->checkCityRadius(player))
					{
						gMessageLib->sendSystemMessage(player,L"You cannot place this structure inside a no-build zone.");
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
						gMessageLib->sendSystemMessage(player,L"","player_structure","not_inside");
						return;
					}

					//check available Lots and remove ... grml
					if(!player->useLots(data->requiredLots))
					{
						gMessageLib->sendSystemMessage(player, L"","player_structure","not_enough_lots","","",L"",data->requiredLots);
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
						gStructureManager->UpdateCharacterLots(player->getId());
					}
					else
					{
						//we cannot differ whether its a no build planet
						//or just the house type isnt permitted here
						//wrong_planet 
						//not_permitted
						gMessageLib->sendSystemMessage(player, L"","player_structure","wrong_planet","","",L"",data->requiredLots);
						gStructureManager->UpdateCharacterLots(player->getId());
						return;
					}

				}
			}
			break;
			default: break;
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

	string	tmpValueStr = string(BSTRType_Unicode16,64);
	string	value;

	tmpValueStr.setLength(swprintf(tmpValueStr.getUnicode16(),50,L"%u/%u",mMaxCondition - mDamage,mMaxCondition));

	gMessageFactory->addString(BString("condition"));
	gMessageFactory->addString(tmpValueStr);

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

			//gLogger->logMsgF("menu item id : %u",MSG_HIGH,(*it)->sIdentifier);
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