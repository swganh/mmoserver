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

#include "ZoneServer/GameSystemManagers/NPC Manager/AttackableStaticNpc.h"

#include "MessageLib/MessageLib.h"

#include "ZoneServer/GameSystemManagers/Structure Manager/CellObject.h"
#include "ZoneServer/Objects/Player Object/PlayerObject.h"
#include "ZoneServer/WorldConfig.h"
#include "ZoneServer/WorldManager.h"

#include "ZoneServer\Services\ham\ham_service.h"
#include <ZoneServer\Services\terrain\terrain_service.h>
#include <anh\app\swganh_kernel.h>
#include <anh\service/service_manager.h>



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
        auto terrain = gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::terrain::TerrainService>("TerrainService");
		position.y = terrain->GetHeight(gWorldManager->getZoneId(), position.x, position.z);
    }

    this->mPosition = this->getSpawnPosition();		// Default spawn position.

    // mSpawned = false;
	auto ham = gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::ham::HamService>("HamService");

    if (this->hasInternalAttribute("creature_xp"))
    {
        uint32 xp = this->getInternalAttribute<uint32>("creature_xp");
        this->setWeaponXp(xp);
    }
    else
    {
        LOG(error) << "AttackableStaticNpc::respawn no creature_xp attribute";
        this->setWeaponXp(0);
    }


    if (this->hasAttribute("creature_health"))
    {
        int32 health = this->getAttribute<int32>("creature_health");
		ham->SetHam(this, HamBar_Health, health);
		
    }
    else
    {
        LOG(error) << "AttackableStaticNpc::respawn No health attribute";
        ham->SetHam(this, HamBar_Health, 500);
    }

    if (this->hasAttribute("creature_strength"))
    {
        int32 strength = this->getAttribute<int32>("creature_strength");
        ham->SetHam(this, HamBar_Strength, strength);
    }
    else
    {
        LOG(error) << "AttackableStaticNpc::respawn No strength attribute";
        ham->SetHam(this, HamBar_Strength, 500);
    }

    if (this->hasAttribute("creature_constitution"))
    {
        int32 constitution = this->getAttribute<int32>("creature_constitution");
        ham->SetHam(this, HamBar_Strength, constitution);
    }
    else
    {
        LOG(error) << "AttackableStaticNpc::respawn No constitution attribute";
        ham->SetHam(this, HamBar_Strength, 500);
    }


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
     LOG(error) << "Attackablstaticnpc spawnshit";
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
                DLOG(info) << "AttackableStaticNpc::spawn: Spawned a private object.";
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
                
                DLOG(info) << "AttackableStaticNpc::spawn: Failed to spawn a private object.";
            }
        }
    }
    else
    {
        DLOG(info) << "AttackableStaticNpc::spawn: Spawned an object.";
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
