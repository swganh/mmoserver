/*
 This file is part of MMOServer. For more information, visit http://swganh.com
 
 Copyright (c) 2006 - 2011 The SWG:ANH Team

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef ANH_ZONESERVER_OBJECTCONTROLLERCOMMANDMAP_H
#define ANH_ZONESERVER_OBJECTCONTROLLERCOMMANDMAP_H

#include <cstdint>
#include <functional>
#include <map>

#ifdef _MSC_VER
#include <functional>  // NOLINT
#else
#include <tr1/functional>  // NOLINT
#endif

#include "Utils/typedefs.h"
#include "ScriptEngine/ScriptEventListener.h"
#include "DatabaseManager/DatabaseCallback.h"


//======================================================================================================================

class ObjectController;
class ObjectControllerCmdProperties;
class Object;
class Message;
class Database;
class DatabaseCallback;
class DatabaseResult;
class Script;

#define gObjectControllerCommands		ObjectControllerCommandMap::getSingletonPtr()
#define gObjControllerCmdMap			((ObjectControllerCommandMap::getSingletonPtr())->mCommandMap)
#define gObjControllerCmdPropertyMap	((ObjectControllerCommandMap::getSingletonPtr())->mCmdPropertyMap)

// Rename the old style handler and map.
typedef std::function<void (ObjectController*, uint64, Message*, ObjectControllerCmdProperties*)> OriginalObjectControllerHandler;
typedef std::map<uint32,OriginalObjectControllerHandler> OriginalCommandMap;

// New style ObjectController handlers accept an Object* as the first arguement.
typedef std::function<bool (Object* object, Object* target, Message*, ObjectControllerCmdProperties*)> ObjectControllerHandler;
typedef std::map<uint32,ObjectControllerHandler> CommandMap;

typedef std::map<uint32_t,ObjectControllerCmdProperties*>	CmdPropertyMap;

//======================================================================================================================

class ObjectControllerCommandMap : public DatabaseCallback
{
public:

    static ObjectControllerCommandMap*  getSingletonPtr() {
        return mSingleton;
    }
    static ObjectControllerCommandMap*	Init(Database* database);

    void								handleDatabaseJobComplete(void* ref,DatabaseResult* result);

    void								ScriptRegisterEvent(void* script,std::string eventFunction);

    const CommandMap& getCommandMap();

    ~ObjectControllerCommandMap();

    OriginalCommandMap				mCommandMap;
    CmdPropertyMap			mCmdPropertyMap;
    ScriptEventListener		mCmdScriptListener;

private:

    ObjectControllerCommandMap(Database* database);

    void								_registerCppHooks();

    // This is here for utility purposes during the transition and is used to load
    // up the new command map.
    void RegisterCppHooks_();

    static bool							mInsFlag;
    static ObjectControllerCommandMap*	mSingleton;
    CommandMap  command_map_;
    Database*							mDatabase;
};

//======================================================================================================================

class ObjectControllerCmdProperties
{
public:

    ObjectControllerCmdProperties()
        :mCmdCrc(0),mAbilityCrc(0),mStates(0),mCmdGroup(0) {}

    ObjectControllerCmdProperties(uint32 cmdCrc,uint32 abilityCrc,uint64 states,uint8 cmdGroup)
        : mCmdCrc(cmdCrc),mAbilityCrc(abilityCrc),mStates(states),mCmdGroup(cmdGroup) {}

    ~ObjectControllerCmdProperties() {}

    Script*	mScript;

    // generic
    uint32	mCmdCrc;
    uint32	mAbilityCrc;
    uint64	mStates;
    uint8	mCmdGroup;
    BString	mScriptHook;
    BString	mFailScriptHook;
    BString	mCommandStr;
    BString	mAbilityStr;
    uint64	mDefaultTime;
    float	mMaxRangeToTarget;
    uint8	mAddToCombatQueue;
    int32	mHealthCost;
    int32	mActionCost;
    int32	mMindCost;
    uint64	mPostureMask;
    uint64	mLocomotionMask;

    // combat
    uint32	mAnimationCrc;
    uint32	mRequiredWeaponGroup;
    BString	mCbtSpam;
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


