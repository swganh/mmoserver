/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "NPCObject.h"
#include "QuadTree.h"
#include "Weapon.h"
#include "MathLib/Quaternion.h"
#include "MathLib/Vector3.h"
#include "MessageLib/MessageLib.h"
#include "Utils/rand.h"
#include "WorldManager.h"
#include "Heightmap.h"
#include "CellObject.h"
#include "ZoneTree.h"
#include "PlayerObject.h"

class DamageDealer
{
	public:
		 DamageDealer() {}
		~DamageDealer() {}

	// private:
		uint64	mPlayerId;
		uint64	mGroupId;
		uint32	mWeaponGroupMasks;
		int32	mDamage;
		float	mAggroPoints;
};

//=============================================================================

NPCObject::NPCObject() : CreatureObject(),
mLastConversationTarget(0),
mLastConversationRequest(0),
mAiState(NpcIsDormant),
mLootGroupId(0),
mSpeciesId(0),
mNpcTemplateId(0),
mRespawnDelay(0),
mCellIdForSpawn(0),
mBaseAggro(0),
mAttackRange(64),
mLootAllowedById(0)
{
	mType = ObjType_NPC;
	mCreoGroup = CreoGroup_PersistentNpc;
}

//=============================================================================

NPCObject::~NPCObject()
{
	mDamageDealers.clear();
}

//=============================================================================
//
//	Returns a new random position given the rectangle of +- offsetX and +- offsetY.
//

Anh_Math::Vector3 NPCObject::getRandomPosition(Anh_Math::Vector3& currentPos, int32 offsetX, int32 offsetZ) const
{
	Anh_Math::Vector3 v(currentPos);
	
	// TODO: Validate map bounderies.
	v.mX = (float)(v.mX - (offsetX/2)) + gRandom->getRand() % (int)(offsetX+1);
	v.mZ = (float)(v.mZ - (offsetZ/2)) + gRandom->getRand() % (int)(offsetZ+1);

	v.mY = this->getHeightAt2DPosition(v.mX, v.mZ);

	/*
	if (Heightmap::isHeightmapCacheAvaliable())
	{
		v.mY = Heightmap::Instance()->getCachedHeightAt2DPosition(v.mX, v.mZ);
	}
	else
	{
		v.mY = Heightmap::Instance()->getHeight(v.mX, v.mZ ); 
	}
	if (v.mY == FLT_MIN)
	{
		assert(false);
	}
	*/
	// gLogger->logMsgF("NPCObject::getRandomPosition: %.0f, %.0f, %.0f ", MSG_NORMAL, v.mX, v.mY, v.mZ);
	return v;
}

//=============================================================================
//
//	Retrieve the height for a given 2D x,z position.
//

float NPCObject::getHeightAt2DPosition(float xPos, float zPos, bool bestOffer) const
{
	float yPos = FLT_MIN;

	if (Heightmap::isHeightmapCacheAvaliable() && (Heightmap::Instance()->isHighResCache() || !bestOffer))
	{
		yPos = Heightmap::Instance()->getCachedHeightAt2DPosition(xPos, zPos);
	}
	else
	{
		yPos = Heightmap::Instance()->getHeight(xPos, zPos); 
	}
	if (yPos == FLT_MIN)
	{
		assert(false);
	}
	return yPos;
}


//=============================================================================
//
//	Set the direction of the npc.
//

void NPCObject::setDirection(float deltaX, float deltaZ)
{
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
			this->mDirection.mW = static_cast<float>(cos((3.14159354 * 0.5) + 0.5f*acos(-z/h)));
			this->mDirection.mY = static_cast<float>(sin((3.14159354 * 0.5) + 0.5f*acos(-z/h)));
		}
		else
		{
			this->mDirection.mY = static_cast<float>(sin(0.5f*acos(z/h)));
			this->mDirection.mW = static_cast<float>(cos(0.5f*acos(z/h)));
		}
	}
	else
	{
		this->mDirection.mY = static_cast<float>(sin(0.5f*asin(x/h)));	
		this->mDirection.mW = static_cast<float>(cos(0.5f*acos(z/h)));
	}

	// send out position updates to known players
	if (this->getKnownPlayers()->empty())
	{
		return;
	}

	this->setInMoveCount(this->getInMoveCount() + 1);

	if (this->getParentId())
	{
		// We are inside a cell.
		gMessageLib->sendDataTransformWithParent(this);
		gMessageLib->sendUpdateTransformMessageWithParent(this);
	}
	else
	{
		gMessageLib->sendDataTransform(this);
		gMessageLib->sendUpdateTransformMessage(this);
	}
}

//=============================================================================
//
//	Move npc and update position in game world.
//

void NPCObject::moveAndUpdatePosition(void)
{
	Anh_Math::Vector3 position(this->mPosition);

	if (!Heightmap::isHeightmapCacheAvaliable())
	{
		position += this->getPositionOffset();
	}
	else
	{
		// Testing to actually use a somewhat real height value.
		position.mX += this->getPositionOffset().mX;
		position.mZ += this->getPositionOffset().mZ;
		position.mY = Heightmap::Instance()->getCachedHeightAt2DPosition(position.mX, position.mZ);
	}
	// send out position updates to known players
	this->updatePosition(this->getParentId(),position);

}

//=============================================================================
//
//	Update npc position in game world.
//

void NPCObject::updatePosition(uint64 parentId, Anh_Math::Vector3 newPosition)
{
	// gLogger->logMsgF("NPCObject::updatePosition: update NPC position)", MSG_NORMAL);

	if (parentId == 0)
	{
		if (this->getParentId() != 0)
		{
			// if we just left a building
			if (!this->getKnownPlayers()->empty())
			{
				gMessageLib->broadcastContainmentMessage(this->getId(),this->getParentId(),0,this);
			}

			// remove us from the last cell we were in
			if (CellObject* cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(this->getParentId())))
			{
				cell->removeChild(this);
			}
			else
			{
				gLogger->logMsgF("Error removing %llu from cell(%lld)",MSG_HIGH,this->getId(),this->getParentId());
			}

			// we are outside again
			this->setParentId(0);

			this->mPosition = newPosition;

			// Add us to the world.
			if (!this->getKnownPlayers()->empty())
			{
				gMessageLib->broadcastContainmentMessage(this->getId(),0,4,this);
			}

			// add us to the qtree
			if (QTRegion* newRegion = gWorldManager->getSI()->getQTRegion((double)this->mPosition.mX,(double)this->mPosition.mZ))
			{
				this->setSubZoneId((uint32)newRegion->getId());
				newRegion->mTree->addObject(this);
			}
			else
			{
				// we should never get here !
				gLogger->logMsg("NPCObject::updatePosition: could not find zone region in map\n");
			}
		}
		else
		{
			// We are still outside.

			// get the qt of the new position
			// if (QTRegion* newRegion = gWorldManager->getSI()->getQTRegion((double)this->mPosition.mX,(double)this->mPosition.mZ))
			if (QTRegion* newRegion = gWorldManager->getSI()->getQTRegion((double)newPosition.mX, (double)newPosition.mZ))
			{
				// we didnt change so update the old one
				if((uint32)newRegion->getId() == this->getSubZoneId())
				{
					// this also updates the object (npcs) position
					newRegion->mTree->updateObject(this, newPosition);
				}
				else
				{
					// remove from old
					if (QTRegion* oldRegion = gWorldManager->getQTRegion(this->getSubZoneId()))
					{
						oldRegion->mTree->removeObject(this);
					}

					// put into new
					this->mPosition = newPosition;
					this->setSubZoneId((uint32)newRegion->getId());
					newRegion->mTree->addObject(this);
				}			
			}
		}
	}
	else
	{
		uint64 oldParentId = this->getParentId();
		if (oldParentId != parentId)
		{
			// We changed cell
			CellObject* cell = NULL;

			// Remove us.
			if (!this->getKnownPlayers()->empty())
			{
				gMessageLib->broadcastContainmentMessage(this->getId(),oldParentId,0,this);
			}
			// gLogger->logMsgF("NPC changed cell from (%lld) to (%lld)",MSG_NORMAL, oldParentId, parentId);

			// only remove us from si, if we just entered the building
			if (oldParentId != 0) 
			{
				// We are still inside.
				if (cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(oldParentId)))
				{
					cell->removeChild(this);
				}
				else
				{
					gLogger->logMsgF("Error removing %lld from cell(%lld)",MSG_NORMAL,this->getId(),oldParentId);
				}
			}
			else
			{
				// remove us from qt
				// We just entered a building.
				if (this->getSubZoneId())
				{
					if (QTRegion* region = gWorldManager->getQTRegion(this->getSubZoneId()))
					{
						this->setSubZoneId(0);
						region->mTree->removeObject(this);
					}
				}
			}
				
			// put us into new one
			if (!this->getKnownPlayers()->empty())
			{
				gMessageLib->broadcastContainmentMessage(this->getId(),parentId,4,this);
			}
			if (cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(parentId)))
			{
				cell->addChild(this);
			}
			else
			{
				gLogger->logMsgF("Error adding %lld to cell(%lld)",MSG_NORMAL,this->getId(),parentId);
			}
			// update the player
			this->setParentId(parentId);
		}
	}
	this->mPosition = newPosition;

	// send out position updates to known players
	this->setInMoveCount(this->getInMoveCount() + 1);
	
	//check whether updates are necessary before building the packet and then destroying it
	if (this->getKnownPlayers()->empty())
	{
		return;
	}

	if (this->getParentId())
	{
		// We are inside a cell.
		// gMessageLib->sendDataTransformWithParent(this);
		gMessageLib->sendUpdateTransformMessageWithParent(this);
	}
	else
	{
		// gMessageLib->sendDataTransform(this);
		gMessageLib->sendUpdateTransformMessage(this);
	}
}


//=============================================================================
//
//	Set AI state.
//
void NPCObject::setAiState(NPCObject::Npc_AI_State state)
{
	mAiState = state;
}

//=============================================================================
//
//	Get AI state.
//
NPCObject::Npc_AI_State NPCObject::getAiState(void) const
{
	return mAiState;
}


//=============================================================================
//
//	Set a random direction.
//

void NPCObject::setRandomDirection(void)
{

	int64 ux = (int64)(gRandom->getRand() % (int32)201);	// 0 <= ux <= 200
	int64 uz = (int64)(gRandom->getRand() % (int32)201);	// 0 <= uz <= 200

	// Let's turn to the direction we are heading.
	float x = (float)(ux - 100);	// -100.0 <= x <= 100.0
	float z = (float)(uz - 100);	// -100.0 <= z <= 100.0
	
	// gLogger->logMsgF("NPCObject::setRandomDirection: %.0f, %.0f", MSG_NORMAL, x, z);
	
	float h = sqrt(x*x + z*z);

	// if ((z/h) < 0.0) h always positive.
	if (z < 0.0)
	{	
		// if (x/h < 0.0) h always positive.
		if (x < 0.0)
		{
			this->mDirection.mW = static_cast<float>(cos((3.14159354 * 0.5) + 0.5f*acos(-z/h)));
			this->mDirection.mY = static_cast<float>(sin((3.14159354 * 0.5) + 0.5f*acos(-z/h)));
		}
		else
		{
			this->mDirection.mY = sin(0.5f*acos(z/h));
			this->mDirection.mW = cos(0.5f*acos(z/h));
		}
	}
	else
	{
		this->mDirection.mY = sin(0.5f*asin(x/h));	
		this->mDirection.mW = cos(0.5f*acos(z/h));
	}
	// gLogger->logMsgF("Translates to: %.3f, %.3f", MSG_NORMAL, this->mDirection.mY, this->mDirection.mW);
}

//=============================================================================
//
//	Track all that do damage to me.
//

void NPCObject::updateDamage(uint64 playerId, uint64 groupId, uint32 weaponGroup, int32 damage, uint8 attackerPosture, float attackerDistance)
{
	// gLogger->logMsgF("NPCObject::updateDamage() %d", MSG_NORMAL, damage);

	// Players alone, player in group A, same player in group B, another player in group A...
	DamageDealer* damageDealer = NULL;
	DamageDealers::iterator it = mDamageDealers.begin();
	while (it != mDamageDealers.end())
	{
		if (((*it)->mPlayerId == playerId) && ((*it)->mGroupId == groupId))
		{
			// Already have this attacker in list. Let's update damage and weapon usage.
			(*it)->mDamage += damage;
			(*it)->mWeaponGroupMasks |= weaponGroup;
			damageDealer = (*it);
			break;
		}
		it++;
	}
	
	if (it == mDamageDealers.end())
	{
		// Add this attacker.
		// gLogger->logMsgF("New damage dealer, id = %llu, group %llu", MSG_NORMAL, playerId, groupId);
		damageDealer = new DamageDealer();
		damageDealer->mPlayerId = playerId;
		damageDealer->mGroupId = groupId;
		damageDealer->mWeaponGroupMasks = weaponGroup;
		damageDealer->mDamage = damage;
		damageDealer->mAggroPoints = 0;
		mDamageDealers.push_back(damageDealer);
	}

	// If attacker are within creature aggro range, give max aggro whatever attacker posture.
	assert(damageDealer);
	if (damageDealer->mAggroPoints < 100.1)
	{
		// float aggroPoints = 4;		// This is max aggro you can get at one single occation when doing damage and be in range of the creature aggro.
		float aggroPoints = mBaseAggro;

		if (attackerDistance > this->getAttackRange())
		{
			// Attacker is outside attacking range.
			if (attackerPosture == CreaturePosture_Prone)
			{
				// aggroPoints = 2.5;
				aggroPoints *= 0.5625; // 0.75 * 0.75
			}
			else
			{
				aggroPoints *= 0.75;
			}

			if (attackerDistance <= 64.0)
			{
				if (attackerDistance < 32.0)
				{
					aggroPoints /= 0.75;
				}
				else
				{
					assert(attackerDistance != 0);
					aggroPoints *= static_cast<float>(64.0 / attackerDistance);
				}
			}
		}
		damageDealer->mAggroPoints += aggroPoints;
		// gLogger->logMsgF("NPCObject::updateDamage() Damage=%d aggro = %.0f total aggro = %.0f", MSG_NORMAL, damage, aggroPoints, damageDealer->mAggroPoints);
	}
}

//=============================================================================
//
//	Track aggro made.
//

void NPCObject::updateAggro(uint64 playerId, uint64 groupId, uint8 attackerPosture)
{
	// gLogger->logMsgF("NPCObject::updateAggro()", MSG_NORMAL);

	DamageDealer* damageDealer = NULL;
	DamageDealers::iterator it = mDamageDealers.begin();
	while (it != mDamageDealers.end())
	{
		if (((*it)->mPlayerId == playerId) && ((*it)->mGroupId == groupId))
		{
			// Already have this attacker in list. Let's update damage and weapon usage.
			// gLogger->logMsgF("Already have this attacker in list.", MSG_NORMAL);
			damageDealer = (*it);
			break;
		}
		it++;
	}
	
	if (it == mDamageDealers.end())
	{
		// Add aggro for this attacker.
		// gLogger->logMsgF("New aggro, id = %llu, group %llu", MSG_NORMAL, playerId, groupId);
		damageDealer = new DamageDealer();
		damageDealer->mPlayerId = playerId;
		damageDealer->mGroupId = groupId;
		damageDealer->mWeaponGroupMasks = 0;
		damageDealer->mDamage = 0;
		damageDealer->mAggroPoints = 0;
		mDamageDealers.push_back(damageDealer);
	}

	assert(damageDealer);
	if ((attackerPosture == CreaturePosture_Incapacitated) || (attackerPosture == CreaturePosture_Dead))
	{
		// gLogger->logMsgF("Removed aggro, target is incapped or dead.", MSG_NORMAL);
		damageDealer->mAggroPoints = 0;

		// Let's this player rebuild is aggro before we attack him again.
		/*
		if (this->mLairNeedAsssistanceWithId == playerId)
		{
			this->mAsssistanceNeededWithId = 0;
			this->mLairNeedAsssistanceWithId = 0;
			this->mIsAssistingLair = false;
		}
		*/
	}
	else
	{
		if (damageDealer->mAggroPoints < 100.1)
		{
			// This is max aggro you can get at one single occation by just be in range of the creature aggro.
			// Attacker have not yet attacked this creature.
			float aggroPoints;
			if (attackerPosture == CreaturePosture_Prone)
			{
				// aggroPoints = 1.5;
				aggroPoints = static_cast<float>(mBaseAggro * 0.5625);
			}
			else
			{
				aggroPoints = static_cast<float>(mBaseAggro * 0.75);
			}
			damageDealer->mAggroPoints += aggroPoints;
		}
	}
}

//=============================================================================
//
//	Attacker got aggro?
//

bool NPCObject::attackerHaveAggro(uint64 attackerId)
{
	bool aggro = false;
	float aggroPoints = 0;

	// Check all occurnaces, since switching between groups does not matter here.
	DamageDealers::iterator it = mDamageDealers.begin();
	while (it != mDamageDealers.end())
	{
		if (attackerId == (*it)->mPlayerId)
		{
			aggroPoints += (*it)->mAggroPoints;
		}
		if (aggroPoints > 100.0)
		{
			aggro = true;
			break;
		}
		it++;
	}
	/*
	if (aggro)
	{
		gLogger->logMsgF("AttackableCreature::attackerHaveAggro = TRUE", MSG_NORMAL);
	}
	else
	{
		gLogger->logMsgF("AttackableCreature::attackerHaveAggro = FALSE", MSG_NORMAL);
	}
	*/
	return aggro;
}



//=============================================================================
//
//	Grant XP to the ones that deserve it.
//

void NPCObject::updateAttackersXp(void)
{
	// gLogger->logMsgF("NPCObject::updateAttackersXp() Entering", MSG_NORMAL);

	// First we need to figure out who has done most damage. 
	
	// It can be an individual player or a group of players.
	// If a player has made damage before or after he was a group member, that damage will not count into the group damage.
	// It will be added to his damage as an individual player.

	// Weapons used by a player before or after he was a group member, does not give XP if a group has done the most damage.
	// It will give him XP if he as an individual player has done the most damage.

	// You will only be given XP for the weapon types you used in the group, given that the group has done the most damage.

	// Start with the easy part, get the individual players that have done the most damage.
	int32 topDamageByPlayer = 0;
	uint64 topPlayerId = 0;
	int32 topPlayerWeaponMasks = 0;

	DamageDealers::iterator it = mDamageDealers.begin();
	while (it != mDamageDealers.end())
	{
		// gLogger->logMsgF("Handling Player id %llu, group %llu, damage %u", MSG_NORMAL, (*it)->mPlayerId, (*it)->mGroupId, (*it)->mDamage);
		if ((*it)->mGroupId == 0)
		{
			// gLogger->logMsgF("Player %llu hit with %u", MSG_NORMAL, (*it)->mPlayerId, (*it)->mDamage);
			if ((*it)->mDamage >= topDamageByPlayer)
			{
				topDamageByPlayer = (*it)->mDamage;
				topPlayerId = (*it)->mPlayerId;
				topPlayerWeaponMasks = (*it)->mWeaponGroupMasks;
			}
		}
		else
		{
			// This was damage dealt with someone grouped.
			updateGroupDamage(*it);
		}
		it++;
	}

	int32 topDamageByGroup = 0;
	uint64 groupTopId = 0;

	// We have to go through the group list now, and find out the group doing the most damage.
	DamageDealers::iterator groupIt = mDamageByGroups.begin();
	while (groupIt != mDamageByGroups.end())
	{
		// gLogger->logMsgF("Group %llu hit with %u", MSG_NORMAL, (*groupIt)->mGroupId, (*groupIt)->mDamage);
		if ((*groupIt)->mDamage >= topDamageByGroup)
		{
			topDamageByGroup = (*groupIt)->mDamage;
			groupTopId = (*groupIt)->mGroupId;
		}
		groupIt++;
	}

	// The weapon xp will be dived equally between your weapons used, 
	// (and the rest is a comlete made up rule... )
	// mapped to the average HAM value of this creature, lol.

	// TODO: Use real data for the experiance gained.
	// int32 damageDone = this->getHam()->mHealth.getMaxHitPoints();
	// damageDone += this->getHam()->mAction.getMaxHitPoints();
	// damageDone += this->getHam()->mMind.getMaxHitPoints();
	// damageDone = damageDone/6;	// Use half of average HAM.

	int32 weaponXp = this->getWeaponXp();

	// And the winner is....
	if (topDamageByPlayer >= topDamageByGroup)
	{
		// gLogger->logMsgF("Player %llu did the most damage", MSG_NORMAL, topPlayerId);
		// An individual player did the most damage.
		this->updateAttackersWeaponAndCombatXp(topPlayerId, 0, weaponXp, topPlayerWeaponMasks);
		this->mLootAllowedById = topPlayerId;
	}
	else
	{
		// gLogger->logMsgF("Group %llu did the most damage", MSG_NORMAL, groupTopId);
		// The players in this group have to divide the XP, any group bonuses are not implemented yet.
		this->mLootAllowedById = groupTopId;

		// But first, we need to find out how many players we are (have been) in this group during the fight that have done any damage.
		uint32 membersInGroup = 0;
		it = mDamageDealers.begin();
		while (it != mDamageDealers.end())
		{
			if ((*it)->mGroupId == groupTopId)
			{
				// We have found a player belonging to this winning group.
				membersInGroup++;
			}
			it++;
		}

		// Now let's update the players beloning to this group, if still exist, and if players are in range.
		if (membersInGroup != 0)
		{
			it = mDamageDealers.begin();
			while (it != mDamageDealers.end())
			{
				if ((*it)->mGroupId == groupTopId)
				{
					// We have found a player belonging to this winning group.
					this->updateAttackersWeaponAndCombatXp((*it)->mPlayerId, groupTopId, weaponXp/membersInGroup, (*it)->mWeaponGroupMasks);
				}
				it++;
			}
		}
	}
}

//=============================================================================
//
//	Track all that do damage to me.
//

void NPCObject::updateGroupDamage(DamageDealer* damageDealer)
{
	DamageDealers::iterator it = mDamageByGroups.begin();
	while (it != mDamageByGroups.end())
	{
		// gLogger->logMsgF("Comparing group %llu with existing group %llu", MSG_NORMAL, (*it)->mGroupId, damageDealer->mGroupId);
		if ((*it)->mGroupId == damageDealer->mGroupId)
		{
			// gLogger->logMsgF("Adding group %llu with damage %u", MSG_NORMAL, damageDealer->mGroupId, damageDealer->mDamage);
			// Already have this group as an attacker in list. Let's update damage.
			(*it)->mDamage += damageDealer->mDamage;
			break;
		}
		it++;
	}
	
	if (it == mDamageByGroups.end())
	{
		// gLogger->logMsgF("New group as damage dealer, group = %llu, damage = %u", MSG_NORMAL, damageDealer->mGroupId, damageDealer->mDamage);
		// Add this attacker.
		DamageDealer* groupDamageDealer = new DamageDealer();
		groupDamageDealer->mGroupId = damageDealer->mGroupId;
		groupDamageDealer->mDamage = damageDealer->mDamage;
		mDamageByGroups.push_back(groupDamageDealer);
	}
}


//=============================================================================
//
//	Update attacker
//
void NPCObject::updateAttackersWeaponAndCombatXp(uint64 playerId, uint64 groupId, int32 damageDone, int32 weaponUsedMask)
{
	// Combat XP is one tenth of the total weapon XP.
	int32 combatXp = damageDone/10;
	uint32 noOfWeaponsUsed = 0;

	// Now we need to figure out how many weapons that was involved in this mess.
	for (uint32 i = WeaponGroup_Unarmed; i < WeaponGroup_Flamethrower; i = i << 1)
	{
		if ((i & weaponUsedMask) ==  i)
		{
			// We got a weapon type.
			noOfWeaponsUsed++;
		}
	}

	// Here is the XP to grant for each type of damage done.
	if (noOfWeaponsUsed)	
	{
		int32 weaponXp = damageDone/(noOfWeaponsUsed);

		if (PlayerObject* playerObject = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(playerId)))
		{
			// Only update XP for players in range and still in group, if specified.
			if (playerObject && this->checkKnownPlayer(playerObject))
			{
				bool valid = true;
				if (groupId)
				{
					valid = (playerObject->getGroupId() == (groupId));
				}
				if (valid)
				{
					this->sendAttackersWeaponXp(playerObject, weaponUsedMask, weaponXp);
					gSkillManager->addExperience(XpType_combat_general, combatXp, playerObject);
				}
			}
		}
	}
}


//=============================================================================
//
//	Update attackers XP.
//

void NPCObject::sendAttackersWeaponXp(PlayerObject* playerObject, uint32 weaponMask, int32 xp)
{
	// Valid input?
	if (playerObject && weaponMask && xp)
	{
		// Then, we have to have a connected player..still alive.
		if (playerObject->isConnected() && !playerObject->isDead())
		{
			// Now send the XP update messages to player.
			for (uint32 i = WeaponGroup_Unarmed; i < WeaponGroup_Flamethrower; i = i << 1)
			{
				switch (i & weaponMask)
				{
					case WeaponGroup_Unarmed:
						gSkillManager->addExperience(XpType_combat_meleespecialize_unarmed, xp, playerObject);
						break;

					case WeaponGroup_1h:
						gSkillManager->addExperience(XpType_combat_meleespecialize_onehand, xp, playerObject);
						break;

					case WeaponGroup_2h:
						gSkillManager->addExperience(XpType_combat_meleespecialize_twohand, xp, playerObject);
						break;

					case WeaponGroup_Polearm:
						gSkillManager->addExperience(XpType_combat_meleespecialize_polearm, xp, playerObject);
						break;

					case WeaponGroup_Rifle:
						gSkillManager->addExperience(XpType_combat_rangedspecialize_rifle, xp, playerObject);
						break;

					case WeaponGroup_Pistol:
						gSkillManager->addExperience(XpType_combat_rangedspecialize_pistol, xp, playerObject);
						break;

					case WeaponGroup_Carbine:
						gSkillManager->addExperience(XpType_combat_rangedspecialize_carbine, xp, playerObject);
						break;

					default:
						break;
				}
			}
		}
	}
}

//=============================================================================
//
//	Returns true if target is allowed to loot.
//

bool NPCObject::allowedToLoot(uint64 targetId, uint64 groupId)
{
	return ((targetId == mLootAllowedById) || (groupId == mLootAllowedById));
}

