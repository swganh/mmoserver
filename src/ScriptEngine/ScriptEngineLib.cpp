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

#include    "ScriptEngineLib.h"
#include    "Script.h"
#include    "ScriptEngine.h"
#include    "ScriptEvent.h"
#include    <stdio.h>
#include    <string.h>

#include    <string>

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#ifdef ERROR
#undef ERROR
#endif

#include "Utils/logger.h"




//======================================================================================================================

static Script*	getScriptObject		(lua_State* l);
static int		luaWaitFrame		(lua_State* l);
static int		luaWaitTime			(lua_State* l);
static int		luaWaitMSec			(lua_State* l);
static int		luagetScriptObject  (lua_State* l);
static int		luaSplitString		(lua_State* l);
static int		luaWaitEvent		(lua_State* l);
static int		luaOnEvent			(lua_State* l);
static int		luaRemoveEventHandler(lua_State* l);
static Script*	checkScriptObject	(lua_State* l, const char* function);
static int		luaPrint			(lua_State* l);

//======================================================================================================================

static const luaL_reg scriptLib[] =
{
    {"WaitFrame",		luaWaitFrame},
    {"WaitTime",		luaWaitTime},
    {"WaitMSec",		luaWaitMSec},
    {"getScriptObj",	luagetScriptObject},
    {"splitString",		luaSplitString},
    {"WaitEvent",		luaWaitEvent},
    {"OnEvent",			luaOnEvent},
    {"RemoveEventHandler",	luaRemoveEventHandler},
    {NULL, NULL}
};

//======================================================================================================================

void LuaOpenScriptEngineLib(lua_State* l)
{
    luaL_register(l,"LuaScriptEngine",scriptLib);
    lua_pop(l,1);

    // Scripts' print() goes to the server's log: the standard print writes to
    // stdout, which is buffered into the log file and lost when the server stops.
    lua_register(l,"print",luaPrint);
}

//======================================================================================================================

static int luaPrint(lua_State* l)
{
    std::string line;
    int n = lua_gettop(l);

    lua_getglobal(l,"tostring");
    for(int i = 1; i <= n; ++i)
    {
        lua_pushvalue(l,-1);
        lua_pushvalue(l,i);
        lua_call(l,1,1);
        const char* s = lua_tostring(l,-1);
        if(i > 1)
        {
            line += "\t";
        }
        line += s ? s : "?";
        lua_pop(l,1);
    }
    lua_pop(l,1);

    LOG(INFO) << "[script] " << line;

    return(0);
}

//======================================================================================================================

static int luaWaitFrame(lua_State* l)
{
    Script* script = checkScriptObject(l, "WaitFrame");

    script->mWaitFrame = (int32)luaL_checknumber(l,1);
    script->mState     = SS_Wait_Frame;

    return(lua_yield(l,1));
}

//======================================================================================================================

static int luaWaitTime(lua_State* l)
{
    Script* script = checkScriptObject(l, "WaitTime");

    script->mWaitTimeStamp = (uint32)luaL_checknumber(l,1);
    script->mState         = SS_Wait_Time;

    return(lua_yield(l,1));
}

//======================================================================================================================

static int luaWaitMSec(lua_State* l)
{
    Script* script = checkScriptObject(l, "WaitMSec");

    script->mWaitTimeStamp = script->mTime + (uint32)luaL_checknumber(l,1);
    script->mState         = SS_Wait_Time;

    return(lua_yield(l,1));
}

//======================================================================================================================

static Script* getScriptObject(lua_State* l)
{
    lua_pushlightuserdata(l,l);
    lua_gettable(l,LUA_GLOBALSINDEX);

    Script* script = (Script*)lua_touserdata(l,-1);
    lua_pop(l,1);

    return(script);
}

//======================================================================================================================

// The Wait functions pause a script, so they only work in one: not in an
// OnEvent handler or other code running on the engine's main state.
static Script* checkScriptObject(lua_State* l, const char* function)
{
    Script* script = getScriptObject(l);

    if(!script)
    {
        luaL_error(l,"LuaScriptEngine.%s can only be called from a script, not from an event handler",function);
    }

    return(script);
}

//======================================================================================================================

// LuaScriptEngine.WaitEvent(name [, subject [, timeoutMs]])
// Pauses the script until the event is raised (for that subject, unless the
// subject is 0 or left out) and returns it as a table, or nil after timeoutMs.
static int luaWaitEvent(lua_State* l)
{
    Script* script = checkScriptObject(l, "WaitEvent");

    const char* name    = luaL_checkstring(l,1);
    uint64      subject = (uint64)luaL_optnumber(l,2,0);
    uint32      timeout = (uint32)luaL_optnumber(l,3,0);

    script->waitForEvent(name, subject, timeout);

    return(lua_yield(l,0));
}

//======================================================================================================================

// LuaScriptEngine.OnEvent(name, function(event) ... end [, subject])
// Calls the function every time the event is raised; returns an id for
// RemoveEventHandler. The function runs to completion and cannot wait.
static int luaOnEvent(lua_State* l)
{
    const char* name    = luaL_checkstring(l,1);
    luaL_checktype(l,2,LUA_TFUNCTION);
    uint64      subject = (uint64)luaL_optnumber(l,3,0);

    lua_pushvalue(l,2);
    int ref = luaL_ref(l,LUA_REGISTRYINDEX);

    lua_pushnumber(l,gScriptEngine->addEventHandler(name, subject, ref));

    return(1);
}

//======================================================================================================================

static int luaRemoveEventHandler(lua_State* l)
{
    gScriptEngine->removeEventHandler((uint32)luaL_checknumber(l,1));

    return(0);
}

//======================================================================================================================

static int luagetScriptObject(lua_State* l)
{
    Script* script = getScriptObject(l);

    lua_pushlightuserdata(l,script);

    return(1);
}

//======================================================================================================================

static int luaSplitString(lua_State* l)
{
    const char *s		= luaL_checkstring(l,1);
    const char *sep	= luaL_checkstring(l,2);
    const char *e;
    int i = 1;

    lua_newtable(l);

    while((e = strchr(s,*sep)) != NULL)
    {
        lua_pushlstring(l,s,e-s);
        lua_rawseti(l,-2,i++);
        s = e + 1;
    }

    lua_pushstring(l,s);
    lua_rawseti(l,-2,i);

    return 1;
}

//======================================================================================================================

