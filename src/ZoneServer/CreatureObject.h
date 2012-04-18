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

#ifndef ANH_ZONESERVER_CREATURE_OBJECT_H
#define ANH_ZONESERVER_CREATURE_OBJECT_H

#include "EquipManager.h"
#include "Ham.h"
#include "MovingObject.h"
#include "SkillManager.h"
#include "CreatureEnums.h"
#include <map>
#include <list>

//=============================================================================

class Buff;
class IncapRecoveryEvent;

//=============================================================================

typedef std::map<uint32,void*>			SkillCommandMap;
typedef std::pair<uint32,int32>			FactionPoints;
typedef std::vector<FactionPoints>		FactionList;
typedef std::vector<CreatureObject*>	CreatureList;
typedef std::list<Object*>				ObjectList;
typedef std::list<Buff*>				BuffList;

//=============================================================================

static const uint64 LootedCorpseTimeout = 30*1000;


class CreatureObject : public MovingObject
{
    public:

        friend class PersistentNpcFactory;
        friend class NonPersistentNpcFactory;

        CreatureObject();
        ~CreatureObject();

        // inherited from moving object
        virtual void		updateMovementProperties();

        Ham*				getHam(){ return &mHam; }

        BString				getFirstName() const { return mFirstName; }
        void				setFirstName(BString name){ mFirstName = name; }
        BString				getLastName() const { return mLastName; }
        void				setLastName(BString name){ mLastName = name; }

        //uint32				getPosture() const { return mPosture; }
        // calls setLocomotion as well
        //void				setPosture(uint32 posture){ setLocomotionByPosture(posture); mPosture = posture; }

        //uint64				getLocomotion() const { return mLocomotion; }
        //void				setLocomotion(uint32 loco){ mLocomotion = loco ;}
        // Locomotion set only through setPosture
        //void				setLocomotionByPosture(uint32 posture);
        // Postures are NOT bitwise constants.
        // Can NOT use bitwise operation on non bitwise constants.
        // bool				checkPostures(uint8 postures) const { return((mPosture & postures) == postures); }
        //bool				checkPosture(uint32 postures) const { return (mPosture == postures); }

        // Can NOT use bitwise operation on non bitwise constants.
        // bool				checkPosturesEither(uint8 postures){ return((mPosture & postures) != 0); }

        float				getScale(){ return mScale; }
        void				setScale(float scale){ mScale = scale; }
        uint16				getCL(){ return mCL; }
        void				setCL(uint16 cl){ mCL = cl; }
        uint8				getRaceId() const { return mRaceId; }
        void				setRaced(uint8 id){ mRaceId = id; }
        BString				getSpeciesString(){ return mSpecies; }
        void				setSpeciesString(const int8* species){ mSpecies = species; }
        BString				getSpeciesGroup(){ return mSpeciesGroup; }
        void				setSpeciesGroup(const int8* speciesGroup){ mSpeciesGroup = speciesGroup; }
        //Object*			getTarget() const { return mTargetObject; }
        Object*				getTarget() const;
        // void				setTarget(Object* object){ mTargetObject = object; }
        void				setTarget(uint64 targetId){ mTargetId = targetId; }
        // uint64			getTargetId() const { return(mTargetObject != NULL) ? mTargetObject->getId():0; }
        uint64				getTargetId() const { return mTargetId; }
        uint64				getGroupId() const { return mGroupId; }
        void				setGroupId(uint64 groupId) { mGroupId = groupId; }

        uint16*				getCustomization(){ return &mCustomization[0]; }
        void				setCustomization(uint8 index, uint16 val){ mCustomization[index] = val; }
        BString				getCustomizationStr(){ return mCustomizationStr; }
        void				setCustomizationStr(const int8* customization){ mCustomizationStr = customization; }

        //we need to reference hair outside of the equipmanager as the hairslot can be occupied by helmets
        Object*				getHair(){ return mHair; }
        void				setHair(Object* hair){ mHair = hair; }

        CreatureGroup		getCreoGroup(){ return mCreoGroup; }
        void				setCreoGroup(CreatureGroup group){ mCreoGroup = group; }

        uint8				getMoodId() const { return mMoodId; }
        void				setMoodId(uint8 id){ mMoodId = id; }

        // skills
        void				addSkill(Skill* skill){ mSkills.push_back(skill); }
        bool				removeSkill(Skill* skill);

        //whether we have a certain skill as found in skillenums.h SMSkillType or the skills table
        bool				checkSkill(uint32 skillId);
        uint32				getSkillPointsLeft();
        SkillList*			getSkills(){ return &mSkills; }
        SkillModsList*		getSkillMods(){ return &mSkillMods; }
        SkillCommandList*	getSkillCommands(){ return &mSkillCommands; }

        //the values of a skillmod with the skillmod as in skillenums.h SMSkillMod
        int32				getSkillModValue(uint32 modId);
        bool				setSkillModValue(uint32 modId,int32 value);
        bool				modifySkillModValue(uint32 modId,int32 modifier);
        uint32				getSkillUpdateCounter(){ return mSkillUpdateCounter; }
        void				setSkillUpdateCounter(uint32 count){ mSkillUpdateCounter = count; }
        uint32				getAndIncrementSkillUpdateCounter(){ mSkillUpdateCounter++; return mSkillUpdateCounter; }
        uint32				getAndIncrementSkillModUpdateCounter(uint32 amount){ mSkillModUpdateCounter+=amount; return mSkillModUpdateCounter; }
        void				prepareSkillMods();
        void				prepareSkillCommands();
        bool				verifyAbility(uint32 abilityCRC);

        SkillModsList::iterator	findSkillMod(uint32 modId);

        // equipped objects
        EquipManager*		getEquipManager(){ return &mEquipManager; }

        // pvp status

        CreaturePvPStatus	getPvPStatus(){ return mPvPStatus; }
        void				setPvPStatus(CreaturePvPStatus status){ mPvPStatus = status; }
        void				togglePvPStateOn(CreaturePvPStatus state){ mPvPStatus = (CreaturePvPStatus)(mPvPStatus | state); }
        void				togglePvPStateOff(CreaturePvPStatus state){ mPvPStatus = (CreaturePvPStatus)(mPvPStatus & ~state); }
        void				togglePvPState(CreaturePvPStatus state){ mPvPStatus = (CreaturePvPStatus)(mPvPStatus ^ state); }
        bool				checkPvPState(CreaturePvPStatus state){ return((CreaturePvPStatus)(mPvPStatus & state) == state); }
        
        // ONLY SWITCH STATES THROUGH THE STATE MANAGER!
        struct STATES
        {
            uint32_t          posture;
            uint32_t          locomotion;
            uint64_t          action;
            bool              blockPosture;
            bool              blockAction;
            bool              blockLocomotion;

            void            blockLayers() { blockPosture = true; blockAction = true; blockLocomotion = true; }
            void            unblock() { blockPosture = false; blockAction = false; blockLocomotion = false; }
            // posture states
            uint32_t        getPosture() { return posture; } 
            void            setPosture(uint32_t pos) { posture = pos; }
            bool			checkPosture(uint32_t pos) const { return (posture == pos); }
            // locomotion states
            uint32_t        getLocomotion() { return locomotion; }
            void            setLocomotion(uint32_t loco) { locomotion = loco; }
            bool			checkLocomotion(uint32_t loco) const { return (locomotion == loco); }
            // action states
            uint64_t        getAction(){return action;}
            void            toggleActionOn(CreatureState state){ action = action | state; }
            void            toggleActionOff(CreatureState state){ action = action & ~ state; }
            bool            checkState(CreatureState state){ return ((action & state) == state); }
            bool            checkStates(uint64_t states){ return ((action & states) == states); }
            bool            checkStatesEither(uint64_t states){ return ((action & states) != 0); }
            // clear states, do not call directly
            void            clearAllStates() { action = CreatureState_ClearState;}
        } states;

        // factions
        BString				getFaction(){ return mFaction; }
        void				setFaction(const int8* faction){ mFaction = faction; }
        uint8				getFactionRank(){ return mFactionRank; }
        void				setFactionRank(uint8 rank){ mFactionRank = rank; }
        FactionList*		getFactionList(){ return &mFactionList; }
        int32				getFactionPointsByFactionId(uint32 id);
        bool				updateFactionPoints(uint32 factionId,int32 value);

        // language
        uint32				getLanguage() const { return mLanguage; }
        void				setLanguage(uint32 language){ mLanguage = language; }

        //entertaining
        void*				getPerformance(){return mPerformance;}
        void				setPerformance(void* performance){mPerformance = performance;}
        uint64				getEntertainerListenToId(){return mEntertainerListenToId;}
        void				setEntertainerListenToId(uint64 listenToId){mEntertainerListenToId= listenToId;}

        PerformingPause		getPerformancePaused(){return mPerformancePaused;}
        void				setPerformancePaused(PerformingPause paused){mPerformancePaused = paused;}

        float				getLastEntertainerXP(){mLastEntertainerXP --; if (mLastEntertainerXP <= 0){mLastEntertainerXP = 0;} return mLastEntertainerXP;}
        void				setLastEntertainerXP(float lastEntertainerXP){mLastEntertainerXP = lastEntertainerXP;}

        bool				handlePerformanceTick(uint64 time,void* ref);
        bool				handleImagedesignerTimeOut(uint64 time,void* ref);

        PerformingState		getPerformingState() const { return mPendingPerform; }
        void				setPerformingState(PerformingState state){ mPendingPerform = state; }

        uint32				getPerformanceId(){ return mPerformanceId; }
        void				setPerformanceId(uint32 Id){ mPerformanceId = Id; }

        BString				getCurrentAnimation(){ return mCurrentAnimation; }
        void				setCurrentAnimation(BString state){ mCurrentAnimation = state; }

        bool				isStationary(){ return mStationary; }
        void				setStationary(bool val){ mStationary = val; }

        uint32				UpdatePerformanceCounter();


        ObjectIDList*		getDefenders(){ return &mDefenders; }
        void				addDefender(uint64 defenderId);
        void				removeAllDefender(void);

        void				removeDefenderAndUpdateList(uint64 defenderId);
        bool				setAsActiveDefenderAndUpdateList(uint64 defenderId);

        void				clearDefenders();
        bool				checkDefenderList(uint64 defenderId);

        uint32				getLastMoveTick(){ return mLastMoveTick; }
        void				setLastMoveTick(uint32 tick){ mLastMoveTick = tick; }

        bool				getReady(){ return mReady; }
        void				setReady(bool b){ mReady = b; }

        uint32				getRaceGenderMask(){ return mRaceGenderMask; }
        void				setRaceGenderMask(uint32 mask){ mRaceGenderMask = mask; }

        void				buildCustomization(uint16 customization[]);

        // called to initialize the mask after the object is loaded
        void				updateRaceGenderMask(bool female);

        // incapacitation
        void				incap();
        void				die();
        bool				isIncapacitated(){ return(states.posture == CreaturePosture_Incapacitated); }
        bool				isDead(){ return(states.posture == CreaturePosture_Dead); }

        // nr of current incaps until death, base values are retrieved through world config
        uint8				getIncapCount(){ return mIncapCount; }
        void				setIncapCount(uint8 count){ mIncapCount = count; }
        void				resetIncapCount(){ mIncapCount = 0; }

        // current incap time
        uint64				getCurrentIncapTime(){ return mCurrentIncapTime; }
        void				setCurrentIncapTime(uint64 incapTime){ mCurrentIncapTime = incapTime; }
        uint64				getFirstIncapTime(){ return mFirstIncapTime; }
        void				setFirstIncapTime(uint64 incapTime){ mFirstIncapTime = incapTime; }

        // new event helper functions
        void                creaturePostureUpdate();
        void                creatureLocomotionUpdate();
        void                creatureActionStateUpdate();
        // event functions
        void				onIncapRecovery(const IncapRecoveryEvent* event);

        virtual void		prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount);
        virtual void		ResetRadialMenu(){mRadialMenu.reset();RadialMenu* radial	= NULL;RadialMenuPtr radialPtr(radial);	mRadialMenu = radialPtr;}

        void				makePeaceWithDefender(uint64 targetId);
        uint64				getNearestDefender(void);
        uint64				getNearestAttackingDefender(void);

        virtual void		inPeace(void) { }
        virtual void		killEvent(void) { }
        virtual void		respawn(void) { }

        
        // flow control vars
        uint64				mTargetId;
        uint32				mDefenderUpdateCounter;
        uint32				mSkillCmdUpdateCounter;
        uint32				mSkillModUpdateCounter;


    protected:

        BuffList			mBuffList;
        FactionList			mFactionList;
        ObjectIDList		mDefenders;
        SkillCommandMap		mSkillCommandMap;
        SkillCommandList	mSkillCommands;
        SkillList			mSkills;
        SkillModsList		mSkillMods;
        EquipManager		mEquipManager;
        Ham					mHam;
        BString				mCurrentAnimation;
        BString				mCustomizationStr;
        BString				mFaction;
        BString				mFirstName;
        BString				mLastName;
        BString				mSpecies;
        BString				mSpeciesGroup;

        //reference Hair here so it is not lost when we put on a helmet that occupies the equipslot!!!
        //on unequipping the helmet and on logout we use this reference to destroy the hair!!!!!
        Object*				mHair;
        void*				mPerformance;

        CreatureGroup		mCreoGroup;
        CreaturePvPStatus	mPvPStatus;
        PerformingPause		mPerformancePaused;
        PerformingState		mPendingPerform;

        uint64				mCurrentIncapTime;
        uint64				mEntertainerListenToId;
        uint64				mFirstIncapTime;
        uint64				mGroupId;
    
        uint64				mState; //char states like peace, combat etc
        float				mLastEntertainerXP;
        float				mScale;
        uint32				mBuffAsyncCount;
        uint32				mLanguage;
        uint32				mLastMoveTick;
        uint32				mPerformanceCounter;
        uint32				mPerformanceId;
        uint32				mRaceGenderMask;
        uint32				mSkillUpdateCounter;
        uint16				mCL;
        uint16				mCustomization[0xFF];
        uint8				mFactionRank;
        uint8				mIncapCount;
        uint8				mMoodId;
        uint32				mPosture;
        uint64				mLocomotion;
        uint8				mRaceId;
        bool				mReady;
        bool				mStationary;			 //sets the stationary flag in the tano3 so better move it there
        // entertaining

        virtual void	handleObjectMenuSelect(uint8 messageType,Object* srcObject);

    public:

        void				ClearAllBuffs();
        void				AddBuff(Buff* buff,  bool stackable = false, bool overwrite = false);
        void				RemoveBuff(Buff* buff);
        void				CleanUpBuffs();
        Buff*				GetBuff(uint32 BuffIcon);
        bool				GetBuffExists(uint32 BuffIcon);
        int					GetNoOfBuffs();
        BuffList*			GetBuffList(){return(&mBuffList);}

        //asynccount is our asyncquery counter when we store buffs async
        uint32				GetBuffAsyncCount(){return(mBuffAsyncCount);}
        void				SetBuffAsyncCount(uint32 count){mBuffAsyncCount = count; }
        void				IncBuffAsyncCount(){mBuffAsyncCount++; }
        void				DecBuffAsyncCount(){mBuffAsyncCount--; }
        //CreatureObject::STATES  states;
};

//=============================================================================

#endif
