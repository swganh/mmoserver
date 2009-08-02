/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_ATTACKABLECREATURE_H
#define ANH_ZONESERVER_ATTACKABLECREATURE_H

#include "NPCObject.h"
#include "NpcManager.h"
#include "SpawnData.h"

class DamageDealer
{
	public:
		 DamageDealer() {}
		~DamageDealer() {}

	// private:
		uint64	mPlayerId;
		uint64	mGroupId;
		uint32	mWeaponGroupMasks;
		int32	mDamage;
		float	mAggroPoints;
};

typedef std::vector<DamageDealer*> DamageDealers;

typedef enum _Npc_Combat_State
{
	State_Idle = 0,
	State_Dead = 1,
	State_Unspawned = 2,
	State_Alerted = 3,
	State_CombatReady = 4,
	State_Combat = 5,
	State_Halted = 6


} Npc_Combat_State;

//=============================================================================

class AttackableCreature : public NPCObject
{

	friend class PersistentNpcFactory;
	friend class NonPersistentNpcFactory;

	public:
		AttackableCreature();
		virtual ~AttackableCreature();
		virtual void prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount);
		virtual void handleObjectMenuSelect(uint8 messageType,Object* srcObject);
		// void setDeathEffect(uint32 effectId) {mDeathEffectId = effectId;}

		virtual void	addKnownObject(Object* object);

		virtual void	handleEvents(void);
		virtual uint64	handleState(uint64 timeOverdue);
		virtual void	inPeace(void);
		virtual void	killEvent(void);
		virtual void	respawn(void);

		void	assistCreature(uint64 targetId);
		void	assistLair(uint64 targetId);

		// void					haltAllActions(void) {mCombatState = State_Halted;}

		// void					getSpawnData(SpawnData &spawn);
		const SpawnData&	getSpawnData(void) const;
		void				setSpawnData(const SpawnData *spawn);
		void				setSpawnData(const SpawnData &spawn);

		const Weapon* getPrimaryWeapon(void) const {return mPrimaryWeapon;}
		void	setPrimaryWeapon(Weapon* primaryWeapon) {mPrimaryWeapon = primaryWeapon;}

		const Weapon* getSecondaryWeapon(void) const {return mSecondaryWeapon;}
		void	setSecondaryWeapon(Weapon* secondaryWeapon) {mSecondaryWeapon = secondaryWeapon;}

		void	equipPrimaryWeapon(void);
		void	equipSecondaryWeapon(void);
		void	unequipWeapon(void);

		int32	getMinDamage(void) const {return mSpawn.mWeapon.minDamage;}
		int32	getMaxDamage(void) const {return mSpawn.mWeapon.maxDamage;}

		uint64	getRespawnPeriod(void) const {return mSpawn.mBasic.respawnPeriod;}
		uint64	getTemplateId(void) const {return mSpawn.mBasic.templateId;}
		
		uint64	getLairId(void) { return mSpawn.mBasic.lairId;}
		void	setSpawned(void) {mSpawn.mBasic.spawned = true;}
		void	clearSpawned(void) {mSpawn.mBasic.spawned = false;}

		bool	isSpawned(void) const {return mSpawn.mBasic.spawned;}
		bool	isAgressiveMode(void) const {return mSpawn.mProfile.agressiveMode;}
		bool	isRoaming(void) const {return mSpawn.mProfile.roaming;}

		bool	isKiller(void) const {return mSpawn.mProfile.killer;}
		bool	isGroupAssist(void) const {return mSpawn.mProfile.groupAssist;}
		
		Anh_Math::Vector3 getDestination(void) const {return mSpawn.mProfile.destination;} 
		void	setDestination(Anh_Math::Vector3 destination) {mSpawn.mProfile.destination = destination;} 
		float	getRoamingSpeed(void) const {return mSpawn.mProfile.roamingSpeed;}
		

		int32	getStalkerSteps(void) const {return mSpawn.mProfile.stalkerSteps;}


		/*
		void	setMinDamage(int32 minDamage) {mMinDamage = minDamage;}

		void	setMaxDamage(int32 maxDamage) {mMaxDamage = maxDamage;}

		float	getAttackRange(void) {return mAttackRange;}
		void	setAttackRange(float attackRange) {mAttackRange = attackRange;}

		void	setWeaponMaxRange(int32 weaponMaxRange) {mWeaponMaxRange = weaponMaxRange;}

		void	setAttackSpeed(uint64 attackSpeed) {mAttackSpeed = attackSpeed;}

		void	setMaxAggroRange(int32 maxAggroRange) {mMaxAggroRange = maxAggroRange;}

		// Used with taunts or warning strings, if defined.

		void	setAttackWarningRange(float attackWarningRange) {mAttackWarningRange = attackWarningRange;}

		void	setAttackWarningMessage(string theMessage) {mAttackWarningMessage = theMessage;}

		const	string& getAttackStartMessage(void) {return mAttackStartMessage;}
		void	setAttackStartMessage(string theMessage) {mAttackStartMessage = theMessage;}

		void	setAttackedMessage(string theMessage) {mAttackedMessage = theMessage;}

		
		void	setRespawnPeriod(uint64 respawnPeriod) {mRespawnPeriod = respawnPeriod;}

		
		void	setTemplateId(uint64 templateId) {mTemplateId = templateId;}


		void	setCellForSpawn(uint64 cellForSpawn) {mCellForSpawn = cellForSpawn;}

		Anh_Math::Quaternion getSpawnDirection(void) {return mSpawnDirection;}
		void	setSpawnDirection(Anh_Math::Quaternion spawnDirection) {mSpawnDirection = spawnDirection;}


		void	setSpawnPosition(Anh_Math::Vector3 spawnPosition) {mSpawnPosition = spawnPosition;}
		*/
		void	updateDamage(uint64 playerId, uint64 groupId, uint32 weaponGroup, int32 damage, uint8 attackerPosture, float attackerDistance);
		bool	attackerHaveAggro(uint64 attackerId);
		void	updateAggro(uint64 playerId, uint64 groupId, uint8 attackerPosture);
		void	updateAttackersXp(void);
		bool	allowedToLoot(uint64 targetId, uint64 groupId);


	private:
		bool	needAssist(void);
		void	executeAssist(void);
		bool	needToAssistLair(void);
		void	executeLairAssist(void);

		void	updateGroupDamage(DamageDealer* damageDealer);
		void	sendAttackersWeaponXp(PlayerObject* playerObject, uint32 weaponMask, int32 xp);
		void	updateAttackersWeaponAndCombatXp(uint64 playerId, uint64 groupId, int32 damageDone, int32 weaponUsedMask);
		void	setupRoaming(int32 maxRangeX, int32 maxRangeZ);
		void	stalk(void);
		bool	atStalkLimit() const;
		bool	insideRoamingLimit() const;
		bool	targetOutsideRoamingLimit(void) const;
		bool	isTargetWithinMaxRange(uint64 targetId);

		float	getWeaponMaxRange(void) const {return mSpawn.mWeapon.weaponMaxRange;}
		float	getMaxAggroRange(void) const {return mSpawn.mProfile.maxAggroRange;}
		float	getAttackRange(void) const {return mSpawn.mProfile.attackRange;}
		float	getAttackWarningRange(void) const {return mSpawn.mProfile.attackWarningRange;}
		bool	isStalker(void) const {return mSpawn.mProfile.stalker;}
		float	getStalkerSpeed(void) const {return mSpawn.mProfile.stalkerSpeed;}
		float	getStalkerDistanceMax(void) const {return mSpawn.mProfile.stalkerDistanceMax;}
		Anh_Math::Vector3 getSpawnPosition(void) const {return mSpawn.mBasic.spawnPosition;}
		Anh_Math::Vector3 getHomePosition(void) const {return mSpawn.mBasic.homePosition;}
		void	setStalkerSteps(int32 stalkerSteps) {mSpawn.mProfile.stalkerSteps = stalkerSteps;}
		bool	setTargetInAttackRange(void);
		uint64	getDefenderOutOfAggroRange(void);
		bool	showWarningInRange(void);
		bool	isTargetValid(void);
		bool	isTargetWithinWeaponRange(void) const;
		int64	getCombatTimer(void) const {return mCombatTimer;}
		void	setCombatTimer(int64 combatTimer) {mCombatTimer = combatTimer;}
		uint64	getAttackSpeed(void) const {return mSpawn.mWeapon.attackSpeed;}
		uint64	getRoamingPeriodTime(void) const {return mSpawn.mProfile.roamingPeriodTime;}
		float	getRoamingDistanceMax(void) const {return mSpawn.mProfile.roamingDistanceMax;}
		bool	isHoming(void) const {return mHoming;}
		void	enableHoming(void) {mHoming = true;}
		void	disableHoming(void) {mHoming = false;}
		int64	getReadyDelay(void) const {return mReadyDelay;}
		void	SetReadyDelay(int64 readyDelay) {mReadyDelay = readyDelay;}
		int32	getRoamingSteps(void) const {return mSpawn.mProfile.roamingSteps;}
		void	setRoamingSteps(int32 roamingSteps) {mSpawn.mProfile.roamingSteps = roamingSteps;}
		uint64	getCellForSpawn(void) const {return mSpawn.mBasic.cellForSpawn;}

		int32	getWeaponXp(void) const {return mSpawn.mWeapon.weaponXp;}
		bool	isAttackTauntSent(void) const;
		void	setAttackTauntSent(void);
		bool	isWarningTauntSent(void) const;
		void	setWarningTauntSent(void);
		void	clearWarningTauntSent(void);

		const string getAttackStartMessage(void) const {return mSpawn.mProfile.attackStartMessage;}
		const string getAttackWarningMessage(void) const {return mSpawn.mProfile.attackWarningMessage;}
		const string getAttackedMessage(void) const {return mSpawn.mProfile.attackedMessage;}

		// The transfered functions are placed here.
		void	spawn(void);
		bool	setTargetDefenderWithinWeaponRange(void);
		bool	setTargetDefenderWithinMaxRange(void);
		void	setupStalking(uint64 updatePeriodTime);
		
		int64	mReadyDelay;
		int64	mCombatTimer;
		bool	mHoming;
		Anh_Math::Vector3	mStalkingTargetDestination;

		bool	mAttackTauntSent;
		bool	mWarningTauntSent;
		uint64	mAssistedTargetId;
		uint64	mAsssistanceNeededWithId;
		uint64	mLairNeedAsssistanceWithId;
		bool	mIsAssistingLair;

		uint64	mLootAllowedById;

		// uint32 mDeathEffectId;

		Npc_Combat_State	mCombatState;
		
		SpawnData	mSpawn;

		DamageDealers	mDamageDealers;
		DamageDealers	mDamageByGroups;

		Weapon* mPrimaryWeapon;
		Weapon* mSecondaryWeapon;
};

//=============================================================================

#endif

