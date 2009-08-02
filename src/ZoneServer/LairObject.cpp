/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "MathLib/Quaternion.h"
#include "LairObject.h"
#include "AttackableCreature.h"
#include "PlayerObject.h"
#include "WorldManager.h"
#include "ZoneServer/NonPersistentNpcFactory.h"

static const int64 dormantDefaultPeriodTime = 10000;
static const int64 readyDefaultPeriodTime = 1000;



LairData::LairData() :
mSpawnPositionFixed(true),
mActiveWaves(0),
mPassiveWaves(0),
mWaveSize(0)
{
}


// Copy constructor
LairData::LairData(const LairData& lairData)
{
	this->mSpawnCell = lairData.mSpawnCell;
	this->mSpawnPosition = lairData.mSpawnPosition;
	this->mSpawnDirection = lairData.mSpawnDirection;

	this->mTemplateId = lairData.mTemplateId;
	this->mSpawnPositionFixed = lairData.mSpawnPositionFixed;
	this->mSpawnArea = lairData.mSpawnArea;
	this->mActiveWaves = lairData.mActiveWaves;
	this->mPassiveWaves = lairData.mPassiveWaves;
	this->mWaveSize = lairData.mWaveSize;

	for (int8 i = 0; i < MaxCreatureTypes; i++)
	{
		this->mCreatureTemplates[i] = lairData.mCreatureTemplates[i];
		this->mCreatureSpawnRate[i] = lairData.mCreatureSpawnRate[i];
	}
	for (int8 i = 0; i < MaxWaveSize; i++)
	{
		this->mCreatureId[i] = lairData.mCreatureId[i];
		this->mPassiveCreature[i] = lairData.mPassiveCreature[i];
	}

	this->mCreatureBabySpawnRate = lairData.mCreatureBabySpawnRate;

	this->mRespawnPeriod = lairData.mRespawnPeriod;
	this->mInitialSpawnPeriod = lairData.mInitialSpawnPeriod;
}

LairData& LairData::operator=(const LairData& lairData)
{
	this->mSpawnCell = lairData.mSpawnCell;
	this->mSpawnPosition = lairData.mSpawnPosition;
	this->mSpawnDirection = lairData.mSpawnDirection;

	this->mTemplateId = lairData.mTemplateId;
	this->mSpawnPositionFixed = lairData.mSpawnPositionFixed;
	this->mSpawnArea = lairData.mSpawnArea;
	this->mActiveWaves = lairData.mActiveWaves;
	this->mPassiveWaves = lairData.mPassiveWaves;
	this->mWaveSize = lairData.mWaveSize;

	for (int8 i = 0; i < MaxCreatureTypes; i++)
	{
		this->mCreatureTemplates[i] = lairData.mCreatureTemplates[i];
		this->mCreatureSpawnRate[i] = lairData.mCreatureSpawnRate[i];
	}
	for (int8 i = 0; i < MaxWaveSize; i++)
	{
		this->mCreatureId[i] = lairData.mCreatureId[i];
		this->mPassiveCreature[i] = lairData.mPassiveCreature[i];
	}

	this->mCreatureBabySpawnRate = lairData.mCreatureBabySpawnRate;

	this->mRespawnPeriod = lairData.mRespawnPeriod;
	this->mInitialSpawnPeriod = lairData.mInitialSpawnPeriod;
	return *this;
}


//=============================================================================

LairObject::LairObject() : AttackableStaticNpc(),
mLairState(State_LairUnspawned),
mSpawned(false)
{
	mNpcFamily	= NpcFamily_NaturalLairs;
	mType = ObjType_Lair;
	
	// Use default radial.
	// mRadialMenu = RadialMenuPtr(new RadialMenu());

	// this->togglePvPStateOn(CreaturePvPStatus_Attackable);

}

//=============================================================================

/*
LairObject::LairObject(const LairObject &lair) : AttackableStaticNpc(),
mLairState(State_LairUnspawned),
mSpawned(false)
{
	mNpcFamily	= NpcFamily_NaturalLairs;
	mType = ObjType_Lair;
	
	// Use default radial.
	mRadialMenu = RadialMenuPtr(new RadialMenu());

	this->setParentId(lair.getParentId());
	this->mDirection = lair.mDirection;
	this->mPosition = lair.mPosition;

	this->mTemplateId = lair.mTemplateId;
	this->mSpawnPositionFixed = lair.mSpawnPositionFixed;
	this->mSpawnArea = lair.mSpawnArea;
	this->mActiveWaves = lair.mActiveWaves;
	this->mPassiveWaves = lair.mPassiveWaves;
	this->mWaveSize = lair.mWaveSize;

	for (int8 i = 0; i < MaxCreatureTypes; i++)
	{
		this->mCreatureTemplates[i] = lair.mCreatureTemplates[i];
		this->mCreatureSpawnRate[i] = lair.mCreatureSpawnRate[i];
	}

	this->mCreatureBabySpawnRate = lair.mCreatureBabySpawnRate;

	mSpawned = false;
	mRespawnPeriod = lair.mRespawnPeriod;
	// mInitialSpawnPeriod = mInitialSpawnPeriod;

	mLairState = State_LairUnspawned;
}
*/

//=============================================================================
//
//
void LairObject::setLairData(const LairData &lairData)
{
	this->mLairData = lairData;

	this->mSpawned = false;
	this->mLairState = State_LairUnspawned;
}


//=============================================================================

LairObject::~LairObject()
{
	// mRadialMenu.reset();
	gLogger->logMsgF("LairObject::~LairObject() DESTRUCTED", MSG_NORMAL);
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
			// gLogger->logMsgF("AttackableCreature::addKnownObject() Wakie-wakie!", MSG_NORMAL);
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
				if (mLairData.mInitialSpawnPeriod <= 0)
				{
					this->spawn();
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
			mLairData.mInitialSpawnPeriod -= dormantDefaultPeriodTime;
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
			gLogger->logMsgF("UNKNOWN state", MSG_NORMAL);
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

	for (int32 i = 0; i < mLairData.mWaveSize; i++)
	{
		// Creature alive? (and it's not I)
		if ((this->mLairData.mPassiveCreature[i] > 0) && (sourceId != this->mLairData.mCreatureId[i]))
		{
			// Yes. Is creature reference valid.?
			if (AttackableCreature* creature = dynamic_cast<AttackableCreature*>(gWorldManager->getObjectById(this->mLairData.mCreatureId[i])))
			{
				// Yes. Is creature still alive and not in combat?
				if (!creature->isDead() && (creature->getDefenders()->empty()))
				{
					if (creature->isGroupAssist())
					{
						// Creature in range?
						if (gWorldManager->objectsInRange(sourceId, this->mLairData.mCreatureId[i], 45.0))
						{
							// Would be lovley if this creature could help me.
							// gLogger->logMsgF("LairObject::requestAssistance() Asking creature %llu for help.", MSG_NORMAL, creature->getId());
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

	for (int32 i = 0; i < mLairData.mWaveSize; i++)
	{
		// Creature alive?
		if (this->mLairData.mPassiveCreature[i] > 0)
		{
			// Yes. Is creature reference valid.?
			if (AttackableCreature* creature = dynamic_cast<AttackableCreature*>(gWorldManager->getObjectById(this->mLairData.mCreatureId[i])))
			{
				// Yes. Is creature still alive?
				if (!creature->isDead())
				{
					// Would be lovley if this creature could help me.
					// gLogger->logMsgF("LairObject::requestLairAssistance() Lair asking creature %llu for help.", MSG_NORMAL, creature->getId());
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
			gLogger->logMsgF("LairObject::spawn: couldn't find cell %llu",MSG_HIGH, this->getParentId());
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
	uint64 creatureTemplate;
	for (int32 i = 0; i < mLairData.mWaveSize; i++)
	{
		// lair->mInitialSpawnPeriod = (uint64)(gRandom->getRand() % (int32)lair->mRespawnPeriod);
		creatureTypeIndex = gRandom->getRand() % (int32)100;	// Better to add all values...
		for (int32 y = 0; y < MaxCreatureTypes; y++)
		{
			if (creatureTypeIndex <= mLairData.mCreatureSpawnRate[y])
			{
				creatureTemplate = mLairData.mCreatureTemplates[y];
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
			// Save the id.
			this->mLairData.mCreatureId[i] = npcNewId;

			// Let us get an object from/via the WRONG database (the Persistent... one).
			// gLogger->logMsgF("LairObject::spawnInitialWave: Creating NPC with template = %llu and new id = %llu", MSG_NORMAL, creatureTemplate, npcNewId);
			
			Anh_Math::Vector3 position;

			// Let us get the spawn point. It's x meters from the lair :)
			position.mX = (this->mPosition.mX - 60.0) + (float)(gRandom->getRand() % (int32)(120));
			position.mZ = (this->mPosition.mZ - 60.0) + (float)(gRandom->getRand() % (int32)(120));
			position.mY = this->getHeightAt2DPosition(position.mX, position.mZ);
			
			// gLogger->logMsgF("Setting up spawn of creature at %.0f %.0f %.0f", MSG_NORMAL, position.mX, position.mY, position.mZ);

			// The data used below should be taken from DB, not hard coded as it is now.
			// We have to live with the hard coding, as this is still "under construction".

			SpawnData* spawn = new SpawnData;

			spawn->mBasic.templateId = creatureTemplate;

			spawn->mBasic.cellForSpawn = this->getParentId();		// Same cell as the lair.
			// spawn->mBasic.timeToFirstSpawn = 1000;		// Will not be used, since the creature class will think this is a respawn.
			
			// TODO: Do not use this for respawn or not....
			// spawn->mBasic.respawnPeriod = 0;  // Should be 0, since this creature will never respawn himself.

			spawn->mBasic.respawnPeriod = 120000;// 2 min max.
			spawn->mBasic.timeToFirstSpawn = (uint64)(gRandom->getRand() % (int32)spawn->mBasic.respawnPeriod);

			spawn->mBasic.spawnDirection = this->mDirection;	// TODO: Give them a random dir.
			spawn->mBasic.spawnPosition = position;
			spawn->mBasic.spawned = false;
			spawn->mBasic.lairId = this->getId();
			spawn->mBasic.homePosition = this->mPosition;

			// Hard coded test data.
			if (this->mLairData.mTemplateId == pileOfRocksTemplateId)
			{
				// Womp Rats.
				spawn->mWeapon.minDamage = 90;
				spawn->mWeapon.maxDamage = 110;
				spawn->mWeapon.weaponMaxRange = 6;
				spawn->mWeapon.attackSpeed = 2000;
				spawn->mWeapon.weaponXp = 356;			// a womp rat.

				spawn->mProfile.agressiveMode = true;
				spawn->mProfile.roaming = true;
				spawn->mProfile.stalker = true;
				spawn->mProfile.killer = false;

				spawn->mProfile.attackWarningRange = 20;
				spawn->mProfile.attackRange = 15;
				spawn->mProfile.maxAggroRange = 65;
				spawn->mProfile.attackWarningMessage = "";
				spawn->mProfile.attackStartMessage = "";
				spawn->mProfile.attackedMessage = "";

				spawn->mProfile.roamingPeriodTime = 120000;
				spawn->mProfile.roamingSteps = -1;
				spawn->mProfile.roamingSpeed = 0.5;
				spawn->mProfile.roamingDistanceMax = 32;

				spawn->mProfile.stalkerSpeed = 4.0;
				spawn->mProfile.stalkerDistanceMax = 64;
				// spawn->mProfile.groupAssist = true;
				spawn->mProfile.groupAssist = false;
			}
			else if (this->mLairData.mTemplateId == nestTemplateId)
			{
				// Rills.
				spawn->mWeapon.minDamage = 50;
				spawn->mWeapon.maxDamage = 55;
				spawn->mWeapon.weaponMaxRange = 6;
				spawn->mWeapon.attackSpeed = 2000;
				spawn->mWeapon.weaponXp = 113;			// a womp rat.

				spawn->mProfile.agressiveMode = true;
				spawn->mProfile.roaming = true;
				spawn->mProfile.stalker = true;
				spawn->mProfile.killer = false;

				spawn->mProfile.attackWarningRange = 35;
				spawn->mProfile.attackRange = 25;
				spawn->mProfile.maxAggroRange = 65;
				spawn->mProfile.attackWarningMessage = "";
				spawn->mProfile.attackStartMessage = "";
				spawn->mProfile.attackedMessage = "";

				spawn->mProfile.roamingPeriodTime = 120000;
				spawn->mProfile.roamingSteps = -1;
				spawn->mProfile.roamingSpeed = 0.75;
				spawn->mProfile.roamingDistanceMax = 32;

				spawn->mProfile.stalkerSpeed = 4.5;
				spawn->mProfile.stalkerDistanceMax = 75;
				spawn->mProfile.groupAssist = true;
			}
			nonPersistentNpcFactory->requestObject(NpcManager::Instance(), creatureTemplate, npcNewId, (const SpawnData) (*spawn));
			delete spawn;
		}
	}
}

//=============================================================================
//
//	Check if we have any player near us.

/*
bool LairObject::playerInRange(float range)
{
	bool playerFound = false;

	PlayerObjectSet* knownPlayers = this->getKnownPlayers();
	PlayerObjectSet::iterator it = knownPlayers->begin();
	while(it != knownPlayers->end())
	{
		if (gWorldManager->objectsInRange(this->getId(), (*it)->getId(), range))
		{
			playerFound = true;
			break;
		}
		++it;
	}
	return playerFound;
}
*/

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
	// gLogger->logMsgF("LairObject::killEvent: Entering", MSG_NORMAL);
	if (mLairData.mRespawnPeriod != 0)
	{
		// gLogger->logMsgF("LairObject::killEvent: Creating a new lair with template = %llu", MSG_NORMAL, mLairData.mTemplateId);

		uint64 npcNewId = gWorldManager->getRandomNpNpcIdSequence();	
		if (npcNewId != 0)
		{
			// Let's put this sucker into play again.
			NonPersistentNpcFactory::Instance()->requestObject(NpcManager::Instance(), mLairData.mTemplateId, npcNewId, mLairData);
		}
	}
}

void LairObject::respawn(void)
{
	// gLogger->logMsgF("LairObject::respawn() Entering", MSG_NORMAL);

	Anh_Math::Quaternion	direction;
	Anh_Math::Vector3		position;

	// The data used below should be taken from DB, not hard coded as it is now.
	// We have to live with the hard coding, as this is still "under construction".

	// Create a lair, passive style in the wilderness.

	// Spawn position. Can be fixed or random within given "region".

	this->setParentId(mLairData.mSpawnCell);

	if (mLairData.mSpawnPositionFixed)
	{
		// Use the existing position.
		position = mLairData.mSpawnPosition;
		direction = mLairData.mSpawnDirection;
	}
	else
	{
		// Rectangle(float lowX,float lowZ,float width,float height) : Shape(lowX,0.0f,lowZ),mWidth(width),mHeight(height){}
		Anh_Math::Vector3 *pos = mLairData.mSpawnArea.getPosition();
		position.mX = pos->mX;
		// position.mY = pos->mY;
		position.mZ = pos->mZ;

		float xWidth = mLairData.mSpawnArea.getHeight();
		float zHeight = mLairData.mSpawnArea.getWidth();

		// Ge a random position withing given region.
		// Note that creature can spawn outside the region, since thay have a radius from the lair where thet are allowed to spawn.
		// position.mX = position.mX + (gRandom->getRand() % (int32)(mSpawnArea.getHeight()));
		// position.mZ = position.mZ + (gRandom->getRand() % (int32)(mSpawnArea.getWidth()));
		position.mX = position.mX + (gRandom->getRand() % (int32)(xWidth));
		position.mZ = position.mZ + (gRandom->getRand() % (int32)(zHeight));

		position.mY = this->getHeightAt2DPosition(position.mX, position.mZ);

		direction = mDirection;	// TODO: Random direction.
	}
	// gLogger->logMsgF("Spawn at %.0f %.0f %.0f", MSG_NORMAL, position.mX, position.mY, position.mZ);

	mDirection = direction;
	mPosition = position;

	// We do not spawn any babys yet.

	for (int32 i = 0; i < mLairData.mWaveSize; i++)
	{
		mLairData.mPassiveCreature[i] = mLairData.mPassiveWaves;
	}
	for (int32 i = mLairData.mWaveSize; i < MaxWaveSize; i++)
	{
		mLairData.mPassiveCreature[i] = 0;
	}
	
	// Register object with WorldManager.
	gWorldManager->addObject(this, true);

	// Put the lair in the Dormant queue. 
	// I do not want all lair running at same respawn period to do their initial spawn at the same time.
	mLairData.mInitialSpawnPeriod = mLairData.mRespawnPeriod + (int64)(gRandom->getRand() % (int32)mLairData.mRespawnPeriod);
	// gLogger->logMsgF("LairObject::respawn: Respawning in %lld seconds", MSG_NORMAL, mLairData.mInitialSpawnPeriod/1000);

	// When re-spawning, the lair awaits activation in the dormant queue, so we either load the lair instantly and have a wait timer running before the spawn,
	// or let the lair wait in the timed queue system, and then spawn instantly at activation.

	// Since we can force a lair (any object) out of the dormant queue, we have to do the actual spwan countdown with a created object.
	// gWorldManager->addDormantNpc(getId(), mLairData.mInitialSpawnPeriod);
	gWorldManager->addDormantNpc(getId(), 0);
	


	}

void LairObject::reportedDead(uint64 deadCreatureId)
{
	// gLogger->logMsgF("LairObject::reportedDead() Entering", MSG_NORMAL);
	bool found = false;

	// this->mCreatureId[i] = npcNewId;
	for (int32 i = 0; i < mLairData.mWaveSize; i++)
	{
		if ((deadCreatureId == this->mLairData.mCreatureId[i]) && (this->mLairData.mPassiveCreature[i] > 0))
		{
			found = true;

			// We have to make a new passive spawn.
			this->mLairData.mPassiveCreature[i]--;

			uint64 creatureTemplate;
			uint8 creatureTypeIndex = gRandom->getRand() % (int32)100;	// Better to add all values...
			for (int32 y = 0; y < MaxCreatureTypes; y++)
			{
				if (creatureTypeIndex <= mLairData.mCreatureSpawnRate[y])
				{
					creatureTemplate = mLairData.mCreatureTemplates[y];
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
				// Save the id.
				this->mLairData.mCreatureId[i] = npcNewId;

				// Let us get an object from/via the WRONG database (the Persistent... one).
				// gLogger->logMsgF("LairObject::reportedDead: Creating NPC with template = %llu and new id = %llu", MSG_NORMAL, creatureTemplate, npcNewId);
				
				Anh_Math::Vector3 position;

				// Let us get the spawn point. It's x meters from the lair :)
				position.mX = (this->mPosition.mX - 60.0) + (float)(gRandom->getRand() % (int32)(120));
				position.mZ = (this->mPosition.mZ - 60.0) + (float)(gRandom->getRand() % (int32)(120));
				position.mY = this->getHeightAt2DPosition(position.mX, position.mZ);
				
				// gLogger->logMsgF("Setting up spawn of creature at %.0f %.0f %.0f", MSG_NORMAL, position.mX, position.mY, position.mZ);
				// gLogger->logMsgF("This creature will spawn %d more times.", MSG_NORMAL, this->mLairData.mPassiveCreature[i]);

				// The data used below should be taken from DB, not hard coded as it is now.
				// We have to live with the hard coding, as this is still "under construction".

				SpawnData* spawn = new SpawnData;

				spawn->mBasic.templateId = creatureTemplate;

				spawn->mBasic.cellForSpawn = this->getParentId();		// Same cell as the lair.
				// spawn->mBasic.timeToFirstSpawn = 1000;		// Will not be used, since the creature class will think this is a respawn.
				
				// TODO: Do not use this for respawn or not....
				// spawn->mBasic.respawnPeriod = 0;  // Should be 0, since this creature will never respawn himself.
				// Well, problem is that this spawn will be handled like a respawn...
				spawn->mBasic.respawnPeriod = 120000;// 2 min max.
				spawn->mBasic.timeToFirstSpawn = (uint64)(gRandom->getRand() % (int32)spawn->mBasic.respawnPeriod);

				spawn->mBasic.spawnDirection = this->mDirection;	// TODO: Give them a random dir.
				spawn->mBasic.spawnPosition = position;
				spawn->mBasic.spawned = false;
				spawn->mBasic.lairId = this->getId();
				spawn->mBasic.homePosition = this->mPosition;

				// Hard coded test data.
				if (this->mLairData.mTemplateId == pileOfRocksTemplateId)
				{
					spawn->mWeapon.minDamage = 90;
					spawn->mWeapon.maxDamage = 110;
					spawn->mWeapon.weaponMaxRange = 6;
					spawn->mWeapon.attackSpeed = 2000;
					spawn->mWeapon.weaponXp = 356;			// a womp rat.

					spawn->mProfile.agressiveMode = true;
					spawn->mProfile.roaming = true;
					spawn->mProfile.stalker = true;
					spawn->mProfile.killer = false;

					spawn->mProfile.attackWarningRange = 20;
					spawn->mProfile.attackRange = 15;
					spawn->mProfile.maxAggroRange = 65;
					spawn->mProfile.attackWarningMessage = "";
					spawn->mProfile.attackStartMessage = "";
					spawn->mProfile.attackedMessage = "";

					spawn->mProfile.roamingPeriodTime = 120000;
					spawn->mProfile.roamingSteps = -1;
					spawn->mProfile.roamingSpeed = 0.5;
					spawn->mProfile.roamingDistanceMax = 32;

					spawn->mProfile.stalkerSpeed = 4.0;
					spawn->mProfile.stalkerDistanceMax = 64;
					spawn->mProfile.groupAssist = false;
				}
				else if (this->mLairData.mTemplateId == nestTemplateId)
				{
					// Rills.
					spawn->mWeapon.minDamage = 50;
					spawn->mWeapon.maxDamage = 55;
					spawn->mWeapon.weaponMaxRange = 6;
					spawn->mWeapon.attackSpeed = 2000;
					spawn->mWeapon.weaponXp = 113;			// a womp rat.

					spawn->mProfile.agressiveMode = true;
					spawn->mProfile.roaming = true;
					spawn->mProfile.stalker = true;
					spawn->mProfile.killer = false;

					spawn->mProfile.attackWarningRange = 35;
					spawn->mProfile.attackRange = 25;
					spawn->mProfile.maxAggroRange = 65;
					spawn->mProfile.attackWarningMessage = "";
					spawn->mProfile.attackStartMessage = "";
					spawn->mProfile.attackedMessage = "";

					spawn->mProfile.roamingPeriodTime = 120000;
					spawn->mProfile.roamingSteps = -1;
					spawn->mProfile.roamingSpeed = 0.75;
					spawn->mProfile.roamingDistanceMax = 32;

					spawn->mProfile.stalkerSpeed = 4.5;
					spawn->mProfile.stalkerDistanceMax = 75;
					spawn->mProfile.groupAssist = true;
				}
						
				nonPersistentNpcFactory->requestObject(NpcManager::Instance(), creatureTemplate, npcNewId, (const SpawnData) (*spawn));
				delete spawn;
			}
		}
	}
	if (!found)
	{
		gLogger->logMsgF("LairObject::reportedDead() Will NOT respawn", MSG_NORMAL);
	}
}


