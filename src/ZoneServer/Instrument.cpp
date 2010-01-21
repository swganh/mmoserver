			  /*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

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
#include "MathLib/Quaternion.h"
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
					float range = gWorldConfig->getConfiguration("Zone_Player_ItemUse",(float)6.0);
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
								gMessageLib->sendSystemMessage(player,L"You cannot do this at this time.");
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
	// gLogger->logMsgF("Instrument::prepareCustomRadialMenu() Entered", MSG_HIGH);

	// NOTE: player is also of type CreatureObject* !!!
	PlayerObject* playerObject = dynamic_cast<PlayerObject*>(player);

	mRadialMenu.reset();
	mRadialMenu = RadialMenuPtr(new RadialMenu());

	// RadialMenu* radial	= new RadialMenu();

	//string mInstrumentString = instrument->getName();
	uint32 instrumentNr = this->getItemType();

	if ((instrumentNr == ItemType_Nalargon) || (instrumentNr == ItemType_omni_box) || (instrumentNr == ItemType_nalargon_max_reebo))
	{
		//gLogger->logMsgF("Nalargon : %s", MSG_NORMAL,instrument->getName().getAnsi());

		uint32 radId = 1;
		//  We have to know if this is the real one or the copy.
		if (playerObject->getPlacedInstrumentId() == this->getId())
		{
			// We are handling the copy
			if ((playerObject->getId() == this->getOwner()) && this->getPlaced())
			{
				// gLogger->logMsgF("Radial: A placed instrument", MSG_NORMAL);
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
				// gLogger->logMsgF("Radial: Not our instrument, but is should be", MSG_NORMAL);
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
					// gLogger->logMsgF("Radial: Original instrument in inventory.", MSG_NORMAL);

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
						// gLogger->logMsgF("Radial: We don't have any copy placed, activate Use radial option.", MSG_NORMAL);
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
							// gLogger->logMsgF("Radial: Original instrument in a cell.", MSG_NORMAL);
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
						// gLogger->logMsgF("Radial: Not my instrument", MSG_NORMAL);
					}
				}
			}
		}
	}
	// mRadialMenu = RadialMenuPtr(radial);

	// RadialMenuPtr radialPtr(radial);
	// mRadialMenu = radialPtr;

}
