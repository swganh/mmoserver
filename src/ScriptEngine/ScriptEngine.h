/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_SCRIPTENGINE_SCRIPTENGINE_H
#define ANH_SCRIPTENGINE_SCRIPTENGINE_H

//======================================================================================================================

extern "C" 
{
	#include "lua.h"
	#include "lauxlib.h"
	#include "lualib.h"
}

#include "Script.h"
#include "Utils/typedefs.h"

#include <boost/pool/pool.hpp>
#include <boost/thread/mutex.hpp>
#include <vector>

#define	 gScriptEngine	ScriptEngine::getSingletonPtr()

class Tutorial;

typedef std::vector<Script*>	ScriptList;

//======================================================================================================================

class ScriptEngine
{
	public:

		static ScriptEngine*	getSingletonPtr() { return mSingleton; }
		static ScriptEngine*	Init();

		lua_State*				getMasterState(){ return mMasterState; }

		Script*					createScript();
		void					removeScript(Script* script);

		void 					shutdown();
		void 					process();

		~ScriptEngine();
		Tutorial*				getTutorial(void* script);


	private:

		ScriptEngine();

		static ScriptEngine*	mSingleton;
		static bool				mInsFlag;

		lua_State*				mMasterState;
		// Anh_Utils::Clock*		mClock;
		uint64					mLastProcessTime;
        boost::mutex			mScriptMutex;

		ScriptList				mScripts;
		boost::pool<boost::default_user_allocator_malloc_free>	mScriptPool;
};

//======================================================================================================================

#endif 




