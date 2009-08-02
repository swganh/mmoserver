/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_LAIR_OBJECT_H
#define ANH_ZONESERVER_LAIR_OBJECT_H

#include "AttackableStaticNpc.h"
#include "NPCObject.h"
#include "MathLib/Rectangle.h"

//=============================================================================

/*
- creature lair
*/

#define MaxWaveSize 6
#define MaxCreatureTypes 5

// Some hardcoded stuff for test.
const uint64 pileOfRocksTemplateId = 47513085701;		// A pile of rocks.
const uint64 nestTemplateId = 47513085709;		// A nest.



class LairData
{
	public:
		LairData::LairData();
		LairData::~LairData() { }

		// Copy constructor
		LairData::LairData(const LairData& data);		

		// Assignment operator
		LairData& operator=(const LairData& lairData);
		
		uint64 mSpawnCell;
		Anh_Math::Vector3 mSpawnPosition;
		Anh_Math::Quaternion mSpawnDirection;

		uint64	mTemplateId;
		bool	mSpawnPositionFixed;
		Anh_Math::Rectangle mSpawnArea;
		int32	mActiveWaves;
		int32	mPassiveWaves;
		int32	mWaveSize;

		uint64	mCreatureTemplates[MaxCreatureTypes];
		int32	mCreatureSpawnRate[MaxCreatureTypes];
		int32	mCreatureBabySpawnRate;

		uint64	mCreatureId[MaxWaveSize];
		int32	mPassiveCreature[MaxWaveSize];
		
		uint64	mRespawnPeriod;
		int64	mInitialSpawnPeriod;
};


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
		LairObject();
		// LairObject::LairObject(const LairObject &lair);
		virtual ~LairObject();

		virtual void prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount);

		virtual void	addKnownObject(Object* object);

		virtual void	handleEvents(void);
		virtual uint64	handleState(uint64 timeOverdue);
		virtual void	inPeace(void);
		
		virtual void	killEvent(void);
		virtual void	respawn(void);

		void	setLairData(const LairData &lairData);
		void	spawn(void);
		void	reportedDead(uint64 deadCreatureId);
		bool	requestAssistance(uint64 targetId, uint64 sourceId) const;
		void	requestLairAssistance(void) const;

		bool	mSpawned;
		LairData mLairData;

	private:
		bool	playerInRange(float range);
		void	spawnInitialWave(void);
		bool	getLairTarget(void);
		void	makePeaceWithDefendersOutOfRange(void);

		Lair_State	mLairState;

	// health, content
};

//=============================================================================

#endif