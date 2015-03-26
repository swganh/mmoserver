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

#ifndef ANH_ZONESERVER_NPC_OBJECT_H
#define ANH_ZONESERVER_NPC_OBJECT_H

#include "CreatureObject.h"
#include "NPC_Enums.h"

#define NPC_CHAT_SPAM_PROTECTION_TIME	10000

//=============================================================================

class ActiveConversation;
class ConversationPage;
class ConversationOption;
class PlayerObject;
class Conversation;

//=============================================================================

class DamageDealer;

typedef std::vector<DamageDealer*> DamageDealers;

class NPCObject : public CreatureObject
{
public:

    friend class PersistentNPCFactory;
    friend class NonPersistentNpcFactory;

    typedef enum _Npc_AI_State
    {
        NpcIsDormant = 0,
        NpcIsReady,
        NpcIsActive
    } Npc_AI_State;

    NPCObject();
    virtual ~NPCObject();

    // Spawn info.
    uint32			getNpcFamily() const {
        return mNpcFamily;
    }
    void			setNpcFamily(uint32 family) {
        mNpcFamily = family;
    }
    uint64			getTemplateId(void) const {
        return mNpcTemplateId;
    }
    void			setTemplateId(uint64 templateId) {
        mNpcTemplateId = templateId;
    }
    uint64			getRespawnDelay(void) const {
        return mRespawnDelay;
    }
    void			setRespawnDelay(uint64 respawnDelay) {
        mRespawnDelay = respawnDelay;
    }
    uint64			getCellIdForSpawn(void) const {
        return mCellIdForSpawn;
    }
    void			setCellIdForSpawn(uint64 cellIdForSpawn) {
        mCellIdForSpawn = cellIdForSpawn;
    }
    void			setSpawnPosition(const glm::vec3& spawnPosition) {
        mSpawnPosition = spawnPosition;
    }
    const glm::vec3& getSpawnPosition() const {
        return mSpawnPosition;
    }
    void			setSpawnDirection(const glm::quat& spawnDirection) {
        mSpawnDirection = spawnDirection;
    }
    const glm::quat& getSpawnDirection() const {
        return mSpawnDirection;
    }


    BString			getTitle() const {
        return mTitle;
    }
    void			setTitle(BString title) {
        mTitle = title;
    }

    virtual void	filterConversationOptions(ConversationPage* page,std::vector<ConversationOption*>* filteredOptions,PlayerObject* player) {}
    virtual uint32	handleConversationEvent(ActiveConversation* av,ConversationPage* page,ConversationOption* option,PlayerObject* player) {
        return 0;
    }
    virtual	bool	preProcessfilterConversation(ActiveConversation* av, Conversation* conversation, PlayerObject* player) {
        return false;
    }
    virtual void	postProcessfilterConversation(ActiveConversation* av,ConversationPage* page,PlayerObject* player) {}
    virtual void	prepareConversation(PlayerObject* player) {}
    virtual void	stopConversation(PlayerObject* player) {}
    void			restoreDefaultDirection() {
        mDirection = mDefaultDirection;
    }
    void			storeDefaultDirection() {
        mDefaultDirection = mDirection;
    }
    virtual void	restorePosition(PlayerObject* player) {}

    virtual void	handleEvents(void) { }
    virtual uint64	handleState(uint64 timeOverdue) {
        return 0;
    }

    virtual float	getMaxSpawnDistance(void) {
        return 0.0;
    }


    // Used for NPC movements
    void			setPositionOffset(const glm::vec3& positionOffset) {
        mPositionOffset = positionOffset;
    }
    const glm::vec3&	getPositionOffset() const {
        return mPositionOffset;
    }

    glm::vec3       getRandomPosition(const glm::vec3& currentPos, int32 offsetX, int32 offsetZ) const;
    float			getHeightAt2DPosition(float xPos, float zPos, bool bestOffer = false) const;
    void			setDirection(float deltaX, float deltaZ);

    void			moveAndUpdatePosition(void);

    uint64			getLastConversationTarget()const {
        return mLastConversationTarget;
    }
    uint64			getLastConversationRequest() const {
        return mLastConversationRequest;
    }

    void			setLastConversationTarget(uint64 target) {
        mLastConversationTarget = target;
    }
    void			setLastConversationRequest(uint64 request) {
        mLastConversationRequest = request;
    }


    void			setRandomDirection(void);
    Npc_AI_State	getAiState(void) const;

    void	updateDamage(uint64 playerId, uint64 groupId, uint32 weaponGroup, int32 damage, uint8 attackerPosture, float attackerDistance);
    bool	attackerHaveAggro(uint64 attackerId);
    void	updateAggro(uint64 playerId, uint64 groupId, uint8 attackerPosture);
    void	updateAttackersXp(void);
    void	setBaseAggro(float baseAggro) {
        mBaseAggro = baseAggro;
    }
    bool	allowedToLoot(uint64 targetId, uint64 groupId);


private:
    void	updateGroupDamage(DamageDealer* damageDealer);
    void	sendAttackersWeaponXp(PlayerObject* playerObject, uint32 weaponMask, int32 xp);
    void	updateAttackersWeaponAndCombatXp(uint64 playerId, uint64 groupId, int32 damageDone, int32 weaponUsedMask);
    int32	getWeaponXp(void) const {
        return mWeaponXp;
    }


protected:
    float	getAttackRange(void) {
        return mAttackRange;
    }
    void	setAttackRange(float attackRange) {
        mAttackRange = attackRange;
    }
    void	setWeaponXp(int32 weaponXp) {
        mWeaponXp = weaponXp;
    }

    void			setAiState(Npc_AI_State state);

    BString	mTitle;
    uint64	mLastConversationRequest;
    uint64	mLastConversationTarget;
    uint64  mSpeciesId;
    uint32	mNpcFamily;


private:

    DamageDealers	mDamageDealers;
    DamageDealers	mDamageByGroups;

    glm::quat	mDefaultDirection;	// Default direction for npc-objects. Needed when players start turning the npc around.
    glm::vec3	mPositionOffset;
    glm::quat	mSpawnDirection;
    glm::vec3	mSpawnPosition;

    Npc_AI_State	mAiState;

    float mAttackRange; // Players that come within this range will (may) be attacked.
    float mBaseAggro; // Default aggro, without any modifiers.

    // Spawn info.
    uint64  mCellIdForSpawn;
    uint64	mLootAllowedById;
    uint64	mLootGroupId;
    uint64	mNpcTemplateId;
    uint64 mRespawnDelay;		// Delay before the object will respawn. Period time taken from time of destruction from world (not the same as when you "die").
    int32	mWeaponXp;
};

//=============================================================================


class	NpcConversionTime
{
public:
    NPCObject*	mNpc;
    uint64		mTargetId;		// This is used when we run instances, and to differ
    uint64		mGroupId;		// Player belonging to same group, will be handled as one "unit" regarding the timeer.
    uint64		mInterval;
};

#endif
