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

#ifndef ANH_SCRIPTENGINE_SCRIPT_H
#define ANH_SCRIPTENGINE_SCRIPT_H

//======================================================================================================================

#include "Utils/typedefs.h"

#include <cstring>
#include <string>

//======================================================================================================================

enum ScriptState
{
    SS_Wait_Frame,
    SS_Wait_Time,
    SS_Wait_Event,
    SS_Running,
    SS_Not_Loaded,
    SS_Done
};

class ScriptEngine;
class Tutorial;
struct ScriptEvent;

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

    // LuaScriptEngine.WaitEvent: wait for an event called eventName about
    // subject (0: any), for at most timeoutMs (0: no limit).
    void			waitForEvent(const std::string& eventName, uint64 subject, uint32 timeoutMs);
    bool			isWaitingFor(const ScriptEvent& event) const;
    // Resumes a script waiting for an event: WaitEvent returns the event as a
    // table, or nil for a timeout (event == NULL).
    void			resumeWithEvent(const ScriptEvent* event);

    uint32			getPriority() {
        return mPriority;
    }
    void			setPriority(uint32 priority) {
        mPriority = priority;
    }

    void			setFileName(const int8* fileName) {
        strcpy(mFile,fileName);
    }
    const int8*		getFileName() {
        return mFile;
    }
    const int8*		getLastError() const {
        return mLastError;
    }

    // refs to other classes, dirty as hell but right now I'm learning LUA, not lua-interfaces... (Eru)
    void			setTutorial(Tutorial* tutorial) {
        mTutorial = tutorial;
    }
    Tutorial*		getTutorial() {
        return mTutorial;
    }


    ScriptEngine*	mEngine;
    ScriptState		mState;
    uint32			mWaitTimeStamp;
    uint32			mTime;
    int32			mWaitFrame;
    std::string		mWaitEventName;
    uint64			mWaitEventSubject;
    bool			mWaitEventHasTimeout;

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


