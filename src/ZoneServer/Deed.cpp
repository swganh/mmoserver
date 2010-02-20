/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "Deed.h"
#include "Camp.h"
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
#include "MathLib/Quaternion.h"
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
					Datapad*		datapad = dynamic_cast<Datapad*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Datapad));

					if(datapad->getCapacity())
					{
						gVehicleFactory->createVehicle(this->getItemType(),player);

						Inventory* inventory = dynamic_cast<Inventory*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
						inventory->removeObject(this);
						gMessageLib->sendDestroyObject(this->getId(),player);
						gObjectFactory->deleteObjectFromDB(this);
						gWorldManager->destroyObject(this);
					}
					else
					{
						gMessageLib->sendSystemMessage(player,L"Error datapad at max capacity. Couldn't create the vehicle.");
					}
				}
				else
				{
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

					//check available Lots
					if(!player->useLots(data->requiredLots))
					{
						gMessageLib->sendSystemMessage(player, L"","player_structure","not_enough_lots");
						return;
					}

					//TODO
					//check for city boundaries
					player->togglePlayerCustomFlagOn(PlayerCustomFlag_StructurePlacement);	

					gMessageLib->sendEnterStructurePlacement(this,data->structureObjectString,player);

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

	radial->addItem(1,0,radId_itemUse,radAction_ObjCallback,"");
	radial->addItem(2,0,radId_examine,radAction_ObjCallback,"");
	radial->addItem(3,0,radId_itemDestroy,radAction_ObjCallback,"");
	RadialMenuPtr radialPtr(radial);
	mRadialMenu = radialPtr;
}

//=============================================================================