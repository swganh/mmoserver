/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

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
	State_LairCombat = 5,
} Lair_State;

class LairObject :	public AttackableStaticNpc, ObjectFactoryCallback
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
		virtual float getMaxSpawnDistance(void) {  return mMaxSpawnDistance;}

		uint64	mCreatureId[MaxWaveSize];
		int32	mPassiveCreature[MaxWaveSize];

	private:
		LairObject();

		bool	playerInRange(float range);
		void	spawnInitialWave(void);
		bool	getLairTarget(void);
		void	makePeaceWithDefendersOutOfRange(void);

		
		uint64	mLairsTypeId;
		bool	mInitialized;
		bool	mSpawned;

		uint64 mSpawnCell;
		Anh_Math::Rectangle mSpawnArea;

		// Attribute related data
		bool	mSpawnPositionFixed;
		int32	mActiveWaves;
		int32	mPassiveWaves;
		int32	mWaveSize;

		// This is the max stalking distance from the spawn point or other central point, like lair.
		// Will be used when calculating spawn positions within a region. We do not allow the creatures to move outside the region.
		float	mMaxStalkerDistance;	// Raduis from lair.
		float	mMaxSpawnDistance;		// Max distance from lair where creature should spawn.

		int32	mCreatureBabySpawnRate;		// Not uses yet.

		int64	mInitialSpawnDelay;			// in ms

		// Creature templates and spwan rate to be used by lair.
		uint64	mCreatureTemplates[MaxCreatureTypes];
		int32	mCreatureSpawnRate[MaxCreatureTypes];

		Lair_State	mLairState;
};

//=============================================================================

#endif