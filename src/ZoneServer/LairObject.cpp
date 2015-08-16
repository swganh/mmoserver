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

#include "LairObject.h"
#include "AttackableCreature.h"
#include "CellObject.h"
#include "NonPersistentNpcFactory.h"
#include "NpcManager.h"
#include "PlayerObject.h"
#include "SpatialIndexManager.h"
#include "WorldManager.h"

#include "MessageLib/MessageLib.h"
#include "Utils/rand.h"

#include <cassert>

static const int64 dormantDefaultPeriodTime = 10000;
static const int64 readyDefaultPeriodTime = 1000;

// For test.
static int64 gLairSpawnCounter = 0;
static int64 gLairDeathCounter = 0;

//=============================================================================

LairObject::LairObject(uint64 templateId) : AttackableStaticNpc()
    , mLairState(State_LairUnspawned)
    , mLairsTypeId(templateId)
    , mActiveWaves(0)
    , mPassiveWaves(0)
    , mWaveSize(0)
    , mInitialized(false)
    , mSpawned(false)
    , mSpawnPositionFixed(true)
{
    mNpcFamily	= NpcFamily_NaturalLairs;
    mType = ObjType_Lair;

    for (uint8 i = 0; i < MaxCreatureTypes; i++)
    {
        this->mCreatureTemplates[i] = 0;
        this->mCreatureSpawnRate[i] = 100;
    }
}

//=============================================================================

LairObject::~LairObject()
{
    // mRadialMenu.reset();
}

//=============================================================================
void LairObject::prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount)
{
    mRadialMenu.reset();
    mRadialMenu = RadialMenuPtr(new RadialMenu());

    // The peace option will be filled in by the client when we have started the combat.
    if (this->checkPvPState(CreaturePvPStatus_Attackable))
    {
        mRadialMenu->addItem(1,0,radId_combatAttack,radAction_Default);
        mRadialMenu->addItem(2,0,radId_examine,radAction_Default);
    }
    else
    {
        mRadialMenu->addItem(1,0,radId_examine,radAction_Default);
    }
}

//=============================================================================
//
//	Routines realted to NPC AI
//
//=============================================================================


void LairObject::addKnownObject(Object* object)
{}

void LairObject::handleEvents(void)
{
    // General issues like life and death first.
    if (this->isDead())
    {
        mLairState = State_LairDead;
        return;
    }

    //switch (mLairState)
    //    {}
}

uint64 LairObject::handleState(uint64 timeOverdue)
{
    uint64 waitTime = 0;
    // General issues like life and death first.

    switch (mLairState)
    {
    case State_LairUnspawned:
    {
        waitTime = dormantDefaultPeriodTime - timeOverdue;
        this->mInitialSpawnDelay -= dormantDefaultPeriodTime;
    }
    break;

    case State_LairIdle:
    {
        waitTime = (10*dormantDefaultPeriodTime); // overdue does not matter, we are just idling. // - timeOverdue;
    }
    break;

    case State_LairAlerted:
    {
        waitTime = readyDefaultPeriodTime;
    }
    break;

    case State_LairCombatReady:
    {
        this->makePeaceWithDefendersOutOfRange();
        waitTime = readyDefaultPeriodTime;
    }
    break;

    case State_LairDead:
    {
    }
    break;

    default:
    {
    }
    break;

    }
    return waitTime;
}

//=============================================================================
//
//	We have no more defenders.
//	This method should be called as an "event" when we gets compleatly out of combat.
//

void LairObject::inPeace(void)
{

}


//=============================================================================
//
//	Request assistance from creatures within 45m and not in combat.
//


bool LairObject::requestAssistance(uint64 targetId, uint64 sourceId) const
{
    // All creature not in combat will assist.
    bool found = false;

    for (int32 i = 0; i < this->mWaveSize; i++)
    {
        // Creature alive? (and it's not I)
        if ((this->mPassiveCreature[i] > 0) && (sourceId != this->mCreatureId[i]))
        {
            // Yes. Is creature reference valid.?
            if (AttackableCreature* creature = dynamic_cast<AttackableCreature*>(gWorldManager->getObjectById(this->mCreatureId[i])))
            {
                // Yes. Is creature still alive and not in combat?
                if (!creature->isDead() && (creature->getDefenders()->empty()))
                {
                    if (creature->isGroupAssist())
                    {
                        // Creature in range?
                        if (gWorldManager->objectsInRange(sourceId, this->mCreatureId[i], 45.0))
                        {
                            // Would be lovley if this creature could help me.
                            creature->assistCreature(targetId);
                            found = true;
                        }
                    }
                }
            }
        }
    }
    return found;
}

//=============================================================================
//
//	Request assistance from creatures.
//
void LairObject::requestLairAssistance(void) const
{
    // All creatures will assist.

    // Set the first defender as target.
    uint64 targetId = this->getTarget()->getId();

    for (int32 i = 0; i < this->mWaveSize; i++)
    {
        // Creature alive?
        if (this->mPassiveCreature[i] > 0)
        {
            // Yes. Is creature reference valid.?
            if (AttackableCreature* creature = dynamic_cast<AttackableCreature*>(gWorldManager->getObjectById(this->mCreatureId[i])))
            {
                // Yes. Is creature still alive?
                if (!creature->isDead())
                {
                    // Would be lovley if this creature could help me.
                    creature->assistLair(targetId);
                }
            }
        }
    }
}

bool LairObject::getLairTarget(void)
{
    bool newTarget = false;
    float nearestDistanceFromLair = 128.0;	// Todo: Use a real value.
    uint64 nearestDefenderId = 0;

    // Attack nearest target or the first target found within range or the one doing most damage or random? lol
    ObjectIDList::iterator defenderIt = this->getDefenders()->begin();

    while (defenderIt != this->getDefenders()->end())
    {
        CreatureObject* creatureObject = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById((*defenderIt)));
        if (creatureObject)
        {
            // We may have a target, since he is attacking this lair.
            if (!creatureObject->isIncapacitated() && !creatureObject->isDead())
            {
                float distanceFromLair = glm::distance(this->mPosition, creatureObject->mPosition);
                if (distanceFromLair < nearestDistanceFromLair)
                {
                    nearestDistanceFromLair = distanceFromLair;
                    nearestDefenderId = (*defenderIt);
                }
                newTarget = true;
            }
            else
            {
                // Make peace with this poor fellow.
                this->makePeaceWithDefender(*defenderIt);
                defenderIt = this->mDefenders.begin();
                continue;
            }
        }
        defenderIt++;
    }

    if (newTarget)
    {
        this->setTarget(nearestDefenderId);
    }
    return newTarget;
}

//=============================================================================
//
//	Make peace with targets that are outside the lairs creature limit.
//

void LairObject::makePeaceWithDefendersOutOfRange(void)
{
    float maxRange = 65.0;	// Todo: Use a real value.

    ObjectIDList::iterator defenderIt = this->getDefenders()->begin();
    while (defenderIt != this->getDefenders()->end())
    {
        if (!gWorldManager->objectsInRange(this->getId(), *defenderIt, maxRange))
        {
            // We have a target (defender) outside our range, make peace with him.
            this->makePeaceWithDefender(*defenderIt);
            defenderIt = this->getDefenders()->begin();
            continue;
        }
        defenderIt++;
    }
}

//=============================================================================
//
//	Spawn this lair and initial wave.
//

void LairObject::spawn(void)
{
    gLairSpawnCounter++;
    DLOG(INFO) << "Spawned lair # " <<gLairSpawnCounter << "( " <<gLairSpawnCounter - gLairDeathCounter<<")";

    // Update the world about my presence.
    gSpatialIndexManager->createInWorld(this);

    // Add us to the world.
    gMessageLib->broadcastContainmentMessage(this,this->getParentId(), -1);

    // send out position updates to known players
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
//	Spawn the initial wave.
//

void LairObject::spawnInitialWave(void)
{
    // Let's do this very basic as a starter...

    // First we need X creatures :)
    uint8 creatureTypeIndex;
    uint64 creatureTemplate = 0;
    for (int32 i = 0; i < this->mWaveSize; i++)
    {
        // lair->mInitialSpawnPeriod = (uint64)(gRandom->getRand() % (int32)lair->mRespawnPeriod);
        creatureTypeIndex = gRandom->getRand() % (int32)100;	// Better to add all values...
        for (int32 y = 0; y < MaxCreatureTypes; y++)
        {
            if (creatureTypeIndex <= this->mCreatureSpawnRate[y])
            {
                creatureTemplate = this->mCreatureTemplates[y];
                break;
            }
        }
        // Here we have the type of creature to spawn.
        // But first we have to create him.
        NonPersistentNpcFactory* nonPersistentNpcFactory = gNonPersistentNpcFactory;

        // We need two id's in sequence, since nps'c have an inventory.
        uint64 npcNewId = gWorldManager->getRandomNpNpcIdSequence();

        if (npcNewId != 0)
        {
            // Save the id of the creatures in my lair.
            this->mCreatureId[i] = npcNewId;
            nonPersistentNpcFactory->requestNpcObject(NpcManager::Instance(), creatureTemplate, npcNewId, this->getParentId(), this->mPosition, this->mDirection, 0, this->getId());
        }
    }
}

//=============================================================================
//
//	Check if we have any player near us.

bool LairObject::playerInRange(float range)
{
    // to in-range folks
    ObjectSet resultSet;

    gSpatialIndexManager->getObjectsInRange(this,&resultSet,ObjType_Creature,30.0,true);

    return !resultSet.empty();
}

//=============================================================================
//
// Last call to this instance....
//

void LairObject::killEvent(void)
{
    // One lair less in the system.
    gLairDeathCounter++;

    if (this->getRespawnDelay() != 0)
    {
        uint64 npcNewId = gWorldManager->getRandomNpNpcIdSequence();
        if (npcNewId != 0)
        {
            // Let's put this sucker into play again.
            NonPersistentNpcFactory::Instance()->requestLairObject(NpcManager::Instance(), mLairsTypeId, npcNewId);
        }
    }
}

// This will become the standard create and spawn routine...

void LairObject::respawn(void)
{

    // The data used below ARE taken from DB, not hard coded as the script version above.

    // What kind of lair?
    if (this->mNpcFamily == NpcFamily_NaturalLairs)
    {
        // Create a lair, passive style.

        // Spawn position. Can be fixed or random within given "region".
        this->mSpawnPositionFixed = false;
        if (this->hasInternalAttribute("lair_fix_position"))
        {
            this->mSpawnPositionFixed = this->getInternalAttribute<bool>("lair_fix_position");
        }
        // We only support lair spawning outside, for now.
        this->mSpawnCell = 0;

        this->setParentId(this->mSpawnCell);

        if (this->mSpawnPositionFixed)
        {
            // Use the existing position, read from DB.
            // position = this->mSpawnPosition;
            // direction = this->mSpawnDirection;
        }
        else
        {
            // Rectangle(float lowX,float lowZ,float width,float height) : Shape(lowX,0.0f,lowZ),mWidth(width),mHeight(height){}
            const glm::vec3& position = this->mSpawnArea.getPosition();

            float xWidth = this->mSpawnArea.getHeight();
            float zHeight = this->mSpawnArea.getWidth();

            // Ge a random position withing given region.
            // Note that creature can spawn outside the region, since thay have a radius from the lair where thet are allowed to spawn.
            this->mPosition.x = position.x + (gRandom->getRand() % (int32)(xWidth+1));
            this->mPosition.z = position.z + (gRandom->getRand() % (int32)(zHeight+1));
            if (this->getParentId() == 0)
            {
                // Heightmap only works outside.
                this->mPosition.y = this->getHeightAt2DPosition(this->mPosition.x, this->mPosition.z, true);
            }

            // Random direction.
            this->setRandomDirection();
        }

        if (this->hasInternalAttribute("lair_wave_size"))
        {
            this->mWaveSize = this->getInternalAttribute<int32>("lair_wave_size");
        }
        else
        {
            assert(false && "LairObject::respawn missing lair_wave_size attribute");
            this->mWaveSize = 3;
        }

        if (this->hasInternalAttribute("lair_passive_waves"))
        {
            this->mPassiveWaves = this->getInternalAttribute<int32>("lair_passive_waves");
        }
        else
        {
            assert(false && "LairObject::respawn missing lair_passive_waves attribute");
            this->mPassiveWaves = 3;
        }

        // We do not spawn any babys yet.

        for (int32 i = 0; i < this->mWaveSize; i++)
        {
            this->mPassiveCreature[i] = this->mPassiveWaves;
        }
        for (int32 i = this->mWaveSize; i < MaxWaveSize; i++)
        {
            this->mPassiveCreature[i] = 0;
        }


        // Lair respawn period.
        if (this->hasInternalAttribute("lair_respawn_delay"))
        {
            this->setRespawnDelay(this->getInternalAttribute<uint64>("lair_respawn_delay"));
        }
        else
        {
            assert(false && "LairObject::respawn missing lair_respawn_delay attribute");
            this->setRespawnDelay(5*60*1000);
        }


        // Max spawn distance for creatures belonging to the lair.
        if (this->hasInternalAttribute("lair_creatures_max_spawn_distance"))
        {
            this->mMaxSpawnDistance = this->getInternalAttribute<float>("lair_creatures_max_spawn_distance");
        }
        else
        {
            assert(false && "LairObject::respawn missing lair_creatures_max_spawn_distance attribute");
            this->mMaxSpawnDistance = 10;
        }

        if (this->hasInternalAttribute("creature_xp"))
        {
            uint32 xp = this->getInternalAttribute<uint32>("creature_xp");
            this->setWeaponXp(xp);
        }
        else
        {
            // assert(false);
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
            assert(false && "LairObject::respawn missing creature_health attribute");
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
            assert(false && "LairObject::respawn missing creature_strength attribute");
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
            assert(false && "LairObject::respawn missing creature_constitution attribute");
            this->mHam.mConstitution.setCurrentHitPoints(500);
            this->mHam.mConstitution.setMaxHitPoints(500);
            this->mHam.mConstitution.setBaseHitPoints(500);
        }
        this->mHam.calcAllModifiedHitPoints();

        /*
        if (this->hasAttribute("challenge_level"))	// Difficulty Level
        {
        	int32 challenge_level = this->getAttribute<int32>("challenge_level");
        }

        if (this->hasAttribute("consider"))	// Combat Difficulty
        {
        	int32 consider = this->getAttribute<int32>("consider");
        }
        */



        // Register object with WorldManager.
        // Done when creating the object.
        // gWorldManager->addObject(this, true);

        // Put the lair in the Dormant queue.
        // I do not want all lair running at same respawn period to do their initial spawn at the same time.
        this->mInitialSpawnDelay = (int64)this->getRespawnDelay() + (int64)(((uint64)gRandom->getRand() * 1000) % (this->getRespawnDelay() + 1));

        // When spawning, the lair awaits activation in the dormant queue, so we load the lair instantly and have a wait timer running before the spawn.
        // Since we can force a lair (any object) out of the dormant queue, we have to do the actual spwan countdown with a created object.
        gWorldManager->addDormantNpc(getId(), 0);
    }
    else
    {
        assert(false && "LairObject::respawn mNpcFamily != NpcFamily_NaturalLairs");
    }
}

void LairObject::reportedDead(uint64 deadCreatureId)
{
    bool found = false;

    // this->mCreatureId[i] = npcNewId;
    for (int32 i = 0; i < this->mWaveSize; i++)
    {
        if ((deadCreatureId == this->mCreatureId[i]) && (this->mPassiveCreature[i] > 0))
        {
            found = true;

            // We have to make a new passive spawn( of the same kind of creature ??).
            this->mPassiveCreature[i]--;

            uint64 creatureTemplate = 0;
            uint8 creatureTypeIndex = gRandom->getRand() % (int32)100;	// Better to add all values...
            for (int32 y = 0; y < MaxCreatureTypes; y++)
            {
                if (creatureTypeIndex <= this->mCreatureSpawnRate[y])
                {
                    creatureTemplate = this->mCreatureTemplates[y];
                    break;
                }
            }
            assert(creatureTemplate != 0 && "LairObject::reportedDead invalid creature template");

            // Here we have the type of creature to spawn.

            // But first we have to create him.
            NonPersistentNpcFactory* nonPersistentNpcFactory = gNonPersistentNpcFactory;

            // We need two id's in sequence, since nps'c have an inventory.
            uint64 npcNewId = gWorldManager->getRandomNpNpcIdSequence();
            if (npcNewId != 0)
            {
                // Save the id of the creatures in my lair.
                this->mCreatureId[i] = npcNewId;
                // nonPersistentNpcFactory->requestNpcObject(NpcManager::Instance(), creatureTemplate, npcNewId, this->getId());
                nonPersistentNpcFactory->requestNpcObject(NpcManager::Instance(), creatureTemplate, npcNewId, this->getParentId(), this->mPosition, this->mDirection, 0, this->getId());
            }
        }
    }
    if (!found)
    {
    }
}


void LairObject::setSpawnArea(const Anh_Math::Rectangle &mSpawnArea)
{
    this->mSpawnArea = mSpawnArea;
}

void LairObject::setCreatureTemplate(uint32 index, uint64 creatureTemplateId)
{
    assert(index < MaxCreatureTypes && "LairObject::setCreatureTemplate index parameter must be greater than MaxCreatureTypes");
    this->mCreatureTemplates[index] = creatureTemplateId;
}

void LairObject::setCreatureSpawnRate(uint32 index, uint32 spawnRate)
{
    assert(index < MaxCreatureTypes && "LairObject::setCreatureSpawnRate index parameter must be greater than MaxCreatureTypes");
    this->mCreatureSpawnRate[index] = spawnRate;
}
