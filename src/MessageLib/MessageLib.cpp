/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "MessageLib.h"

#include "ZoneServer/BuildingObject.h"
#include "ZoneServer/CellObject.h"
#include "ZoneServer/CharSheetManager.h"
#include "ZoneServer/Conversation.h"
#include "ZoneServer/CraftingTool.h"
#include "ZoneServer/CurrentResource.h"
#include "ZoneServer/Datapad.h"
#include "ZoneServer/HarvesterObject.h"
#include "ZoneServer/Inventory.h"
#include "ZoneServer/ManufacturingSchematic.h"
#include "ZoneServer/MissionBag.h"
#include "ZoneServer/MissionObject.h"
#include "ZoneServer/NPCObject.h"
#include "ZoneServer/ObjectControllerOpcodes.h"
#include "ZoneServer/ObjectFactory.h"
#include "ZoneServer/PlayerObject.h"
#include "ZoneServer/ResourceContainer.h"
#include "ZoneServer/UIOpcodes.h"
#include "ZoneServer/Vehicle.h"
#include "ZoneServer/Wearable.h"
#include "ZoneServer/WorldConfig.h"
#include "ZoneServer/WorldManager.h"
#include "ZoneServer/ZoneOpcodes.h"

#include "LogManager/LogManager.h"

#include "Common/atMacroString.h"
#include "Common/DispatchClient.h"
#include "Common/Message.h"
#include "Common/MessageDispatch.h"
#include "Common/MessageFactory.h"
#include "Common/MessageOpcodes.h"

#include <boost/lexical_cast.hpp>

//======================================================================================================================

bool		MessageLib::mInsFlag    = false;
MessageLib*	MessageLib::mSingleton  = NULL;

//======================================================================================================================

MessageLib::MessageLib()
{
}

//======================================================================================================================

MessageLib*	MessageLib::Init()
{

	if(!mInsFlag)
	{
		mSingleton = new MessageLib();
		mInsFlag = true;
		return mSingleton;
	}
	else
		return mSingleton;
}

//======================================================================================================================

MessageLib::~MessageLib()
{
	mInsFlag = false;
	delete(mSingleton);
}

//======================================================================================================================
//
// Checks the validity of the player in the global map
//


bool MessageLib::_checkPlayer(const PlayerObject* const player) const
{
	if(!player)
		return false;

	PlayerObject* tested = gWorldManager->getPlayerByAccId(player->getAccountId());
	if(!tested)
	{
		gLogger->logMsgF("Player account (%u) invalid",MSG_NORMAL,player->getAccountId());
		return false;
	}

	return((tested->isConnected())&&(tested->getClient()));
}

bool MessageLib::_checkPlayer(uint64 playerId) const
{
	PlayerObject* tested = dynamic_cast <PlayerObject*> (gWorldManager->getObjectById(playerId));
	if(!tested)
	{
		gLogger->logMsgF("Player Id (%I64u) invalid",MSG_NORMAL,playerId);
		return false;
	}
	return((tested->isConnected())&&(tested->getClient()));
}

//======================================================================================================================
//
// broadcasts a message to all players in range of the given object
//

void MessageLib::_sendToInRangeUnreliable(Message* message, Object* const object,uint16 priority,bool toSelf)
{
	PlayerObjectSet*	inRangePlayers	= object->getKnownPlayers();
	PlayerObjectSet::iterator	it				= inRangePlayers->begin();
	// PlayerObjectSet::const_iterator 	it				= inRangePlayers->begin();
	Message*					clonedMessage;

	while(it != inRangePlayers->end())
	{
		if(_checkPlayer((*it)))
		{
			// clone our message
			gMessageFactory->StartMessage();
			gMessageFactory->addData(message->getData(),message->getSize());
			clonedMessage = gMessageFactory->EndMessage();

			((*it)->getClient())->SendChannelAUnreliable(clonedMessage,(*it)->getAccountId(),CR_Client,static_cast<uint8>(priority));
		}

		++it;
	}

	if(toSelf)
	{
		const PlayerObject* const srcPlayer = dynamic_cast<const PlayerObject*>(object);
		if(_checkPlayer(srcPlayer))
		{
			(srcPlayer->getClient())->SendChannelAUnreliable(message,srcPlayer->getAccountId(),CR_Client,static_cast<uint8>(priority));
			return;
		}
	}

	gMessageFactory->DestroyMessage(message);

}

void MessageLib::_sendToInRange(Message* message, Object* const object,uint16 priority,bool toSelf)
{
	PlayerObjectSet*	inRangePlayers	= object->getKnownPlayers();
	PlayerObjectSet::iterator	it			= inRangePlayers->begin();
	Message*					clonedMessage;

	while(it != inRangePlayers->end())
	{
		if(_checkPlayer((*it)))
		{
			// clone our message
			gMessageFactory->StartMessage();
			gMessageFactory->addData(message->getData(),message->getSize());
			clonedMessage = gMessageFactory->EndMessage();

			((*it)->getClient())->SendChannelA(clonedMessage,(*it)->getAccountId(),CR_Client,static_cast<uint8>(priority));
		}

		++it;
	}

	if(toSelf)
	{
		const PlayerObject* const srcPlayer = dynamic_cast<const PlayerObject*>(object);
		if(_checkPlayer(srcPlayer))
		{
			(srcPlayer->getClient())->SendChannelA(message,srcPlayer->getAccountId(),CR_Client, static_cast<uint8>(priority));
			return;
		}
	}

	gMessageFactory->DestroyMessage(message);

}


//======================================================================================================================
//
// Broadcasts a message to players in group and in range of the given object, used by tutorial and other instances
//

void MessageLib::_sendToInstancedPlayers(Message* message,uint16 priority, const PlayerObject* const playerObject) const
{
	if (!_checkPlayer(playerObject))
	{
		gMessageFactory->DestroyMessage(message);
		return;
	}

	PlayerList const inRangeMembers = playerObject->getInRangeGroupMembers(true);
	PlayerList::const_iterator player	= inRangeMembers.begin();
	Message* clonedMessage;

	while (player != inRangeMembers.end())
	{
		if (_checkPlayer(*player))
		{
			// Clone the message.
			gMessageFactory->StartMessage();
			gMessageFactory->addData(message->getData(),message->getSize());
			clonedMessage = gMessageFactory->EndMessage();

			((*player)->getClient())->SendChannelA(clonedMessage,(*player)->getAccountId(),CR_Client,static_cast<uint8>(priority));
		}
		++player;
	}
	gMessageFactory->DestroyMessage(message);
}


//======================================================================================================================
//
// Broadcasts a message to players in group and in range of the given object, used by tutorial and other instances
//

void MessageLib::_sendToInstancedPlayersUnreliable(Message* message,uint16 priority, const PlayerObject* const playerObject) const
{
	if (!_checkPlayer(playerObject))
	{
		gMessageFactory->DestroyMessage(message);
		return;
	}

	PlayerList const inRangeMembers = playerObject->getInRangeGroupMembers(true);
	PlayerList::const_iterator player	= inRangeMembers.begin();
	Message* clonedMessage;

	while (player != inRangeMembers.end())
	{
		if (_checkPlayer(*player))
		{
			// Clone the message.
			gMessageFactory->StartMessage();
			gMessageFactory->addData(message->getData(),message->getSize());
			clonedMessage = gMessageFactory->EndMessage();


			((*player)->getClient())->SendChannelAUnreliable(clonedMessage,(*player)->getAccountId(),CR_Client,static_cast<uint8>(priority));

		}
		++player;
	}
	gMessageFactory->DestroyMessage(message);
}


//======================================================================================================================
//
// broadcasts a message to all players on the current zone
//

void MessageLib::_sendToAll(Message* message,uint16 priority,bool unreliable) const
{
	const PlayerAccMap* const players		= gWorldManager->getPlayerAccMap();
	PlayerAccMap::const_iterator it	= players->begin();
	Message* clonedMessage;

	while(it != players->end())
	{
		const PlayerObject* const player = (*it).second;

		if(_checkPlayer(player))
		{
			gMessageFactory->StartMessage();
			gMessageFactory->addData(message->getData(),message->getSize());
			clonedMessage = gMessageFactory->EndMessage();

			if(unreliable)
				(player->getClient())->SendChannelAUnreliable(clonedMessage,player->getAccountId(),CR_Client,static_cast<uint8>(priority));
			else
				(player->getClient())->SendChannelA(clonedMessage,player->getAccountId(),CR_Client,static_cast<uint8>(priority));
		}

		++it;
	}

	gMessageFactory->DestroyMessage(message);
}

//======================================================================================================================
//
// send creates of the equipped items from player to player
// iterates all inventory items of the source and sends creates to the target
//

bool MessageLib::sendEquippedItems(PlayerObject* srcObject,PlayerObject* targetObject)
{
	if(!_checkPlayer(targetObject))
		return(false);

	ObjectList*				invObjects		= dynamic_cast<Inventory*>(srcObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory))->getEquippedObjects();
	ObjectList::iterator	invObjectsIt	= invObjects->begin();

	while(invObjectsIt != invObjects->end())
	{
		// items
		if(Item* item = dynamic_cast<Item*>(*invObjectsIt))
		{
			if(item->hasInternalAttribute("equipped"))
			{
				if(item->getInternalAttribute<bool>("equipped"))
				{
					gMessageLib->sendCreateTangible(item,targetObject);
				}
				else
				{
					gLogger->logMsgF("MssageLib send equipped objects: Its not equipped ... %I64u",MSG_NORMAL,item->getId());
				}
			}
		}

		++invObjectsIt;
	}

	return(true);
}

//======================================================================================================================
//
// create player
//

bool MessageLib::sendCreatePlayer(PlayerObject* playerObject,PlayerObject* targetObject)
{
	if(!_checkPlayer(targetObject))
		return(false);

	//gLogger->logMsg("sendCreatePlayer",MSG_NORMAL);

	sendCreateObjectByCRC(playerObject,targetObject,false);

	if(targetObject == playerObject)
	{
		sendBaselinesCREO_1(playerObject);
		sendBaselinesCREO_4(playerObject);
	}

	sendBaselinesCREO_3(playerObject,targetObject);
	sendBaselinesCREO_6(playerObject,targetObject);

	sendCreateObjectByCRC(playerObject,targetObject,true);
	sendContainmentMessage(playerObject->getPlayerObjId(),playerObject->getId(),4,targetObject);

	sendBaselinesPLAY_3(playerObject,targetObject);
	sendBaselinesPLAY_6(playerObject,targetObject);

	if(targetObject == playerObject)
	{
		sendBaselinesPLAY_8(playerObject,targetObject);
		sendBaselinesPLAY_9(playerObject,targetObject);
	}

	sendEndBaselines(playerObject->getPlayerObjId(),targetObject);

	sendPostureMessage(playerObject,targetObject);

	if(playerObject->getParentId())
	{
		sendContainmentMessage(playerObject->getId(),playerObject->getParentId(),0xffffffff,targetObject);
	}

	// tangible objects
	if(TangibleObject* hair = dynamic_cast<TangibleObject*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Hair)))
	{
		//only create the hair as the helmet will be created at a different time
		if(hair->getTangibleType() == TanType_Hair)
		{
			sendCreateTangible(hair,targetObject);
		}
	}

	if(targetObject == playerObject)
	{
		// create inventory and contents
		if(dynamic_cast<TangibleObject*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory)))
		{
			sendInventory(playerObject);
		}

		// mission bag
		if(TangibleObject* missionBag = dynamic_cast<TangibleObject*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_MissionBag)))
		{
			gMessageLib->sendCreateTangible(missionBag,playerObject);

			//Now iterate through the missions and create them clientside
			MissionBag* mbag = dynamic_cast<MissionBag*> (missionBag);
			MissionList::iterator it = mbag->getMissions()->begin();
			while(it != mbag->getMissions()->end())
			{
				MissionObject* mission = dynamic_cast<MissionObject*>(*it);
				sendCreateObjectByCRC(mission, targetObject, false);
				sendContainmentMessage(mission->getId(), mbag->getId(), 0xffffffff, targetObject);
				sendBaselinesMISO_3(mission, targetObject);
				sendBaselinesMISO_6(mission, targetObject);
				sendBaselinesMISO_8(mission, targetObject);
				sendBaselinesMISO_9(mission, targetObject);
				sendEndBaselines(mission->getId(), targetObject);
				++it;
			}

		}



		// datapad
		if(TangibleObject* datapad = dynamic_cast<TangibleObject*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Datapad)))
		{
			gMessageLib->sendCreateTangible(datapad,playerObject);

			//now iterate through the schematics and create them clientside
			Datapad* dpad = dynamic_cast<Datapad*> (datapad);

			ManufacturingSchematicList*	manufacturingSchematics = dpad->getManufacturingSchematics();
			ManufacturingSchematicList::iterator it = manufacturingSchematics->begin();

			while(it != manufacturingSchematics->end())
			{
				gMessageLib->sendCreateManufacturingSchematic((dynamic_cast<ManufacturingSchematic*>(*it)),playerObject ,false);
				++it;
			}

			//Send player's intangibles vehicles,pets,droids...etc
			DataList* intangibles = dpad->getData();
			DataList::iterator ite = intangibles->begin();

			while(ite != intangibles->end())
			{
				if(IntangibleObject* itno = dynamic_cast<IntangibleObject*>(*ite))
				{
					gMessageLib->sendCreateObjectByCRC(itno,playerObject,false);
					gMessageLib->sendBaselinesITNO_3(itno,playerObject);
					gMessageLib->sendBaselinesITNO_6(itno,playerObject);
					gMessageLib->sendBaselinesITNO_8(itno,playerObject);
					gMessageLib->sendBaselinesITNO_9(itno,playerObject);
					gMessageLib->sendContainmentMessage(itno->getId(), dpad->getId(), 0xffffffff, playerObject);
					gMessageLib->sendEndBaselines(itno->getId(),playerObject);

					//gWorldManager->addObject(itno,true);

					switch(itno->getItnoGroup())
					{
						case ItnoGroup_Vehicle:
						{
							// set Owner for vehicles
							if(Vehicle* vehicle = dynamic_cast<Vehicle*>(itno))
							{
								vehicle->setOwner(playerObject);
							}
						}
						break;

					default: {} break;
					}
				}

				++ite;
			}




			//Should send accepted missions here

		}


		sendEndBaselines(playerObject->getId(),targetObject);
	}
	else
	{
		sendEndBaselines(playerObject->getId(),targetObject);

		sendEquippedItems(playerObject,targetObject);
	}

	sendUpdatePvpStatus(playerObject,targetObject);

	if(targetObject == playerObject)
	{
		// We are actually sending this info from CharacterLoginHandler::handleDispatchMessage at the opCmdSceneReady event.
		// sendFriendListPlay9(playerObject);
		// sendIgnoreListPlay9(playerObject);

		//request the GRUP baselines from chatserver if grouped
		if(playerObject->getGroupId() != 0)
		{
			gMessageLib->sendIsmGroupBaselineRequest(playerObject);
		}
	}

	//Player mounts
	if(playerObject->checkIfMountCalled())
	{
		if(playerObject->getMount())
		{
			gMessageLib->sendCreateObject(playerObject->getMount(),targetObject);
			if(playerObject->checkIfMounted())
			{
				gMessageLib->sendContainmentMessage(playerObject->getId(), playerObject->getMount()->getId(), 0xffffffff, targetObject);
			}
		}
	}

	return(true);
}

//======================================================================================================================
//
// create creature
//

bool MessageLib::sendCreateCreature(CreatureObject* creatureObject,PlayerObject* targetObject)
{
	if(!_checkPlayer(targetObject))
		return(false);

	sendCreateObjectByCRC(creatureObject,targetObject,false);

	sendBaselinesCREO_3(creatureObject,targetObject);
	sendBaselinesCREO_6(creatureObject,targetObject);

	if(creatureObject->getParentId() && creatureObject->getCreoGroup() != CreoGroup_Vehicle)
	{
		sendContainmentMessage(creatureObject->getId(),creatureObject->getParentId(),0xffffffff,targetObject);
	}

	sendEndBaselines(creatureObject->getId(),targetObject);

	sendUpdatePvpStatus(creatureObject,targetObject);

	sendPostureMessage(creatureObject,targetObject);

	return(true);
}

//======================================================================================================================
//
// create tangible
//

bool MessageLib::sendCreateTangible(TangibleObject* tangibleObject,const PlayerObject* const targetObject) const
{
	//gLogger->logMsgF("MessageLib::send create tangible  %I64u name %s",MSG_HIGH,tangibleObject->getId(),tangibleObject->getName().getAnsi());

	if(!_checkPlayer(targetObject))
		return(false);

	uint64 parentId = tangibleObject->getParentId();

	sendCreateObjectByCRC(tangibleObject,targetObject,false);

	if(parentId != 0)
	{

		// its in a cell, container, inventory
		if(parentId != targetObject->getId())
		{
			// could be inside a crafting tool
			Object* parent = gWorldManager->getObjectById(parentId);

			if(parent && dynamic_cast<CraftingTool*>(parent))
			{
				sendContainmentMessage(tangibleObject->getId(),parentId,0,targetObject);
			}
			// if equipped, also tie it to the object
			else if(tangibleObject->hasInternalAttribute("equipped"))
			{
				Item* item = dynamic_cast<Item*>(tangibleObject);

				if(item->getInternalAttribute<bool>("equipped"))
				{

					// get the parent of inventory
					if(CreatureObject* creatureObject = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById(parentId)))
					{
						sendContainmentMessage(tangibleObject->getId(),creatureObject->getId(),4,targetObject);
					}
					else
					{
						gLogger->logMsgF("Inventory:: cant find parent for equipped item",MSG_NORMAL);
					}
				}
				else
				{
					sendContainmentMessage(tangibleObject->getId(),tangibleObject->getParentId(),0xffffffff,targetObject);
				}
			}
			else
			{
				sendContainmentMessage(tangibleObject->getId(),tangibleObject->getParentId(),0xffffffff,targetObject);
			}
		}
		// or tied directly to an object
		else
		{
			//printf("tied directly to Owner : %I64u",tangibleObject->getParentId());
			sendContainmentMessage(tangibleObject->getId(),tangibleObject->getParentId(),4,targetObject);
		}
	}

	sendBaselinesTANO_3(tangibleObject,targetObject);
	sendBaselinesTANO_6(tangibleObject,targetObject);

	//if(tangibleObject->getTangibleGroup() != TanGroup_Terminal)
	//{
	//sendBaselinesTANO_8(tangibleObject,targetObject);
	//sendBaselinesTANO_9(tangibleObject,targetObject);
	//}

	sendEndBaselines(tangibleObject->getId(),targetObject);

	return(true);
}

//======================================================================================================================
//
// create resource container
//

bool MessageLib::sendCreateResourceContainer(ResourceContainer* resourceContainer,PlayerObject* targetObject)
{
	if(!_checkPlayer(targetObject))
		return(false);

	sendCreateObjectByCRC(resourceContainer,targetObject,false);

	uint64 parentId = resourceContainer->getParentId();


	if(parentId)
	{
		sendContainmentMessage(resourceContainer->getId(),parentId,0xffffffff,targetObject);
		//gLogger->logMsgF("rcno baseline :: parent Id : %I64u",MSG_NORMAL,parentId);
	}

	sendBaselinesRCNO_3(resourceContainer,targetObject);
	sendBaselinesRCNO_6(resourceContainer,targetObject);

	sendBaselinesRCNO_8(resourceContainer,targetObject);
	sendBaselinesRCNO_9(resourceContainer,targetObject);

	sendEndBaselines(resourceContainer->getId(),targetObject);

	return(true);
}

//======================================================================================================================
//
// create building
//

bool MessageLib::sendCreateBuilding(BuildingObject* buildingObject,PlayerObject* playerObject)
{
	if(!_checkPlayer(playerObject))
		return(false);

	sendCreateObjectByCRC(buildingObject,playerObject,false);

	sendBaselinesBUIO_3(buildingObject,playerObject);
	sendBaselinesBUIO_6(buildingObject,playerObject);

	uint64 buildingId = buildingObject->getId();

	CellObjectList*				cellList	= buildingObject->getCellList();
	CellObjectList::iterator	cellIt		= cellList->begin();

	while(cellIt != cellList->end())
	{
		CellObject* cell = (*cellIt);
		uint64 cellId = cell->getId();

		sendCreateObjectByCRC(cell,playerObject,false);
		sendContainmentMessage(cellId,buildingId,0xffffffff,playerObject);
		sendBaselinesSCLT_3(cell,cellId - buildingId,playerObject);
		sendBaselinesSCLT_6(cell,playerObject);
		sendUpdateCellPermissionMessage(cell,1,playerObject);
		sendEndBaselines(cellId,playerObject);

		++cellIt;
	}

	sendEndBaselines(buildingId,playerObject);

	return(true);
}


//======================================================================================================================
//
// create a harvester
//

bool MessageLib::sendCreateHarvester(HarvesterObject* harvester,PlayerObject* player)
{
	if(!_checkPlayer(player))
		return(false);

	//gLogger->logMsgF("MessageLib::sendCreateHarvester:ID %I64u parentId %I64u x : %f   y : %f",MSG_HIGH,harvester->getId(),harvester->getParentId(),harvester->mPosition.mX,harvester->mPosition.mZ);

	sendCreateObjectByCRC(harvester,player,false);

	sendBaselinesHINO_3(harvester,player);
	sendBaselinesHINO_6(harvester,player);


	sendEndBaselines(harvester->getId(),player);

	return(true);
}


//======================================================================================================================
//
// create a harvester
//

bool MessageLib::sendCreateStructure(PlayerStructure* structure,PlayerObject* player)
{
	if(!_checkPlayer(player))
		return(false);

	HarvesterObject* harvester = dynamic_cast<HarvesterObject*>(structure);

	if(harvester)
	{

		return(sendCreateHarvester(harvester, player));
	}

	gLogger->logMsgF("MessageLib::sendCreateStructure:ID %I64u : couldnt cast structure",MSG_HIGH,structure->getId());
	return(false);

}



//======================================================================================================================
//
// create camp
//


bool MessageLib::sendCreateCamp(TangibleObject* camp,PlayerObject* player)
{
	if(!_checkPlayer(player))
		return(false);

	sendCreateObjectByCRC(camp,player,false);

	sendBaselinesBUIO_3(camp,player);
	sendBaselinesBUIO_6(camp,player);

	uint64 campId = camp->getId();


	sendEndBaselines(campId,player);

	return(true);
}

//======================================================================================================================
//
// create Installation
//


bool MessageLib::sendCreateInstallation(PlayerStructure* structure,PlayerObject* player)
{
	if(!_checkPlayer(player))
		return(false);

	sendCreateObjectByCRC(structure,player,false);

	sendBaselinesINSO_3(structure,player);
	sendBaselinesINSO_6(structure,player);

	uint64 structureId = structure->getId();


	sendEndBaselines(structureId,player);

	return(true);
}


//======================================================================================================================
//
// create manufacturing schematic
//

bool MessageLib::sendCreateManufacturingSchematic(ManufacturingSchematic* manSchem,PlayerObject* playerObject,bool attributes)
{
	if(!_checkPlayer(playerObject))
		return(false);

	sendCreateObjectByCRC(manSchem,playerObject,false);

	// parent should always be a crafting tool for now
	sendContainmentMessage(manSchem->getId(),manSchem->getParentId(),4,playerObject);

	sendBaselinesMSCO_3(manSchem,playerObject,attributes);
	sendBaselinesMSCO_6(manSchem,playerObject);

	sendBaselinesMSCO_8(manSchem,playerObject);
	sendBaselinesMSCO_9(manSchem,playerObject);

	sendEndBaselines(manSchem->getId(),playerObject);

	return(true);
}

//======================================================================================================================
//
// create the inventory contents for its owner
//

void MessageLib::sendInventory(PlayerObject* playerObject)
{
	if(!_checkPlayer(playerObject))
		return;

	Inventory*	inventory	= dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
	//uint64		parentId	= inventory->getParentId();

	inventory->setTypeOptions(256);
	//gLogger->logMsgF("MessageLib::inventory: ID %I64u parentId %I64u",MSG_HIGH,inventory->getId(),inventory->getParentId());

	// create the inventory
	sendCreateObjectByCRC(inventory,playerObject,false);
	sendContainmentMessage(inventory->getId(),inventory->getParentId(),4,playerObject);
	sendBaselinesTANO_3(inventory,playerObject);
	sendBaselinesTANO_6(inventory,playerObject);

	sendEndBaselines(inventory->getId(),playerObject);

	// create objects contained
	ObjectList* invObjects		= inventory->getObjects();
	ObjectList::iterator objIt	= invObjects->begin();


	//todo separate objects who are contained by inventory and by the player!!!!!!!!!!!!!!!!!!!!

	//done this is the inventory list - equipped items follow later
	while(objIt != invObjects->end())
	{
		if(ResourceContainer* resContainer = dynamic_cast<ResourceContainer*>(*objIt))
		{
			//gLogger->logMsgF("MessageLib::inventory:Resource %I64u parentId %I64u",MSG_HIGH,resContainer->getId(),resContainer->getParentId());
			sendCreateResourceContainer(resContainer,playerObject);
		}
		else
			if(TangibleObject* tangible = dynamic_cast<TangibleObject*>(*objIt))
			{
				//gLogger->logMsgF("MessageLib::inventory:Tangible  %I64u parentId %I64u",MSG_HIGH,tangible->getId(),tangible->getParentId());
				sendCreateTangible(tangible,playerObject);
			}

			++objIt;
	}

	invObjects		= inventory->getEquippedObjects();
	objIt			= invObjects->begin();

	while(objIt != invObjects->end())
	{
		if(TangibleObject* tangible = dynamic_cast<TangibleObject*>(*objIt))
		{
			//gLogger->logMsgF("MessageLib::inventory: equipped tangible %I64u parentId %I64u",MSG_HIGH,tangible->getId(),tangible->getParentId());
			sendCreateTangible(tangible,playerObject);
		}

		++objIt;
	}

	//gLogger->logMsgF("MessageLib::inventory: end",MSG_HIGH);
}

//======================================================================================================================
//
// send the matching object creates
//

void MessageLib::sendCreateObject(Object* object,PlayerObject* player,bool sendSelftoTarget)
{
	switch(object->getType())
	{
		case ObjType_NPC:
		// creatures
		case ObjType_Creature:
		{
			/*
			if (!gWorldConfig->isInstance())
			{
			if(CreatureObject* targetCreature = dynamic_cast<CreatureObject*>(object))
			{
			gMessageLib->sendCreateCreature(targetCreature,player);
			}
			}
			else
			{
			// If it's a creature owned by me or my group I want to see it.
			if (CreatureObject* targetCreature = dynamic_cast<CreatureObject*>(object))
			{
			if (targetCreature->getPrivateOwner())
			{
			if (targetCreature->isOwnedBy(player))
			{
			gMessageLib->sendCreateCreature(targetCreature,player);
			}
			}
			else
			{
			// No owner.. standard creature
			gMessageLib->sendCreateCreature(targetCreature,player);
			}
			}
			}
			*/
			// If it's a creature owned by me or my group I want to see it.
			if (CreatureObject* targetCreature = dynamic_cast<CreatureObject*>(object))
			{
				if (targetCreature->getPrivateOwner())
				{
					if (targetCreature->isOwnedBy(player))
					{
						gMessageLib->sendCreateCreature(targetCreature,player);
					}
				}
				else
				{
					// No owner.. a "normal" creature
					gMessageLib->sendCreateCreature(targetCreature,player);
				}
			}
		}
		break;

		// players
		case ObjType_Player:
		{
			// send creates to each other
			if (!gWorldConfig->isInstance())
			{
				if(PlayerObject* targetPlayer = dynamic_cast<PlayerObject*>(object))
				{
					if(sendSelftoTarget)
					{
						gMessageLib->sendCreatePlayer(player,targetPlayer);
					}

					gMessageLib->sendCreatePlayer(targetPlayer,player);
				}
			}
			else
			{
				if (PlayerObject* targetPlayer = dynamic_cast<PlayerObject*>(object))
				{
					// Update players in instanced group only.
					if (targetPlayer->getGroupId())
					{
						if (targetPlayer->getGroupId() == player->getGroupId())
						{
							if(sendSelftoTarget)
							{
								gMessageLib->sendCreatePlayer(player,targetPlayer);
							}
							gMessageLib->sendCreatePlayer(targetPlayer,player);
						}
					}
				}
			}
		}
		break;

		// tangibles
		case ObjType_Tangible:
		{
			// skip, if its static
#if defined(_MSC_VER)
			if(object->getId() > 0x0000000100000000)
#else
			if(object->getId() > 0x0000000100000000LLU)
#endif
			{
				if(TangibleObject* tangibleObject = dynamic_cast<TangibleObject*>(object))
				{
					// resource containers
					if(ResourceContainer* resContainer = dynamic_cast<ResourceContainer*>(tangibleObject))
					{
						gMessageLib->sendCreateResourceContainer(resContainer,player);
					}
					// all other tangibles
					else
					{
						gMessageLib->sendCreateTangible(tangibleObject,player);
					}
				}
			}
		}
		break;

		// buildings
		case ObjType_Building:
		{
			// skip, if its static
#if defined(_MSC_VER)
			if(object->getId() > 0x0000000100000000)
#else
			if(object->getId() > 0x0000000100000000LLU)
#endif
			{
				if(BuildingObject* building = dynamic_cast<BuildingObject*>(object))
				{
					gMessageLib->sendCreateBuilding(building,player);
				}
			}
		}
		break;

		// buildings
		case ObjType_Harvester:
		{
			// skip, if its static
#if defined(_MSC_VER)
			if(object->getId() > 0x0000000100000000)
#else
			if(object->getId() > 0x0000000100000000LLU)
#endif
			{
				if(HarvesterObject* harvester = dynamic_cast<HarvesterObject*>(object))
				{
					gMessageLib->sendCreateHarvester(harvester,player);
				}
			}
		}
		break;

		// unknown types
	default:
		{
			gLogger->logMsgF("MessageLib::createObject: Unhandled object type: %i",MSG_HIGH,object->getType());
		}
		break;
	}
}

//======================================================================================================================



