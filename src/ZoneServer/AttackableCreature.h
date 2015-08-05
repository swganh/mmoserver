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

#ifndef ANH_ZONESERVER_ATTACKABLECREATURE_H
#define ANH_ZONESERVER_ATTACKABLECREATURE_H

#include "NPCObject.h"

// #include "SpawnData.h"

/*
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
*/

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
    AttackableCreature(uint64 templateId);
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
    virtual float	getMaxSpawnDistance(void);

    void	assistCreature(uint64 targetId);
    void	assistLair(uint64 targetId);

    // void					haltAllActions(void) {mCombatState = State_Halted;}

    // void					getSpawnData(SpawnData &spawn);
    // const SpawnData&	getSpawnData(void) const;
    // void				setSpawnData(const SpawnData *spawn);
    // void				setSpawnData(const SpawnData &spawn);

    const Weapon* getPrimaryWeapon(void) const {
        return mPrimaryWeapon;
    }
    void	setPrimaryWeapon(Weapon* primaryWeapon) {
        mPrimaryWeapon = primaryWeapon;
    }

    const Weapon* getSecondaryWeapon(void) const {
        return mSecondaryWeapon;
    }
    void	setSecondaryWeapon(Weapon* secondaryWeapon) {
        mSecondaryWeapon = secondaryWeapon;
    }

    void	equipPrimaryWeapon(void);
    void	equipSecondaryWeapon(void);
    void	unequipWeapon(void);

    int32	getMinDamage(void) const {
        return mMinDamage;
    }
    int32	getMaxDamage(void) const {
        return mMaxDamage;
    }

    uint64	getLairId(void) {
        return mLairId;
    }
    void	setSpawned(void) {
        mSpawned = true;
    }
    void	clearSpawned(void) {
        mSpawned = false;
    }

    bool	isSpawned(void) const {
        return mSpawned;
    }
    bool	isAgressive(void) const {
        return mIsAgressive;
    }
    bool	isRoaming(void) const {
        return mIsRoaming;
    }

    bool	isKiller(void) const {
        return mIsKiller;
    }
    bool	isGroupAssist(void) const {
        return mIsGroupAssist;
    }

    const glm::vec3& getDestination(void) const {
        return mDestination;
    }
    void	setDestination(const glm::vec3& destination) {
        mDestination = destination;
    }
    float	getRoamingSpeed(void) const {
        return mRoamingSpeed;
    }


    int32	getStalkerSteps(void) const {
        return mStalkerSteps;
    }


    /*
    void	setMinDamage(int32 minDamage) {mMinDamage = minDamage;}

    void	setMaxDamage(int32 maxDamage) {mMaxDamage = maxDamage;}


    void	setWeaponMaxRange(int32 weaponMaxRange) {mWeaponMaxRange = weaponMaxRange;}

    void	setAttackSpeed(uint64 attackSpeed) {mAttackSpeed = attackSpeed;}

    void	setMaxAggroRange(int32 maxAggroRange) {mMaxAggroRange = maxAggroRange;}

    // Used with taunts or warning strings, if defined.

    void	setAttackWarningRange(float attackWarningRange) {mAttackWarningRange = attackWarningRange;}

    void	setAttackWarningMessage(BString theMessage) {mAttackWarningMessage = theMessage;}

    const	string& getAttackStartMessage(void) {return mAttackStartMessage;}
    void	setAttackStartMessage(BString theMessage) {mAttackStartMessage = theMessage;}

    void	setAttackedMessage(BString theMessage) {mAttackedMessage = theMessage;}


    void	setRespawnPeriod(uint64 respawnPeriod) {mRespawnPeriod = respawnPeriod;}


    void	setTemplateId(uint64 templateId) {mTemplateId = templateId;}


    void	setCellForSpawn(uint64 cellForSpawn) {mCellForSpawn = cellForSpawn;}

    Anh_Math::Quaternion getSpawnDirection(void) {return mSpawnDirection;}
    void	setSpawnDirection(Anh_Math::Quaternion spawnDirection) {mSpawnDirection = spawnDirection;}


    void	setSpawnPosition(Anh_Math::Vector3 spawnPosition) {mSpawnPosition = spawnPosition;}
    */
    // void	updateDamage(uint64 playerId, uint64 groupId, uint32 weaponGroup, int32 damage, uint8 attackerPosture, float attackerDistance);
    // bool	attackerHaveAggro(uint64 attackerId);
    // void	updateAggro(uint64 playerId, uint64 groupId, uint8 attackerPosture);
    // void	updateAttackersXp(void);
    void	setLairId(uint64 lairId) {
        mLairId = lairId;
    }


private:
    // Default constructor, should not be used.
    AttackableCreature();

    bool	needAssist(void);
    void	executeAssist(void);
    bool	needToAssistLair(void);
    void	executeLairAssist(void);

    // void	updateGroupDamage(DamageDealer* damageDealer);
    // void	sendAttackersWeaponXp(PlayerObject* playerObject, uint32 weaponMask, int32 xp);
    // void	updateAttackersWeaponAndCombatXp(uint64 playerId, uint64 groupId, int32 damageDone, int32 weaponUsedMask);
    void	setupRoaming(int32 maxRangeX, int32 maxRangeZ);
    void	stalk(void);
    bool	atStalkLimit() const;
    bool	insideRoamingLimit() const;
    bool	targetOutsideRoamingLimit(void) const;
    bool	isTargetWithinMaxRange(uint64 targetId);

    float	getWeaponMaxRange(void) const {
        return mWeaponMaxRange;
    }
    float	getMaxAggroRange(void) const {
        return mMaxAggroRange;
    }
    // float	getAttackRange(void) const {return mAttackRange;}
    float	getAttackWarningRange(void) const {
        return mAttackWarningRange;
    }
    bool	isStalker(void) const {
        return mIsStalker;
    }
    float	getStalkerSpeed(void) const {
        return mStalkerSpeed;
    }
    float	getStalkerDistanceMax(void) const {
        return mStalkerDistanceMax;
    }
    // Anh_Math::Vector3 getSpawnPosition(void) const {return mSpawn.mBasic.spawnPosition;}
    const glm::vec3& getHomePosition(void) const {
        return mHomePosition;
    }
    void	setStalkerSteps(int32 stalkerSteps) {
        mStalkerSteps = stalkerSteps;
    }
    bool	setTargetInAttackRange(void);
    uint64	getDefenderOutOfAggroRange(void);
    bool	showWarningInRange(void);
    bool	isTargetValid(void);
    bool	isTargetWithinWeaponRange(void) const;
    int64	getCombatTimer(void) const {
        return mCombatTimer;
    }
    void	setCombatTimer(int64 combatTimer) {
        mCombatTimer = combatTimer;
    }
    int64	getAttackSpeed(void) const {
        return mAttackSpeed;
    }
    uint64	getRoamingDelay(void) const {
        return mRoamingDelay;
    }
    float	getRoamingDistanceMax(void) const {
        return mRoamingDistanceMax;
    }
    bool	isHoming(void) const {
        return mHoming;
    }
    void	enableHoming(void) {
        mHoming = true;
    }
    void	disableHoming(void) {
        mHoming = false;
    }
    int64	getReadyDelay(void) const {
        return mReadyDelay;
    }
    void	SetReadyDelay(int64 readyDelay) {
        mReadyDelay = readyDelay;
    }
    int32	getRoamingSteps(void) const {
        return mRoamingSteps;
    }
    void	setRoamingSteps(int32 roamingSteps) {
        mRoamingSteps = roamingSteps;
    }


    // int32	getWeaponXp(void) const {return mWeaponXp;}
    bool	isAttackTauntSent(void) const;
    void	setAttackTauntSent(void);
    bool	isWarningTauntSent(void) const;
    void	setWarningTauntSent(void);
    void	clearWarningTauntSent(void);
    bool	isAttackedTauntSent(void) const;
    void	setAttackedTauntSent(void);

    const BString getAttackStartMessage(void) const {
        return mAttackStartMessage;
    }
    const BString getAttackWarningMessage(void) const {
        return mAttackWarningMessage;
    }
    const BString getAttackedMessage(void) const {
        return mAttackedMessage;
    }

    // The transfered functions are placed here.
    void	spawn(void);
    bool	setTargetDefenderWithinWeaponRange(void);
    bool	setTargetDefenderWithinMaxRange(void);
    void	setupStalking(uint64 updatePeriodTime);

    // Data from attributes.

public:
    // Delay from creation to first spawn.
    uint64 mTimeToFirstSpawn;

    // True if the object is spawned in the world.
    bool	mSpawned;

    // Home position for object, often a lair or POI.
    glm::vec3 mHomePosition;

    // Id of controling unit (lair), or 0 if none.
    uint64	mLairId;

    // Min damage made by equipped weapon
    int32 mMinDamage;

    // Man damage made by equipped weapon
    int32 mMaxDamage;

    // Damage range.
    float mWeaponMaxRange;

    // Speed of weapon fire.
    int64 mAttackSpeed;

    // Weapon XP.
    // int32	mWeaponXp;



    //If the object is a baby! Awww.
    bool mIsBaby;

    // If object shall attack other objects (for now players) when they get inside "attackRange".
    bool mIsAgressive;

    // Default aggro, without any modifiers.
    // float mBaseAggro;

    // If creature should be "roaming" around when idle in the dormant queue.
    bool mIsRoaming;

    // If the creature moves forward to the target and stalk him to his "max stalk range".
    bool mIsStalker;

    bool mIsKiller;

    // Range where "attackWarningMessage" will be sent.
    float	mAttackWarningRange;

    // Players that come within this range will (may) be attacked.
    // float mAttackRange;

    // Max range of enemy to hold aggro (be in combat).
    float mMaxAggroRange;

    // Taunt message, to be used when enemy comes near the "attackRange" when in "aggressiveMode".
    BString mAttackWarningMessage;

    // Taunt message, to be used when creature initiates combat.
    BString mAttackStartMessage;

    // Taunt message, to be used when enemy initiates combat with this creature.
    BString mAttackedMessage;

    // Position we are moving towards. Used by roaming among others...
    glm::vec3 mDestination;

    // Roaming, we go from point A to point B. The movement may be divided in several updates.
    // We chose a new romaing point (can use some rand) and calculates the direction and speed.

    // Speed in m/s when "roaming"
    float mRoamingSpeed;

    // Delay until starting a new roaming.
    uint64 mRoamingDelay;

    // This is the max roaming distance from the spawn point or other central point, like lair
    float mRoamingDistanceMax;

    // Speed in m/s when "stalking"
    float mStalkerSpeed;

    // Number of updates before we are near the target.
    int32 mStalkerSteps;

    // This is the max stalking distance from the spawn point or other central point, like lair
    float mStalkerDistanceMax;

    bool mIsGroupAssist;


private:
    glm::vec3	mStalkingTargetDestination;
    Npc_Combat_State	mCombatState;
    Weapon* 					mPrimaryWeapon;
    Weapon* 					mSecondaryWeapon;
    int64							mReadyDelay;
    int64							mCombatTimer;
    int32		mRoamingSteps; 		// Number of updates before we are done roaming.
    uint64	mAssistanceNeededWithId;
    uint64	mAssistedTargetId;
    uint64	mLairNeedAssistanceWithId;
    bool		mAttackTauntSent;
    bool		mAttackedTauntSent;
    bool		mHoming;
    bool		mIsAssistingLair;
    bool		mWarningTauntSent;
};

//=============================================================================

#endif

