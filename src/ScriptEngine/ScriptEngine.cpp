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

#include "ScriptEngine.h"

#include "Script.h"
#include "ScriptEngineLib.h"

#include "glue_files/tolua++.h"
#include "glue_files/LuaInterface.h"


#ifdef ERROR
#undef ERROR
#endif

#include "Utils/logger.h"

#include "Utils/clock.h"




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
    boost::mutex::scoped_lock lk(mScriptMutex);

    ScriptList::iterator it = mScripts.begin();

    while(it != mScripts.end())
    {
        mScriptPool.free(*it);
        it = mScripts.erase(it);
    }

    lk.unlock();

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


    boost::mutex::scoped_lock lk(mScriptMutex);

    ScriptList::iterator it = mScripts.begin();

    while(it != mScripts.end())
    {
        (*it)->process(elTime);

        ++it;
    }
}

//======================================================================================================================

Script* ScriptEngine::createScript()
{
    Script* script = new(mScriptPool.ordered_malloc()) Script(this);

    boost::mutex::scoped_lock lk(mScriptMutex);

    mScripts.push_back(script);

    return(script);
}

//======================================================================================================================

void ScriptEngine::removeScript(Script* script)
{
    boost::mutex::scoped_lock lk(mScriptMutex);

    ScriptList::iterator it = mScripts.begin();

    while(it != mScripts.end())
    {
        if((*it) == script)
        {
            DLOG(INFO) << "ScriptEngine::removeScript found a script";
            (*it)->mState = SS_Not_Loaded;
            mScriptPool.free(*it);
            mScripts.erase(it);
            break;
        }
        ++it;
    }
}

//======================================================================================================================

// Access member data from script class.

//======================================================================================================================

Tutorial* ScriptEngine::getTutorial(void* script)
{
    Tutorial* tutorial = NULL;

    boost::mutex::scoped_lock lk(mScriptMutex);

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

    return tutorial;
}

//======================================================================================================================


