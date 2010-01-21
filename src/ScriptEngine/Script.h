/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_SCRIPTENGINE_SCRIPT_H
#define ANH_SCRIPTENGINE_SCRIPT_H

//======================================================================================================================

#include "Utils/typedefs.h"

#include <cstring>

//======================================================================================================================

enum ScriptState
{
	SS_Wait_Frame,
	SS_Wait_Time,
	SS_Running,
	SS_Not_Loaded,
	SS_Done
};

class ScriptEngine;
class Tutorial;

typedef struct lua_State lua_State;

//======================================================================================================================

class Script
{
	public:

		friend class ScriptEngine;
		friend class WorldManager;


		explicit Script(ScriptEngine* scriptEngine);
		~Script();

		void			createThread();
		void			process(uint32 elTime);
		void			run();
		void			runFile(const int8* fileName);
		uint32			runString(const int8* cmdString);
		void			callFunction(const char *func,const char *sig,...);
		void			abortWait();

		uint32			getPriority(){ return mPriority; }
		void			setPriority(uint32 priority){ mPriority = priority; }

		void			setFileName(const int8* fileName){ strcpy(mFile,fileName); }
		const int8*		getFileName(){ return mFile; }

		// refs to other classes, dirty as hell but right now I'm learning LUA, not lua-interfaces... (Eru)
		void			setTutorial(Tutorial* tutorial) { mTutorial = tutorial; }
		Tutorial*		getTutorial() {return mTutorial;}


		ScriptEngine*	mEngine;
		ScriptState		mState;
		uint32			mWaitTimeStamp;
		uint32			mTime;
		int32			mWaitFrame;

	private:

		void			_resumeScript(uint32 param);
		void			_formatError();

		// Since Dante is going to re-write the script engine, I feelt free to misuse it already when learning LUA (Eruptor).
		// refs to other classes.
		Tutorial*			mTutorial;

		lua_State*		mThreadState;
		uint32			mPriority;
		int8			mFile[256];
		int8			mLastError[256];
};

//======================================================================================================================

#endif


