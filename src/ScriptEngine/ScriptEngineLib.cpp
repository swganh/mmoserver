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
#include    <stdio.h>
#include    <string.h>

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}




//======================================================================================================================

static Script*	getScriptObject		(lua_State* l);
static int		luaWaitFrame		(lua_State* l);
static int		luaWaitTime			(lua_State* l);
static int		luaWaitMSec			(lua_State* l);
static int		luagetScriptObject  (lua_State* l);
static int		luaSplitString		(lua_State* l);

//======================================================================================================================

static const luaL_reg scriptLib[] =
{
    {"WaitFrame",		luaWaitFrame},
    {"WaitTime",		luaWaitTime},
    {"WaitMSec",		luaWaitMSec},
    {"getScriptObj",	luagetScriptObject},
    {"splitString",		luaSplitString},
    {NULL, NULL}
};

//======================================================================================================================

void LuaOpenScriptEngineLib(lua_State* l)
{
    luaL_register(l,"LuaScriptEngine",scriptLib);
}

//======================================================================================================================

static int luaWaitFrame(lua_State* l)
{
    Script* script = getScriptObject(l);

    script->mWaitFrame = (int32)luaL_checknumber(l,1);
    script->mState     = SS_Wait_Frame;

    return(lua_yield(l,1));
}

//======================================================================================================================

static int luaWaitTime(lua_State* l)
{
    Script* script = getScriptObject(l);

    script->mWaitTimeStamp = (uint32)luaL_checknumber(l,1);
    script->mState         = SS_Wait_Time;

    return(lua_yield(l,1));
}

//======================================================================================================================

static int luaWaitMSec(lua_State* l)
{
    Script* script = getScriptObject(l);

    script->mWaitTimeStamp = script->mTime + (uint32)luaL_checknumber(l,1);
    script->mState         = SS_Wait_Time;

    return(lua_yield(l,1));
}

//======================================================================================================================

static Script* getScriptObject(lua_State* l)
{
    lua_pushlightuserdata(l,l);
    lua_gettable(l,LUA_GLOBALSINDEX);

    return((Script*)lua_touserdata(l,-1));
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

