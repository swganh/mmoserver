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

#include "ScriptSupport.h"

#include "ZoneServer/AttackableCreature.h"
#include "ZoneServer/Inventory.h"
#include "ZoneServer/Heightmap.h"
#include "ZoneServer/LairObject.h"
#include "ZoneServer/NonPersistentItemFactory.h"
#include "ZoneServer/NonPersistentNpcFactory.h"
#include "ZoneServer/NpcManager.h"
#include "ZoneServer/PlayerObject.h"
#include "ZoneServer/WorldConfig.h"
#include "ZoneServer/WorldManager.h"

#include "MessageLib/MessageLib.h"


#ifdef ERROR
#undef ERROR
#endif

#include "Utils/logger.h"


// const uint64 wompratTemplateId = 47513085687;
// const uint64 rillTemplateId = 47513085693;
//======================================================================================================================

ScriptSupport*	ScriptSupport::mInstance = NULL;

//======================================================================================================================

ScriptSupport::ScriptSupport()
{
}


ScriptSupport::~ScriptSupport()
{
    // All references that are owned by WorldManager will be deleted by WorldManager.
    /*
    ScriptSupportObjectMap::iterator objMapIt = mObjectMap.begin();
    while (objMapIt != mObjectMap.end())
    {
    	objMapIt = mObjectMap.erase(objMapIt);
    }
    */
    mObjectMap.clear();
}

//======================================================================================================================
void ScriptSupport::destroyInstance(void)
{
    delete mInstance;
    mInstance = NULL;
}


//======================================================================================================================
//
// available for scripts
//
ScriptSupport* ScriptSupport::Instance()
{
    if (!mInstance)
    {
        mInstance = new ScriptSupport();
    }
    return mInstance;
}


//======================================================================================================================
//
//	General purpose
// this will add an object to the worldmanager
// NOT to the spatial index
// they will be added later by hand

void ScriptSupport::handleObjectReady(Object* object)
{
    if (object)
    {
        // Save this object internally with it's id.
        mObjectMap.insert(std::make_pair(object->getId(), object));

        // Creatures are added to WorldManager list when they have been fully initilaized (spawned) successfully.
        // when we re-spawn them.

        if (object->getType() == ObjType_Tangible)
        {
            gWorldManager->addObject(object,true);
        }
    }
}

bool ScriptSupport::objectIsReady(uint64 objectId)
{
    return (getObject(objectId) != NULL);
}

Object* ScriptSupport::getObject(uint64 id)
{
    ScriptSupportObjectMap::iterator it = mObjectMap.find(id);

    if (it != mObjectMap.end())
    {
        return((*it).second);
    }
    return (NULL);
}

//======================================================================================================================
//
// returns the id of the first object that has a private owner that match the requested one.
//
uint64 ScriptSupport::getObjectOwnedBy(uint64 theOwner)
{
    ScriptSupportObjectMap::iterator it = mObjectMap.begin();
    uint64 ownerId = 0;

    while (it != mObjectMap.end())
    {
        if ( ((*it).second)->getPrivateOwner() == theOwner)
        {
            ownerId = (*it).first;
            break;
        }
        it++;
    }
    return ownerId;
}

void ScriptSupport::eraseObject(uint64 id)
{
    ScriptSupportObjectMap::iterator it = mObjectMap.find(id);
    if (it != mObjectMap.end())
    {
        mObjectMap.erase(it);
    }
}


// Used when testing instances, normally we start up disabled (except Tutorial).
void ScriptSupport::enableInstance()
{
    gWorldConfig->enableInstance();
}


bool ScriptSupport::isInstance()
{
    return gWorldConfig->isInstance();
}


//======================================================================================================================
//
//	NPC's
//

NPCObject* ScriptSupport::npcGetObject(uint64 id)
{
    return dynamic_cast<NPCObject*>(getObject(id));
}

uint64 ScriptSupport::npcCreate(uint64 templateId) //, uint64 npcPrivateOwnerId, uint64 cellForSpawn, std::string firstname, std::string lastname, float dirY, float dirW, float posX, float posY, float posZ, uint64 respawnDelay)
{
    DLOG(INFO) << "ScriptSupport::npcCreate template id " << templateId;

    uint64 npcId = gWorldManager->getRandomNpNpcIdSequence();
    if (npcId != 0)
    {
        // Let's create a npc.
        NonPersistentNpcFactory::Instance()->requestNpcObject(this, templateId, npcId, 0, glm::vec3(), glm::quat(), 0);
    }
    else
    {
        // @TODO: WorldManager::getRandomNpNpcIdSequence must return a valid value.
        assert(false);
    }
    return npcId;
}


void ScriptSupport::npcSpawnPersistent(NPCObject* npc, uint64 npcId, uint64 cellForSpawn, std::string firstname, std::string lastname, float dirY, float dirW, float posX, float posY, float posZ,
                                       uint64 respawnPeriod, uint64 templateId)
{
    // Do not allow any spwans if not heightmap avaliable.
    if (gHeightmap->Open())
    {
        // Test
        // respawnPeriod = 15000;
        npcSpawnGeneral(npcId, 0, cellForSpawn, firstname, lastname, dirY, dirW, posX, posY, posZ, respawnPeriod);
    }
    else
    {
        LOG(WARNING) << "ScriptSupport::npcSpawnPersistent: Heightmap is missing, can NOT use dynamic spawned npc's.";
    }
}

void ScriptSupport::npcSpawn(NPCObject* npc, uint64 npcId, uint64 cellForSpawn, std::string firstname, std::string lastname, float dirY, float dirW, float posX, float posY, float posZ)
{
    npcSpawnGeneral(npcId, 0, cellForSpawn, firstname, lastname, dirY, dirW, posX, posY, posZ, 0);
}

void ScriptSupport::npcSpawnPrivate(NPCObject* npc, uint64 npcId, uint64 npcPrivateOwnerId, uint64 cellForSpawn, std::string firstname, std::string lastname, float dirY, float dirW, float posX, float posY, float posZ)
{
    npcSpawnGeneral(npcId, npcPrivateOwnerId, cellForSpawn, firstname, lastname, dirY, dirW, posX, posY, posZ, 0);
}

void ScriptSupport::npcSpawnGeneral(uint64 npcId, uint64 npcPrivateOwnerId, uint64 cellForSpawn, std::string firstname, std::string lastname, float dirY, float dirW, float posX, float posY, float posZ, uint64 respawnDelay) // , uint64 templateId)
{
    glm::quat direction;
    glm::vec3 position;

    direction.x = 0.0;
    direction.y = dirY;
    direction.z = 0.0;
    direction.w = dirW;

    position.x = posX;
    position.y = posY;
    position.z = posZ;

    NPCObject* npc = dynamic_cast<NPCObject*>(gWorldManager->getObjectById(npcId));
    assert(npc);
    if (!npc)
    {
        // Fallback for running in release mode.
        LOG(WARNING) << "ScriptSupport::npcSpawnGeneral: Failed to access NPC id " << npcId;
        return;
    }

    // npc->setId(npcId);
    npc->setParentId(cellForSpawn);	// The cell we will spawn in.
    npc->setCellIdForSpawn(cellForSpawn);



    // THESE TWO ARE NOT GENERALLY FIXED YET.
    npc->setFirstName((int8*)firstname.c_str());
    npc->setLastName((int8*)lastname.c_str());

    // THIS ONE IS NOT GENERALLY FIXED YET.
    // If used for re-spawning npc's, add this id as an internal attribute.
    npc->setPrivateOwner(npcPrivateOwnerId);


    npc->mPosition = position;
    npc->setSpawnPosition(position);

    npc->mDirection = direction;
    npc->setSpawnDirection(direction);
    npc->setRespawnDelay(respawnDelay);

    // Register object with WorldManager.
    // gWorldManager->addObject(npc, true);

    // Update the world about my presence.
    /*
    if (npc->getParentId())
    {
    	// insert into cell
    	npc->setSubZoneId(0);

    	if (CellObject* cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(npc->getParentId())))
    	{
    		cell->addChild(npc);
    	}
    	else
    	{
    	}
    }
    else
    {
    	if (QTRegion* region = gWorldManager->getSI()->getQTRegion(npc->mPosition.x,npc->mPosition.z))
    	{
    		npc->setSubZoneId((uint32)region->getId());
    		region->mTree->addObject(npc);
    	}
    }
    */

    //Inventory* inventory = dynamic_cast<Inventory*>(npc->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));

    // Register the NPC with the NPC AI-manager.
    if (npc->getTemplateId() == 0)
    {
        // Just to make sure we do not have any old scripts running.
        assert(false);
    }
    npc->respawn();

    // Now we can remove this object from our internal list. WorldManager will handle the destruction.
    // Except for the npc's used in tutorial, they are spawned-despawned with the player.
    if (!gWorldConfig->isTutorial())
    {
        this->eraseObject(npcId);
    }
    /*
    	// The dynamic spawned private owned npc MUST register with their owner.
    	// It's not always the case that the player have had time to track this newly spawned objects,

    	// Private owned objects are invisible to most players and using getKnownPlayers()->empty() to try and save the
    	// sendDataTransformWithParent
    	// and
    	// sendUpdateTransformMessageWithParent
    	// from being called is just ridicules, it's not like we going to spwan a npc from script every second or so.

    	// So please stop messing with this code!!!

    	// Add us to the world.
    	gMessageLib->broadcastContainmentMessage(npc->getId(),npc->getParentId(),-1,npc);

    	// send out position updates to known players
    	npc->setInMoveCount(npc->getInMoveCount() + 1);
    	if (gWorldConfig->isTutorial())
    	{
    		// We need to get the player object that is the owner of this npc.
    		if (npcPrivateOwnerId != 0)
    		{
    			PlayerObject* playerObject = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(npcPrivateOwnerId));
    			if (playerObject)
    			{

    				if (npc->getParentId())
    				{
    					// We are inside a cell.
    					gMessageLib->sendDataTransformWithParent(npc, playerObject);
    					gMessageLib->sendUpdateTransformMessageWithParent(npc, playerObject);
    				}
    				else
    				{
    					gMessageLib->sendDataTransform(npc, playerObject);
    					gMessageLib->sendUpdateTransformMessage(npc, playerObject);
    				}
    				return;
    			}
    		}
    	}
    	else
    	{
    		if (npc->getParentId())
    		{
    			// We are inside a cell.
    			gMessageLib->sendDataTransformWithParent(npc);
    			gMessageLib->sendUpdateTransformMessageWithParent(npc);
    		}
    		else
    		{
    			gMessageLib->sendDataTransform(npc);
    			gMessageLib->sendUpdateTransformMessage(npc);
    		}
    	}
    	*/
}

void ScriptSupport::npcMove(NPCObject* npc, float posX, float posY, float posZ)
{
    // send out position updates to known players
    npc->updatePosition(npc->getParentId(), glm::vec3(posX, posY, posZ));
}

void ScriptSupport::npcMoveToZone(NPCObject* npc, uint64 zoneId, float posX, float posY, float posZ)
{
    // send out position updates to known players
    npc->updatePosition(zoneId, glm::vec3(posX, posY, posZ));
}


uint32 ScriptSupport::getZoneId()
{
    return gWorldManager->getZoneId();
}


void ScriptSupport::npcTestDir(NPCObject* npc, float dirX, float dirZ)
{
    npc->mDirection.x = dirX;
    npc->mDirection.x = dirZ;
}

void ScriptSupport::npcDirection(NPCObject* npc, float deltaX, float deltaZ) {
    // Turn to the direction heading.
    npc->facePosition(glm::vec3(deltaX, 0.0f, deltaZ));

    // send out position updates to known players
    // updateNpcPosition(npc);
    if(npc->getParentId()) {
        return;
    }

    // Send out the appropriate data transform depending if the npc is in a cell or not.
    if (npc->getParentId())	{
        gMessageLib->sendDataTransformWithParent053(npc);
    } else {
        gMessageLib->sendDataTransform053(npc);
    }
}


// Copy direction from "npcLeader" to "npcMember".
// Also pre-create offset from member to leader.
// Will make use of less cpu cycles for math.

void ScriptSupport::npcFormationPosition(NPCObject* npcMember, float xOffset, float zOffset)
{
    // Update formation members direction.
    // npcMember->mDirection = npcLeader->mDirection;

    // Get offset to leader so we can stay in formation.
    float length = sqrt((xOffset * xOffset) + (zOffset * zOffset));
    float alpha = atan(xOffset/zOffset);

    float w = npcMember->mDirection.w;

    if (w > 0.0)
    {
        if (npcMember->mDirection.y < 0.0)
        {
            w *= -1;
        }
    }
    float angle = 2.0f*acos(w);

    // We assume all formation is following the leader, ie. located behind him.
    angle += static_cast<float>(alpha + 3.1415936539);	// alpha + 180

    glm::vec3 positionOffset;

    positionOffset.x = (sin(angle) * length);
    positionOffset.y = 0;
    positionOffset.z = (cos(angle) * length);

    npcMember->setPositionOffset(positionOffset);

    // send out position updates to known players
    // updateNpcPosition(npcMember);
}


void ScriptSupport::npcFormationMoveEx(NPCObject* npc, float posX, float posY, float posZ , float xOffset, float zOffset)
{
    float length = sqrt((xOffset * xOffset) + (zOffset * zOffset));

    float alpha = atan(xOffset/zOffset);

    float w = npc->mDirection.w;

    if (w > 0.0)
    {
        if (npc->mDirection.y < 0.0)
        {
            w *= -1;
        }
    }
    float angle = 2.0f*acos(w);

    // We assume all formation is following the leader, ie. located behind him.
    angle += static_cast<float>(alpha + 3.1415936539);	// alpha + 180

    // npc->mPosition.x = posX + (sin(angle) * length);
    // npc->mPosition.y = posY;
    // npc->mPosition.z = posZ + (cos(angle) * length);

    posX += (sin(angle) * length);
    posZ += (cos(angle) * length);

    // send out position updates to known players
    npc->updatePosition(npc->getParentId(), glm::vec3(posX, posY, posZ));
}

// To be used with "ScriptSupport::npcFormationDirection".
void ScriptSupport::npcFormationMove(NPCObject* npc, float posX, float posY, float posZ)
{
    // New destination, and take care of any offset from formation leader.
    // npc->mPosition = npc->getPositionOffset();

    // send out position updates to known players
    npc->updatePosition(npc->getParentId(), glm::vec3(posX, posY, posZ) + npc->getPositionOffset());
}

void ScriptSupport::npcClonePosition(NPCObject* npcDest, NPCObject* npcSrc)
{
    // New destination, and take care of any offset from formation leader.
    // npcDest->mPosition = npcSrc->mPosition;
    npcDest->mDirection = npcSrc->mDirection;

    // send out position updates to known players
    npcDest->updatePosition(npcDest->getParentId(), npcSrc->mPosition);
}


// TODO: Making this a more general method, by uisng Object*.
// void ScriptSupport::scriptPlayMusic(uint32 soundId, Object* creatureObject)
void ScriptSupport::scriptPlayMusic(uint32 soundId, NPCObject* creatureObject)
{
    Object* object = dynamic_cast<Object*>(creatureObject);
    gMessageLib->sendPlayMusicMessage(soundId, object);
}

//======================================================================================================================
//
// Other objects
//

/*
uint64 ScriptSupport::containerCreate(uint64 containerId)
{
	// We need a new id to our new container.
	uint64 newId = gWorldManager->getRandomNpId();

	// Let us create an object that has no relations whatsoever with existing ones.
	// "tatt_drum" has id 11 in container_types... TODO:
	// const uint64 tatt_drum = 11;
	NonPersistentContainerFactory::getSingletonPtr()->requestObject(this, containerId, newId);

	return newId;
}
*/

/*
Container*	ScriptSupport::containerGetObject(uint64 id)
{
	return dynamic_cast<Container*>(getObject(id));
}
*/

/*
void ScriptSupport::containerSpawn(Container* container,
									uint64 containerId,
									uint64 playerId,
									uint64 cellForSpawn,
									std::string customName,
									float dirY,
									float dirW,
									float posX,
									float posY,
									float posZ)
{

	Anh_Math::Quaternion	direction;
	Anh_Math::Vector3		position;

	direction.x = 0.0;
	direction.y = dirY;
	direction.z = 0.0;
	direction.w = dirW;

	position.x = posX;
	position.y = posY;
	position.z = posZ;

	container->setId(containerId);
	container->setParentId(cellForSpawn);	// The cell we will spawn in.
	container->mDirection = direction;
	container->mPosition = position;
	container->setType(ObjType_Tangible);
	container->setPrivateOwner(playerId);

	// container->setMaxCondition(100);	// ??
	container->setCustomName((int8*)customName.c_str());

	// WorldManager will only add us to the global object map if we use true.
	gWorldManager->addObject(container, false);

	// Add us to the world.
	gMessageLib->broadcastContainmentMessage(container->getId(),container->getParentId(),4,container);

	// send out position updates to known players
	// npc->setInMoveCount(npc->getInMoveCount() + 1);

	PlayerObject* playerObject = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(playerId));

	// Need to take care if we are doing the Tutorial or not.
	if (!gWorldConfig->isTutorial())
	{
		if (container->getParentId())
		{
			// We are inside a cell.
			// insert into cell
			container->setSubZoneId(0);
			gMessageLib->sendDataTransformWithParent(container);
			// gMessageLib->sendUpdateTransformMessageWithParent(container, false);
		}
		else
		{
			gMessageLib->sendDataTransform(container);
			// gMessageLib->sendUpdateTransformMessage(container, false);
		}
	}
	else
	{
		// Let's play with both the static and dynamically created objects.
		if (container->getPrivateOwner())
		{
			// We are used as Instance-objects
			if (container->getParentId())
			{
				// We are inside a cell.
				gMessageLib->sendDataTransformWithParent(container, playerObject);
				// gMessageLib->sendUpdateTransformMessageWithParent(container, playerObject, false);
			}
			else
			{
				gMessageLib->sendDataTransform(container, playerObject);
				// gMessageLib->sendUpdateTransformMessage(container, playerObject, false);
			}
		}
		else
		{
			// We will change static objects, good luck.
			if (container->getParentId())
			{
				// We are inside a cell.
				gMessageLib->sendDataTransformWithParent(container);
				// gMessageLib->sendUpdateTransformMessageWithParent(container, false);
			}
			else
			{
				gMessageLib->sendDataTransform(container);
				// gMessageLib->sendUpdateTransformMessage(container, false);
			}
		}
	}
}
*/

// We need an id.


uint64 ScriptSupport::itemCreate(uint64 itemTypesId)
{
    // We need a new id to our new item.
    uint64 itemNewId = gWorldManager->getRandomNpId();
    if (itemNewId != 0)
    {
        // Let us get an object from/via the WRONG database (the Persistent... one).
        NonPersistentItemFactory::Instance()->requestObject(this, itemTypesId, itemNewId);
    }
    return itemNewId;
}

// TODO: Fix this
void ScriptSupport::itemPopulateInventory(uint64 itemId, uint64 npcId, uint64 playerId)
{
    // NOTE: For now we only check and validates TangibleObject.

    CreatureObject* creature = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById(npcId));
    Object* itemObject = gWorldManager->getObjectById(itemId);

    PlayerObject* playerObject = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(playerId));

    if (creature && itemObject && playerObject)
    {
        Inventory* inventory = dynamic_cast<Inventory*>(creature->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
        if (inventory)
        {
            //  and (tutorial:getRoom() < 7)
            inventory->addObjectSecure(itemObject);
            itemObject->setParentId(npcId+1);
        }
    }
}


/*
void ScriptSupport::itemSpawn(Item* item,
									uint64 itemId,
									uint64 playerId,
									uint64 parentId,
									std::string customName,
									float dirY,		// In case of spawning in a cell or the world.
									float dirW,
									float posX,
									float posY,
									float posZ)
{

	Anh_Math::Quaternion	direction;
	Anh_Math::Vector3		position;

	direction.x = 0.0;
	direction.y = dirY;
	direction.z = 0.0;
	direction.w = dirW;

	position.x = posX;
	position.y = posY;
	position.z = posZ;

	item->setId(itemId);
	item->setParentId(parentId);	// The cell we will spawn in, or the conatiner we belongs to.
	item->mDirection = direction;
	item->mPosition = position;
	item->setType(ObjType_Tangible);
	item->setPrivateOwner(playerId);

	// item->setMaxCondition(100);	// ??
	item->setCustomName((int8*)customName.c_str());

	// WorldManager will only add us to the global object map if using true below.
	gWorldManager->addObject(item, true);
	// gWorldManager->addObject(item, false);

	if (parentId == 0)
	{
		// gWorldManager->getSI()->InsertPoint(itemId, item->mPosition.x, item->mPosition.z);
	}
	else
	{

		// We KNOW we would like to put this object into a conatiner, don't we?
		PlayerObject* playerObject = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(playerId));
		gMessageLib->sendCreateTangible(item,playerObject,false);
	}
}
*/


//======================================================================================================================
//
// Get id of player target.
//

uint64 ScriptSupport::getTarget(uint64 playerId)
{
    uint64 targetId = 0;
    PlayerObject* playerObject = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(playerId));
    if (playerObject)
    {
        targetId = playerObject->getTargetId();
    }
    return targetId;
}

//======================================================================================================================
//
// Get parent id of player target.
//

uint64 ScriptSupport::getParentOfTarget(uint64 playerId)
{
    uint64 parentId = 0;
    PlayerObject* playerObject = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(playerId));
    if (playerObject)
    {
        uint64 targetId = playerObject->getTargetId();
        Object* object = dynamic_cast<Object*>(gWorldManager->getObjectById(targetId));
        if (object)
        {
            parentId = object->getParentId();
        }
    }
    return parentId;
}

//======================================================================================================================
//
// Send flytext.
//
void ScriptSupport::sendFlyText(uint64 targetId, uint64 playerId, std::string stfFile, std::string stfVar,uint8 red,uint8 green,uint8 blue,uint8 display)
{
    Object* object = gWorldManager->getObjectById(targetId);
    PlayerObject* playerObject = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(playerId));
    if (object && playerObject)
    {
        gMessageLib->sendFlyText(object,playerObject, (int8*)(stfFile.c_str()),(int8*)(stfVar.c_str()),red,green,blue, display);
    }
}

/*
void Tutorial::scriptSystemMessage(std::string message)
{
	BString msg = (int8*)message.c_str();

	msg.convert(BSTRType_Unicode16);

	if (mPlayerObject->isConnected())
	{
		gMessageLib->sendSystemMessage(mPlayerObject, msg);
	}
}
*/

void ScriptSupport::scriptSystemMessage(uint64 playerId, uint64 targetId, std::string message)
{
    PlayerObject* playerObject = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(playerId));
    Object* object = gWorldManager->getObjectById(targetId);
    CreatureObject* creature = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById(targetId));
    if (object && creature && playerObject && playerObject->isConnected())
    {
        std::string msg = (int8*)message.c_str();
        // gMessageLib->sendPlayClientEffectLocMessage(msg, object->mPosition, playerObject);
        gMessageLib->sendPlayClientEffectObjectMessage(msg,"",creature,playerObject);
        // "clienteffect/combat_explosion_lair_large.cef"
    }
}

bool ScriptSupport::npcInCombat(uint64 npcId)
{
	bool inCombat = false;
	CreatureObject* creature = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById(npcId));
	if (creature)
	{
		inCombat = creature->states.checkState(CreatureState_Combat);
	}
	return inCombat;
}

bool ScriptSupport::npcIsDead(uint64 npcId)
{
    bool isDead = true;
    CreatureObject* creature = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById(npcId));
    if (creature)
    {
        isDead = creature->isDead();
    }
    return isDead;
}

void ScriptSupport::npcKill(uint64 npcId)
{
    CreatureObject* creature = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById(npcId));
    if (creature)
    {
        // Kill him!
        creature->die();
    }
}

//======================================================================================================================
//
//	Set player position.
//	To be used when you may have to restrict player movement, as when doing quest tasks or parts of the tutorial.
//
//	Not to be used as a warp-function.
//

void ScriptSupport::setPlayerPosition(uint64 playerId, uint64 cellId, float posX, float posY, float posZ)
{
    PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(playerId));
    if (player)
    {
        // Anh_Math::Quaternion	direction;
        glm::vec3 position;
        position.x = posX;
        position.y = posY;
        position.z = posZ;

        player->mPosition = position;
        player->setParentId(cellId);

        if (cellId)
        {
            // We are inside a cell.
            gMessageLib->sendDataTransformWithParent053(player);
            gMessageLib->sendUpdateTransformMessageWithParent(player);
        }
        else
        {
            gMessageLib->sendDataTransform053(player);
            gMessageLib->sendUpdateTransformMessage(player);
            //If our player is mounted move his mount aswell
            if(player->checkIfMounted() && player->getMount())
            {
                player->getMount()->mPosition = position;
                gMessageLib->sendDataTransform053(player->getMount());
                gMessageLib->sendUpdateTransformMessage(player->getMount());
            }
        }
    }
}

void ScriptSupport::lairSpawn(uint64 lairTypeId)
{
    uint64 npcNewId = gWorldManager->getRandomNpNpcIdSequence();
    if (npcNewId != 0)
    {
        // Let's put this sucker into play again.
        NonPersistentNpcFactory::Instance()->requestLairObject(NpcManager::Instance(), lairTypeId, npcNewId);
    }
}
