/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_OBJECTCONTROLLERCOMMANDMAP_H
#define ANH_ZONESERVER_OBJECTCONTROLLERCOMMANDMAP_H

#include "Utils/typedefs.h"
#include "ScriptEngine/ScriptEventListener.h"
#include "DatabaseManager/DatabaseCallback.h"
#include <map>


//======================================================================================================================

class ObjectController;
class ObjectControllerCmdProperties;
class Message;
class Database;
class DatabaseCallback;
class DatabaseResult;
class Script;

#define gObjectControllerCommands		ObjectControllerCommandMap::getSingletonPtr()
#define gObjControllerCmdMap			((ObjectControllerCommandMap::getSingletonPtr())->mCommandMap)
#define gObjControllerCmdPropertyMap	((ObjectControllerCommandMap::getSingletonPtr())->mCmdPropertyMap)

typedef void											(ObjectController::*funcPointer)(uint64,Message*,ObjectControllerCmdProperties*);
typedef std::map<uint32,funcPointer>					CommandMap;
typedef std::map<uint32,ObjectControllerCmdProperties*>	CmdPropertyMap;

//======================================================================================================================

class ObjectControllerCommandMap : public DatabaseCallback
{
	public:

		static ObjectControllerCommandMap*  getSingletonPtr() { return mSingleton; }
		static ObjectControllerCommandMap*	Init(Database* database);

		void								handleDatabaseJobComplete(void* ref,DatabaseResult* result);

		void								ScriptRegisterEvent(void* script,std::string eventFunction);

		~ObjectControllerCommandMap();

		CommandMap				mCommandMap;
		CmdPropertyMap			mCmdPropertyMap;
		ScriptEventListener		mCmdScriptListener;
		
	private:

		ObjectControllerCommandMap(Database* database);

		void								_registerCppHooks();

		static bool							mInsFlag;
		static ObjectControllerCommandMap*	mSingleton;
		Database*							mDatabase;
};

//======================================================================================================================

class ObjectControllerCmdProperties
{
	public:

		ObjectControllerCmdProperties()
			:mCmdCrc(0),mAbilityCrc(0),mStates(0),mCmdGroup(0){}

		ObjectControllerCmdProperties(uint32 cmdCrc,uint32 abilityCrc,uint64 states,uint8 cmdGroup) 
			: mCmdCrc(cmdCrc),mAbilityCrc(abilityCrc),mStates(states),mCmdGroup(cmdGroup){}

		~ObjectControllerCmdProperties(){}

		Script*	mScript;

		// generic
		uint32	mCmdCrc;
		uint32	mAbilityCrc;
		uint64	mStates;
		uint8	mCmdGroup;
		string	mScriptHook;
		string	mFailScriptHook;
		string	mCommandStr;
		string	mAbilityStr;
		uint64	mDefaultTime;
		float	mMaxRangeToTarget;
		uint8	mAddToCombatQueue;
		int32	mHealthCost;
		int32	mActionCost;
		int32	mMindCost;
		uint32	mPostureMask;

		// combat
		uint32	mAnimationCrc;
		uint32	mRequiredWeaponGroup;
		string	mCbtSpam;
		uint8	mTrail1;
		uint8	mTrail2;
		float	mHealthHitChance;
		float	mActionHitChance;
		float	mMindHitChance;
		float	mKnockdownChance;
		float	mDizzyChance;
		float	mBlindChance;
		float	mStunChance;
		float	mIntimidateChance;
		float	mPostureDownChance;
		float	mExtendedRange;
		float	mDamageMultiplier;
		float	mDelayMultiplier;
};

//======================================================================================================================

#endif


