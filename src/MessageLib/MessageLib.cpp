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

#include <algorithm>

#include <boost/lexical_cast.hpp>


#include "Utils/logger.h"

#include "Common/atMacroString.h"
#include "Common/Crc.h"

#include "NetworkManager/DispatchClient.h"
#include "NetworkManager/Message.h"
#include "NetworkManager/MessageDispatch.h"
#include "NetworkManager/MessageFactory.h"
#include "NetworkManager/MessageOpcodes.h"

#include "ZoneServer/BuildingObject.h"
#include "ZoneServer/CellObject.h"
#include "ZoneServer/CharSheetManager.h"
#include "ZoneServer/ContainerManager.h"
#include "ZoneServer/Conversation.h"
#include "ZoneServer/CraftingTool.h"
#include "ZoneServer/CurrentResource.h"
#include "ZoneServer/Datapad.h"
#include "ZoneServer/HouseObject.h"
#include "ZoneServer/IntangibleObject.h"
#include "ZoneServer/HarvesterObject.h"
#include "ZoneServer/FactoryObject.h"
#include "ZoneServer/FactoryCrate.h"
#include "ZoneServer/Inventory.h"
#include "ZoneServer/ManufacturingSchematic.h"

#include "ZoneServer/NPCObject.h"
#include "ZoneServer/ObjectControllerOpcodes.h"
#include "ZoneServer/ObjectFactory.h"
#include "ZoneServer/PlayerObject.h"
#include "ZoneServer/ResourceContainer.h"
#include "ZoneServer/Tutorial.h"
#include "ZoneServer/UIOpcodes.h"
#include "ZoneServer/VehicleController.h"
#include "ZoneServer/Wearable.h"
#include "ZoneServer/WorldConfig.h"
#include "ZoneServer/WorldManager.h"

#include "ZoneServer/ZoneOpcodes.h"
#include "ZoneServer/Zmap.h"


//======================================================================================================================

bool		MessageLib::mInsFlag    = false;
MessageLib*	MessageLib::mSingleton  = NULL;

//======================================================================================================================

MessageLib::MessageLib()
{
    mMessageFactory = gMessageFactory;
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
    //player gets PlayerConnState_LinkDead when he disconnects but is still in the world
    //we in theory could still send updates
    //return((player->isConnected())&&(player->getClient()));

    //the idea is that this check gets useless when the SI / knownobjectscode is stable

    return((player)&&(player->getClient()));
}

//======================================================================================================================

bool MessageLib::_checkPlayer(uint64 playerId) const
{
    PlayerObject* tested = dynamic_cast <PlayerObject*> (gWorldManager->getObjectById(playerId));

    if(!tested)
    {
        LOG(WARNING) << "Invalid player id [" << playerId << "]";
        return false;
    }

    return((tested->isConnected())&&(tested->getClient()));
}

//================================================================================================0
//send movement based on messageheap size and distance
bool MessageLib::_checkDistance(const glm::vec3& mPosition1, Object* object, uint32 heapWarningLevel)
{

    //just send everything we have
    if(heapWarningLevel < 4)
        return true;
    else if (heapWarningLevel < 6)
    {
        if(glm::distance(object->mPosition, mPosition1) < 96)
            return object->movementMessageToggle();
    }
    else if (heapWarningLevel < 8)
    {
        if(glm::distance(object->mPosition, mPosition1) < 64)
            return object->movementMessageToggle();
    }
    else if (heapWarningLevel < 10)
    {
        float distance = glm::distance(object->mPosition, mPosition1);
        if(distance <= 32)
            return true;
        else if(distance > 32)
            return object->movementMessageToggle();
        else if(distance > 64)
            return false;
    }
    else if (heapWarningLevel >= 10)
        return false;




    return false;
}



//======================================================================================================================
//
// broadcasts a message to all players in range of the given player
// we use our registered playerlist here so it will be pretty fast :)
void MessageLib::_sendToInRangeUnreliable(Message* message, Object* const object, unsigned char priority, bool to_self) {
    gContainerManager->sendToRegisteredPlayers(object, [=] (PlayerObject* const recipient) {
        //thats something for debugmode only
        if(!_checkPlayer(recipient)) {
            //an invalid player at this point is like armageddon and Ultymas birthday combined at one time
            //if this happens we need to know about it
            assert(false && "Invalid Player in sendtoInrange");
            return;
        }

        if(_checkDistance(recipient->mPosition, object, mMessageFactory->HeapWarningLevel())) {
            // clone our message
            mMessageFactory->StartMessage();
            mMessageFactory->addData(message->getData(),message->getSize());

            recipient->getClient()->SendChannelAUnreliable(mMessageFactory->EndMessage(), recipient->getAccountId(), CR_Client, priority);
        }
    });

    if(to_self) {
        const PlayerObject* const player = dynamic_cast<const PlayerObject*>(object);

        if(_checkPlayer(player)) {
            player->getClient()->SendChannelAUnreliable(message, player->getAccountId(), CR_Client, priority);
        }
		else
			mMessageFactory->DestroyMessage(message);
    } else {
        // If the message is sent to self then the process of sending destroys it
        // otherwise we have to destroy it ourselves.
        mMessageFactory->DestroyMessage(message);
    }
}

void MessageLib::_sendToInRangeUnreliableChat(Message* message, const CreatureObject* object, unsigned char priority, uint32_t crc) {
    ObjectListType in_range_players;
    mGrid->GetChatRangeCellContents(object->getGridBucket(), &in_range_players);

    Message* cloned_message;

    std::for_each(in_range_players.begin(), in_range_players.end(), [=, &cloned_message] (Object* object) {
        PlayerObject* player = dynamic_cast<PlayerObject*>(object);

        if(_checkPlayer(player) && (!player->checkIgnoreList(crc))) {
            // clone our message
            mMessageFactory->StartMessage();
            mMessageFactory->addData(message->getData(), message->getSize());
            cloned_message = mMessageFactory->EndMessage();

            // replace the target id
            char* data = cloned_message->getData() + 12;
            *(reinterpret_cast<uint64_t*>(data)) = player->getId();
            player->getClient()->SendChannelAUnreliable(cloned_message, player->getAccountId(), CR_Client, priority);
        }
    });

    mMessageFactory->DestroyMessage(message);
}

void MessageLib::SendSpatialToInRangeUnreliable_(Message* message, Object* const object, PlayerObject* const player_object) {
    uint32_t senders_name_crc = 0;
    PlayerObject* source_player = NULL;

    // Is this a player object sending the message? If so we need a crc of their name
    // for checking recipient's ignore lists.
    if (object->getType() == ObjType_Player) {
        if ((source_player = dynamic_cast<PlayerObject*>(object))) {
            // Make sure the player is valid and online.
            if (!_checkPlayer(source_player) || !source_player->isConnected()) {
                // This is an invalid player, clean up the message and exit.
                assert(false && "MessageLib::SendSpatialToInRangeUnreliable Message sent from an invalid player, this should never happen");
                mMessageFactory->DestroyMessage(message);
                return;
            }

            std::string tmp = source_player->getFirstName().getAnsi();
            std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);
            senders_name_crc = common::memcrc(tmp);

            //@todo: This check for the tutorial is a hack and shouldn't be here.
            if (gWorldConfig->isTutorial()) {
                source_player->getTutorial()->tutorialResponse("chatActive");
            }
        }
    }

    // Create a container for cloned messages.
    // @todo: We shouldn't have to create whole copies of messages that are (with exception of the
    // target id) the exact same. This is a waste of memory especially with denser populations.
    Message* cloned_message = NULL;

    // Create our lambda that we'll use to handle the inrange sending
    auto send_rule = [=, &cloned_message ] (PlayerObject* const recipient) {
        // If the player is not online, or if the sender is in the player's ignore list
        // then pass over this iteration.
        if (!_checkPlayer(recipient) || (senders_name_crc && recipient->checkIgnoreList(senders_name_crc))) {
            //mMessageFactory->DestroyMessage(message);
            return;
        }

        // Clone the message and send it out to this player.
        mMessageFactory->StartMessage();
        mMessageFactory->addData(message->getData(), message->getSize());
        cloned_message = mMessageFactory->EndMessage();

        // Replace the target id.
        char* data = cloned_message->getData() + 12;
        *(reinterpret_cast<uint64_t*>(data)) = recipient->getId();

        recipient->getClient()->SendChannelAUnreliable(cloned_message, recipient->getAccountId(), CR_Client, 5);
    };

    // If no player_object is passed it means this is not an instance, send to the known
    // players of the object initiating the spatial message.
    //
    // @todo For now this is how we have to deal with instances, the whole instance system
    // needs to be redone and when it does it will simplify these types of functions.
    if (!player_object) {
        // Loop through the in range players and send them the message.
        gSpatialIndexManager->sendToChatRange(object, send_rule);
    } else {
        // This is an instance message, so only send it out to known players in the instance.
        gContainerManager->sendToGroupedRegisteredPlayers(player_object, send_rule, true);
    }

    mMessageFactory->DestroyMessage(message);
}

void MessageLib::_sendToInRangeUnreliableChatGroup(Message* message, const CreatureObject* object, unsigned char priority, uint32_t crc) {
    glm::vec3 position = object->getWorldPosition();

    ObjectListType in_range_players;
    mGrid->GetPlayerViewingRangeCellContents(mGrid->getCellId(position.x, position.z), &in_range_players);

    Message* cloned_message;
    bool failed = false;

    std::for_each(in_range_players.begin(), in_range_players.end(), [=, &cloned_message] (Object* iter_object) {
        PlayerObject* player = static_cast<PlayerObject*>(iter_object);

        if(_checkPlayer(player) && object->getGroupId()
                && (player->getGroupId() == object->getGroupId())
        && !player->checkIgnoreList(crc)) {
            // clone our message
            mMessageFactory->StartMessage();
            mMessageFactory->addData(message->getData(),message->getSize());
            cloned_message = mMessageFactory->EndMessage();

            // replace the target id
            char* data = cloned_message->getData() + 12;
            *(reinterpret_cast<uint64_t*>(data)) = player->getId();
            player->getClient()->SendChannelAUnreliable(cloned_message, player->getAccountId(), CR_Client, priority);
        }
    });

	mMessageFactory->DestroyMessage(message);
}

//======================================================================================================================

void MessageLib::_sendToInRange(Message* message, Object* const object, unsigned char priority, bool to_self) const {
    glm::vec3 position = object->getWorldPosition();

    ObjectListType in_range_players;
	mGrid->GetPlayerViewingRangeCellContents(object->getGridBucket(), &in_range_players);

    std::for_each(in_range_players.begin(), in_range_players.end(), [=] (Object* object) {
        PlayerObject* player = static_cast<PlayerObject*>(object);
        if(_checkPlayer(player)) {
            // clone our message
            mMessageFactory->StartMessage();
            mMessageFactory->addData(message->getData(),message->getSize());

            player->getClient()->SendChannelA(mMessageFactory->EndMessage(), player->getAccountId(), CR_Client, priority);
        }
    });

    if(to_self) {
        const PlayerObject* const player = dynamic_cast<const PlayerObject*>(object);

        if(_checkPlayer(player)) {
            player->getClient()->SendChannelA(message, player->getAccountId(), CR_Client, priority);
        }
		else
			mMessageFactory->DestroyMessage(message);
    } else {
        // If the message is sent to self then the process of sending destroys it
        // otherwise we have to destroy it ourselves.
        mMessageFactory->DestroyMessage(message);
    }
}

//======================================================================================================================
//
// Broadcasts a message to players in group and in range of the given object, used by tutorial and other instances
//
void MessageLib::_sendToInstancedPlayers(Message* message, unsigned char priority, PlayerObject* const player) const {
    if (!_checkPlayer(player)) {
        mMessageFactory->DestroyMessage(message);
        return;
    }

    glm::vec3 position = player->getWorldPosition();

    ObjectListType in_range_players;
    mGrid->GetPlayerViewingRangeCellContents(mGrid->getCellId(position.x, position.z), &in_range_players);

    std::for_each(in_range_players.begin(), in_range_players.end(), [=] (Object* object) {
        PlayerObject* player = dynamic_cast<PlayerObject*>(object);
        if (_checkPlayer(player)) {
            // Clone the message.
            mMessageFactory->StartMessage();
            mMessageFactory->addData(message->getData(),message->getSize());

            (player->getClient())->SendChannelA(mMessageFactory->EndMessage(), player->getAccountId(), CR_Client, priority);
        }
    });

    mMessageFactory->DestroyMessage(message);
}
//======================================================================================================================
//
// Broadcasts a message to players in group and in range of the given object, used by tutorial and other instances
//
void MessageLib::_sendToInstancedPlayersUnreliable(Message* message, unsigned char priority, const PlayerObject* const player) const {
    // If the player is not valid or not in a group there is no point in
    // going further.
    if (!_checkPlayer(player) || player->getGroupId() == 0) {
        mMessageFactory->DestroyMessage(message);
        return;
    }

    ObjectListType in_range_players;
	mGrid->GetPlayerViewingRangeCellContents(player->getGridBucket(), &in_range_players);

    std::for_each(in_range_players.begin(), in_range_players.end(), [=] (Object* object) {
        PlayerObject* in_range_player = static_cast<PlayerObject*>(object);

        if((player->getGroupId() != 0) && (in_range_player->getGroupId() != player->getGroupId())) {
            assert(false && "Player has an invalid player in range in an instance");
            return;
        }

        if (_checkPlayer(in_range_player)) {
            // Clone the message.
            mMessageFactory->StartMessage();
            mMessageFactory->addData(message->getData(),message->getSize());

            in_range_player->getClient()->SendChannelAUnreliable(
                mMessageFactory->EndMessage(),
                in_range_player->getAccountId(), CR_Client, priority);
        }
    });

    mMessageFactory->DestroyMessage(message);
}

//======================================================================================================================
//
// broadcasts a message to all players on the current zone
//
void MessageLib::_sendToAll(Message* message, unsigned char priority, bool unreliable) const {
    const PlayerAccMap* const players = gWorldManager->getPlayerAccMap();

    std::for_each(players->begin(), players->end(), [=] (const std::pair<uint32_t, const PlayerObject*>& element) {
        const PlayerObject* const player = element.second;

        if(_checkPlayer(player)) {
            mMessageFactory->StartMessage();
            mMessageFactory->addData(message->getData(),message->getSize());

            if(unreliable) {
                player->getClient()->SendChannelAUnreliable(
                    mMessageFactory->EndMessage(),
                    player->getAccountId(), CR_Client, priority);
            } else {
                player->getClient()->SendChannelA(
                    mMessageFactory->EndMessage(),
                    player->getAccountId(), CR_Client, priority);
            }
        }
    });

    mMessageFactory->DestroyMessage(message);
}


bool MessageLib::sendCreatePlayer(PlayerObject* player, PlayerObject* target) {
    if (!_checkPlayer(player) || !_checkPlayer(target)) {
        return false;
    }

    sendCreateObjectByCRC(player, target, false);

    if (player == target) {
        sendBaselinesCREO_1(player);
        sendBaselinesCREO_4(player);
    }

    sendBaselinesCREO_3(player, target);
    sendBaselinesCREO_6(player, target);

    sendCreateObjectByCRC(player, target, true);
    sendContainmentMessage(player->getPlayerObjId(), player->getId(), 4, target);

    sendBaselinesPLAY_3(player, target);
    sendBaselinesPLAY_6(player, target);

    if (player == target) {
        sendBaselinesPLAY_8(player, target);
        sendBaselinesPLAY_9(player, target);
    }

    //close the yalp
    sendEndBaselines(player->getPlayerObjId(), target);

    sendPostureMessage(player, target);

    if (player->getParentId()) {
        sendContainmentMessage(player->getId(), player->getParentId(), 4, target);
    }

    //===================================================================================
    // create inventory, datapad, hair, MissionBag and equipped items get created for the player only !!
    // equipped items for other watchers are handled via the equiplists

    //equipped items are already in the creo6 so only send them for ourselves

    sendEndBaselines(player->getId(), target);

    sendUpdatePvpStatus(player, target);

    if (player == target) {
        //request the GRUP baselines from chatserver if grouped
        if (player->getGroupId() != 0) {
            gMessageLib->sendIsmGroupBaselineRequest(player);
        }
    }

    return true;
}

//======================================================================================================================
//
// create creature
//
bool MessageLib::sendCreateCreature(CreatureObject* creature, PlayerObject* target) {
    if (!_checkPlayer(target)) {
        return false;
    }

    sendCreateObjectByCRC(creature, target, false);

    sendBaselinesCREO_3(creature, target);
    sendBaselinesCREO_6(creature, target);

    if(creature->getParentId() && creature->getCreoGroup() != CreoGroup_Vehicle)
    {
        sendContainmentMessage(creature->getId(), creature->getParentId(), 0xffffffff, target);
    }

    sendEndBaselines(creature->getId(), target);

    sendUpdatePvpStatus(creature, target);

    sendPostureMessage(creature, target);

    return true;
}
//======================================================================================================================

bool MessageLib::sendCreateStaticObject(TangibleObject* tangible, PlayerObject* target) {
    if(!_checkPlayer(target) || !tangible) {
        DLOG(INFO) << "MessageLib::sendCreateStaticObject No valid player";
        return(false);
    }

    sendCreateObjectByCRC(tangible, target, false);
    sendBaselinesSTAO_3(tangible, target);
    sendBaselinesSTAO_6(tangible, target);
    sendEndBaselines(tangible->getId(), target);

    return true;
}

//======================================================================================================================
//
// create intangible
//
bool MessageLib::sendCreateInTangible(IntangibleObject* intangibleObject,uint64 containmentId,PlayerObject* targetObject)
{
    if(!_checkPlayer(targetObject) || !intangibleObject)
    {
        DLOG(WARNING) << "MessageLib::sendCreateInTangible No valid player";
        return(false);
    }

    gMessageLib->sendCreateObjectByCRC(intangibleObject,targetObject,false);
    gMessageLib->sendBaselinesITNO_3(intangibleObject,targetObject);
    gMessageLib->sendBaselinesITNO_6(intangibleObject,targetObject);
    gMessageLib->sendBaselinesITNO_8(intangibleObject,targetObject);
    gMessageLib->sendBaselinesITNO_9(intangibleObject,targetObject);
    gMessageLib->sendContainmentMessage(intangibleObject->getId(), containmentId, 0xffffffff, targetObject);
    gMessageLib->sendEndBaselines(intangibleObject->getId(),targetObject);

    return true;
}

//======================================================================================================================
//
// create tangible Object in the world
//
bool MessageLib::sendCreateTano(TangibleObject* tangible, PlayerObject* target) {
    if (!_checkPlayer(target))	{
        DLOG(INFO) << "MessageLib::sendCreateTano No valid player";
        return false;
    }

    uint64 parentId = tangible->getParentId();

    sendCreateObjectByCRC(tangible, target, false);

    if(parentId != 0) {
        // its in a cell, container, inventory
        if (parentId != target->getId()) {
            // could be inside a crafting tool
            Object* parent = gWorldManager->getObjectById(parentId);

            if (parent && dynamic_cast<CraftingTool*>(parent)) {
                sendContainmentMessage(tangible->getId(), parentId, 0, target);
            }
            // if equipped, also tie it to the object
            else if (CreatureObject* creature = dynamic_cast<CreatureObject*>(parent)) {
                sendContainmentMessage(tangible->getId(), creature->getId(), 4, target);
            } else {
                sendContainmentMessage(tangible->getId(), tangible->getParentId(), 0xffffffff, target);
            }
        }
        // or tied directly to an object
        else {
            sendContainmentMessage(tangible->getId(), tangible->getParentId(), 4, target);
        }
    } else {
        sendContainmentMessage(tangible->getId(), tangible->getParentId(), 0xffffffff, target);
    }

    sendBaselinesTANO_3(tangible, target);
    sendBaselinesTANO_6(tangible, target);

    sendEndBaselines(tangible->getId(), target);

    return true;
}

//======================================================================================================================
//
// create resource container
//
bool MessageLib::sendCreateResourceContainer(ResourceContainer* resource_container, PlayerObject* target) {
    if(!_checkPlayer(target)) {
        return false;
    }

    sendCreateObjectByCRC(resource_container, target, false);

    uint64_t parent_id = resource_container->getParentId();

    sendContainmentMessage(resource_container->getId(), parent_id, 0xffffffff, target);

    sendBaselinesRCNO_3(resource_container, target);
    sendBaselinesRCNO_6(resource_container, target);

    sendBaselinesRCNO_8(resource_container, target);
    sendBaselinesRCNO_9(resource_container, target);

    sendEndBaselines(resource_container->getId(), target);

    return true;
}

//======================================================================================================================
//
// create building
//
bool MessageLib::sendCreateBuilding(BuildingObject* buildingObject,PlayerObject* playerObject)
{
    if(!_checkPlayer(playerObject))
        return(false);

    bool publicBuilding = true;

    //test buildings on house basis here
    //perhaps move to on cell basis sometime ?
    if(HouseObject* house = dynamic_cast<HouseObject*>(buildingObject))
    {
        house->checkCellPermission(playerObject);
        publicBuilding = buildingObject->getPublic();
    }

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

        uint64 count = buildingObject->getMinCellId()-1;
        sendCreateObjectByCRC(cell,playerObject,false);
        sendContainmentMessage(cellId,buildingId,0xffffffff,playerObject);

        //cell ids are id based for tutorial cells!
        if(cell->getId() <= 2203318222975)
        {
            sendBaselinesSCLT_3(cell,cellId - buildingId,playerObject);
        }
        else
        {
            sendBaselinesSCLT_3(cell,cellId - count,playerObject);
        }
        sendBaselinesSCLT_6(cell,playerObject);

        sendUpdateCellPermissionMessage(cell,publicBuilding,playerObject);	 //cellpermissions get checked by datatransform
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

    sendCreateObjectByCRC(harvester,player,false);

    sendBaselinesHINO_3(harvester,player);
    sendBaselinesHINO_6(harvester,player);
    sendBaselinesHINO_7(harvester,player);


    sendEndBaselines(harvester->getId(),player);

    //int8 effectStr[400];
    //sprintf(effectStr,"clienteffect/lair_med_damage_smoke.cef");
    //sendPlayClientEffectObjectMessage(effectStr,"",harvester,player);

    return(true);
}

//======================================================================================================================
//
// create a factory
//
bool MessageLib::sendCreateFactory(FactoryObject* factory, PlayerObject* target) {
    if (!_checkPlayer(target))
        return false;

    sendCreateObjectByCRC(factory, target, false);

    sendBaselinesINSO_3(factory, target);
    sendBaselinesINSO_6(factory, target);

    TangibleObject* ingredient_hopper = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById(factory->getIngredientHopper()));
    sendCreateTano(ingredient_hopper, target);

    TangibleObject* output_hopper = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById(factory->getOutputHopper()));
    sendCreateTano(output_hopper, target);


    sendEndBaselines(factory->getId(), target);

    return true;
}


bool MessageLib::sendCreateStructure(PlayerStructure* structure, PlayerObject* target) {
    if (!_checkPlayer(target)) {
        return false;
    }

    if (HarvesterObject* harvester = dynamic_cast<HarvesterObject*>(structure)) {
        return sendCreateHarvester(harvester, target);
    }

    else if (HouseObject* house = dynamic_cast<HouseObject*>(structure)) {
        return sendCreateBuilding(house, target);
    }

    else if (FactoryObject* factory = dynamic_cast<FactoryObject*>(structure)) {
        return sendCreateFactory(factory, target);
    }

    else if (structure->getPlayerStructureFamily() == PlayerStructure_Fence) {
        return sendCreateInstallation(structure, target);
    }

    DLOG(INFO) << "MessageLib::sendCreateStructure:ID  : couldnt cast structure" << structure->getId();

    return false;
}


bool MessageLib::sendCreateCamp(TangibleObject* camp, PlayerObject* target) {
    if(!_checkPlayer(target)) {
        return false;
    }

    sendCreateObjectByCRC(camp, target, false);

    sendBaselinesBUIO_3(camp, target);
    sendBaselinesBUIO_6(camp, target);

    sendEndBaselines(camp->getId(), target);

    return true;
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
// the attributes bool makes the MSCO 3 send the attribute list
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

