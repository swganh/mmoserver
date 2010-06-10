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

#include "Instrument.h"
#include "CellObject.h"
#include "Inventory.h"
#include "PlayerObject.h"
#include "WorldManager.h"
#include "WorldConfig.h"
#include "ZoneOpcodes.h"
#include "MessageLib/MessageLib.h"
#include "Common/Message.h"
#include "Common/MessageFactory.h"

//=============================================================================

Instrument::Instrument() : Item()
{
}

//=============================================================================

Instrument::~Instrument()
{
}

//=============================================================================
//handles the radial selection

void Instrument::handleObjectMenuSelect(uint8 messageType,Object* srcObject)
{
	if(PlayerObject* player = dynamic_cast<PlayerObject*>(srcObject))
	{
		switch(messageType)
		{
			case radId_itemUse:
			{
				if (player->getPlacedInstrumentId() == this->getId())
				{
					float range = gWorldConfig->getConfiguration<float>("Zone_Player_ItemUse",(float)6.0);
					if (!gWorldManager->objectsInRange(player->getId(), this->getId(), range))
					{
						// We where out of range. (using 6.0 m as default range,this value not verified).
						// TODO: Find the proper error-message, the one below is a "made up".
						gMessageLib->sendSystemMessage(player,L"","system_msg","out_of_range");
						return;
					}

					if ((player->getId() == this->getOwner()) && this->getPlaced())
					{
						if ((player->getPerformingState() == PlayerPerformance_Music))
						{
							gEntertainerManager->stopEntertaining(player);
						}
						else
						{
							// Start to play the copy.
							gEntertainerManager->usePlacedInstrument(player,this);
						}
					}
					else
					{
						// Create a new copy of the instrument.
						// gEntertainerManager->useInstrument(player,this);
					}
				}
				else
				{
					// We are handling the original instrument.
					Inventory* inventory = dynamic_cast<Inventory*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
					if (inventory)
					{
						if (inventory->getId() == this->getParentId())
						{
							if (player->getPlacedInstrumentId() == 0)
							{
								// Create a new copy of the instrument.
								gEntertainerManager->useInstrument(player,this);
							}
							else
							{
								gMessageLib->sendSystemMessage(player,L"", "error_message", "wrong_state");
							}
						}
						else if (dynamic_cast<CellObject*>(gWorldManager->getObjectById(this->getParentId())))
						{
							// Is this my instrument?
							if (this->getOwner() == player->getId())
							{
								if (!gWorldManager->objectsInRange(player->getId(), this->getId(), 6.0))
								{
									// We where out of range. (using 6.0 m as default range,this value not verified).
									// TODO: Find the proper error-message, the one below is a "made up".
									gMessageLib->sendSystemMessage(player,L"","system_msg","out_of_range");
									return;
								}

								if ((player->getPerformingState() == PlayerPerformance_Music))
								{
									gEntertainerManager->stopEntertaining(player);
								}
								else
								{
									// Start to play the original.
									gEntertainerManager->usePlacedInstrument(player,this);
								}
							}
						}
					}
				}
			}
			break;

			default:
			{
			}
			break;
		}
	}
}

//=============================================================================

void Instrument::sendAttributes(PlayerObject* playerObject)
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

	//gMessageFactory->addUint32(0xffffffff);

	newMessage = gMessageFactory->EndMessage();

	(playerObject->getClient())->SendChannelAUnreliable(newMessage, playerObject->getAccountId(), CR_Client, 9);
}

//=============================================================================

void Instrument::prepareCustomRadialMenu(CreatureObject* player, uint8 itemCount)
{

	// NOTE: player is also of type CreatureObject* !!!
	PlayerObject* playerObject = dynamic_cast<PlayerObject*>(player);

	mRadialMenu.reset();
	mRadialMenu = RadialMenuPtr(new RadialMenu());

	// RadialMenu* radial	= new RadialMenu();

	//string mInstrumentString = instrument->getName();
	uint32 instrumentNr = this->getItemType();

	if ((instrumentNr == ItemType_Nalargon) || (instrumentNr == ItemType_omni_box) || (instrumentNr == ItemType_nalargon_max_reebo))
	{

		uint32 radId = 1;
		//  We have to know if this is the real one or the copy.
		if (playerObject->getPlacedInstrumentId() == this->getId())
		{
			// We are handling the copy
			if ((playerObject->getId() == this->getOwner()) && this->getPlaced())
			{
				if ((playerObject->getPerformingState() == PlayerPerformance_Music))
				{
					mRadialMenu->addItem(static_cast<uint8>(radId++),0,radId_itemUse,radAction_ObjCallback, "@radial_performance:stop_playing");
				}
				else
				{
					mRadialMenu->addItem(static_cast<uint8>(radId++),0,radId_itemUse,radAction_ObjCallback, "@radial_performance:play_instrument");
				}
			}
			else
			{
				// radial->addItem(radId++,0,radId_examine,radAction_Default);
				// radial->addItem(radId++,0,radId_itemPickup,radAction_Default);
				return;
			}
			mRadialMenu->addItem(static_cast<uint8>(radId++),0,radId_examine,radAction_Default);
			mRadialMenu->addItem(static_cast<uint8>(radId++),0,radId_itemPickup,radAction_Default);
		}
		else
		{
			// We may be handling the original instrument.
			Inventory* inventory = dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
			if (inventory)
			{
				if (inventory->getId() == this->getParentId())
				{
					// We have our real instrument in the inventory.

					// We can't drop if outside in the world.
					if (player->getParentId() == 0)
					{
						// Outside
						mRadialMenu->addItem(static_cast<uint8>(radId++),0,radId_examine,radAction_Default);
						mRadialMenu->addItem(static_cast<uint8>(radId),0,radId_itemDestroy, radAction_Default);
					}
					else
					{
						mRadialMenu->addItem(static_cast<uint8>(radId++),0,radId_examine,radAction_Default);
						mRadialMenu->addItem(static_cast<uint8>(radId),0,radId_itemDrop,radAction_Default);
						mRadialMenu->addItem(static_cast<uint8>(radId),0,radId_itemDestroy, radAction_Default);
					}

					if (playerObject->getPlacedInstrumentId() == 0)
					{
						// We do not have any other placed intrument out.
						mRadialMenu->addItem(static_cast<uint8>(radId++),0,radId_itemUse,radAction_ObjCallback,"Use");
					}
				}
				else if (dynamic_cast<CellObject*>(gWorldManager->getObjectById(this->getParentId())))
				{
					// It's either a original instrument, or someone else instrument, copy or original.

					// Time for some dirty... the original instrument does not have an owner.
					// Let's take advantage of that shortcoming.

					// Is this my instrument?
					if (this->getOwner() == player->getId())
					{
						// Yes, are we handling the original instrument.
						// if (cell->getId() == this->getParentId())
						{
							if ((playerObject->getPerformingState() == PlayerPerformance_Music))
							{
								mRadialMenu->addItem(static_cast<uint8>(radId++),0,radId_itemUse,radAction_ObjCallback, "@radial_performance:stop_playing");
							}
							else
							{
								mRadialMenu->addItem(static_cast<uint8>(radId++),0,radId_itemUse,radAction_ObjCallback, "@radial_performance:play_instrument");
							}

							mRadialMenu->addItem(static_cast<uint8>(radId++),0,radId_examine,radAction_Default);
							mRadialMenu->addItem(static_cast<uint8>(radId++),0,radId_itemPickup,radAction_Default);
						}
					}
					else
					{
						// This is not my instrument.
						// gMessageLib->sendSystemMessage(playerObject,L"","error_message","insufficient_permissions");
						mRadialMenu->addItem(static_cast<uint8>(radId++),0,radId_examine,radAction_Default);
						// radial->addItem(radId++,0,radId_itemPickup,radAction_Default);
					}
				}
			}
		}
	}
	// mRadialMenu = RadialMenuPtr(radial);

	// RadialMenuPtr radialPtr(radial);
	// mRadialMenu = radialPtr;

}
