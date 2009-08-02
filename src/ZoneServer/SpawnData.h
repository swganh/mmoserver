/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_SPAWNDATA_H
#define ANH_ZONESERVER_SPAWNDATA_H

#include "MathLib/Vector3.h"
#include "MathLib/Quaternion.h"

//=============================================================================


//=============================================================================

typedef struct _SpawnBasicData
{
	// The family type of the object.
	uint64 templateId;

	// The id of the object. Need to be unique, and NEVER reused during a zone lifetime.
	// The id will never saved in database. The id will be regenerated at respawn.
	// uint64 objectId;
	
	// Cell id or 0 for outside.
	uint64 cellForSpawn;

	// Delay from creation to first spawn.
	uint64 timeToFirstSpawn;

	// How often tho object will respawn. Period time taken from time of destruction from world (not the same as when you "die").
	uint64 respawnPeriod;

	// Direction of object when spawning.
	Anh_Math::Quaternion spawnDirection;

	// Position where object will spawn.
	Anh_Math::Vector3 spawnPosition;

	// True if the object is spwaned in the world.
	bool	spawned;

	// Id of lair, or 0 if no lair.
	uint64	lairId;

	// Home position for object, often a lair or POI.
	Anh_Math::Vector3 homePosition;


} SpawnBasicData;

// TODO: This SpawnWeaponData should be taken from actually object. Not be faked as today.
typedef struct _SpawnWeaponData
{
	// Min damage made by equipped weapon
	int32 minDamage;

	// Man damage made by equipped weapon
	int32 maxDamage;

	// Damage range.
	float weaponMaxRange;

	// Speed of weapon fire.
	int32 attackSpeed;

	// Weapon XP.
	int32	weaponXp;

} SpawnWeaponData;

typedef struct _SpawnCreatureProfile
{
	// If object shall attack other objects (for now players) when they get inside "attackRange".
	bool agressiveMode;

	// If creature should be "roaming" around when idle in the dormant queue.
	bool roaming;

	// If the creature moves forward to the target and stalk him to his "max stalk range".
	bool stalker;

	bool killer;


	// Range where "attackWarningMessage" will be sent.
	float	attackWarningRange;

	// Players that come within this range will (may) be attacked.
	float attackRange;

	// Max range of enemy to hold aggro (be in combat).
	float maxAggroRange;
	
	// Taunt message, to be used when enemy comes near the "attackRange" when in "aggressiveMode".
	string attackWarningMessage;

	// Taunt message, to be used when creature initiates combat.
	string attackStartMessage;

	// Taunt message, to be used when enemy initiates combat with this creature.
	string attackedMessage;

	// Position we are moving towards. Used by roaming among others...
	Anh_Math::Vector3 destination;

	// Roaming, we go from point A to point B. The movement may be divided in several updates.
	// We chose a new romaing point (can use some rand) and calculates the direction and speed.

	// Speed in m / m when "roaming"
	float roamingSpeed;

	// Number of updates before we are done roaming.
	int32 roamingSteps;

	// How often we should start a new roaming.
	uint64 roamingPeriodTime;

	// This is the max roaming distance from the spawn point or other central point, like lair
	float roamingDistanceMax;	

	// Speed in m / m when "stalking"
	float stalkerSpeed;

	// Number of updates before we are near the target.
	int32 stalkerSteps;

	// This is the max stalking distance from the spawn point or other central point, like lair
	float stalkerDistanceMax;	

	bool groupAssist;	

} SpawnCreatureProfile;

class SpawnData
{
	public:
		// SpawnData::SpawnData() {}
		SpawnData::SpawnData();
		SpawnData::~SpawnData();
		
		// Copy constructor
		SpawnData::SpawnData(const SpawnData& spawn);			

		// Assignment operator
		SpawnData& operator=(const SpawnData& spawn);
		SpawnData& operator=(const SpawnData* spawn);

		SpawnBasicData mBasic;
		SpawnWeaponData mWeapon;
		SpawnCreatureProfile mProfile;
};

#endif

