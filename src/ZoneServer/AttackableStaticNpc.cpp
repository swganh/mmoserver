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

#include "AttackableStaticNpc.h"
#include "CellObject.h"
#include "PlayerObject.h"
#include "QuadTree.h"
#include "WorldConfig.h"
#include "WorldManager.h"
#include "ZoneTree.h"

#include "MessageLib/MessageLib.h"
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
        BString effect = gWorldManager->getClientEffect(mDeathEffectId);
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
        gLogger->log(LogManager::DEBUG,"AttackableStaticNpc::prepareCustomRadialMenu IS attackable\n");

        // mRadialMenu = RadialMenuPtr(new RadialMenu());
        mRadialMenu->addItem(1,0,radId_combatAttack,radAction_Default);
        mRadialMenu->addItem(2,0,radId_examine,radAction_Default);
    }
    else
    {
        gLogger->log(LogManager::DEBUG,"AttackableStaticNpc::prepareCustomRadialMenu is NOT attackable\n");
        mRadialMenu->addItem(1,0,radId_examine,radAction_Default);
    }
}

void AttackableStaticNpc::respawn(void)
{
    gLogger->log(LogManager::DEBUG,"AttackableStaticNpc::respawn: Added new static object for spawn, with id = %"PRIu64"", this->getId());

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


    /*
    if (this->hasAttribute("creature_action"))
    {
    	int32 action = this->getAttribute<int32>("creature_action");
    	this->mHam.mAction.setCurrentHitPoints(action);
    	this->mHam.mAction.setMaxHitPoints(action);
    	this->mHam.mAction.setBaseHitPoints(action);
    }
    else
    {
    	assert(false);
    	this->mHam.mAction.setCurrentHitPoints(500);
    	this->mHam.mAction.setMaxHitPoints(500);
    	this->mHam.mAction.setBaseHitPoints(500);
    }

    if (this->hasAttribute("creature_quickness"))
    {
    	int32 quickness = this->getAttribute<int32>("creature_quickness");
    	this->mHam.mQuickness.setCurrentHitPoints(quickness);
    	this->mHam.mQuickness.setMaxHitPoints(quickness);
    	this->mHam.mQuickness.setBaseHitPoints(quickness);
    }
    else
    {
    	assert(false);
    	this->mHam.mQuickness.setCurrentHitPoints(500);
    	this->mHam.mQuickness.setMaxHitPoints(500);
    	this->mHam.mQuickness.setBaseHitPoints(500);
    }

    if (this->hasAttribute("creature_stamina"))
    {
    	int32 stamina = this->getAttribute<int32>("creature_stamina");
    	this->mHam.mStamina.setCurrentHitPoints(stamina);
    	this->mHam.mStamina.setMaxHitPoints(stamina);
    	this->mHam.mStamina.setBaseHitPoints(stamina);
    }
    else
    {
    	assert(false);
    	this->mHam.mStamina.setCurrentHitPoints(500);
    	this->mHam.mStamina.setMaxHitPoints(500);
    	this->mHam.mStamina.setBaseHitPoints(500);
    }


    if (this->hasAttribute("creature_mind"))
    {
    	int32 mind = this->getAttribute<int32>("creature_mind");
    	this->mHam.mMind.setCurrentHitPoints(mind);
    	this->mHam.mMind.setMaxHitPoints(mind);
    	this->mHam.mMind.setBaseHitPoints(mind);
    }
    else
    {
    	assert(false);
    	this->mHam.mMind.setCurrentHitPoints(500);
    	this->mHam.mMind.setMaxHitPoints(500);
    	this->mHam.mMind.setBaseHitPoints(500);
    }

    if (this->hasAttribute("creature_focus"))
    {
    	int32 focus = this->getAttribute<int32>("creature_focus");
    	this->mHam.mFocus.setCurrentHitPoints(focus);
    	this->mHam.mFocus.setMaxHitPoints(focus);
    	this->mHam.mFocus.setBaseHitPoints(focus);
    }
    else
    {
    	assert(false);
    	this->mHam.mFocus.setCurrentHitPoints(500);
    	this->mHam.mFocus.setMaxHitPoints(500);
    	this->mHam.mFocus.setBaseHitPoints(500);
    }

    if (this->hasAttribute("creature_willpower"))
    {
    	int32 willpower = this->getAttribute<int32>("creature_willpower");
    	this->mHam.mWillpower.setCurrentHitPoints(willpower);
    	this->mHam.mWillpower.setMaxHitPoints(willpower);
    	this->mHam.mWillpower.setBaseHitPoints(willpower);
    }
    else
    {
    	assert(false);
    	this->mHam.mWillpower.setCurrentHitPoints(500);
    	this->mHam.mWillpower.setMaxHitPoints(500);
    	this->mHam.mWillpower.setBaseHitPoints(500);
    }
    */
    this->mHam.calcAllModifiedHitPoints();


    // All init is done, just the spawn in the world is left.
    this->spawn();
}

//=============================================================================
//
//	Spawn.
//

void AttackableStaticNpc::spawn(void)
{
    // Update the world about my presence.

    if (this->getParentId())
    {
        // insert into cell
        this->setSubZoneId(0);

        if (CellObject* cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(this->getParentId())))
        {
            cell->addObjectSecure(this);
        }
        else
        {
            gLogger->log(LogManager::DEBUG,"AttackableStaticNpc::spawn: couldn't find cell %"PRIu64"\n", this->getParentId());

            // It's a serious isse that we need to investigate.
            assert(cell);
        }
    }
    else
    {
        if (QTRegion* region = gWorldManager->getSI()->getQTRegion(this->mPosition.x, this->mPosition.z))
        {
            this->setSubZoneId((uint32)region->getId());
            region->mTree->addObject(this);
        }
    }

    // Add us to the world.
    gMessageLib->broadcastContainmentMessage(this,this->getParentId(),4);

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
                gLogger->log(LogManager::DEBUG,"AttackableStaticNpc::spawn: Spawned a private object.");
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
                gLogger->log(LogManager::DEBUG,"AttackableStaticNpc::spawn: Failed to spawn a private object.");
            }
        }
    }
    else
    {
        gLogger->log(LogManager::DEBUG,"AttackableStaticNpc::spawn: Spawned an object.");
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
