/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "LairObject.h"
#include "AttackableCreature.h"
#include "CellObject.h"
#include "NonPersistentNpcFactory.h"
#include "NpcManager.h"
#include "PlayerObject.h"
#include "QuadTree.h"
#include "WorldManager.h"
#include "ZoneTree.h"
#include "MessageLib/MessageLib.h"
#include "MathLib/Quaternion.h"

static const int64 dormantDefaultPeriodTime = 10000;
static const int64 readyDefaultPeriodTime = 1000;

// For test.
static int64 gLairSpawnCounter = 0;
static int64 gLairDeathCounter = 0;


//=============================================================================

LairObject::LairObject() : AttackableStaticNpc(),
mLairState(State_LairUnspawned),
mSpawned(false)
{
	assert(false);	// We should not use this constructor.

	mNpcFamily	= NpcFamily_NaturalLairs;
	mType = ObjType_Lair;

	// this->mInitialized = false;

	// Use default radial.
	// mRadialMenu = RadialMenuPtr(new RadialMenu());

	// this->togglePvPStateOn(CreaturePvPStatus_Attackable);

}

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
	gLogger->logMsgF("LairObject::~LairObject() DESTRUCTED\n", MSG_NORMAL);
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
{
	if (object->getType() == ObjType_Player)
	{
		mKnownPlayers.insert(dynamic_cast<PlayerObject*>(object));

		if ((this->getAiState() == NpcIsDormant))
		{
			// gLogger->logMsgF("LairObject::addKnownObject Wakie-wakie!", MSG_NORMAL);
			gWorldManager->forceHandlingOfDormantNpc(this->getId());
		}
	}
	else
	{
		mKnownObjects.insert(object);
	}
}

void LairObject::handleEvents(void)
{
	// gLogger->logMsgF("LairObject::handleEvents: Entering", MSG_NORMAL);
	// General issues like life and death first.
	if (this->isDead())
	{
		mLairState = State_LairDead;
		return;
	}

	switch (mLairState)
	{
		case State_LairUnspawned:
		{
			// gLogger->logMsgF("State_LairUnspawned", MSG_NORMAL);

			// Are there any reasons for me to be alerted?

			// We may not be alone. Any player in range?
			if (playerInRange(150.0))
			{
				// gLogger->logMsgF("Players within spawn range", MSG_NORMAL);

				// Yes, we have to spawn, if the initial spawn timer has expired.
				if (this->mInitialSpawnDelay <= 0)
				{
					this->spawn();
					// gLogger->logMsgF("Spawned this lair.", MSG_NORMAL);
					this->spawnInitialWave();
					this->mLairState = State_LairAlerted;
					this->setAiState(NpcIsReady);
				}
				else
				{
					// We could chose to decrese the period time...
				}
			}
		}
		break;

		case State_LairIdle:
		{
			// gLogger->logMsgF("State_LairIdle", MSG_NORMAL);
			if (!this->getKnownPlayers()->empty())
			{
				// gLogger->logMsgF("Players nearby", MSG_NORMAL);
				mLairState = State_LairAlerted;
				this->setAiState(NpcIsReady);
			}
		}
		break;

		case State_LairAlerted:
		{
			// gLogger->logMsgF("State_LairAlerted", MSG_NORMAL);
			if (this->getKnownPlayers()->empty())
			{
				// gLogger->logMsgF("No players nearby", MSG_NORMAL);
				mLairState = State_LairIdle;
				this->setAiState(NpcIsDormant);
			}
			// Any attacked us?
			else if (this->getDefenders()->size() != 0)
			{
				// gLogger->logMsgF("We are attacked", MSG_NORMAL);
				// Request Assistance.
				if (getLairTarget())
				{
					// gLogger->logMsgF("We have a target, requesting help", MSG_NORMAL);
					this->requestLairAssistance();

					mLairState = State_LairCombatReady;
					this->setAiState(NpcIsReady);
				}
				else
				{
					// We have no target.
					this->setTarget(NULL);

				}


				// Spawn first wave, if a Mission lair.

			}
		}
		break;

		case State_LairCombatReady:
		{
			// gLogger->logMsgF("State_LairCombatReady", MSG_NORMAL);
			if (this->getKnownPlayers()->empty())
			{
				// gLogger->logMsgF("No players nearby", MSG_NORMAL);
				mLairState = State_LairIdle;
				this->setAiState(NpcIsDormant);
			}
			else
			{
				// If current lair hitter is "gone", check for new one.
				// if (gWorldManager->getObjectById(this->getTargetId()))
				{
				}

				// Right now, lets attack the defender nearest the lair.
				// At least it gives us some kind of mechanichs for shifting target.
				uint64 currentTargetId = this->getTargetId();
				if (getLairTarget())
				{
					if (currentTargetId != this->getTargetId())
					{
						// gLogger->logMsgF("We have a NEW target, requesting help", MSG_NORMAL);
						this->requestLairAssistance();
					}
				}
				else
				{
					// We have no target.
					this->setTarget(NULL);

				}
				// THE solution...
				// When a defender have not made any damage of the lair for X seconds, check for next defender.

				// The one doing the MOST damage, and still avaliable as a target should get the aggro...
			}
		}
		break;

		default:
		{
		}
		break;
	}
}

uint64 LairObject::handleState(uint64 timeOverdue)
{
	uint64 waitTime = 0;
	// gLogger->logMsgF("LairObject::handleState: Entering", MSG_NORMAL);
	// General issues like life and death first.

	switch (mLairState)
	{
		case State_LairUnspawned:
		{
			// gLogger->logMsgF("State_LairUnspawned", MSG_NORMAL);
			waitTime = dormantDefaultPeriodTime - timeOverdue;
			this->mInitialSpawnDelay -= dormantDefaultPeriodTime;
		}
		break;

		case State_LairIdle:
		{
			// gLogger->logMsgF("State_LairIdle", MSG_NORMAL);
			waitTime = (10*dormantDefaultPeriodTime); // overdue does not matter, we are just idling. // - timeOverdue;
		}
		break;

		case State_LairAlerted:
		{
			// gLogger->logMsgF("State_LairAlerted", MSG_NORMAL);
			waitTime = readyDefaultPeriodTime;
		}
		break;

		case State_LairCombatReady:
		{
			// gLogger->logMsgF("State_LairCombatReady", MSG_NORMAL);
			this->makePeaceWithDefendersOutOfRange();
			waitTime = readyDefaultPeriodTime;
		}
		break;

		case State_LairDead:
		{
			// gLogger->logMsgF("State_LairDead", MSG_NORMAL);
		}
		break;

		default:
		{
			gLogger->logMsgF("UNKNOWN state\n", MSG_NORMAL);
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
	// gLogger->logMsgF("LairObject::requestAssistance() Entering", MSG_NORMAL);
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
							// gLogger->logMsgF("LairObject::requestAssistance() Asking creature %"PRIu64" for help.", MSG_NORMAL, creature->getId());
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
	// gLogger->logMsgF("LairObject::requestLairAssistance() Entering", MSG_NORMAL);

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
					// gLogger->logMsgF("LairObject::requestLairAssistance() Lair asking creature %"PRIu64" for help.", MSG_NORMAL, creature->getId());
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
				float distanceFromLair = this->mPosition.distance2D(creatureObject->mPosition);
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
	gLogger->logMsgF("Spawned lair # %"PRIu64" (%"PRIu64")", MSG_NORMAL, gLairSpawnCounter, gLairSpawnCounter - gLairDeathCounter);

	// Update the world about my presence.
	if (this->getParentId())
	{
		// gLogger->logMsg("LairObject::spawn Inside");
		// insert into cell
		this->setSubZoneId(0);

		if (CellObject* cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(this->getParentId())))
		{
			cell->addChild(this);
		}
		else
		{
			gLogger->logMsgF("LairObject::spawn: couldn't find cell %"PRIu64"",MSG_HIGH, this->getParentId());
		}
	}
	else
	{
		// gLogger->logMsg("LairObject::spawn Outside");
		if (QTRegion* region = gWorldManager->getSI()->getQTRegion(this->mPosition.mX, this->mPosition.mZ))
		{
			// gLogger->logMsg("LairObject::spawn QTRegion found");
			this->setSubZoneId((uint32)region->getId());
			region->mTree->addObject(this);
			// gLogger->logMsgF("Spawn at %.0f %.0f", MSG_NORMAL, this->mPosition.mX, this->mPosition.mZ);
		}
	}

	// Add us to the world.
	gMessageLib->broadcastContainmentMessage(this->getId(),this->getParentId(), -1, this);

	// send out position updates to known players
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
	ObjectSet inRangeObjects;
	// ObjectSet::iterator objectSetIt;

	// Make Set ready,
	// inRangeObjects.clear();
	// objectSetIt = mInRangeObjects.begin();	// Will point to end of Set
	if (QTRegion* region = gWorldManager->getSI()->getQTRegion(this->mPosition.mX, this->mPosition.mZ))
	{
		// gLogger->logMsg("LairObject::playerInRange Looking for players");
		Anh_Math::Rectangle qRect = Anh_Math::Rectangle(this->mPosition.mX - range, this->mPosition.mZ - range, range * 2, range * 2);
		region->mTree->getObjectsInRange(this, &inRangeObjects, ObjType_Player, &qRect);
	}
	return !inRangeObjects.empty();
}

//=============================================================================
//
// Last call to this instance....
//

void LairObject::killEvent(void)
{
	// One lair less in the system.
	gLairDeathCounter++;

	// gLogger->logMsgF("LairObject::killEvent: Entering", MSG_NORMAL);
	if (this->getRespawnDelay() != 0)
	{
		// gLogger->logMsgF("LairObject::killEvent: Creating a new lair with template = %"PRIu64"", MSG_NORMAL, this->mTemplateId);

		uint64 npcNewId = gWorldManager->getRandomNpNpcIdSequence();
		if (npcNewId != 0)
		{
			// Let's put this sucker into play again.
			// gLogger->logMsgF("Requesting lair of type = %"PRIu64" with id %"PRIu64"", MSG_NORMAL, mLairsTypeId, npcNewId);
			NonPersistentNpcFactory::Instance()->requestLairObject(NpcManager::Instance(), mLairsTypeId, npcNewId);
		}
	}
}

// This will become the standard create and spawn routine...

void LairObject::respawn(void)
{
	// gLogger->logMsgF("LairObject::respawn() Entering", MSG_NORMAL);

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
			// gLogger->logMsgF("lair_fix_position = %d", MSG_NORMAL, this->mSpawnPositionFixed);
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
			Anh_Math::Vector3		position;

			Anh_Math::Vector3 *pos = this->mSpawnArea.getPosition();
			position.mX = pos->mX;
			position.mZ = pos->mZ;

			float xWidth = this->mSpawnArea.getHeight();
			float zHeight = this->mSpawnArea.getWidth();

			// Ge a random position withing given region.
			// Note that creature can spawn outside the region, since thay have a radius from the lair where thet are allowed to spawn.
			this->mPosition.mX = position.mX + (gRandom->getRand() % (int32)(xWidth+1));
			this->mPosition.mZ = position.mZ + (gRandom->getRand() % (int32)(zHeight+1));
			if (this->getParentId() == 0)
			{
				// Heightmap only works outside.
				this->mPosition.mY = this->getHeightAt2DPosition(this->mPosition.mX, this->mPosition.mZ, true);
			}

			// Random direction.
			this->setRandomDirection();
		}
		// gLogger->logMsgF("Lair spawn at pos: %.0f %.0f %.0f", MSG_NORMAL, this->mPosition.mX, this->mPosition.mY, this->mPosition.mZ);

		if (this->hasInternalAttribute("lair_wave_size"))
		{
			this->mWaveSize = this->getInternalAttribute<int32>("lair_wave_size");
			// gLogger->logMsgF("lair_wave_size = %d", MSG_NORMAL, this->mWaveSize);
		}
		else
		{
			assert(false);
			this->mWaveSize = 3;
		}

		if (this->hasInternalAttribute("lair_passive_waves"))
		{
			this->mPassiveWaves = this->getInternalAttribute<int32>("lair_passive_waves");
			// gLogger->logMsgF("lair_passive_waves = %d", MSG_NORMAL, this->mPassiveWaves);
		}
		else
		{
			assert(false);
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
			// gLogger->logMsgF("lair_respawn_delay = %"PRIu64"", MSG_NORMAL, this->getRespawnDelay());
		}
		else
		{
			assert(false);
			this->setRespawnDelay(5*60*1000);
		}


		// Max spawn distance for creatures belonging to the lair.
		if (this->hasInternalAttribute("lair_creatures_max_spawn_distance"))
		{
			this->mMaxSpawnDistance = this->getInternalAttribute<float>("lair_creatures_max_spawn_distance");
			// gLogger->logMsgF("lair_creatures_max_spawn_distance = %.0f", MSG_NORMAL, this->mMaxSpawnDistance);
		}
		else
		{
			assert(false);
			this->mMaxSpawnDistance = 10;
		}

		if (this->hasInternalAttribute("creature_xp"))
		{
			uint32 xp = this->getInternalAttribute<uint32>("creature_xp");
			// gLogger->logMsgF("creature_xp = %u", MSG_NORMAL, xp);
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
			// gLogger->logMsgF("creature_health = %d", MSG_NORMAL, health);
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
			// gLogger->logMsgF("creature_strength = %d", MSG_NORMAL, strength);
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
			// gLogger->logMsgF("creature_constitution = %d", MSG_NORMAL, constitution);
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

		/*
		if (this->hasAttribute("challenge_level"))	// Difficulty Level
		{
			int32 challenge_level = this->getAttribute<int32>("challenge_level");
			// gLogger->logMsgF("Difficulty Level (challenge_level) = %d", MSG_NORMAL, challenge_level);
		}

		if (this->hasAttribute("consider"))	// Combat Difficulty
		{
			int32 consider = this->getAttribute<int32>("consider");
			// gLogger->logMsgF("Combat Difficulty (consider) = %d", MSG_NORMAL, consider);
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
		assert(false);
	}
}

void LairObject::reportedDead(uint64 deadCreatureId)
{
	// gLogger->logMsgF("LairObject::reportedDead() Entering", MSG_NORMAL);
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
			assert(creatureTemplate != 0);

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
		// gLogger->logMsgF("LairObject::reportedDead() Creature will NOT respawn\n", MSG_NORMAL);
	}
}


void LairObject::setSpawnArea(const Anh_Math::Rectangle &mSpawnArea)
{
	this->mSpawnArea = mSpawnArea;
}

void LairObject::setCreatureTemplate(uint32 index, uint64 creatureTemplateId)
{
	assert(index < MaxCreatureTypes);
	this->mCreatureTemplates[index] = creatureTemplateId;
}

void LairObject::setCreatureSpawnRate(uint32 index, uint32 spawnRate)
{
	assert(index < MaxCreatureTypes);
	this->mCreatureSpawnRate[index] = spawnRate;
}
