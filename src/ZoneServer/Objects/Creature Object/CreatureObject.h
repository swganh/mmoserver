/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2014 The SWG:ANH Team
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

#include "ZoneServer/GameSystemManagers/Equip Manager/EquipManager.h"
#include "ZoneServer/GameSystemManagers/Ham Manager/Ham.h"
#include "ZoneServer/Objects/MovingObject.h"
#include "ZoneServer\Objects\Creature Object\equipment_item.h"
#include "ZoneServer/GameSystemManagers/Skill Manager/SkillManager.h"
#include <ZoneServer\Objects\Creature Object\CreatureEnums.h>
#include <map>
#include <list>
#include <MessageLib\messages\containers\network_vector.h>
//#include <MessageLib\messages\containers\default_serializer.h>

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
typedef std::vector<int32*>			TargetStats;
typedef swganh::event_dispatcher::ValueEvent<CreatureObject*> CreatureObjectEvent;

//=============================================================================

namespace messages	{
	struct BaseSwgMessage;
}



static const uint64 LootedCorpseTimeout = 30*1000;


class CreatureObject : public MovingObject// , public std::enable_shared_from_this<CreatureObject>
{
    public:

        friend class PersistentNpcFactory;
        friend class NonPersistentNpcFactory;

        CreatureObject();
        ~CreatureObject();

        // inherited from moving object
        virtual void		updateMovementProperties();

		// Equipment List
		void AddEquipmentItem(std::shared_ptr<swganh::object::EquipmentItem>& item);
		void AddEquipmentItem(std::shared_ptr<swganh::object::EquipmentItem>& item, boost::unique_lock<boost::mutex>& lock);

		void RemoveEquipmentItem(uint64_t object_id);
		void RemoveEquipmentItem(uint64_t object_id, boost::unique_lock<boost::mutex>& lock);

		void UpdateEquipmentItem(std::shared_ptr<swganh::object::EquipmentItem>& item);
		void UpdateEquipmentItem(std::shared_ptr<swganh::object::EquipmentItem>& item, boost::unique_lock<boost::mutex>& lock);

		std::vector<std::shared_ptr<swganh::object::EquipmentItem>> GetEquipment();
		std::vector<std::shared_ptr<swganh::object::EquipmentItem>> GetEquipment(boost::unique_lock<boost::mutex>& lock);

		std::shared_ptr<swganh::object::EquipmentItem>& GetEquipmentItem(uint64_t object_id);
		std::shared_ptr<swganh::object::EquipmentItem>& GetEquipmentItem(uint64_t object_id, boost::unique_lock<boost::mutex>& lock);

		void SerializeEquipment(swganh::messages::BaseSwgMessage* message);
		void SerializeEquipment(swganh::messages::BaseSwgMessage* message, boost::unique_lock<boost::mutex>& lock);

        std::string			getFirstName() const { auto lock = AcquireLock(); return getFirstName(lock); }
		std::string			getFirstName(boost::unique_lock<boost::mutex>& lock) const { return first_name; }
		/*	@brief	this method will set a new fist name for the creature. The custom name will be generated and set through setCustomName() this function is threadsafe
		*			
		*	/param std::string first_name the creatures new first name
		*/
        void				setFirstName(std::string name); 
		void				setFirstName(boost::unique_lock<boost::mutex>& lock, std::string name);

        std::string			getLastName() const { auto lock = AcquireLock(); return getLastName(lock); }
		std::string			getLastName(boost::unique_lock<boost::mutex>& lock) const { return last_name; }

		/*	@brief	this method will set a new last name for the creature. The custom name will be generated 
		*			and set through setCustomName()
		*			this function is threadsafe
		*	/param std::string name the creatures new last name
		*/
        void				setLastName(std::string name);
		void				setLastName(boost::unique_lock<boost::mutex>& lock, std::string name);

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
        
		/*	@brief gets the nearest defender of a lair
		*	no matter whether its attacking you or no
		*
		*/
		uint64				getNearestAttackingDefender(void);

		/*	@AddDefender adds a defender (a creature/player(npc) we are attacking
		*	to our defenderlist
		*
		*/
		void AddDefender(uint64_t defenderId);
		void AddDefender(uint64_t defenderId, boost::unique_lock<boost::mutex>& lock);

		uint32 GetDefenderCounter();

		void RemoveDefender(uint64 object_id);
		void RemoveDefender(uint64 object_id, boost::unique_lock<boost::mutex>& lock);

		//void UpdateDefenderItem(uint64_t object_id);
		//nonsense as defenders are id only void UpdateDefenderItem(uint64_t object_id, boost::unique_lock<boost::mutex>& lock);

		void	ClearDefender();
		
		std::vector<uint64> GetDefender();
		std::vector<uint64> GetDefender(boost::unique_lock<boost::mutex>& lock);

		CreatureObject*	GetDefenderItem(uint64 object_id);
		CreatureObject*	GetDefenderItem(uint64 object_id, boost::unique_lock<boost::mutex>& lock);

		bool SerializeDefender(swganh::messages::BaseSwgMessage* message);
		bool SerializeDefender(swganh::messages::BaseSwgMessage* message, boost::unique_lock<boost::mutex>& lock);

		bool	checkDefenderList(uint64 defenderId);

		
        virtual void		inPeace(void) { }
        virtual void		killEvent(void) { }
        virtual void		respawn(void) { }

        
        // flow control vars
        uint64				mTargetId;
        uint32				mDefenderUpdateCounter;
        uint32				mSkillCmdUpdateCounter;
        uint32				mSkillModUpdateCounter;

		
		/**	Max Stats are the maximum we can *currently* have with all buffs and debuffs
		*	@brief	SetStatMax sets the maximum of a stat
		*	/param uint16_t stat_index is the index of the stat we want to modifie (strength, mind, wisdom, agility etc)
		*	/param int32_t value is the value we want to set the stat to
		*/
		void SetStatMax(const uint16_t stat_index, const int32_t value);
		void SetStatMax(const uint16_t stat_index, const int32_t value, boost::unique_lock<boost::mutex>& lock);

		/**	Max Stats are the maximum we can *currently* have with all buffs and debuffs
		*	@brief	SetStatMax Initializes the maximum of a stat - This will NOT trigger neither a persist nor a serialization event
		*	The stats are added to the back of the vector
		*	/param int32_t value is the value we want to set the stat to
		*/
		void InitStatMax(const int32_t value);
		void InitStatMax(const int32_t value, boost::unique_lock<boost::mutex>& lock);

		void InitStatMax(uint16_t stat_index, int32_t value);
		void InitStatMax(uint16_t stat_index, int32_t value, boost::unique_lock<boost::mutex>& lock);

		/*	@brief adds value to the chosen stat max
		*
		*/
		void AddStatMax(const uint16_t stat_index, const int32_t value);
		void AddStatMax(const uint16_t stat_index, const int32_t value, boost::unique_lock<boost::mutex>& lock);

		/*	@brief deducts value from the chosen stat max
		*
		*/
		void DeductStatMax(const uint16_t stat_index, const int32_t value);
		void DeductStatMax(const uint16_t stat_index, const int32_t value, boost::unique_lock<boost::mutex>& lock);

		/*	@brief gets a copy (!) ofthe vector containing all statmaxes
		*
		*/
		std::vector<int32_t> GetMaxStats();
		std::vector<int32_t> GetMaxStats(boost::unique_lock<boost::mutex>& lock);

		/*	@brief gets the value for the statmax with index stat_index
		*
		*/
		int32_t GetStatMax(const uint16_t stat_index);
		int32_t GetStatMax(const uint16_t stat_index, boost::unique_lock<boost::mutex>& lock);


		bool SerializeMaxStats(swganh::messages::BaseSwgMessage* message);
		bool SerializeMaxStats(swganh::messages::BaseSwgMessage* message, boost::unique_lock<boost::mutex>& lock);

		// Stat Encumberance
		/**	Initializes the Wound Stat. 
		*	@brief	InitStatWound Initializes the wound of a stat - This will NOT trigger neither a persist nor a serialization event
		*	The stats are added to the back of the vector
		*	/param int32_t value is the value we want to set the stat to
		*/
		void InitStatEncumberance(int32_t value);
		void InitStatEncumberance(int32_t value, boost::unique_lock<boost::mutex>& lock);

		void InitStatEncumberance(uint16_t stat_index, int32_t value);
		void InitStatEncumberance(uint16_t stat_index, int32_t value, boost::unique_lock<boost::mutex>& lock);

		void AddStatEncumberance(uint16_t stat_index, int32_t value);
		void AddStatEncumberance(uint16_t stat_index, int32_t value, boost::unique_lock<boost::mutex>& lock);

		void DeductStatEncumberance(uint16_t stat_index, int32_t value);
		void DeductStatEncumberance(uint16_t stat_index, int32_t value, boost::unique_lock<boost::mutex>& lock);

		void SetStatEncumberance(uint16_t stat_index, int32_t value);
		void SetStatEncumberance(uint16_t stat_index, int32_t value, boost::unique_lock<boost::mutex>& lock);

		std::vector<int32_t> GetStatEncumberances();
		std::vector<int32_t> GetStatEncumberances(boost::unique_lock<boost::mutex>& lock);

		int32_t GetStatEncumberance(const uint16_t stat_index);
		int32_t GetStatEncumberance(const uint16_t stat_index, boost::unique_lock<boost::mutex>& lock);

		bool SerializeStatEncumberances(swganh::messages::BaseSwgMessage* message);
		bool SerializeStatEncumberances(swganh::messages::BaseSwgMessage* message, boost::unique_lock<boost::mutex>& lock);


		 // Wounds
		/**	Initializes the Wound Stat. 
		*	@brief	InitStatWound Initializes the wound of a stat - This will NOT trigger neither a persist nor a serialization event
		*	The stats are added to the back of the vector
		*	/param int32_t value is the value we want to set the stat to
		*/
		void InitStatWound(int32_t value);
		void InitStatWound(int32_t value, boost::unique_lock<boost::mutex>& lock);

		void InitStatWound(uint16_t stat_index, int32_t value);
		void InitStatWound(uint16_t stat_index, int32_t value, boost::unique_lock<boost::mutex>& lock);

		void DeductStatWound(uint16_t stat_index, int32_t value);
		void DeductStatWound(uint16_t stat_index, int32_t value, boost::unique_lock<boost::mutex>& lock);

		void AddStatWound(uint16_t stat_index, int32_t value);
		void AddStatWound(uint16_t stat_index, int32_t value, boost::unique_lock<boost::mutex>& lock);

		void SetStatWound(uint16_t stat_index, int32_t value);
		void SetStatWound(uint16_t stat_index, int32_t value, boost::unique_lock<boost::mutex>& lock);

		std::vector<int32_t> GetStatWounds();
		std::vector<int32_t> GetStatWounds(boost::unique_lock<boost::mutex>& lock);

		int32_t GetStatWound(const uint16_t stat_index);
		int32_t GetStatWound(const uint16_t stat_index, boost::unique_lock<boost::mutex>& lock);

		bool SerializeStatWounds(swganh::messages::BaseSwgMessage* message);
		bool SerializeStatWounds(swganh::messages::BaseSwgMessage* message, boost::unique_lock<boost::mutex>& lock);



		/*	@brief	Sets the Current Stat. That is the value we currently have after all modifications have been applied
		*	This setter will trigger a delta as well as a persist event
		*/
		void SetStatCurrent(uint16_t stat_index, int32_t value);
		void SetStatCurrent(uint16_t stat_index, int32_t value, boost::unique_lock<boost::mutex>& lock);

		/**	Initializes the Current Stat. That is the value we currently have after all modifications have been applied
		*	@brief	InitStatCurrent Initializes the current value of a stat - This will NOT trigger neither a persist nor a serialization event
		*	The stats are added to the back of the vector
		*	/param int32_t value is the value we want to set the stat to
		*/
		void InitStatCurrent(int32_t value);
		void InitStatCurrent(int32_t value, boost::unique_lock<boost::mutex>& lock);

		void InitStatCurrent(uint16_t stat_index, int32_t value);
		void InitStatCurrent(uint16_t stat_index, int32_t value, boost::unique_lock<boost::mutex>& lock);

		/**	adds 'value' to stat 'stat_index'
		*	
		*/
		void AddStatCurrent(uint16_t stat_index, int32_t value);
		void AddStatCurrent(uint16_t stat_index, int32_t value, boost::unique_lock<boost::mutex>& lock);

		void DeductStatCurrent(uint16_t stat_index, int32_t value);
		void DeductStatCurrent(uint16_t stat_index, int32_t value, boost::unique_lock<boost::mutex>& lock);

		/**	returns  the vector containing all current stats
		*	use GetStatCurrent to get a discrete current stat
		*/
		std::vector<int32_t> GetCurrentStats();
		std::vector<int32_t> GetCurrentStats(boost::unique_lock<boost::mutex>& lock);

		/**	Get a discrete current stat
		*	returns int_32_t
		*	current stat is the stat describing the hitpoints we *currently* have
		*/
		int32_t GetStatCurrent(const uint16_t stat_index);
		int32_t GetStatCurrent(const uint16_t stat_index, boost::unique_lock<boost::mutex>& lock);

		bool SerializeCurrentStats(swganh::messages::BaseSwgMessage* message);
		bool SerializeCurrentStats(swganh::messages::BaseSwgMessage* message, boost::unique_lock<boost::mutex>& lock);

		// Stat Base
		/**	Max Stats are the maximum we can *currently* have with all buffs and debuffs
		*	@brief	SetStatMax Initializes the maximum of a stat - This will NOT trigger neither a persist nor a serialization event
		*	The stats are added to the back of the vector
		*	/param int32_t value is the value we want to set the stat to
		*/
		void InitStatBase(int32_t value);
		void InitStatBase(int32_t value, boost::unique_lock<boost::mutex>& lock);

		void InitStatBase(uint16_t stat_index, int32_t value);
		void InitStatBase(uint16_t stat_index, int32_t value, boost::unique_lock<boost::mutex>& lock);
		
		void SetStatBase(uint16_t stat_index, int32_t value);
		void SetStatBase(uint16_t stat_index, int32_t value, boost::unique_lock<boost::mutex>& lock);

		void AddStatBase(uint16_t stat_index, int32_t value);
		void AddStatBase(uint16_t stat_index, int32_t value, boost::unique_lock<boost::mutex>& lock);

		void DeductStatBase(uint16_t stat_index, int32_t value);
		void DeductStatBase(uint16_t stat_index, int32_t value, boost::unique_lock<boost::mutex>& lock);

		std::vector<int32_t> GetBaseStats();
		std::vector<int32_t> GetBaseStats(boost::unique_lock<boost::mutex>& lock);

		int32_t GetStatBase(uint16_t stat_index);
		int32_t GetStatBase(uint16_t stat_index, boost::unique_lock<boost::mutex>& lock);

		bool SerializeBaseStats(swganh::messages::BaseSwgMessage* message);
		bool SerializeBaseStats(swganh::messages::BaseSwgMessage* message, boost::unique_lock<boost::mutex>& lock);

		// Battle Fatigue
		void AddBattleFatigue(uint32_t battle_fatigue);
		void AddBattleFatigue(uint32_t battle_fatigue, boost::unique_lock<boost::mutex>& lock);

		void DeductBattleFatigue(uint32_t battle_fatigue);
		void DeductBattleFatigue(uint32_t battle_fatigue, boost::unique_lock<boost::mutex>& lock);

		void SetBattleFatigue(uint32_t battle_fatigue);
		void SetBattleFatigue(uint32_t battle_fatigue, boost::unique_lock<boost::mutex>& lock);

		void InitBattleFatigue(uint32_t battle_fatigue) {battle_fatigue_ = battle_fatigue;}

		uint32_t GetBattleFatigue();
		uint32_t GetBattleFatigue(boost::unique_lock<boost::mutex>& lock);



    protected:

		//HAM
		TargetStats		mTargetStats; //used for Imagedesign statmigration
		
		swganh::containers::NetworkVector<int32_t> stat_base_list_; //plain base hitpoints w/o mods
		swganh::containers::NetworkVector<int32_t> stat_wound_list_;
		swganh::containers::NetworkVector<int32_t> stat_current_list_;
		swganh::containers::NetworkVector<int32_t> stat_max_list_;
		swganh::containers::NetworkVector<int32_t> stat_encumberance_list_;

		swganh::containers::NetworkVector<std::shared_ptr<swganh::object::EquipmentItem>> equipment_list_;

		swganh::containers::NetworkVector<uint64, swganh::containers::DefaultSerializer<uint64>> defender_list_;
		
		//swganh::containers::NetworkSet<std::string> skills_;

		uint32_t			battle_fatigue_;

        BuffList			mBuffList;
        FactionList			mFactionList;
        SkillCommandMap		mSkillCommandMap;
        SkillCommandList	mSkillCommands;
        SkillList			mSkills;
        SkillModsList		mSkillMods;

        BString				mCurrentAnimation;
        BString				mCustomizationStr;
        BString				mFaction;
        std::string			first_name;
        std::string			last_name;

        BString				mSpecies;
        BString				mSpeciesGroup;

        
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
