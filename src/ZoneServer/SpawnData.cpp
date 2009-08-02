/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

---------------------------------------------------------------------------------------
*/
#include "Utils/utils.h"
#include "SpawnData.h"
#include <assert.h>

//=============================================================================


SpawnData::SpawnData()
{
	mBasic.spawned = false;
	mBasic.lairId = 0;

	mWeapon.weaponXp = 100;	
	mWeapon.attackSpeed = 2000;	// This is default for most animals

	mProfile.agressiveMode = false;
	mProfile.roaming = false;
	mProfile.stalker = false;
	mProfile.killer = false;

	mProfile.roamingSpeed = 1.0;
	mProfile.roamingPeriodTime = 30000;
	mProfile.roamingDistanceMax = 50.0;

	mProfile.stalkerSpeed = 5.0;
	mProfile.stalkerDistanceMax = 100.0;
	mProfile.groupAssist = false;
}

SpawnData::SpawnData(const SpawnData& spawn)
{
	// Do not make any shallow copy.

	mBasic.templateId = spawn.mBasic.templateId;

	mBasic.cellForSpawn = spawn.mBasic.cellForSpawn;
	mBasic.timeToFirstSpawn = spawn.mBasic.timeToFirstSpawn;
	mBasic.respawnPeriod = spawn.mBasic.respawnPeriod;
	mBasic.spawnDirection = spawn.mBasic.spawnDirection;
	mBasic.spawnPosition = spawn.mBasic.spawnPosition;
	mBasic.spawned = spawn.mBasic.spawned;
	mBasic.lairId = spawn.mBasic.lairId;
	mBasic.homePosition = spawn.mBasic.homePosition;
	
	mWeapon.minDamage = spawn.mWeapon.minDamage;
	mWeapon.maxDamage = spawn.mWeapon.maxDamage;
	mWeapon.weaponMaxRange = spawn.mWeapon.weaponMaxRange;
	mWeapon.attackSpeed = spawn.mWeapon.attackSpeed;
	mWeapon.weaponXp = spawn.mWeapon.weaponXp;

	mProfile.agressiveMode = spawn.mProfile.agressiveMode;
	mProfile.roaming = spawn.mProfile.roaming;
	mProfile.stalker = spawn.mProfile.stalker;
	mProfile.killer = spawn.mProfile.killer;

	mProfile.attackWarningRange = spawn.mProfile.attackWarningRange;
	mProfile.attackRange = spawn.mProfile.attackRange;
	mProfile.maxAggroRange = spawn.mProfile.maxAggroRange;
	mProfile.attackWarningMessage = spawn.mProfile.attackWarningMessage;
	mProfile.attackStartMessage = spawn.mProfile.attackStartMessage;
	mProfile.attackedMessage = spawn.mProfile.attackedMessage;

	mProfile.destination = spawn.mProfile.destination;

	mProfile.roamingSpeed = spawn.mProfile.roamingSpeed;
	mProfile.roamingSteps = spawn.mProfile.roamingSteps;
	mProfile.roamingPeriodTime = spawn.mProfile.roamingPeriodTime;
	mProfile.roamingDistanceMax = spawn.mProfile.roamingDistanceMax;

	mProfile.stalkerSpeed = spawn.mProfile.stalkerSpeed;
	mProfile.stalkerDistanceMax = spawn.mProfile.stalkerDistanceMax;
	mProfile.stalkerSteps = spawn.mProfile.stalkerSteps;
	mProfile.groupAssist = spawn.mProfile.groupAssist;

};

SpawnData::~SpawnData()
{

}




SpawnData& SpawnData::operator=(const SpawnData& spawn)
{
	mBasic.templateId = spawn.mBasic.templateId;

	mBasic.cellForSpawn = spawn.mBasic.cellForSpawn;
	mBasic.timeToFirstSpawn = spawn.mBasic.timeToFirstSpawn;
	mBasic.respawnPeriod = spawn.mBasic.respawnPeriod;
	mBasic.spawnDirection = spawn.mBasic.spawnDirection;
	mBasic.spawnPosition = spawn.mBasic.spawnPosition;
	mBasic.spawned = spawn.mBasic.spawned;
	mBasic.lairId = spawn.mBasic.lairId;
	mBasic.homePosition = spawn.mBasic.homePosition;

	mWeapon.minDamage = spawn.mWeapon.minDamage;
	mWeapon.maxDamage = spawn.mWeapon.maxDamage;
	mWeapon.weaponMaxRange = spawn.mWeapon.weaponMaxRange;
	mWeapon.attackSpeed = spawn.mWeapon.attackSpeed;
	mWeapon.weaponXp = spawn.mWeapon.weaponXp;

	mProfile.agressiveMode = spawn.mProfile.agressiveMode;
	mProfile.roaming = spawn.mProfile.roaming;
	mProfile.stalker = spawn.mProfile.stalker;
	mProfile.killer = spawn.mProfile.killer;

	mProfile.attackWarningRange = spawn.mProfile.attackWarningRange;
	mProfile.attackRange = spawn.mProfile.attackRange;
	mProfile.maxAggroRange = spawn.mProfile.maxAggroRange;
	mProfile.attackWarningMessage = spawn.mProfile.attackWarningMessage;
	mProfile.attackStartMessage = spawn.mProfile.attackStartMessage;
	mProfile.attackedMessage = spawn.mProfile.attackedMessage;

	mProfile.destination = spawn.mProfile.destination;

	mProfile.roamingSpeed = spawn.mProfile.roamingSpeed;
	mProfile.roamingSteps = spawn.mProfile.roamingSteps;
	mProfile.roamingPeriodTime = spawn.mProfile.roamingPeriodTime;
	mProfile.roamingDistanceMax = spawn.mProfile.roamingDistanceMax;

	mProfile.stalkerSpeed = spawn.mProfile.stalkerSpeed;
	mProfile.stalkerDistanceMax = spawn.mProfile.stalkerDistanceMax;
	mProfile.stalkerSteps = spawn.mProfile.stalkerSteps;
	mProfile.groupAssist = spawn.mProfile.groupAssist;

	return *this;
}


SpawnData& SpawnData::operator=(const SpawnData* spawn)
{
	mBasic.templateId = spawn->mBasic.templateId;

	mBasic.cellForSpawn = spawn->mBasic.cellForSpawn;
	mBasic.timeToFirstSpawn = spawn->mBasic.timeToFirstSpawn;
	mBasic.respawnPeriod = spawn->mBasic.respawnPeriod;
	mBasic.spawnDirection = spawn->mBasic.spawnDirection;
	mBasic.spawnPosition = spawn->mBasic.spawnPosition;
	mBasic.spawned = spawn->mBasic.spawned;
	mBasic.lairId = spawn->mBasic.lairId;
	mBasic.homePosition = spawn->mBasic.homePosition;

	mWeapon.minDamage = spawn->mWeapon.minDamage;
	mWeapon.maxDamage = spawn->mWeapon.maxDamage;
	mWeapon.weaponMaxRange = spawn->mWeapon.weaponMaxRange;
	mWeapon.attackSpeed = spawn->mWeapon.attackSpeed;
	mWeapon.weaponXp = spawn->mWeapon.weaponXp;

	mProfile.agressiveMode = spawn->mProfile.agressiveMode;
	mProfile.roaming = spawn->mProfile.roaming;
	mProfile.stalker = spawn->mProfile.stalker;
	mProfile.killer = spawn->mProfile.killer;

	mProfile.attackWarningRange = spawn->mProfile.attackWarningRange;
	mProfile.attackRange = spawn->mProfile.attackRange;
	mProfile.maxAggroRange = spawn->mProfile.maxAggroRange;
	mProfile.attackWarningMessage = spawn->mProfile.attackWarningMessage;
	mProfile.attackStartMessage = spawn->mProfile.attackStartMessage;
	mProfile.attackedMessage = spawn->mProfile.attackedMessage;

	mProfile.destination = spawn->mProfile.destination;
	mProfile.roamingSpeed = spawn->mProfile.roamingSpeed;
	mProfile.roamingSteps = spawn->mProfile.roamingSteps;
	mProfile.roamingPeriodTime = spawn->mProfile.roamingPeriodTime;
	mProfile.roamingDistanceMax = spawn->mProfile.roamingDistanceMax;

	mProfile.stalkerSpeed = spawn->mProfile.stalkerSpeed;
	mProfile.stalkerDistanceMax = spawn->mProfile.stalkerDistanceMax;
	mProfile.stalkerSteps = spawn->mProfile.stalkerSteps;
	mProfile.groupAssist = spawn->mProfile.groupAssist;


	return *this;
}


