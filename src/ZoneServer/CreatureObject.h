/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

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

//=============================================================================

class Buff;
class IncapRecoveryEvent;

//=============================================================================

typedef std::map<uint32,void*>			SkillCommandMap;
typedef std::pair<uint32,int32>			FactionPoints;
typedef std::vector<FactionPoints>		FactionList;
typedef std::vector<CreatureObject*>	CreatureList;
typedef std::vector<Object*>			ObjectList;
typedef std::vector<Buff*>				BuffList;

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

		string				getFirstName() const { return mFirstName; }
		void				setFirstName(string name){ mFirstName = name; }
		string				getLastName() const { return mLastName; }
		void				setLastName(string name){ mLastName = name; }

		uint8				getPosture() const { return mPosture; }
		void				setPosture(uint8 posture){ mPosture = posture; }

		// Postures are NOT bitwise constants.
		// Can NOT use bitwise operation on non bitwise constants.
		// bool				checkPostures(uint8 postures) const { return((mPosture & postures) == postures); }
		bool				checkPosture(uint8 postures) const { return (mPosture == postures); }

		// Can NOT use bitwise operation on non bitwise constants.
		// bool				checkPosturesEither(uint8 postures){ return((mPosture & postures) != 0); }

		float				getScale(){ return mScale; }
		void				setScale(float scale){ mScale = scale; }
		uint16				getCL(){ return mCL; }
		void				setCL(uint16 cl){ mCL = cl; }
		uint8				getRaceId() const { return mRaceId; }
		void				setRaced(uint8 id){ mRaceId = id; }
		string				getSpeciesString(){ return mSpecies; }
		void				setSpeciesString(const int8* species){ mSpecies = species; }
		string				getSpeciesGroup(){ return mSpeciesGroup; }
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
		string				getCustomizationStr(){ return mCustomizationStr; }
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

		// states
		uint64				getState(){ return mState; }
		//void				setState(uint64 state){ mState = state; }
		void				toggleStateOn(CreatureState state){ mState = mState | state; }
		void				toggleStateOff(CreatureState state){ mState = mState & ~state; }
		//void				toggleState(CreatureState state){ mState = mState ^ state; }
		bool				checkState(CreatureState state){ return((mState & state) == state); }
		bool				checkStates(uint64 states){ return((mState & states) == states); }
		bool				checkStatesEither(uint64 states){ return((mState & states) != 0); }

		// factions
		string				getFaction(){ return mFaction; }
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

		PerformingState		getPerformingState() const { return mPendingPerform; }
		void				setPerformingState(PerformingState state){ mPendingPerform = state; }

		uint32				getPerformanceId(){ return mPerformanceId; }
		void				setPerformanceId(uint32 Id){ mPerformanceId = Id; }

		string				getCurrentAnimation(){ return mCurrentAnimation; }
		void				setCurrentAnimation(string state){ mCurrentAnimation = state; }

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
		bool				isIncapacitated(){ return(mPosture == CreaturePosture_Incapacitated); }
		bool				isDead(){ return(mPosture == CreaturePosture_Dead); }

		// nr of current incaps until death, base values are retrieved through world config
		uint8				getIncapCount(){ return mIncapCount; }
		void				setIncapCount(uint8 count){ mIncapCount = count; }
		void				resetIncapCount(){ mIncapCount = 0; }

		// current incap time
		uint64				getCurrentIncapTime(){ return mCurrentIncapTime; }
		void				setCurrentIncapTime(uint64 incapTime){ mCurrentIncapTime = incapTime; }
		uint64				getFirstIncapTime(){ return mFirstIncapTime; }
		void				setFirstIncapTime(uint64 incapTime){ mFirstIncapTime = incapTime; }

		// event functions
		void				onIncapRecovery(const IncapRecoveryEvent* event);

		virtual void		prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount);

		void				makePeaceWithDefender(uint64 targetId);
		uint64				getNearestDefender(void);
		uint64				getNearestAttackingDefender(void);

		virtual void		inPeace(void) { }
		virtual void		killEvent(void) { }
		virtual void		respawn(void) { }

		//pet,mount,vehicle
		uint64				getOwner() { return mOwner; }
		void				setOwner(uint64 owner_id) { mOwner = owner_id; }

		uint64				getPetController(){ return mController;}
		void				setPetController(uint64 c){mController = c;}

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
		string				mCurrentAnimation;
		string				mCustomizationStr;
		string				mFaction;
		string				mFirstName;
		string				mLastName;
		string				mSpecies;
		string				mSpeciesGroup;

		//reference Hair here so it is not lost when we put on a helmet that occupies the equipslot!!!
		//on unequipping the helmet and on logout we use this reference to destroy the hair!!!!!
		Object*				mHair;
		void*				mPerformance;

		CreatureGroup		mCreoGroup;
		CreaturePvPStatus	mPvPStatus;
		PerformingPause		mPerformancePaused;
		PerformingState		mPendingPerform;

		uint64				mController;
		uint64				mCurrentIncapTime;
		uint64				mEntertainerListenToId;
		uint64				mFirstIncapTime;
		uint64				mGroupId;
		uint64				mOwner; //If creature is a mount,pet,or vehicle it has an owner
		uint64				mState;
		float				mLastEntertainerXP;
		float				mScale;
		uint32				mAsyncCount;
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
		uint8				mPosture;
		uint8				mRaceId;
		bool				mReady;
		// entertaining

		virtual void	handleObjectMenuSelect(uint8 messageType,Object* srcObject);

	public:

		void				AddBuff(Buff* buff,  bool stackable = false, bool overwrite = false);
		void				RemoveBuff(Buff* buff);
		void				CleanUpBuffs();
		Buff*				GetBuff(uint32 BuffIcon);
		bool				GetBuffExists(uint32 BuffIcon);
		int					GetNoOfBuffs();
		BuffList*			GetBuffList(){return(&mBuffList);}

		//asynccount is our asyncquery counter when we store buffs async
		uint32				GetAsyncCount(){return(mAsyncCount);}
		void				SetAsyncCount(uint32 count){mAsyncCount = count;}
		void				IncAsyncCount(){mAsyncCount++;}
		void				DecAsyncCount(){mAsyncCount--;}
};

//=============================================================================

#endif
