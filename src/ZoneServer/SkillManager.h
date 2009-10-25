/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_SKILLMANAGER_H
#define ANH_ZONESERVER_SKILLMANAGER_H

#define	 gSkillManager	SkillManager::getSingletonPtr()

#include "Utils/typedefs.h"
#include "SkillEnums.h"
#include "Skill.h"
#include "DatabaseManager/DatabaseCallback.h"
#include <boost/pool/pool.hpp>


//======================================================================================================================

class Database;
class DatabaseCallback;
class DatabaseResult;
class CreatureObject;
class PlayerObject;
class SMAsyncContainer;

typedef std::vector<Skill*>					SkillList;
typedef std::vector<string>					ModList,CommandList,XpTypeList;
typedef std::vector<int32>					XpCapList;
typedef std::vector<std::pair<uint32,string> >	SkillInfoList;


//======================================================================================================================

class SMAsyncContainer
{
	public:

		SMAsyncContainer(SMQueryType qt){ mQueryType = qt;}
		~SMAsyncContainer(){}

		SMQueryType	mQueryType;
};

//======================================================================================================================

class SMQueryContainer
{
	public:

		SMQueryContainer(){}
		~SMQueryContainer(){}

		uint32	mInt;
		uint32	mInt2;
		int32	mInt3;
		char	mName[64];
		char	mName2[64];
};

//======================================================================================================================

class SkillManager : public DatabaseCallback
{
	public:

		~SkillManager();
		static SkillManager*	Init(Database* database);
		static SkillManager*	getSingletonPtr() { return mSingleton; }

		void					handleDatabaseJobComplete(void* ref,DatabaseResult* result);

		SkillList*				getSkillList(){ return &mSkillList; }
		SkillList*				getMasterProfessionList(){ return &mMasterProfessionList; }
		ModList*				getSkillModList(){ return &mSkillModList; }
		CommandList*			getSkillCommandList(){ return &mSkillCommandList; }
		XpTypeList*				getXPTypeList(){ return &mXpTypeList; }

		Skill*					getSkillById(uint32 skillId){ return mSkillList[skillId-1]; }
		Skill*					getSkillByName(string skillName);
		string					getSkillInfoById(uint32 skillId);
		string					getSkillModById(uint32 skillModId){ return mSkillModList[skillModId-1]; }
		string					getSkillCmdById(uint32 skillCmdId){ return mSkillCommandList[skillCmdId-1]; }
		string					getXPTypeById(uint32 xpId){ return mXpTypeList[xpId-1]; }
		string					getXPTypeExById(uint32 xpId){ return mXpTypeListEx[xpId-1]; }
		int32					getDefaultXPCapById(uint32 xpId){ return mDefaultXpCapList[xpId-1]; }

		bool					learnSkill(uint32 skillId,CreatureObject* creatureObject,bool subXp);
		void					dropSkill(uint32 skillId,CreatureObject* creatureObject);
		bool					learnSkillLine(uint32 skillId, CreatureObject* creatureObject, bool subXP);

		void					initExperience(PlayerObject* playerObject);
		void					addExperience(uint32 xpType,int32 valueDiff,PlayerObject* playerObject);
		// void					removeExperience(uint32 xpType,int32 valueDiff,PlayerObject* playerObject);

		bool					checkRaceLearnSkill(uint32 skillId,CreatureObject* creatureObject);
		bool					checkTeachSkill(uint32 skillId,PlayerObject* pupilObject);
		void					teach(PlayerObject* pupilObject,PlayerObject* teacherObject,string show);
		bool					checkLearnSkill(uint32 skillId,PlayerObject* pupilObject);
		string					getSkillProfession(uint32 skillId,string leaveAsIs);

		void					releaseAllPoolsMemory(){ mDBAsyncPool.release_memory(); }

	private:
		int32					getMaxXpCap(uint8 xpType);
		int32					getXpCap(PlayerObject* playerObject, uint8 xpType);

		int32					handleExperienceCap(uint32 xpType,int32 valueDiff, PlayerObject* playerObject);

		SkillManager(Database* database);

		static bool			mInsFlag;
		static SkillManager*	mSingleton;

		boost::pool<boost::default_user_allocator_malloc_free>			mDBAsyncPool;
		XpCapList				mDefaultXpCapList;
		SkillList				mMasterProfessionList;
		CommandList				mSkillCommandList;
		SkillInfoList			mSkillInfoList;
		SkillList				mSkillList;
		ModList					mSkillModList;
		XpTypeList				mXpTypeList;
		XpTypeList				mXpTypeListEx;		// Reader friendly name of the xperience.
		Database*				mDatabase;
		uint32					mLoadCounter;
		uint32					mTotalLoadCount;
};

#endif

