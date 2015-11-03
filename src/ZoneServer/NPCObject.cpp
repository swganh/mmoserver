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

#include "NPCObject.h"

#include "Heightmap.h"
#include "CellObject.h"
#include "PlayerObject.h"
#include "Weapon.h"
#include "WorldManager.h"

#include "MessageLib/MessageLib.h"

#include "Utils/rand.h"

#include <cassert>
#include <cfloat>

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

NPCObject::NPCObject()
    : CreatureObject()
    , mLastConversationRequest(0)
    , mLastConversationTarget(0)
    , mSpeciesId(0)
    , mAiState(NpcIsDormant)
    , mAttackRange(64)
    , mBaseAggro(0)
    , mCellIdForSpawn(0)
    , mLootAllowedById(0)
    , mLootGroupId(0)
    , mNpcTemplateId(0)
    , mRespawnDelay(0)
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

glm::vec3 NPCObject::getRandomPosition(const glm::vec3& currentPos, int32 offsetX, int32 offsetZ) const
{
    glm::vec3 v(currentPos);

    // TODO: Validate map bounderies.
    v.x = (float)(v.x - (offsetX/2)) + gRandom->getRand() % (int)(offsetX+1);
    v.z = (float)(v.z - (offsetZ/2)) + gRandom->getRand() % (int)(offsetZ+1);

    v.y = this->getHeightAt2DPosition(v.x, v.z);

    /*
    if (Heightmap::isHeightmapCacheAvaliable())
    {
    	v.y = Heightmap::Instance()->getCachedHeightAt2DPosition(v.x, v.z);
    }
    else
    {
    	v.y = Heightmap::Instance()->getHeight(v.x, v.z );
    }
    if (v.y == FLT_MIN)
    {
    	assert(false);
    }
    */
    return v;
}

//=============================================================================
//
//	Retrieve the height for a given 2D x,z position.
//

float NPCObject::getHeightAt2DPosition(float xPos, float zPos, bool bestOffer) const
{
    float yPos = FLT_MIN;

    if (Heightmap::isHeightmapCacheAvaliable() && (gHeightmap->isHighResCache() || !bestOffer))
    {
        yPos = gHeightmap->getCachedHeight(xPos, zPos);
    }
    else
    {
        yPos = gHeightmap->getHeight(xPos, zPos);
    }
    if (yPos == FLT_MIN)
    {
        assert(false && "NPCObject::getCachedHeight unable to get height");
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
            this->mDirection.w = static_cast<float>(cos((3.14159354 * 0.5) + 0.5f*acos(-z/h)));
            this->mDirection.y = static_cast<float>(sin((3.14159354 * 0.5) + 0.5f*acos(-z/h)));
        }
        else
        {
            this->mDirection.y = static_cast<float>(sin(0.5f*acos(z/h)));
            this->mDirection.w = static_cast<float>(cos(0.5f*acos(z/h)));
        }
    }
    else
    {
        this->mDirection.y = static_cast<float>(sin(0.5f*asin(x/h)));
        this->mDirection.w = static_cast<float>(cos(0.5f*acos(z/h)));
    }

    
	// send out position updates to known players
	if (this->getRegisteredWatchers()->empty())
	{
		return;
	}

	this->setInMoveCount(this->getInMoveCount() + 1);

	if (this->getParentId())
	{
		// We are inside a cell.
		gMessageLib->sendDataTransformWithParent053(this);
		gMessageLib->sendUpdateTransformMessageWithParent(this);
	}
	else
	{
		gMessageLib->sendDataTransform053(this);
		gMessageLib->sendUpdateTransformMessage(this);
	}
}

//=============================================================================
//
//	Move npc and update position in game world.
//

void NPCObject::moveAndUpdatePosition(void)
{
    glm::vec3 position(this->mPosition);

    if (!Heightmap::isHeightmapCacheAvaliable())
    {
        position += this->getPositionOffset();
    }
    else
    {
        // Testing to actually use a somewhat real height value.
        position.x += this->getPositionOffset().x;
        position.z += this->getPositionOffset().z;
        position.y = gHeightmap->getCachedHeight(position.x, position.z);
    }
    // send out position updates to known players
    this->updatePosition(this->getParentId(),position);

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

    float h = sqrt(x*x + z*z);

    // if ((z/h) < 0.0) h always positive.
    if (z < 0.0)
    {
        // if (x/h < 0.0) h always positive.
        if (x < 0.0)
        {
            this->mDirection.w = static_cast<float>(cos((3.14159354 * 0.5) + 0.5f*acos(-z/h)));
            this->mDirection.y = static_cast<float>(sin((3.14159354 * 0.5) + 0.5f*acos(-z/h)));
        }
        else
        {
            this->mDirection.y = sin(0.5f*acos(z/h));
            this->mDirection.w = cos(0.5f*acos(z/h));
        }
    }
    else
    {
        this->mDirection.y = sin(0.5f*asin(x/h));
        this->mDirection.w = cos(0.5f*acos(z/h));
    }
}

//=============================================================================
//
//	Track all that do damage to me.
//

void NPCObject::updateDamage(uint64 playerId, uint64 groupId, uint32 weaponGroup, int32 damage, uint8 attackerPosture, float attackerDistance)
{

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
        damageDealer = new DamageDealer();
        damageDealer->mPlayerId = playerId;
        damageDealer->mGroupId = groupId;
        damageDealer->mWeaponGroupMasks = weaponGroup;
        damageDealer->mDamage = damage;
        damageDealer->mAggroPoints = 0;
        mDamageDealers.push_back(damageDealer);
    }

    // If attacker are within creature aggro range, give max aggro whatever attacker posture.
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
                    aggroPoints *= static_cast<float>(64.0 / attackerDistance);
                }
            }
        }
        damageDealer->mAggroPoints += aggroPoints;
    }
}

//=============================================================================
//
//	Track aggro made.
//

void NPCObject::updateAggro(uint64 playerId, uint64 groupId, uint8 attackerPosture)
{

    DamageDealer* damageDealer = NULL;
    DamageDealers::iterator it = mDamageDealers.begin();
    while (it != mDamageDealers.end())
    {
        if (((*it)->mPlayerId == playerId) && ((*it)->mGroupId == groupId))
        {
            // Already have this attacker in list. Let's update damage and weapon usage.
            damageDealer = (*it);
            break;
        }
        it++;
    }

    if (it == mDamageDealers.end())
    {
        // Add aggro for this attacker.
        damageDealer = new DamageDealer();
        damageDealer->mPlayerId = playerId;
        damageDealer->mGroupId = groupId;
        damageDealer->mWeaponGroupMasks = 0;
        damageDealer->mDamage = 0;
        damageDealer->mAggroPoints = 0;
        mDamageDealers.push_back(damageDealer);
    }

    if ((attackerPosture == CreaturePosture_Incapacitated) || (attackerPosture == CreaturePosture_Dead))
    {
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
    return aggro;
}



//=============================================================================
//
//	Grant XP to the ones that deserve it.
//

void NPCObject::updateAttackersXp(void)
{

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
        if ((*it)->mGroupId == 0)
        {
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
        // An individual player did the most damage.
        this->updateAttackersWeaponAndCombatXp(topPlayerId, 0, weaponXp, topPlayerWeaponMasks);
        this->mLootAllowedById = topPlayerId;
    }
    else
    {
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
        if ((*it)->mGroupId == damageDealer->mGroupId)
        {
            // Already have this group as an attacker in list. Let's update damage.
            (*it)->mDamage += damageDealer->mDamage;
            break;
        }
        it++;
    }

    if (it == mDamageByGroups.end())
    {
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
			if (playerObject && this->checkRegisteredWatchers(playerObject))
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

