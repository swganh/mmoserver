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

#include "ZoneServer/ObjectController/ObjectController.h"

#include <cassert>

#include "anh/Utils/clock.h"

//#include "DatabaseManager/Database.h"
//#include "DatabaseManager/DatabaseResult.h"
//#include "DatabaseManager/DataBinding.h"

#include "NetworkManager/Message.h"
#include "NetworkManager/MessageFactory.h"

#include "MessageLib/MessageLib.h"

#include "ZoneServer/GameSystemManagers/Conversation Manager/ActiveConversation.h"
#include "ZoneServer/GameSystemManagers/Structure Manager/BuildingObject.h"
#include "ZoneServer/GameSystemManagers/Structure Manager/CellObject.h"
#include "ZoneServer/GameSystemManagers/Structure Manager/HouseObject.h"
#include "ZoneServer/GameSystemManagers/Container Manager/ContainerManager.h"
#include "ZoneServer/GameSystemManagers/Conversation Manager/ConversationManager.h"

#include "ZoneServer/GameSystemManagers/NPC Manager/NPCObject.h"
#include "ZoneServer/ObjectController/ObjectController.h"
#include "ZoneServer/ObjectController/ObjectControllerOpcodes.h"
#include "ZoneServer/ObjectController/ObjectControllerCommandMap.h"
#include "ZoneServer/Objects/Player Object/PlayerObject.h"
#include "ZoneServer/Objects/Item.h"
#include "ZoneServer/Tutorial.h"
#include "ZoneServer/Objects/VehicleController.h"
#include "ZoneServer/WorldConfig.h"
#include "ZoneServer/WorldManager.h"
#include "ZoneServer/GameSystemManagers/Spatial Index Manager/Zmap.h"


//=============================================================================
//
// position update in world
//

void ObjectController::handleDataTransform(Message* message,bool inRangeUpdate)
{
	CreatureObject*	body_	= dynamic_cast<CreatureObject*>(mObject);
	PlayerObject*	player_ = body_->GetGhost();

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
    if(body_->getInMoveCount() > inMoveCount)	{
        return;
    }

    // update tick and move counters...
    body_->setLastMoveTick(tickCount);
    player_->setClientTickCount(tickCount);

    body_->setInMoveCount(inMoveCount);

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
    if(body_->getPerformingState() != PlayerPerformance_None && body_->GetPosture() != CreaturePosture_SkillAnimating)	{
        gEntertainerManager->stopEntertaining(player_);
    }

    // if we just left a building
    // note that we remain in the grid at the worldposition
	//the buildings content remains known until the building leaves range
    if(body_->getParentId() != 0 && (!player_->checkIfMounted()) )
    {

        // remove us from the last cell we were in
        CellObject* cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(body_->getParentId()));
        if(cell)        {
            
			cell->RemoveCreature(body_, body_);
        }
        else        {
            
			LOG(error) << "ObjectController::handleDataTransform cell not found : " << body_->getParentId();
        }        

        // we are outside again
        body_->setParentId(0);

        // Add us to the world.
        gMessageLib->broadcastContainmentMessage(body_->getId(),0,4,player_);

        // Inform tutorial about cell change.
        if (gWorldConfig->isTutorial())        {
            player_->getTutorial()->setCellId(0);
        }

    }

    body_->mDirection = dir;
    body_->setCurrentSpeed(speed);
    body_->mPosition = pos;

    gSpatialIndexManager->UpdateObject(body_);

    // destroy the instanced instrument if out of range
    if (player_->getPlacedInstrumentId())    {

        if (!gWorldManager->objectsInRange(body_->getId(), player_->getPlacedInstrumentId(), 5.0))
		{
            if (Item* item = dynamic_cast<Item*>(gWorldManager->getObjectById(player_->getPlacedInstrumentId())))            {

                gWorldManager->destroyObject(item->getId());
            }
        }
    }

    // Terminate active conversation with npc if to far away (trainers only so far).
    ActiveConversation* ac = gConversationManager->getActiveConversation(player_->getId());
    if (ac != NULL)    {

        // We do have a npc conversation going.
        if (!gWorldManager->objectsInRange(body_->getId(), (ac->getNpc())->getId(), 11.0))        {

            // Terminate conversation, since we are out of range.
            gMessageLib->SendSystemMessage(std::u16string(),player_,"system_msg","out_of_range");
            gConversationManager->stopConversation(player_, true);			// We will get the current dialog text in a chat bubble, only seen by me. Impressive :)
        }
    }

    if (gWorldConfig->isInstance())    {
        // send out position updates to known players in group or self only
        gMessageLib->sendUpdateTransformMessage(body_, player_);
        return;
    }	

    //If player is mounted... move his mount too!
    if(player_->checkIfMounted() && player_->getMount())
    {
        //gMessageLib->sendDataTransform(player->getMount());
        player_->getMount()->mDirection = dir;
        player_->getMount()->mPosition = pos;
        player_->getMount()->setCurrentSpeed(speed);

        //player->setClientTickCount(tickCount);
        player_->getMount()->setLastMoveTick(tickCount);
        player_->getMount()->setInMoveCount((inMoveCount));
        gMessageLib->sendUpdateTransformMessage(player_->getMount());

        gSpatialIndexManager->UpdateObject(player_->getMount());
    }
    else
    {
        // send out position updates to known players
        // please note that these updates mess up our dance performance
        if(body_->getPerformingState() == PlayerPerformance_None)
        {
            gMessageLib->sendUpdateTransformMessage(body_);
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
	CreatureObject*	body_	= dynamic_cast<CreatureObject*>(mObject);
	PlayerObject*	player	= body_->GetGhost();
    
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
    if (body_->getInMoveCount() > inMoveCount)	{
        return;
    }

    uint64 oldParentId = body_->getParentId();

    // update tick and move counters
    player->setClientTickCount(tickCount);
    body_->setInMoveCount(inMoveCount);

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
    if(body_->getPerformingState() != PlayerPerformance_None && body_->GetPosture() != CreaturePosture_SkillAnimating)	{
        gEntertainerManager->stopEntertaining(player);
    }

    // if we changed cell
    if (oldParentId != parentId)	{
        CellObject* cell = NULL;
        // Remove us from whatever we where in before.
        // (4 for add and 0 for remove)
        gMessageLib->broadcastContainmentMessage(body_->getId(),oldParentId,0,player);

        if (oldParentId != 0)	{
            if((cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(oldParentId))))	{
                cell->RemoveCreature(body_, body_);
            }
            else	{
                DLOG(info) << "Error removing  " << player->getId() << " from cell " << oldParentId;
            }
        }
        else	{
            //we just entered the building - go register us

            CellObject* newCell;
            newCell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(parentId));
            if (!newCell)	{
                DLOG(info) << "Player " << body_->getId() << " error casting new cell cell " << parentId;
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
        
        if((cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(parentId))))	{
            cell->AddCreature(body_);
            
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

		gMessageLib->broadcastContainmentMessage(body_->getId(),parentId,body_->GetArrangementId(), player);
    }

    // update the player
    body_->setParentId(parentId);
    body_->mDirection = dir;
    body_->mPosition  = pos;
    body_->setCurrentSpeed(speed);

	gSpatialIndexManager->UpdateObject(body_);

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
        if (!gWorldManager->objectsInRange(body_->getId(), (ac->getNpc())->getId(), 11.0))
        {
            // Terminate conversation, since we are out of range.
            gMessageLib->SendSystemMessage(std::u16string(),player,"system_msg","out_of_range");
            gConversationManager->stopConversation(player, true);			// We will get the current dialog text in a chat bubble, only seen by me. Impressive :)
        }
    }

    if (!gWorldConfig->isInstance())
    {
        // send out updates
        gMessageLib->sendUpdateTransformMessageWithParent(body_);
    }
    else
    {
        // send out position updates to known players in group or self only
        gMessageLib->sendUpdateTransformMessageWithParent(body_, player);
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
