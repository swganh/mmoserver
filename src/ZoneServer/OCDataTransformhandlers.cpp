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

#include "ZoneServer/ObjectController.h"

#include <cassert>

#include "Utils/clock.h"

//#include "DatabaseManager/Database.h"
//#include "DatabaseManager/DatabaseResult.h"
//#include "DatabaseManager/DataBinding.h"

#include "NetworkManager/Message.h"
#include "NetworkManager/MessageFactory.h"

#include "MessageLib/MessageLib.h"

#include "ZoneServer/ActiveConversation.h"
#include "ZoneServer/BuildingObject.h"
#include "ZoneServer/CellObject.h"
#include "ZoneServer/ContainerManager.h"
#include "ZoneServer/ConversationManager.h"
#include "ZoneServer/Heightmap.h"
#include "ZoneServer/HouseObject.h"
#include "ZoneServer/NPCObject.h"
#include "ZoneServer/ObjectController.h"
#include "ZoneServer/ObjectControllerOpcodes.h"
#include "ZoneServer/ObjectControllerCommandMap.h"
#include "ZoneServer/PlayerObject.h"
#include "ZoneServer/Tutorial.h"
#include "ZoneServer/VehicleController.h"
#include "ZoneServer/WorldConfig.h"
#include "ZoneServer/WorldManager.h"
#include "ZoneServer/Zmap.h"


//=============================================================================
//
// position update in world
//

void ObjectController::handleDataTransform(Message* message,bool inRangeUpdate)
{
    PlayerObject*			player = dynamic_cast<PlayerObject*>(mObject);

    glm::vec3		pos;
    glm::quat       dir;
    uint32			inMoveCount;
    uint32			tickCount;
    float			speed;
    bool updateAll = false;

    // get tick and move counters
    tickCount	= message->getUint32();
    inMoveCount = message->getUint32();

    // only process if its in sequence
    if(player->getInMoveCount() > inMoveCount)	{
        return;
    }

    // update tick and move counters...
    player->setLastMoveTick(tickCount);
    player->setClientTickCount(tickCount);

    player->setInMoveCount(inMoveCount);

    // get new direction, position and speed
    dir.x = message->getFloat();
    dir.y = message->getFloat();
    dir.z = message->getFloat();
    dir.w = message->getFloat();

    pos.x = message->getFloat();
    pos.y = message->getFloat();
    pos.z = message->getFloat();
    speed  = message->getFloat();

    // stop entertaining ???
    // important is, that if we move we change our posture to NOT skill animating anymore!
    // so only stop entertaining when we are performing and NOT skillanimationg
    if(player->getPerformingState() != PlayerPerformance_None && player->states.getPosture() != CreaturePosture_SkillAnimating)	{
        gEntertainerManager->stopEntertaining(player);
    }

    // if we just left a building
    // note that we remain in the grid at the worldposition
    if(player->getParentId() != 0)
    {

        // Testing with 4 for add and 0 for remove.
        // Remove us from previous cell.
        gMessageLib->broadcastContainmentMessage(player->getId(),player->getParentId(),0,player);

        // remove us from the last cell we were in
        CellObject* cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(player->getParentId()));
        if(cell)
        {
            cell->removeObject(player);
        }
        else
        {
            assert(false && "cell not found");
        }

        // we are outside again
        player->setParentId(0);

        // Add us to the world.
        gMessageLib->broadcastContainmentMessage(player->getId(),0,4,player);

        // Inform tutorial about cell change.
        if (gWorldConfig->isTutorial())
        {
            player->getTutorial()->setCellId(0);
        }

    }

    player->mDirection = dir;
    player->setCurrentSpeed(speed);
    player->mPosition = pos;

    gSpatialIndexManager->UpdateObject(player);

    // destroy the instanced instrument if out of range
    if (player->getPlacedInstrumentId())
    {
        if (!gWorldManager->objectsInRange(player->getId(), player->getPlacedInstrumentId(), 5.0))
        {
            if (Item* item = dynamic_cast<Item*>(gWorldManager->getObjectById(player->getPlacedInstrumentId())))
            {
                destroyObject(item->getId());
            }
        }
    }

    // Terminate active conversation with npc if to far away (trainers only so far).
    ActiveConversation* ac = gConversationManager->getActiveConversation(player->getId());
    if (ac != NULL)
    {
        // We do have a npc conversation going.
        if (!gWorldManager->objectsInRange(player->getId(), (ac->getNpc())->getId(), 11.0))
        {
            // Terminate conversation, since we are out of range.
            gMessageLib->SendSystemMessage(L"",player,"system_msg","out_of_range");
            gConversationManager->stopConversation(player, true);			// We will get the current dialog text in a chat bubble, only seen by me. Impressive :)
        }
    }

    if (gWorldConfig->isInstance())
    {
        // send out position updates to known players in group or self only
        gMessageLib->sendUpdateTransformMessage(player, player);
        return;
    }	

    //If player is mounted... move his mount too!
    if(player->checkIfMounted() && player->getMount())
    {
        //gMessageLib->sendDataTransform(player->getMount());
        player->getMount()->mDirection = dir;
        player->getMount()->mPosition = pos;
        player->getMount()->setCurrentSpeed(speed);

        //player->setClientTickCount(tickCount);
        player->getMount()->setLastMoveTick(tickCount);
        player->getMount()->setInMoveCount((inMoveCount));
        gMessageLib->sendUpdateTransformMessage(player->getMount());

        gSpatialIndexManager->UpdateObject(player->getMount());
    }
    else
    {
        // send out position updates to known players
        // please note that these updates mess up our dance performance
        if(player->getPerformingState() == PlayerPerformance_None)
        {
            gMessageLib->sendUpdateTransformMessage(player);
        }

    }
}


//=============================================================================
//
// position update in cell
//

void ObjectController::handleDataTransformWithParent(Message* message,bool inRangeUpdate)
{
    // FIXME: for now assume we only get messages from players
    PlayerObject*	player = dynamic_cast<PlayerObject*>(mObject);
    glm::vec3       pos;
    glm::quat       dir;
    uint32          inMoveCount;
    uint32			tickCount;
    uint64			parentId;
    float			speed;
    bool			updateAll = false;

    // get tick and move counters
    tickCount	= message->getUint32();
    inMoveCount = message->getUint32();

    // only process if its in sequence
    if (player->getInMoveCount() > inMoveCount)	{
        return;
    }

    uint64 oldParentId = player->getParentId();

    // update tick and move counters
    player->setClientTickCount(tickCount);
    player->setInMoveCount(inMoveCount);

    // get new direction, position, parent and speed
    parentId = message->getUint64();
    dir.x = message->getFloat();
    dir.y = message->getFloat();
    dir.z = message->getFloat();
    dir.w = message->getFloat();
    pos.x = message->getFloat();
    pos.y = message->getFloat();
    pos.z = message->getFloat();
    speed  = message->getFloat();

    // stop entertaining, if we were
    if(player->getPerformingState() != PlayerPerformance_None && player->states.getPosture() != CreaturePosture_SkillAnimating)	{
        gEntertainerManager->stopEntertaining(player);
    }

    // if we changed cell
    if (oldParentId != parentId)	{
        CellObject* cell = NULL;
        // Remove us from whatever we where in before.
        // (4 for add and 0 for remove)
        gMessageLib->broadcastContainmentMessage(player->getId(),oldParentId,0,player);

        if (oldParentId != 0)	{
            if((cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(oldParentId))))	{
                cell->removeObject(player);
            }
            else	{
                DLOG(INFO) << "Error removing  " << player->getId() << " from cell " << oldParentId;
            }
        }
        else	{
            //we just entered the building - go register us

            CellObject* newCell;
            newCell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(parentId));
            if (!newCell)	{
                DLOG(INFO) << "Player " << player->getId() << " error casting new cell cell " << parentId;
                assert(false);
                return;
            }

            BuildingObject* newBuilding = dynamic_cast<BuildingObject*>(gWorldManager->getObjectById(newCell->getParentId()));
            gContainerManager->registerPlayerToBuilding(newBuilding,player);

            if(player->checkIfMounted() && player->getMount())	{
                //Can't ride into a building with a mount! :-p
                //However, its easy to do so we have handling incase the client is tricked.

                if(VehicleController* datapad_pet = dynamic_cast<VehicleController*>(gWorldManager->getObjectById(player->getMount()->controller())))	{
                    datapad_pet->Store();
                }

            }
        }

        // put us into new cell
        gMessageLib->broadcastContainmentMessage(player->getId(),parentId,4,player);
        if((cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(parentId))))	{
            cell->addObjectSecure(player);
            // Inform tutorial about cell change.
            if (gWorldConfig->isTutorial())
            {
                player->getTutorial()->setCellId(parentId);
            }

        }
        else
        {
            assert(false && "handleDataTransformWithParent::cell cannot be found");
        }
    }

    // update the player
    player->setParentId(parentId);
    player->mDirection = dir;
    player->mPosition  = pos;
    player->setCurrentSpeed(speed);

    gSpatialIndexManager->UpdateObject(player);

    // destroy the instanced instrument if out of range
    if (player->getPlacedInstrumentId())
    {
        if (!gWorldManager->objectsInRange(player->getId(), player->getPlacedInstrumentId(), 5.0))
        {
            if (Item* item = dynamic_cast<Item*>(gWorldManager->getObjectById(player->getPlacedInstrumentId())))
            {
                destroyObject(item->getId());
            }
        }
    }

    // Terminate active conversation with npc if to far away (trainers only so far).
    ActiveConversation* ac = gConversationManager->getActiveConversation(player->getId());
    if (ac != NULL)
    {
        // We do have a npc conversation going.
        if (!gWorldManager->objectsInRange(player->getId(), (ac->getNpc())->getId(), 11.0))
        {
            // Terminate conversation, since we are out of range.
            gMessageLib->SendSystemMessage(L"",player,"system_msg","out_of_range");
            gConversationManager->stopConversation(player, true);			// We will get the current dialog text in a chat bubble, only seen by me. Impressive :)
        }
    }

    if (!gWorldConfig->isInstance())
    {
        // send out updates
        gMessageLib->sendUpdateTransformMessageWithParent(player);
    }
    else
    {
        // send out position updates to known players in group or self only
        gMessageLib->sendUpdateTransformMessageWithParent(player, player);
    }
}



//=========================================================================================
//

float ObjectController::_GetMessageHeapLoadViewingRange()
{
    uint32 heapWarningLevel = gMessageFactory->HeapWarningLevel();

    if(gMessageFactory->getHeapsize() > 99.0)
        return 16.0;

    //just send everything we have
    if(heapWarningLevel < 3)
        return (float)gWorldConfig->getPlayerViewingRange();
    else if(heapWarningLevel < 4)
        return 96.0;
    else if (heapWarningLevel < 5)
    {
        return 64.0;
    }
    else if (heapWarningLevel < 8)
    {
        return 32.0;
    }
    else if (heapWarningLevel <= 10)
    {
        return 16.0;
    }
    else if (heapWarningLevel > 10)
        return 8.0;

    return (float)gWorldConfig->getPlayerViewingRange();
}

//=========================================================================================
//


void ObjectController::_findInRangeObjectsOutside(bool updateAll)
{}

//=========================================================================================
//

bool ObjectController::_updateInRangeObjectsOutside()
{
    return true;
}


//=========================================================================================
//
// Find the objects observed/known objects when inside
//
void ObjectController::_findInRangeObjectsInside(bool updateAll)
{
}


//=========================================================================================
//
// Update the objects observed/known objects when inside
//

bool ObjectController::_updateInRangeObjectsInside()
{
    return true;
}

//=========================================================================================
//
// destroy known objects not in range anymore
// compares the given list with the players known objects
//

bool ObjectController::_destroyOutOfRangeObjects(ObjectSet *inRangeObjects)
{
    return true;
}
