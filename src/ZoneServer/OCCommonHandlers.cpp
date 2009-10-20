/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "Bank.h"
#include "BankTerminal.h"
#include "CellObject.h"
#include "Container.h"
#include "ContainerObjectFactory.h"
#include "CraftingTool.h"
#include "CurrentResource.h"
#include "Datapad.h"
#include "IntangibleObject.h"
#include "Inventory.h"
#include "Item.h"
#include "ManufacturingSchematic.h"
#include "MissionObject.h"
#include "NPCObject.h"
#include "ObjectController.h"
#include "ObjectControllerCommandMap.h"
#include "ObjectControllerOpcodes.h"
#include "ObjectFactory.h"
#include "PlayerObject.h"
#include "QuadTree.h"
#include "ResourceContainer.h"
#include "ResourceManager.h"
#include "Shuttle.h"
#include "SurveyTool.h"
#include "TravelMapHandler.h"
#include "TravelTerminal.h"
#include "TreasuryManager.h"
#include "Tutorial.h"
#include "UIManager.h"
#include "Wearable.h"
#include "WorldConfig.h"
#include "WorldManager.h"
#include "ZoneOpcodes.h"
#include "ZoneTree.h"
#include "MessageLib/MessageLib.h"
#include "LogManager/LogManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/DatabaseResult.h"
#include "Common/Message.h"
#include "Common/MessageFactory.h"


void ObjectController::_handleBoardTransport(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*	playerObject	= dynamic_cast<PlayerObject*>(mObject);

	ObjectSet		inRangeObjects;
	float boardingRange = 25.0;

	if(playerObject->getPosture() == CreaturePosture_SkillAnimating)
	{
		gMessageLib->sendSystemMessage(playerObject,L"You cannot do that at this time.");
		return;
		//gEntertainerManager->stopEntertaining(player);
	}

	string str;
	message->getStringUnicode16(str);
	str.convert(BSTRType_ANSI);
	str.toLower();
	if((str.getCrc() != BString("transport").getCrc()))
	{
		gMessageLib->sendSystemMessage(playerObject,L"","travel","boarding_what_shuttle");
		return;
	}


	mSI->getObjectsInRange(playerObject,&inRangeObjects,ObjType_Creature | ObjType_NPC,boardingRange);

	// iterate through the results
	ObjectSet::iterator it = inRangeObjects.begin();

	while(it != inRangeObjects.end())
	{
		CreatureObject* creature = dynamic_cast<CreatureObject*> (*it);

		if(creature)
		{

			if(creature->getCreoGroup() == CreoGroup_Shuttle)
			{
				Shuttle* shuttle = dynamic_cast<Shuttle*> (creature);

				if(shuttle)
				{

					// in range check
					if(playerObject->getParentId() !=  shuttle->getParentId() )
					{
						gMessageLib->sendSystemMessage(playerObject,L"","travel","boarding_too_far");
						return;
					}

					if (!shuttle->avaliableInPort())
					{
						gMessageLib->sendSystemMessage(playerObject,L"","travel","shuttle_not_available");
						return;
					}

					shuttle->useShuttle(playerObject);
					return;
				}
			}

		}

		++it;
	}

	gMessageLib->sendSystemMessage(playerObject,L"","structure/structure_messages","boarding_what_shuttle");
}

//=============================================================================
//
// open container
//

void ObjectController::_handleOpenContainer(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	// gLogger->logMsg("ObjController::_handleOpenContainer:");

	PlayerObject*	playerObject	= dynamic_cast<PlayerObject*>(mObject);
	Object*			itemObject		= gWorldManager->getObjectById(targetId);

	// gLogger->hexDump(message->getData(),message->getSize());
/*
	46 5E CE 80 23 00 00 00 16 01 00 00 79 69 00 00
	02 00 00 00 00 00 00 00 00 00 00 00 86 75 17 70
	B3 2C 81 00 00 00 00 00 02 00 00 00 31 00 20 00

	// 86 75 17 70 = The command OpenContainer
	// B3 2C 81 00 00 00 00 00 = container id
	// 02 00 00 00 31 00 20 00 = 2, "1 ", i.e Number of uni-chars (32 bits) and no of Open " 1", " 2".... " 21" where the " " is the last byte in the data.
*/

	// Let's make a "fake open" if the object is not a container.
	if (itemObject)
	{
		bool aContainer = false;

		if (gWorldConfig->isTutorial())
		{
			playerObject->getTutorial()->containerOpen(targetId);
		}

		if (itemObject->getType() == ObjType_Tangible)
		{
			TangibleObject* tangObj = dynamic_cast<TangibleObject*>(itemObject);
			if (tangObj->getTangibleGroup() == TanGroup_Container)
			{
				// Request container contence.
				gContainerFactory->requestObject(this,targetId,TanGroup_Container,0,playerObject->getClient());
				aContainer = true;
			}
		}
		if (!aContainer)
		{
			// Open an empty container. And we will actually see what kind of container it is.
			gMessageLib->sendOpenedContainer(targetId, playerObject);
		}
	}
	else
	{
		gLogger->logMsgF("ObjectController::_handleOpenContainer: INVALID Object id %lld",MSG_NORMAL,targetId);
	}
}

void ObjectController::_handleCloseContainer(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	// gLogger->logMsg("ObjController::_handleCloseContainer:");

	PlayerObject*	playerObject	= dynamic_cast<PlayerObject*>(mObject);
	Object*			itemObject		= gWorldManager->getObjectById(targetId);

	if (gWorldConfig->isTutorial())
	{
		playerObject->getTutorial()->containerClose(targetId);
	}


	// gLogger->hexDump(message->getData(),message->getSize());
	// gLogger->logMsgF("ObjectController::_handleCloseContainer: targetId = %lld",MSG_NORMAL,targetId);
	// gMessageLib->sendOpenedContainer(targetId, playerObject);
}


//=============================================================================
//
// transfer item
//

void ObjectController::_handleTransferItem(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*	playerObject	=	dynamic_cast<PlayerObject*>(mObject);
	Object*			itemObject		=	gWorldManager->getObjectById(targetId);
	Inventory*		inventory		=	dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));

	string			dataStr;
	uint64			targetContainerId;
	uint32			linkType;
	float			x,y,z;

	message->getStringUnicode16(dataStr);

	// gLogger->logMsg("ObjectController::_handleTransferItem() THIS IS (almost) UNSUPPORTED");

	if(swscanf(dataStr.getUnicode16(),L" %lld %u %f %f %f",&targetContainerId,&linkType,&x,&y,&z) != 5)
	{
		gLogger->logMsg("ObjController::handleTransferItem: Error in parameters");
		return;
	}
	// gLogger->logMsgF("Parameters:  %lld %d %.1f %.1f %.1f", MSG_NORMAL, targetContainerId, linkType, x, y, z);
	// gLogger->logMsgF("TargetId = %lld", MSG_NORMAL, targetId);

	if(itemObject)
	{
		Item* item = dynamic_cast<Item*>(itemObject);

		// We may want to transfer other things than items...
		// Right now is this code used for "/pickup" of all objects, and a non-item object will crash the server.
		if (item)
		{
			//check if its only temporarily placed
			if(item->getItemFamily() == ItemFamily_Instrument)
			{
				if(item->getPersistantCopy())
				{
					gMessageLib->sendSystemMessage(playerObject,L"You cannot pick this up.");
					return;
				}
			}
		}
		else
		{
			gLogger->logMsg("ObjController::_handleTransferItem: This is not an item");
			return;
		}


		itemObject->mPosition = Anh_Math::Vector3(x,y,z);

		// other transfers, skip for now
		// drop in cell
		CellObject* cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(targetContainerId));
		if(cell)
		{
			itemObject->setParentId(targetContainerId);
			//gMessageLib->sendContainmentMessage(targetId,targetContainerId,linkType,playerObject);

			playerObject->addKnownObjectSafe(itemObject);

			if(targetContainerId)
			{
				if(CellObject* cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(targetContainerId)))
				{
					cell->addChild(itemObject);
				}
				else
				{
					gLogger->logMsgF("ObjectController::_handleTransferItem: couldn't find cell %lld",MSG_HIGH,targetContainerId);
				}
			}
			else
			{
				if(QTRegion* region = gWorldManager->getSI()->getQTRegion(itemObject->mPosition.mX,itemObject->mPosition.mZ))
				{
					itemObject->setSubZoneId((uint32)region->getId());
					region->mTree->addObject(itemObject);
				}
				else
				{
					// we should never get here !
					gLogger->logMsg("ObjectController::_handleTransferItem: could not find zone region in map");
					return;
				}
			}

			// initialize objects in range

			TangibleObject* tangible = dynamic_cast<TangibleObject*>(itemObject);
			//gWorldManager->initPlayersInRange(itemObject);

			PlayerObjectSet*			inRangePlayers	= playerObject->getKnownPlayers();
			PlayerObjectSet::iterator	it				= inRangePlayers->begin();
			while(it != inRangePlayers->end())
			{
				PlayerObject* targetObject = (*it);
				gMessageLib->sendCreateTangible(tangible,targetObject);
				targetObject->addKnownObjectSafe(tangible);
				tangible->addKnownObjectSafe(targetObject);
				++it;
			}
			//mDatabase->ExecuteSqlAsync(0,0,"UPDATE items SET parent_id=%I64u WHERE item_id=%I64u",itemObject->getParentId() ,itemObject->getId());
			//printf("linktype %u",linkType);
			//gMessageLib->sendContainmentMessage(targetId,targetContainerId,linkType,playerObject);
			gMessageLib->sendDestroyObject(tangible->getId(),playerObject);
			tangible->mPosition = playerObject->mPosition;
			tangible->mDirection = playerObject->mDirection;

			gMessageLib->sendCreateTangible(tangible,playerObject);
			//gMessageLib->sendDataTransformWithParent(tangible);
			playerObject->addKnownObjectSafe(tangible);
			tangible->addKnownObjectSafe(playerObject);

			inventory->removeObject(tangible);

			return;//
		}

		// pick up - check whether target object is a container or an inventory
		cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(itemObject->getParentId()));
		if(cell)
		{
			//stop playing if we pick up the (permanently placed) instrument we are playing
			Item* item = dynamic_cast<Item*>(itemObject);
			if(item)
			{
				if(item->getItemFamily() == ItemFamily_Instrument)
				{
					//is it the one we are playing ?
					if((playerObject->getPerformingState() == PlayerPerformance_Music)&&(gEntertainerManager->gettargetedInstrument(playerObject)==item->getId()))
					gEntertainerManager->stopEntertaining(playerObject);
				}
			}

			itemObject->setParentId(targetContainerId);

			if(TangibleObject* tangible = dynamic_cast<TangibleObject*>(itemObject))
			{
				uint64 parentId = itemObject->getParentId();
				cell->removeChild(itemObject);

				//update known players
				PlayerObjectSet*			inRangePlayers	= playerObject->getKnownPlayers();
				PlayerObjectSet::iterator	it				= inRangePlayers->begin();
				while(it != inRangePlayers->end())
				{
					PlayerObject* targetObject = (*it);
					gMessageLib->sendDestroyObject(tangible->getId(),targetObject);
					targetObject->removeKnownObject(tangible);
					++it;
				}
				itemObject->destroyKnownObjects();
				//mDatabase->ExecuteSqlAsync(0,0,"UPDATE items SET parent_id=%I64u WHERE item_id=%I64u",itemObject->getParentId() ,itemObject->getId());
				gMessageLib->sendDestroyObject(tangible->getId(),playerObject);
				gMessageLib->sendCreateTangible(tangible,playerObject);
				inventory->addObject(tangible);
				//tangible->mPosition = entertainer->mPosition;
			//usedInstrument->mDirection = entertainer->mDirection;
			//gMessageLib->sendDataTransformWithParent(usedInstrument);
			}


				// destroy known objects

			//if (itemObject->getType() ==)
			//itemObject->setInternalAttribute("instrument_placed","0");
			//remove from the sis
			return;//itemObject->setParentId(playerObject->getInventory()->getId());
		}

		switch(itemObject->getType())
		{
			case ObjType_Tangible:
			{
				if(dynamic_cast<TangibleObject*>(itemObject)->getTangibleGroup() == TanGroup_Item && itemObject->hasInternalAttribute("equipped"))
				{
					Item*						item			= dynamic_cast<Item*>(itemObject);
					PlayerObjectSet*			inRangePlayers	= playerObject->getKnownPlayers();
					PlayerObjectSet::iterator	it				= inRangePlayers->begin();

					// unequip it
					if(item->getInternalAttribute<bool>("equipped"))
					{
						inventory->unEquipItem(item);
						return;
					}
					// equip it
					else
					{
						inventory->EquipItem(item);
						return;

					}
				}
			}
			break;

			default:break;
		}
	}
}

//=============================================================================
//
// transfer misc item
//

//	Transfer items between player inventories, containers and cells. Also handles transfer from  creature inventories (looting).
//	Also handles trandfer between player inventory and player (equipping items).
//	We don't handle transfer to the world outside.
//

void ObjectController::_handleTransferItemMisc(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	//we need to make sure that ONLY equipped items are contained by the player
	//all other items are contained by the inventory!!!!!!!!

	PlayerObject*	playerObject	=	dynamic_cast<PlayerObject*>(mObject);
	Object*			itemObject		=	gWorldManager->getObjectById(targetId);
	Inventory*		inventory		=	dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));

	string			dataStr;
	uint64			targetContainerId;
	uint32			linkType;
	float			x,y,z;

	// gLogger->logMsg("ObjController::_handleTransferItemMisc: Entered");

	message->getStringUnicode16(dataStr);

	if(swscanf(dataStr.getUnicode16(),L" %lld %u %f %f %f",&targetContainerId,&linkType,&x,&y,&z) != 5)
	{
		gLogger->logMsg("ObjController::_handleTransferItemMisc: Error in parameters");
		return;
	}

	// gLogger->logMsgF("Parameters:  %llu %d %.1f %.1f %.1f", MSG_NORMAL, targetContainerId, linkType, x, y, z);
	// gLogger->logMsgF("TargetId = %llu", MSG_NORMAL, targetId);
	if (itemObject)
	{
		Item* item = dynamic_cast<Item*>(itemObject);

		// We may want to transfer other things than items...
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
		else
		{
			// gLogger->logMsg("ObjController::_handleTransferItemMisc: This is not an item");
			return;
		}

		// A FYI: When we drop items, we use player pos.
		itemObject->mPosition = Anh_Math::Vector3(x,y,z);


// DROP or PICKUP
// Drop item (from inventory) into a cell.
		if (targetContainerId)
		{
			CellObject* cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(targetContainerId));
			if (cell)
			{
				// TODO: THIS WILL BE EVALUATED WHEN WE CAN HAVE ACCESS TO PLAYER BUILDINGS.

				// drop in a cell
				// gLogger->logMsgF("ObjectController::_handleTransferItemMisc: Drop into cell %llu", MSG_NORMAL, targetContainerId);

				// Remove the object from whatever contains it.
				// Item* item = dynamic_cast<Item*>(itemObject);
				TangibleObject* tangible = dynamic_cast<TangibleObject*>(itemObject);

				if (inventory && item && tangible)
				{
					// Fully validated, continue the operation...
					if (itemObject->getParentId() == playerObject->getId())
					{
						if (item->getTangibleGroup() == TanGroup_Item && itemObject->hasInternalAttribute("equipped"))
						{
							// unequip it
							if (item->getInternalAttribute<bool>("equipped"))
							{
								// gLogger->logMsgF("Item is equipped  - unequipping it before drop", MSG_NORMAL);
								inventory->unEquipItem(item);
							}
						}
					}
					gMessageLib->sendDestroyObject(targetId,playerObject);
					inventory->removeObject(itemObject);
					// gLogger->logMsg("Removed item from an inventory/container and dropped into a cell.");

					// Add the item to the cell.

					// If we drop (some) instruments, we have to remove the non persistent copy, if any.
					if (item->getItemFamily() == ItemFamily_Instrument)
					{
						if (playerObject->getPlacedInstrumentId())
						{
							// We do have a placed instrument.
							uint32 instrumentType = item->getItemType();
							if ((instrumentType == ItemType_Nalargon) || (instrumentType == ItemType_omni_box) || (instrumentType == ItemType_nalargon_max_reebo))
							{
								// We are about to drop the real thing, remove any copied instrument.
								// item->setOwner(playerObject->getId();
								playerObject->getController()->destroyObject(playerObject->getPlacedInstrumentId());


								// As a test we will mark the instrument with a "owner tag". This tag will disappear when server restarts.
								// But it gives us some simulated control of access to dropped items.
								// TODO: Fix this when we implement cell-vuilding access verification system.
								// item->setOwner(playerObject->getId());
							}
						}
					}
					// As a test we will mark the instrument with a "owner tag". This tag will disappear when server restarts.
					// But it gives us some simulated control of access to dropped items.
					// TODO: Fix this when we implement cell-building access verification system.
					item->setOwner(playerObject->getId());

					itemObject->setParentId(targetContainerId);
					itemObject->mPosition = playerObject->mPosition;
					itemObject->mDirection = playerObject->mDirection;
					cell->addChild(itemObject);

					gMessageLib->sendContainmentMessage(targetId,targetContainerId,linkType,playerObject);
					mDatabase->ExecuteSqlAsync(0,0,"UPDATE items SET parent_id ='%I64u', oY='%f', oZ='%f', oW='%f', x='%f', y='%f', z='%f' WHERE id='%I64u'",itemObject->getParentId(), itemObject->mDirection.mY, itemObject->mDirection.mZ, itemObject->mDirection.mW, itemObject->mPosition.mX, itemObject->mPosition.mY, itemObject->mPosition.mZ, itemObject->getId());

					// NOTE: It's quite possible to have the player update do this kind of updates manually,
					// but that will have a performance cost we don't ready to take yet.
					PlayerObjectSet* inRangePlayers	= playerObject->getKnownPlayers();
					PlayerObjectSet::iterator it = inRangePlayers->begin();
					while(it != inRangePlayers->end())
					{
						PlayerObject* targetObject = (*it);
						gMessageLib->sendCreateTangible(tangible,targetObject);
						targetObject->addKnownObjectSafe(tangible);
						tangible->addKnownObjectSafe(targetObject);
						++it;
					}
				}
				else
				{
					// We do not allow to drop items from conatiners directly to cells (yet).
					// gLogger->logMsg("Can't drop items from that container into the cell.");
				}
				return;
			}
			else
			{
				// Continue...
				// gLogger->logMsgF("ObjectController::_handleTransferItemMisc: Target %llu was not a cell, continue...", MSG_NORMAL, targetContainerId);


				// pick up - check whether target object is a container or an inventory

				// If we are picking up, it's to the players inventory.

// Pick up from cell (into player inventory).
				if (inventory && (inventory->getId() == targetContainerId))	// Valid player inventory.
				{
					// Picking up into the inventory.
					CellObject* cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(itemObject->getParentId()));
					if (cell)
					{
						// gLogger->logMsgF("ObjectController::_handleTransferItemMisc: pick up from cell %llu", MSG_NORMAL, itemObject->getParentId());

						// Remove object from cell.
						TangibleObject* tangible = dynamic_cast<TangibleObject*>(itemObject);
						if (item && tangible)
						{
							if (targetId <= uint64(0x0000000100000000))
							{
								// Attempt to pickup one of our static items.
								// gLogger->logMsgF("ObjectController::_handleTransferItemMisc: Item %llu is a static item owned by this galaxy.", MSG_NORMAL, targetId);
								gMessageLib->sendSystemMessage(playerObject,L"","error_message","insufficient_permissions");
								return;
							}

							// Are we the owner of this item?
							if (item->getOwner() != playerObject->getId())
							{
								// We should not pickup what we don't own.

								// But, as long as we allow people to drop items in all buildings,
								// we allow to pickup items not owned, i.e. been left during a server restart.
								if (item->getOwner() != 0)
								{
									gMessageLib->sendSystemMessage(playerObject,L"","error_message","insufficient_permissions");
									return;
								}
							}

							// Stop playing if we pick up the (permanently placed) instrument we are playing
							if (item->getItemFamily() == ItemFamily_Instrument)
							{
								uint32 instrumentType = item->getItemType();
								if ((instrumentType == ItemType_Nalargon) || (instrumentType == ItemType_omni_box) || (instrumentType == ItemType_nalargon_max_reebo))
								{
									// It's a placeable original instrument.
									// Are we targeting the instrument we actually play on?
									if (playerObject->getActiveInstrumentId() == item->getId())
									{
										gEntertainerManager->stopEntertaining(playerObject);
										// gLogger->logMsgF("ObjectController::_handleTransferItemMisc: Picking up the active instrument", MSG_NORMAL);
									}
								}
							}
							if (cell->removeChild(itemObject))
							{
								gMessageLib->sendDestroyObject(tangible->getId(),playerObject);
								gMessageLib->sendContainmentMessage(targetId,targetContainerId,linkType,playerObject);

								PlayerObjectSet* inRangePlayers	= playerObject->getKnownPlayers();
								PlayerObjectSet::iterator it = inRangePlayers->begin();
								while(it != inRangePlayers->end())
								{
									PlayerObject* targetObject = (*it);
									gMessageLib->sendDestroyObject(tangible->getId(),targetObject);
									targetObject->removeKnownObject(tangible);
									++it;
								}
								itemObject->destroyKnownObjects();

								// Add object to inventory.
								itemObject->setParentId(targetContainerId);
								inventory->addObject(itemObject);
								gMessageLib->sendCreateTangible(tangible,playerObject);
								// mDatabase->ExecuteSqlAsync(0,0,"UPDATE items SET parent_id=%I64u, privateowner_id=%I64u WHERE id=%I64u" ,targetContainerId, itemObject->getPrivateOwner(), targetId);
								mDatabase->ExecuteSqlAsync(0,0,"UPDATE items SET parent_id ='%I64u', oY='0', oZ='0', oW='0', x='0', y='0', z='0' WHERE id='%I64u'",itemObject->getParentId(), itemObject->getId());
							}
							else
							{
								// This should not happend.
								gLogger->logMsg("Can't delete-find item in cell.\n");
								gMessageLib->sendSystemMessage(playerObject,L"","error_message","insufficient_permissions");
							}
						}
						else
						{
							// gLogger->logMsg("Can't pickup item from that cell.");
						}
						return;
					}
					// Continue...
					// gLogger->logMsgF("ObjectController::_handleTransferItemMisc: Parent of item %llu was not a cell to pickup from, continue...", MSG_NORMAL, itemObject->getParentId());
				}
			}
		}

// TRANSFER ITEMS. (manual looting included)

		// We have to figure out if we are adding or deleting an item from the inventory.

// Transfer item to player inventory.
		// Inventory* inventory = dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
		if (inventory && (inventory->getId() == targetContainerId))
		{
			// gLogger->logMsg("Transfer item to player inventory");
			bool removedFailed = false;

			// Remove it from the container.
			// We dont have any access validations yet.
			uint64 sourceId = itemObject->getParentId();
			Container* container = dynamic_cast<Container*>(gWorldManager->getObjectById(sourceId));
			if (container)
			{
				// gLogger->logMsg("Transfer item from container to player inventory");
				container->removeObject(itemObject);
				if (gWorldConfig->isTutorial())
				{
					playerObject->getTutorial()->transferedItemFromContainer(targetId, sourceId);

					// If object is owned by player (private owned for instancing), we remove the owner from the object.
					if (itemObject->getPrivateOwner() == playerObject->getId())
					{
						itemObject->setPrivateOwner(0);
					}
				}

				// Add object to inventory.
				dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory))->addObject(itemObject);
				itemObject->setParentId(targetContainerId);
				mDatabase->ExecuteSqlAsync(0,0,"UPDATE items SET parent_id=%I64u, privateowner_id=%I64u WHERE id=%I64u" ,targetContainerId, itemObject->getPrivateOwner(), targetId);
				gMessageLib->sendContainmentMessage(targetId,targetContainerId,linkType,playerObject);
				return;
			}
			// The long version...
			/*
			else
			{
				// Can it be an object in a creature inventory?

				// Get objects parent.
				unit64 parentId = itemObject->getParentId();

				if (parentId)
				{
					// Assuming this is an Inventory, it's parent would have the if of parentId - 1.
					uint64 grandParentId = parentId - 1;

					// Check if this is a registred object.
					Object* unknownObject = gWorldManager->getObjectById(grandParentId);
					if (unknownObject)
					{
						CreatureObject* unknownCreature = dynamic_cast<CreatureObject*>(unknownObject);
						if (unknownCreature)
						{
							Inventory* creatureInventory  = dynamic_cast<Inventory*>(unknownCreature->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
							if (creatureInventory)
							{
								if (creatureInventory->getId() == itemObject->getParentId())
								{
									// It was an item in an inventory... sigh...
					*/
			// The short version!
			else
			{
				CreatureObject* unknownCreature;
				Inventory* creatureInventory;
				if (itemObject->getParentId() &&
					(unknownCreature = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById(itemObject->getParentId() - 1))) &&
					(creatureInventory = dynamic_cast<Inventory*>(unknownCreature->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory))) &&
					(creatureInventory->getId() == itemObject->getParentId()))
				{
					Item* item = dynamic_cast<Item*>(itemObject);
					if (creatureInventory && item)
					{
						// Fully validated, continue the operation...
						gLogger->logMsg("Transfer item from creature inventory to player inventory (looting)");
						// gMessageLib->sendContainmentMessage(targetId,itemObject->getParentId(),-1,playerObject);

						gMessageLib->sendDestroyObject(targetId,playerObject);
						creatureInventory->removeObject(itemObject);
						// gLogger->logMsg("Removed item from a creature inventory");

						ObjectList* invObjList = creatureInventory->getObjects();
						if (invObjList->size() == 0)
						{
							// Put this creature in the pool of delayed destruction and remove the corpse from scene.
							gWorldManager->addCreatureObjectForTimedDeletion(creatureInventory->getParentId(), LootedCorpseTimeout);
						}

						if (gWorldConfig->isTutorial())
						{
							// TODO: Update tutorial about the loot.
							// playerObject->getTutorial()->transferedItemFromContainer(targetId, sourceId);
						}
						// This ensure that we do not use/store any of the temp id's in the database.
						gObjectFactory->requestNewDefaultItem(inventory, item->getItemFamily(), item->getItemType(), inventory->getId(),99,Anh_Math::Vector3(),"");
						return;
					}
					else
					{
						// gLogger->logMsg("Could not transfer item to any inventory, continue..");
					}
				}
			}
		}
// Transfer item from player inventory.
		else if ((inventory && inventory->getId() == itemObject->getParentId()) && (targetContainerId != playerObject->getId())) // Is this last part a test for equipped items...?
		{
			// It's an item in the inventory we are to manipulate.
			// gLogger->logMsg("Source is an inventory");
			// gLogger->logMsgF("Target container id = %lld", MSG_NORMAL, targetContainerId);

			if (inventory->getId() == targetContainerId)
			{
				gLogger->logMsg("Target is THE same inventory\n");
			}
			if (gWorldConfig->isTutorial())
			{
				// We don't allow users to place item in the container.
				// gMessageLib->sendSystemMessage(playerObject,L"","event_perk","chest_can_not_add");
				gMessageLib->sendSystemMessage(playerObject,L"","error_message","remove_only");
			}
			else
			{
				// Validate target before removing items form inventory. We don't wanna lose the item, do we?
				Container* container = dynamic_cast<Container*>(gWorldManager->getObjectById(targetContainerId));
				Item* item = dynamic_cast<Item*>(itemObject);
				// TangibleObject* tangible = dynamic_cast<TangibleObject*>(itemObject);

				if (container && item)
				{
					// Unequipp
					if (itemObject->getParentId() == playerObject->getId())
					{
						if (item->getTangibleGroup() == TanGroup_Item && itemObject->hasInternalAttribute("equipped"))
						{
							// unequip it
							if (item->getInternalAttribute<bool>("equipped"))
							{
								// gLogger->logMsgF("Item is equipped  - unequiping it before drop in world", MSG_NORMAL);
								inventory->unEquipItem(item);
							}
						}
					}
					// gLogger->logMsg("Transfer item from player inventory to container.");

					// Remove object from inventory.
					dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory))->removeObject(itemObject);

					// Add it to the container.
					// We dont have any access validations yet.
					container->addObject(itemObject);

					itemObject->setParentId(targetContainerId);
					mDatabase->ExecuteSqlAsync(0,0,"UPDATE items SET parent_id=%I64u WHERE id=%I64u" ,targetContainerId, targetId);
					gMessageLib->sendContainmentMessage(targetId,targetContainerId,linkType,playerObject);
				}
				else
				{
					// gLogger->logMsg("Could not add item to container");
					// if creature inventory
					// gMessageLib->sendSystemMessage(playerObject,L"","error_message","remove_only_corpse");
				}
			}
			return;
		}

		// gLogger->logMsgF("Object type: %d", MSG_NORMAL, itemObject->getType());
		switch(itemObject->getType())
		{
			case ObjType_Tangible:
			{
				Item* item = dynamic_cast<Item*>(itemObject);
				if (item)
				{
					// gLogger->logMsgF("Tangible item type: %d", MSG_NORMAL, item->getItemType());
					// gLogger->logMsgF("Item family: %d", MSG_NORMAL, item->getItemFamily());

					if (item->getTangibleGroup() == TanGroup_Item && itemObject->hasInternalAttribute("equipped"))
					{

						// unequip it
						if(item->getInternalAttribute<bool>("equipped"))
						{
							// gLogger->logMsgF("it is equipped  - unequip it", MSG_NORMAL);

							inventory->unEquipItem(item);
							return;

						}
						// equip it
						else
						{
							/// gLogger->logMsgF("it is NOT equipped - equip it", MSG_NORMAL);
							// don't equip music instruments or weapons while performing
							inventory->EquipItem(item);
							return;

						}

					}
				}
			}
			break;

			default:
			{
				gLogger->logMsgF("ObjectController::_handleTransferItemMisc: Unhandled object type: %d", MSG_NORMAL, itemObject->getType());
			}
			break;
		}
	}
}

//=============================================================================
//
// purchase ticket
//

void ObjectController::_handlePurchaseTicket(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*	playerObject = dynamic_cast<PlayerObject*>(mObject);
	string			dataStr;
	BStringVector	dataElements;
	uint16			elements;

	ObjectSet		inRangeObjects;
	float			purchaseRange = 10.0;

	if(playerObject->getPosture() == CreaturePosture_SkillAnimating)
	{
		gMessageLib->sendSystemMessage(playerObject,L"You cannot do that at this time.");
		return;
	}

	mSI->getObjectsInRange(playerObject,&inRangeObjects,ObjType_Tangible,purchaseRange);

	//however we are able to use the purchaseticket command in starports
	//without having to use a ticketvendor by just giving commandline parameters
	//when we are *near* a ticket vendor

	// iterate through the results
	ObjectSet::iterator it = inRangeObjects.begin();
	bool found = false;

	while(it != inRangeObjects.end())
	{

		TravelTerminal* terminal = dynamic_cast<TravelTerminal*> (*it);

		if(terminal)
		{
			if(terminal->getTangibleType() == TanType_TravelTerminal)
			{
				//double check the distance
				if(terminal->mPosition.inRange2D(playerObject->mPosition,purchaseRange))
				{
					found = true;
					playerObject->setTravelPoint(terminal);
				}
			}
		}

		++it;
	}

	if(!found)
	{
		gMessageLib->sendSystemMessage(playerObject,L"","travel","too_far");
		return;
	}

	message->getStringUnicode16(dataStr);

	// Have to convert BEFORE using split, since the conversion done there is removed It will assert().. evil grin...
	// Either do the conversion HERE, or better fix the split so it handles unicoe also.
	dataStr.convert(BSTRType_ANSI);

	elements = dataStr.split(dataElements,' ');

	if(elements < 4)
	{
		gMessageLib->sendSystemMessage(playerObject,L"","travel","route_not_available");
		return;
	}

	// get price and planet ids
	TicketProperties ticketProperties;
	gTravelMapHandler->getTicketInformation(dataElements,&ticketProperties);

	if(!ticketProperties.dstPoint)
	{
		gMessageLib->sendSystemMessage(playerObject,L"","travel","route_not_available");
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

	Inventory*	inventory	= dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
	Bank*		bank		= dynamic_cast<Bank*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Bank));

	if(!inventory->checkSlots(static_cast<uint8>(amount)))
	{
		gMessageLib->sendSystemMessage(playerObject,L"","error_message","inv_full");
		return;
	}

	if(roundTrip == 1)
	{
		ticketProperties.price *= 2;
	}

	// update bank or inventory credits
	if(!(inventory->updateCredits(-ticketProperties.price)))
	{
		if(!(bank->updateCredits(-ticketProperties.price)))
		{
			//gMessageLib->sendSystemMessage(entertainer,L"","travel","route_not_available");
			gUIManager->createNewMessageBox(NULL,"ticketPurchaseFailed","The Galactic Travel Commission","You do not have enough money to complete the ticket purchase.",playerObject);
			return;
		}
	}

	if(playerObject->isConnected())
	{
		gMessageLib->sendSystemMessage(playerObject,L"","base_player","prose_pay_acct_success","money/acct_n","travelsystem",L"",ticketProperties.price);

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
	PlayerObject*	playerObject	= dynamic_cast<PlayerObject*>(mObject);
	string			requestStr;
	BStringVector	dataElements;
	BStringVector	dataElements2;
	uint16			elementCount;


	message->getStringUnicode16(requestStr);
	requestStr.convert(BSTRType_ANSI);
	requestStr.getRawData()[requestStr.getLength()] = 0;

	elementCount = requestStr.split(dataElements,' ');

	if(!elementCount)
	{
		gLogger->logMsg("ObjectController::_handleAttributesBatch: Error in requestStr\n");
		return;
	}

	Message* newMessage;

	for(uint16 i = 0;i < elementCount;i++)
	{

		uint64 itemId	= boost::lexical_cast<uint64>(dataElements[i].getAnsi());
		Object* object	= gWorldManager->getObjectById(itemId);

		//gLogger->logMsgF("ObjectController::_handleAttributesBatch: ID %I64u",MSG_HIGH,itemId);

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
			Datapad* datapad				= dynamic_cast<Datapad*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Datapad));
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

			//gLogger->logMsgF("ObjectController::_handleAttributesBatch: get info for %lld",MSG_HIGH,itemId);

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

			//gLogger->logMsgF("ObjectController::_handleAttributesBatch: Object not found %lld",MSG_HIGH,itemId);

			//finally, when we are crafting this could be the new item, not yet added to the worldmanager??
			if(playerObject->getCraftingSession())
			{
				if(playerObject->getCraftingSession()->getItem()&&playerObject->getCraftingSession()->getItem()->getId() == itemId)
				{
					gLogger->logMsgF("ObjectController::_handleAttributesBatch for crafted Item: ID %I64u",MSG_HIGH,itemId);
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
				if (dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory))->getId() == object->getParentId())
				{
					// gLogger->logMsg("Selected an object in Inventory!!!");
					uint64 id = object->getId();

					// Is it an Item?
					Item* item = dynamic_cast<Item*>(object);

					// Check if this item is a food item.
					if (item)
					{
						if (item->getItemFamily() == ItemFamily_Foods)
						{
							// gLogger->logMsg("It's a food item!!!!");
							playerObject->getTutorial()->tutorialResponse("foodSelected");
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
	PlayerObject* playerObject = (PlayerObject*)mObject;
}
//======================================================================================================================
//
// end burst run
//

void ObjectController::_endBurstRun(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{

}

//======================================================================================================================
//
// surrender skill
//

void ObjectController::_handleSurrenderSkill(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*	player		= dynamic_cast<PlayerObject*>(mObject);
	string			skillStr;

	message->getStringUnicode16(skillStr);
	skillStr.convert(BSTRType_ANSI);

	if(!(skillStr.getLength()))
	{
		gLogger->logMsg("ObjectController::handleSurrenderSkill: no skillname\n");
		return;
	}

	Skill* skill = gSkillManager->getSkillByName(skillStr.getAnsi());

	if(skill == NULL)
	{
		gLogger->logMsgF("ObjectController::handleSurrenderSkill: could not find skill %s",MSG_NORMAL,skillStr.getAnsi());
		return;
	}

	gSkillManager->dropSkill(skill->mId,player);
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
	//for ever item where we request a radial the client starts by displaying a radial on his own and additionally sends a
	//objectMenuRequest to the server
	//The server then either just resends the radial as send by the client or adds / modifies options on his own
	//this is why sometimes when lag is involved it takes some time for all options to display


	PlayerObject* playerObject = dynamic_cast<PlayerObject*>(mObject);

	message->getUint32(); // unknown
	uint64 requestedObjectId = message->getUint64();
	message->getUint64(); // player id again ?

	Object* requestedObject = gWorldManager->getObjectById(requestedObjectId);

	uint32 itemCount = message->getUint32();

	string extendedDescription;
	MenuItemList menuItemList;

	MenuItem* menuItem;
	for(uint32 i = 0; i < itemCount;i++)
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

	// gLogger->logMsgF("ObjController::handleObjectMenuRequest: Entered, responseNr = %u, itemCount = %u", MSG_NORMAL, responseNr, itemCount);

	if(!requestedObject)
	{
		if(playerObject->isConnected())
			gMessageLib->sendEmptyObjectMenuResponse(requestedObjectId,playerObject,responseNr,menuItemList);

		//the list is cleared and items are destroyes in the message lib
		//for the default response
		gLogger->logMsgF("ObjController::handleObjectMenuRequest: Couldn't find object %llu",MSG_HIGH,requestedObjectId);
		return;
	}

	requestedObject->prepareCustomRadialMenu(playerObject,static_cast<uint8>(itemCount));

	if (requestedObject->getRadialMenu())
	{
		if(playerObject->isConnected())
		{
			gMessageLib->sendObjectMenuResponse(requestedObject,playerObject,responseNr);
		}
	}
	else
	{
		// putting this for static objects/objects that are not known by the server yet
		// send a default menu,so client stops flooding us with requests

		//empty might just mean that the clients radial is sufficient
		//gLogger->logMsgF("ObjController::handleObjectMenuRequest: Couldn't find object Radial %lld",MSG_HIGH,requestedObjectId);

		if(playerObject->isConnected())
		 	gMessageLib->sendEmptyObjectMenuResponse(requestedObjectId,playerObject,responseNr,menuItemList);

		//the list is cleared and items are destroyes in the message lib
		//for the default response
	}

	//are the menu options pointers removed / deleted by boost ?
	//delete(requestedObject->getRadialMenu());
}

//=============================================================================================================================

void ObjectController::handleObjectReady(Object* object,DispatchClient* client)
{
	PlayerObject* player = gWorldManager->getPlayerByAccId(client->getAccountId());
	PlayerObject* playerObject = dynamic_cast<PlayerObject*>(mObject);

	/*
	if (!playerObject)
	{
		gLogger->logMsg("playerObject == NULL");
	}

	if (!player)
	{
		gLogger->logMsg("player == NULL");
	}

	if (playerObject != player)
	{
		gLogger->logMsg("playerObject != player");
	}
	*/

	// gLogger->logMsgF("ObjectController::handleObjectReady: targetId = %lld for %s",MSG_NORMAL,object->getId(), player->getFirstName().getAnsi());


	// Get the container object.
	Container* container = dynamic_cast<Container*>(object);
	if (container)
	{
		// uint32 counter = container->getObjectLoadCounter();
		// gLogger->logMsgF("We have %d objects in the container", MSG_NORMAL, counter);

		ObjectList*	objList = container->getObjects();
		ObjectList::iterator containerObjectIt = objList->begin();

		while(containerObjectIt != objList->end())
		{
			Object* object = (*containerObjectIt);

			if (TangibleObject* tangibleObject = dynamic_cast<TangibleObject*>(object))
			{
				// reminder: objects are owned by the global map, containers only keeps references
				// send the creates, if we are not owned by any player OR by exactly this player.
				if (playerObject)
				{
					if (!object->getPrivateOwner() || (object->isOwnedBy(playerObject)))
					{
						// could be a resource container, need to check this first, since it inherits from tangible
						if (ResourceContainer* resCont = dynamic_cast<ResourceContainer*>(object))
						{
							gMessageLib->sendCreateResourceContainer(resCont,playerObject);
						}
						// or a tangible
						else
						{
							gMessageLib->sendCreateTangible(tangibleObject,playerObject);
						}
					}
				}
			}
			++containerObjectIt;
		}
		gMessageLib->sendOpenedContainer(object->getId(), player);
	}

}

//======================================================================================================================

void ObjectController::_handleNewbieSelectStartingLocation(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject* player = dynamic_cast<PlayerObject*>(mObject);
	// gLogger->hexDump(message->getData(),message->getSize());

	// Find the planet and position.
	if (gWorldConfig->isTutorial())
	{
		string name;
		message->getStringUnicode16(name);

		if (!(name.getLength()))
		{
			return;
		}
		name.convert(BSTRType_ANSI);
		player->getTutorial()->warpToStartingLocation(name);
	}
}


