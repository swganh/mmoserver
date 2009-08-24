/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "ScriptEngine.h"
#include "ScriptEngineLib.h"
#include "Script.h"
#include "Utils/clock.h"
#include "LogManager/LogManager.h"
#include "glue_files/tolua++.h"
#include "glue_files/LuaInterface.h"


//======================================================================================================================

bool			ScriptEngine::mInsFlag    = false;
ScriptEngine*	ScriptEngine::mSingleton  = NULL;

//======================================================================================================================

ScriptEngine::ScriptEngine() :
mScriptPool(sizeof(Script))
{
	mMasterState = luaL_newstate();

	// load basic libs
	luaL_openlibs(mMasterState);

	// script engine functions
	LuaOpenScriptEngineLib(mMasterState);

	// our custom libs
	tolua_LuaInterface_open(mMasterState);

	// We do have a global clock object, don't use seperate clock and times for every process.
	// mClock = new Anh_Utils::Clock();

	mLastProcessTime = Anh_Utils::Clock::getSingleton()->getLocalTime();
}

//======================================================================================================================

ScriptEngine*	ScriptEngine::Init()
{
	if(!mInsFlag)
	{
		mSingleton = new ScriptEngine();
		mInsFlag = true;
		return mSingleton;
	}
	else
		return mSingleton;
}

//======================================================================================================================

ScriptEngine::~ScriptEngine()
{
	mInsFlag = false;
	// delete(mSingleton);
	mSingleton = NULL;
}

//======================================================================================================================

void ScriptEngine::shutdown()
{
	mScriptMutex.acquire();

	ScriptList::iterator it = mScripts.begin();

	while(it != mScripts.end())
	{
		mScriptPool.free(*it);
		it = mScripts.erase(it);
	}

	mScriptMutex.release();

	if(mMasterState)
	{
		lua_close(mMasterState);
		mMasterState = NULL;
	}

	// delete(mClock);
}

//======================================================================================================================

void ScriptEngine::process()
{
	uint64	currentTime	= Anh_Utils::Clock::getSingleton()->getLocalTime();
	uint32	elTime		= (uint32)(currentTime - mLastProcessTime);
	mLastProcessTime	= currentTime;


	mScriptMutex.acquire();

	ScriptList::iterator it = mScripts.begin();

	while(it != mScripts.end())
	{
		(*it)->process(elTime);

		++it;
	}

	mScriptMutex.release();
}

//======================================================================================================================

Script* ScriptEngine::createScript()
{
	Script* script = new(mScriptPool.ordered_malloc()) Script(this);


	mScriptMutex.acquire();

	mScripts.push_back(script);

	mScriptMutex.release();


	return(script);
}

//======================================================================================================================

void ScriptEngine::removeScript(Script* script)
{
	mScriptMutex.acquire();

	ScriptList::iterator it = mScripts.begin();

	while(it != mScripts.end())
	{
		if((*it) == script)
		{
			gLogger->logMsg("ScriptEngine::removeScript found a script\n");
			(*it)->mState = SS_Not_Loaded;
			mScriptPool.free(*it);
			mScripts.erase(it);
			break;
		}
		++it;
	}

	mScriptMutex.release();
}

//======================================================================================================================

// Access member data from script class.

//======================================================================================================================

Tutorial* ScriptEngine::getTutorial(void* script)
{
	Tutorial* tutorial = NULL;

	mScriptMutex.acquire();

	ScriptList::iterator it = mScripts.begin();
	while(it != mScripts.end())
	{
		if((*it) == reinterpret_cast<Script*>(script))
		{
			tutorial = (*it)->getTutorial();
			break;
		}
		++it;
	}

	mScriptMutex.release();
	return tutorial;
}

//======================================================================================================================


