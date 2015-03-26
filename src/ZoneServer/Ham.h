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

#ifndef ANH_ZONESERVER_HAM_H
#define ANH_ZONESERVER_HAM_H

#include "HamProperty.h"
#include "Utils/typedefs.h"

//=============================================================================

typedef std::vector<HamProperty*>	HamBars;
typedef std::vector<int32*>			TargetStats;

//=============================================================================

class CreatureObject;

//=============================================================================

enum BarIndex
{
    HamBar_Health		=	0,
    HamBar_Strength		=	1,
    HamBar_Constitution	=	2,
    HamBar_Action		=	3,
    HamBar_Quickness	=	4,
    HamBar_Stamina		=	5,
    HamBar_Mind			=	6,
    HamBar_Focus		=	7,
    HamBar_Willpower	=	8
};

//=============================================================================

class Ham
{
    friend class PlayerObjectFactory;
    friend class PersistentNpcFactory;
    friend class NonPersistentNpcFactory;
    friend class ShuttleFactory;
    friend class ObjectController;

public:

    Ham();

    Ham(CreatureObject* parent,uint32 bf = 0);

    ~Ham();

    CreatureObject*	getParent() {
        return mParent;
    }
    void			setParent(CreatureObject* parent) {
        mParent = parent;
    }

    int32			getBattleFatigue() {
        return mBattleFatigue;
    }
    void			setBattleFatigue(int32 bf);
    void			updateBattleFatigue(int32 bf, bool sendUpdate = true);

    void			updateSingleHam(int32 propertyDelta, bool damage);

    int32			getCurrentForce() {
        return mCurrentForce;
    }
    void			setCurrentForce(int32 amount) {
        mCurrentForce = amount;
    }
    bool			updateCurrentForce(int32 forceDelta,bool sendUpdate = true);

    int32			getMaxForce() {
        return mMaxForce;
    }
    void			setMaxForce(int32 amount) {
        mMaxForce = amount;
    }
    bool			updateMaxForce(int32 forceDelta,bool sendUpdate = true);

    uint32			getCurrentHitpointsUpdateCounter();
    void			advanceCurrentHitpointsUpdateCounter(uint32 amount = 1);

    uint32			getBaseHitpointsUpdateCounter(uint32 nextInterval = 0);
    void			advanceBaseHitpointsUpdateCounter(uint32 amount = 1);

    uint32			getMaxHitpointsUpdateCounter(uint32 nextInterval = 0);
    void			advanceMaxHitpointsUpdateCounter(uint32 amount = 1);

    uint32			getWoundsUpdateCounter(uint32 nextInterval = 0);
    void			advanceWoundsUpdateCounter(uint32 amount = 1);

    int32			getTargetStatValue(uint8 propertyIndex);
    void			setTargetStatValue(uint8 propertyIndex, int32 propertyValue);

    int32			getPropertyValue(uint8 propertyIndex,uint8 valueIndex);
    void			setPropertyValue(uint8 propertyIndex,uint8 valueIndex,int32 propertyValue);
    int32			updatePropertyValue(uint8 propertyIndex,uint8 valueIndex,int32 propertyDelta,bool damage = false,bool sendUpdate = true, bool debuff = false);
    void			performSpecialAction(float healthCost, float actionCost, float mindCost, uint8 valueIndex);
    void			updateAllWounds(int32 propertyDelta);
    void			updatePrimaryWounds(int32 propertyDelta);

    void			calcAllModifiedHitPoints();

    bool			regenerate(uint64 time,void*);
    uint64			getLastRegenTick() {
        return mLastRegenTick;
    }
    void			setLastRegenTick(uint64 time) {
        mLastRegenTick = time;
    }

    void			updateRegenRates();

    int32			getHealthRegenRate();
    int32			getActionRegenRate();
    int32			getMindRegenRate();

    uint64			getTaskId() {
        return mTaskId;
    }
    void			setTaskId(uint64 id) {
        mTaskId = id;
    }

    /// Checks that the main pools are greater than the passed values.
    /**
     * This method checks the main pools and verifies that each is
     * currently greater than respective value that is passed in.
     *
     * @param health Value that the health pool should be greater than.
     *
     * @param action Value that the action pool should be greater than.
     *
     * @param mind Value that the mind pool should be greater than.
     *
     * @returns True if the main pools are exceed the specified values, false if not.
     */
    bool			checkMainPools(int32 health,int32 action,int32 mind);
    void			checkForRegen();

    void			resetModifiers();

    // get total hampoints to check for cheaters on statmigration
    int32			getTotalHamCount();

    void			resetCounters();

    TargetStats		mTargetStats;

    HamBars			mHamBars;

    HamProperty		mHealth;
    HamProperty		mStrength;
    HamProperty		mConstitution;
    HamProperty		mAction;
    HamProperty		mQuickness;
    HamProperty		mStamina;
    HamProperty		mMind;
    HamProperty		mFocus;
    HamProperty		mWillpower;

private:

    bool			_regenHealth();
    bool			_regenAction();
    bool			_regenMind();
    bool			_regenForce();

    CreatureObject*	mParent;

    uint64			mLastRegenTick;
    uint64			mTaskId;

    //statmigration
    int32			mBattleFatigue;
    int32			mCurrentForce;
    int32			mMaxForce;

    int32			mHealthRegenRate;
    int32			mActionRegenRate;
    int32			mMindRegenRate;
    int32			mForceRegenRate;

    int32			mTargetHealth;
    int32			mTargetStrength;
    int32			mTargetConstitution;
    int32			mTargetAction;
    int32			mTargetQuickness;
    int32			mTargetStamina;
    int32			mTargetMind;
    int32			mTargetFocus;
    int32			mTargetWillpower;

    uint32			mBaseHitpointsUpdateCounter;
    uint32			mNextBaseHitpointsUpdateInterval;

    uint32			mCurrentHitpointsUpdateCounter;
    uint32			mNextCurrentHitpointsUpdateInterval;

    uint32			mMaxHitpointsUpdateCounter;
    uint32			mNextMaxHitpointsUpdateInterval;

    uint32			mWoundsUpdateCounter;
    uint32			mNextWoundsUpdateInterval;

    bool			mFirstUpdateCounterChange;
    bool			mRegenerating;
};

#endif
