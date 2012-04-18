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

#ifndef ANH_ZONESERVER_LAIR_OBJECT_H
#define ANH_ZONESERVER_LAIR_OBJECT_H

#include "AttackableStaticNpc.h"
#include "MathLib/Rectangle.h"

//=============================================================================

/*
- creature lair
*/

#define MaxWaveSize 6
#define MaxCreatureTypes 5


typedef enum _Lair_State
{
    State_LairIdle = 0,
    State_LairDead = 1,
    State_LairUnspawned = 2,
    State_LairAlerted = 3,
    State_LairCombatReady = 4,
    State_LairCombat = 5
} Lair_State;

class LairObject :	public AttackableStaticNpc//, ObjectFactoryCallback
{
    friend class NonPersistentNpcFactory;

public:
    LairObject(uint64 templateId);
    virtual ~LairObject();

    virtual void prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount);

    virtual void	addKnownObject(Object* object);

    virtual void	handleEvents(void);
    virtual uint64	handleState(uint64 timeOverdue);
    virtual void	inPeace(void);

    virtual void	killEvent(void);
    virtual void	respawn(void);

    void	spawn(void);
    void	reportedDead(uint64 deadCreatureId);
    bool	requestAssistance(uint64 targetId, uint64 sourceId) const;
    void	requestLairAssistance(void) const;

    void	setSpawnArea(const Anh_Math::Rectangle &mSpawnArea);
    void	setCreatureTemplate(uint32 index, uint64 creatureTemplateId);
    void	setCreatureSpawnRate(uint32 index, uint32 spawnRate);
    virtual float getMaxSpawnDistance(void) {
        return mMaxSpawnDistance;
    }

    uint64	mCreatureId[MaxWaveSize];
    int32	mPassiveCreature[MaxWaveSize];

private:
    LairObject();

    bool	playerInRange(float range);
    void	spawnInitialWave(void);
    bool	getLairTarget(void);
    void	makePeaceWithDefendersOutOfRange(void);

    Anh_Math::Rectangle mSpawnArea;

    Lair_State	mLairState;

    int32	mCreatureSpawnRate[MaxCreatureTypes];

    int64	mInitialSpawnDelay;			// in ms
    uint64	mCreatureTemplates[MaxCreatureTypes]; // Creature templates and spwan rate to be used by lair.
    uint64	mLairsTypeId;
    uint64 mSpawnCell;
    // This is the max stalking distance from the spawn point or other central point, like lair.
    // Will be used when calculating spawn positions within a region. We do not allow the creatures to move outside the region.
    float	mMaxStalkerDistance;	// Raduis from lair.
    float	mMaxSpawnDistance;		// Max distance from lair where creature should spawn.
    int32	mActiveWaves;
    int32	mCreatureBabySpawnRate;		// Not uses yet.
    int32	mPassiveWaves;
    int32	mWaveSize;
    bool	mInitialized;
    bool	mSpawned;
    bool	mSpawnPositionFixed;
};

//=============================================================================

#endif
