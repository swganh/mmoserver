/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "ScriptSupport.h"
#include "ZoneServer/WorldManager.h"
#include "ZoneServer/NonPersistentNpcFactory.h"
#include "ZoneServer/NonPersistentItemFactory.h"
#include "ZoneServer/WorldConfig.h"
#include "ZoneServer/NpcManager.h"
#include "ZoneServer/Heightmap.h"
#include "ZoneServer/LairObject.h"
#include "ZoneServer/AttackableCreature.h"

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
// 

void ScriptSupport::handleObjectReady(Object* object)
{
	// gLogger->logMsgF("ScriptSupport::handleObjectReady() Entering", MSG_NORMAL);
	if (object)
	{
		// gLogger->logMsgF("ScriptSupport::handleObjectReady() Got an object and adding it locally", MSG_NORMAL);

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
			// gLogger->logMsgF("ScriptSupport::getObjectOwnedBy: Found an object with id = %llu",MSG_NORMAL, ownerId);
			break;
		}
		it++;
	}
	return ownerId;
}

void ScriptSupport::eraseObject(uint64 id)
{
	// gLogger->logMsgF("ScriptSupport::eraseObject: Invoked",MSG_NORMAL);

	ScriptSupportObjectMap::iterator it = mObjectMap.find(id);
	if (it != mObjectMap.end())
	{
		mObjectMap.erase(it);
		// gLogger->logMsgF("ScriptSupport::eraseObject: Erased an object with id %llu",MSG_NORMAL, id);
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
	// gLogger->logMsgF("ScriptSupport::npcCreate Entering", MSG_NORMAL);
	uint64 npcId = gWorldManager->getRandomNpNpcIdSequence();	
	if (npcId != 0)
	{
		// Let's create a npc.
		//gLogger->logMsgF("ScriptSupport::npcCreate Requesting npc from template %llu with id = %llu", MSG_NORMAL, templateId, npcId);
		NonPersistentNpcFactory::Instance()->requestNpcObject(this, templateId, npcId, 0, 0, 0, 0);
	}
	else
	{
		assert(false);
	}
	return npcId;
}


void ScriptSupport::npcSpawnPersistent(NPCObject* npc, uint64 npcId, uint64 cellForSpawn, std::string firstname, std::string lastname, float dirY, float dirW, float posX, float posY, float posZ,
									   uint64 respawnPeriod, uint64 templateId)
{
	// gLogger->logMsgF("ScriptSupport::npcSpawnPersistent: Added NPC with ID = %llu with template = %llu", MSG_NORMAL, npcId, templateId);

	// Do not allow any spwans if not heightmap avaliable.
	if (Heightmap::Instance()->Open())
	{
		// Test
		// respawnPeriod = 15000;
		npcSpawnGeneral(npcId, 0, cellForSpawn, firstname, lastname, dirY, dirW, posX, posY, posZ, respawnPeriod);
	}
	else
	{
		gLogger->logMsgF("ScriptSupport::npcSpawnPersistent: Heightmap is missing, can NOT use dynamic spawned npc's.", MSG_NORMAL);
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
	Anh_Math::Quaternion	direction;
	Anh_Math::Vector3		position;

	direction.mX = 0.0;
	direction.mY = dirY;
	direction.mZ = 0.0;
	direction.mW = dirW;

	position.mX = posX;
	position.mY = posY;
	position.mZ = posZ;

	NPCObject* npc = dynamic_cast<NPCObject*>(gWorldManager->getObjectById(npcId));
	assert(npc);
	if (!npc)
	{
		// Fallback for running in release mode.
		gLogger->logMsgF("ScriptSupport::npcSpawnGeneral: Failed to access NPC id %llu", MSG_NORMAL, npcId);
		return;
	}

	// gLogger->logMsgF("ScriptSupport::npcSpawnGeneral: Spawning NPC with template = %llu, ID = %llu", MSG_NORMAL, npc->getTemplateId(), npcId);

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

	// gLogger->logMsgF("ScriptSupport::npcSpawnGeneral: Pos and Dir set!", MSG_NORMAL);

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
			gLogger->logMsgF("ScriptSupport::npcSpawn: couldn't find cell %llu",MSG_HIGH,npc->getParentId());
		}
	}
	else
	{
		if (QTRegion* region = gWorldManager->getSI()->getQTRegion(npc->mPosition.mX,npc->mPosition.mZ))
		{
			// gLogger->logMsg("ScriptSupport::npcSpawn QTRegion found");
			npc->setSubZoneId((uint32)region->getId());
			region->mTree->addObject(npc);
		}
	}
	*/

	Inventory* inventory = dynamic_cast<Inventory*>(npc->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));

	// Register the NPC with the NPC AI-manager.
	if (npc->getTemplateId() == 0)
	{
		// Just to make sure we do not have any old scripts running.
		assert(false);
	}
	// gLogger->logMsgF("ScriptSupport::npcSpawnGeneral: Calling respawn()", MSG_NORMAL);
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
		// gLogger->logMsgF("ScriptSupport::npcSpawn: gWorldConfig->isTutorial()", MSG_NORMAL);

		// We need to get the player object that is the owner of this npc.
		if (npcPrivateOwnerId != 0)
		{
			PlayerObject* playerObject = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(npcPrivateOwnerId));
			if (playerObject)
			{
				// gLogger->logMsgF("ScriptSupport::npcSpawn: We have a PlayerObject", MSG_NORMAL);

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
	// gLogger->logMsgF("ScriptSupport::npcMove() this = %ld, NPC is: %ld",MSG_HIGH, this, mSpawnedNpc);

	// send out position updates to known players
	npc->updatePosition(npc->getParentId(), Anh_Math::Vector3(posX, posY, posZ));
}

void ScriptSupport::npcMoveToZone(NPCObject* npc, uint64 zoneId, float posX, float posY, float posZ)
{
	// gLogger->logMsgF("ScriptSupport::npcMoveToZone() %.0f %.0f %.0f", MSG_NORMAL, posX, posY, posZ);
	// send out position updates to known players
	npc->updatePosition(zoneId, Anh_Math::Vector3(posX, posY, posZ));
}


uint32 ScriptSupport::getZoneId()
{
	return gWorldManager->getZoneId();
}


void ScriptSupport::npcTestDir(NPCObject* npc, float dirX, float dirZ)
{
	npc->mDirection.mX = dirX;
	npc->mDirection.mZ = dirZ;
}

void ScriptSupport::npcDirection(NPCObject* npc, float deltaX, float deltaZ)
{
	// gLogger->logMsgF("ScriptSupport::npcDirection() this = %ld, NPC is: %ld",MSG_HIGH, this, mSpawnedNpc);

	// Let's turn to the direction we are heading.
	float x = deltaX;
	float z = deltaZ;
	float h = sqrt(x*x + z*z);

	// if ((z/h) < 0.0)
	if ((z) < 0.0)
	{	
		// if (x/h < 0.0)
		if (x < 0.0)
		{
			npc->mDirection.mW = cos((3.14159354 * 0.5) + 0.5f*acos(-z/h));
			npc->mDirection.mY = sin((3.14159354 * 0.5) + 0.5f*acos(-z/h));
		}
		else
		{
			npc->mDirection.mY = sin(0.5f*acos(z/h));
			npc->mDirection.mW = cos(0.5f*acos(z/h));
		}
	}
	else
	{
		npc->mDirection.mY = sin(0.5f*asin(x/h));	
		npc->mDirection.mW = cos(0.5f*acos(z/h));
	}

	// send out position updates to known players
	// updateNpcPosition(npc);
	if(npc->getKnownPlayers()->empty())
	{
		return;
	}

	if (npc->getParentId())
	{
		// We are inside a cell.
		gMessageLib->sendDataTransformWithParent(npc);
		// gMessageLib->sendUpdateTransformMessageWithParent(npc, false);
	}
	else
	{

		gMessageLib->sendDataTransform(npc);
		// gMessageLib->sendUpdateTransformMessage(npc, false);
	}
}


// Copy direction from "npcLeader" to "npcMember". 
// Also pre-create offset from member to leader.
// Will make use of less cpu cycles for math.

void ScriptSupport::npcFormationPosition(NPCObject* npcMember, float xOffset, float zOffset)
{
	// gLogger->logMsgF("ScriptSupport::npcDirection() this = %ld, NPC is: %ld",MSG_HIGH, this, mSpawnedNpc);

	// Update formation members direction.
	// npcMember->mDirection = npcLeader->mDirection;

	// Get offset to leader so we can stay in formation.
	float length = sqrt((xOffset * xOffset) + (zOffset * zOffset));
	float alpha = atan(xOffset/zOffset);

	float w = npcMember->mDirection.mW;
	float y = 1.0;

	if (w > 0.0)
	{
		if (npcMember->mDirection.mY < 0.0)
		{
			w *= -1;
			y = -1.0;
		}
	}
	float angle = 2.0f*acos(w);
	// gLogger->logMsgF("Heading: %d",MSG_NORMAL, (uint32)((angle / (2 * 3.1415936539)) * 360));
	
	// We assume all formation is following the leader, ie. located behind him.
	angle += (alpha + 3.1415936539);	// alpha + 180

	Anh_Math::Vector3		positionOffset;

	positionOffset.mX = (sin(angle) * length);
	positionOffset.mY = 0;
	positionOffset.mZ = (cos(angle) * length);

	npcMember->setPositionOffset(positionOffset);

	// send out position updates to known players
	// updateNpcPosition(npcMember);
}


void ScriptSupport::npcFormationMoveEx(NPCObject* npc, float posX, float posY, float posZ , float xOffset, float zOffset)
{
	// gLogger->logMsgF("ScriptSupport::npcMove() this = %ld, NPC is: %ld",MSG_HIGH, this, mSpawnedNpc);
	float length = sqrt((xOffset * xOffset) + (zOffset * zOffset));

	float alpha = atan(xOffset/zOffset);

	float w = npc->mDirection.mW;
	float y = 1.0;

	if (w > 0.0)
	{
		if (npc->mDirection.mY < 0.0)
		{
			w *= -1;
			y = -1.0;
		}
	}
	float angle = 2.0f*acos(w);
	// gLogger->logMsgF("Heading: %d",MSG_NORMAL, (uint32)((angle / (2 * 3.1415936539)) * 360));
	
	// We assume all formation is following the leader, ie. located behind him.
	angle += (alpha + 3.1415936539);	// alpha + 180

	// npc->mPosition.mX = posX + (sin(angle) * length);
	// npc->mPosition.mY = posY;
	// npc->mPosition.mZ = posZ + (cos(angle) * length);

	posX += (sin(angle) * length);
	posZ += (cos(angle) * length);

	// send out position updates to known players
	npc->updatePosition(npc->getParentId(), Anh_Math::Vector3(posX, posY, posZ));
}

// To be used with "ScriptSupport::npcFormationDirection".
void ScriptSupport::npcFormationMove(NPCObject* npc, float posX, float posY, float posZ)
{
	// New destination, and take care of any offset from formation leader.
	// npc->mPosition = npc->getPositionOffset();
	 
	// send out position updates to known players
	npc->updatePosition(npc->getParentId(), Anh_Math::Vector3(posX, posY, posZ) + npc->getPositionOffset());
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

	gLogger->logMsg("ScriptSupport::containerCreate");

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
	gLogger->logMsg("ScriptSupport::containerSpawn");
	
	Anh_Math::Quaternion	direction;
	Anh_Math::Vector3		position;

	direction.mX = 0.0;
	direction.mY = dirY;
	direction.mZ = 0.0;
	direction.mW = dirW;

	position.mX = posX;
	position.mY = posY;
	position.mZ = posZ;

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
	
	// gLogger->logMsgF("ScriptSupport::npcSpawn: Added NPC with ID = %lld", MSG_HIGH, npcId);

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
	// gLogger->logMsg("ScriptSupport::itemCreate");
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
		// gLogger->logMsgF("ScriptSupport::itemPopulateInventory() Got an object to add",MSG_NORMAL);
		Inventory* inventory = dynamic_cast<Inventory*>(creature->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
		if (inventory)
		{
			//  and (tutorial:getRoom() < 7) 
			inventory->addObject(itemObject);
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
	gLogger->logMsg("ScriptSupport::itemSpawn");
	
	Anh_Math::Quaternion	direction;
	Anh_Math::Vector3		position;

	direction.mX = 0.0;
	direction.mY = dirY;
	direction.mZ = 0.0;
	direction.mW = dirW;

	position.mX = posX;
	position.mY = posY;
	position.mZ = posZ;

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
		gLogger->logMsgF("ScriptSupport::itemGetObject BIG TODO HERE" ,MSG_NORMAL);
		// gWorldManager->getSI()->InsertPoint(itemId, item->mPosition.mX, item->mPosition.mZ);
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
	string msg = (int8*)message.c_str();

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
		string msg = (int8*)message.c_str();
		// gMessageLib->sendPlayClientEffectLocMessage(msg, object->mPosition, playerObject);
		gMessageLib->sendPlayClientEffectObjectMessage(msg,"",playerObject,creature);
		// "clienteffect/combat_explosion_lair_large.cef"
	}
}

bool ScriptSupport::npcInCombat(uint64 npcId)
{
	bool inCombat = false;
	CreatureObject* creature = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById(npcId));
	if (creature)
	{
		inCombat = creature->checkState(CreatureState_Combat);
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
		Anh_Math::Vector3		position;
		position.mX = posX;
		position.mY = posY;
		position.mZ = posZ;

		player->mPosition = position;
		player->setParentId(cellId);

		if (cellId)
		{
			// We are inside a cell.
			gMessageLib->sendDataTransformWithParent(player);
			gMessageLib->sendUpdateTransformMessageWithParent(player);
		}
		else
		{
			gMessageLib->sendDataTransform(player);
			gMessageLib->sendUpdateTransformMessage(player);
			//If our player is mounted move his mount aswell
			if(player->checkIfMounted() && player->getMount())
			{
				player->getMount()->mPosition = position;
				gMessageLib->sendDataTransform(player->getMount());
				gMessageLib->sendUpdateTransformMessage(player->getMount());
			}
		}
	}
}

void ScriptSupport::lairSpawn(uint64 lairTypeId)
{
	// gLogger->logMsgF("ScriptSupport::lairSpawn Entering", MSG_NORMAL);
	uint64 npcNewId = gWorldManager->getRandomNpNpcIdSequence();	
	if (npcNewId != 0)
	{
		// Let's put this sucker into play again.
		// gLogger->logMsgF("ScriptSupport::lairSpawn Requesting lair of type = %llu with id %llu", MSG_NORMAL, lairTypeId, npcNewId);
		NonPersistentNpcFactory::Instance()->requestLairObject(NpcManager::Instance(), lairTypeId, npcNewId);
	}
}