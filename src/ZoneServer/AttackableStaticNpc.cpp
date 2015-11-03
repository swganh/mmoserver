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

#include "ZoneServer/AttackableStaticNpc.h"

#include "MessageLib/MessageLib.h"

#include "ZoneServer/CellObject.h"
#include "ZoneServer/PlayerObject.h"
#include "ZoneServer/WorldConfig.h"
#include "ZoneServer/WorldManager.h"

//=============================================================================

AttackableStaticNpc::AttackableStaticNpc() : NPCObject(), mDeathEffectId(144)
{
    mNpcFamily	= NpcFamily_AttackableObject;

    // Use default radial.
    mRadialMenu = RadialMenuPtr(new RadialMenu());
    // mRadialMenu->addItem(1,0,radId_combatAttack,radAction_Default);
    // mRadialMenu->addItem(2,0,radId_examine,radAction_Default);
}

//=============================================================================

AttackableStaticNpc::~AttackableStaticNpc()
{
    mRadialMenu.reset();
}

//=============================================================================

void AttackableStaticNpc::playDeathAnimation(void)
{
    if (mDeathEffectId != 0)
    {
        std::string effect = gWorldManager->getClientEffect(mDeathEffectId);
        gMessageLib->sendPlayClientEffectObjectMessage(effect,"",this);
    }
}

//=============================================================================

void AttackableStaticNpc::prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount)
{
    // For test.
    // PlayerObject* playerObject = dynamic_cast<PlayerObject*>(creatureObject);
    // gMessageLib->sendUpdatePvpStatus(this, playerObject);

    mRadialMenu.reset();
    mRadialMenu = RadialMenuPtr(new RadialMenu());

    if (this->checkPvPState(CreaturePvPStatus_Attackable))
    {

        // mRadialMenu = RadialMenuPtr(new RadialMenu());
        mRadialMenu->addItem(1,0,radId_combatAttack,radAction_Default);
        mRadialMenu->addItem(2,0,radId_examine,radAction_Default);
    }
    else
    {
        mRadialMenu->addItem(1,0,radId_examine,radAction_Default);
    }
}

void AttackableStaticNpc::respawn(void)
{
    // The cell we will spawn in.
    this->setParentId(getCellIdForSpawn());

    // Default spawn position.
    glm::vec3 position(getSpawnPosition());

    // Respawn delay. If the creature have an unique delay, use that. Else use the one provided by the parent object.
    this->setRespawnDelay(0);
    if (this->hasInternalAttribute("creature_respawn_delay"))
    {
        uint64 respawnDelay = this->getInternalAttribute<uint64>("creature_respawn_delay");
        // mRespawnDelay = respawnDelay;
        this->setRespawnDelay(respawnDelay);
    }

    // Let us get the spawn point.
    // Use the supplied direction?
    this->mDirection = getSpawnDirection();

    if (this->getParentId() == 0)
    {
        // Heightmap only works outside.
        position.y = this->getHeightAt2DPosition(position.x, position.z, true);
    }

    this->mPosition = this->getSpawnPosition();		// Default spawn position.

    // mSpawned = false;

    if (this->hasInternalAttribute("creature_xp"))
    {
        uint32 xp = this->getInternalAttribute<uint32>("creature_xp");
        this->setWeaponXp(xp);
    }
    else
    {
        assert(false);
        this->setWeaponXp(0);
    }


    if (this->hasAttribute("creature_health"))
    {
        int32 health = this->getAttribute<int32>("creature_health");
        this->mHam.mHealth.setCurrentHitPoints(health);
        this->mHam.mHealth.setMaxHitPoints(health);
        this->mHam.mHealth.setBaseHitPoints(health);
    }
    else
    {
        assert(false);
        this->mHam.mHealth.setCurrentHitPoints(500);
        this->mHam.mHealth.setMaxHitPoints(500);
        this->mHam.mHealth.setBaseHitPoints(500);
    }

    if (this->hasAttribute("creature_strength"))
    {
        int32 strength = this->getAttribute<int32>("creature_strength");
        this->mHam.mStrength.setCurrentHitPoints(strength);
        this->mHam.mStrength.setMaxHitPoints(strength);
        this->mHam.mStrength.setBaseHitPoints(strength);
    }
    else
    {
        assert(false);
        this->mHam.mStrength.setCurrentHitPoints(500);
        this->mHam.mStrength.setMaxHitPoints(500);
        this->mHam.mStrength.setBaseHitPoints(500);
    }

    if (this->hasAttribute("creature_constitution"))
    {
        int32 constitution = this->getAttribute<int32>("creature_constitution");
        this->mHam.mConstitution.setCurrentHitPoints(constitution);
        this->mHam.mConstitution.setMaxHitPoints(constitution);
        this->mHam.mConstitution.setBaseHitPoints(constitution);
    }
    else
    {
        assert(false);
        this->mHam.mConstitution.setCurrentHitPoints(500);
        this->mHam.mConstitution.setMaxHitPoints(500);
        this->mHam.mConstitution.setBaseHitPoints(500);
    }

    this->mHam.calcAllModifiedHitPoints();


    // All init is done, just the spawn in the world is left.
    this->spawn();
}

//=============================================================================
//
//	Spawn.
//

//where the %*@ did he put the methods for creation ??
//this code requires that the object creates are already send .... grml .. what a mess
void AttackableStaticNpc::spawn() {
    // Update the world about my presence.

	gSpatialIndexManager->createInWorld(this);

    Object* object = gWorldManager->getObjectById(this->getId());
    if(!object)	{
        assert(false);
    }

    // Add us to the world.
    gMessageLib->broadcastContainmentMessage(this, this->getParentId(), 4);

    // send out position updates to known players
    this->setInMoveCount(this->getInMoveCount() + 1);

    if (gWorldConfig->isTutorial())
    {
        // We need to get the player object that is the owner of this npc.
        if (this->getPrivateOwner() != 0)
        {
            PlayerObject* playerObject = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(this->getPrivateOwner()));
            if (playerObject)
            {
                DLOG(INFO) << "AttackableStaticNpc::spawn: Spawned a private object.";
                if (this->getParentId())
                {
                    // We are inside a cell.
                    gMessageLib->sendDataTransformWithParent(this, playerObject);
                    gMessageLib->sendUpdateTransformMessageWithParent(this, playerObject);
                }
                else
                {
                    gMessageLib->sendDataTransform(this, playerObject);
                    gMessageLib->sendUpdateTransformMessage(this, playerObject);
                }
            }
            else
            {
                assert(false);
                DLOG(INFO) << "AttackableStaticNpc::spawn: Failed to spawn a private object.";
            }
        }
    }
    else
    {
        DLOG(INFO) << "AttackableStaticNpc::spawn: Spawned an object.";
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
}
