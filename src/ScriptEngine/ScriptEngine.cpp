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

#include <algorithm>




//======================================================================================================================

bool			ScriptEngine::mInsFlag    = false;
ScriptEngine*	ScriptEngine::mSingleton  = NULL;

//======================================================================================================================

ScriptEngine::ScriptEngine() :
    mNextHandlerId(1),
    mScriptPool(sizeof(Script))
{
    mMasterState = luaL_newstate();

    // load basic libs
    luaL_openlibs(mMasterState);

    // script engine functions
    LuaOpenScriptEngineLib(mMasterState);

    // The game bindings (tolua_LuaInterface_open) are opened by the zone
    // server, which they belong to; this keeps the engine itself testable.

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


    deliverEvents();

    boost::mutex::scoped_lock lk(mScriptMutex);

    ScriptList::iterator it = mScripts.begin();

    while(it != mScripts.end())
    {
        (*it)->process(elTime);

        ++it;
    }
}

//======================================================================================================================

void ScriptEngine::raiseEvent(const ScriptEvent& event)
{
    boost::mutex::scoped_lock lk(mEventMutex);
    mPendingEvents.push_back(event);
}

//======================================================================================================================

void ScriptEngine::deliverEvents()
{
    std::vector<ScriptEvent> events;
    {
        boost::mutex::scoped_lock lk(mEventMutex);
        events.swap(mPendingEvents);
    }

    // Events raised while these are delivered wait for the next tick.
    for(const ScriptEvent& event : events)
    {
        // Resume outside the lock: a resumed script may create or remove
        // scripts. A script removed in the meantime is skipped.
        std::vector<Script*> waiting;
        {
            boost::mutex::scoped_lock lk(mScriptMutex);
            for(Script* script : mScripts)
            {
                if(script->isWaitingFor(event))
                {
                    waiting.push_back(script);
                }
            }
        }
        for(Script* script : waiting)
        {
            bool stillThere;
            {
                boost::mutex::scoped_lock lk(mScriptMutex);
                stillThere = std::find(mScripts.begin(), mScripts.end(), script) != mScripts.end();
            }
            if(stillThere && script->isWaitingFor(event))
            {
                script->resumeWithEvent(&event);
            }
        }

        // A copy: a handler may add or remove handlers. One removed by an
        // earlier handler is skipped (its function reference is gone).
        std::vector<EventHandler> handlers = mEventHandlers;
        for(const EventHandler& handler : handlers)
        {
            if(handler.eventName == event.name && (handler.subject == 0 || handler.subject == event.subject)
                    && _hasEventHandler(handler.id))
            {
                _callHandler(handler, event);
            }
        }
    }
}

//======================================================================================================================

bool ScriptEngine::_hasEventHandler(uint32 id) const
{
    for(const EventHandler& handler : mEventHandlers)
    {
        if(handler.id == id)
        {
            return true;
        }
    }
    return false;
}

//======================================================================================================================

uint32 ScriptEngine::addEventHandler(const std::string& eventName, uint64 subject, int handlerRef)
{
    EventHandler handler = { mNextHandlerId++, eventName, subject, handlerRef };
    mEventHandlers.push_back(handler);
    return handler.id;
}

//======================================================================================================================

void ScriptEngine::removeEventHandler(uint32 id)
{
    for(auto it = mEventHandlers.begin(); it != mEventHandlers.end(); ++it)
    {
        if(it->id == id)
        {
            luaL_unref(mMasterState, LUA_REGISTRYINDEX, it->ref);
            mEventHandlers.erase(it);
            return;
        }
    }
}

//======================================================================================================================

// Handlers run to completion on the master state; they cannot wait (a wait
// in a handler is an error, reported like any other). To wait, use a script
// that loops on WaitEvent.
void ScriptEngine::_callHandler(const EventHandler& handler, const ScriptEvent& event)
{
    lua_rawgeti(mMasterState, LUA_REGISTRYINDEX, handler.ref);
    pushEvent(mMasterState, event);
    if(lua_pcall(mMasterState, 1, 0, 0) != 0)
    {
        const char* msg = lua_tostring(mMasterState, -1);
        LOG(ERR) << "Script event handler for " << event.name << " failed: " << (msg ? msg : "no message");
        lua_pop(mMasterState, 1);
    }
}

//======================================================================================================================

void ScriptEngine::pushEvent(lua_State* l, const ScriptEvent& event)
{
    lua_newtable(l);

    for(const auto& field : event.numbers)
    {
        lua_pushnumber(l, field.second);
        lua_setfield(l, -2, field.first.c_str());
    }
    for(const auto& field : event.strings)
    {
        lua_pushstring(l, field.second.c_str());
        lua_setfield(l, -2, field.first.c_str());
    }

    // Last, so that no field can replace them.
    lua_pushstring(l, event.name.c_str());
    lua_setfield(l, -2, "name");
    // Lua 5.1 numbers are doubles: ids up to 2^53 are exact.
    lua_pushnumber(l, static_cast<lua_Number>(event.subject));
    lua_setfield(l, -2, "subject");
    lua_pushnumber(l, static_cast<lua_Number>(event.source));
    lua_setfield(l, -2, "source");
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


