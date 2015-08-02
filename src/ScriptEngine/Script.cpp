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


#include "Script.h"
#include "ScriptEngine.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

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

Script::Script(ScriptEngine* scriptEngine) :
    mEngine(scriptEngine),
    mState(SS_Not_Loaded),
    mWaitTimeStamp(0),
    mTime(0),
    mWaitFrame(0)
{
    mThreadState = lua_newthread(mEngine->getMasterState());

    lua_pushlightuserdata(mEngine->getMasterState(),mThreadState);
    lua_pushlightuserdata(mEngine->getMasterState(),this);
    lua_settable(mEngine->getMasterState(),LUA_GLOBALSINDEX);

    strcpy(mLastError,"None");
}

//======================================================================================================================

Script::~Script()
{

}

//======================================================================================================================

void Script::run()
{
    assert(mEngine->getMasterState() && "Invalid engine master state");
    assert(mThreadState && "Invalid thread state");

    if(luaL_loadfile(mThreadState,mFile) == 0)
    {
        _resumeScript(0);
    }
    else
    {
        _formatError();
    }
}

//======================================================================================================================

void Script::runFile(const int8 *fileName)
{
    assert(mEngine->getMasterState() && "Invalid engine master state");
    assert(mThreadState && "Invalid thread state");

    if(luaL_loadfile(mThreadState,fileName) == 0)
    {
        _resumeScript(0);
    }
    else
    {
        _formatError();
    }
}

//======================================================================================================================

uint32 Script::runString(const int8 *cmdString)
{
    assert(mEngine->getMasterState() && "Invalid engine master state");
    assert(mThreadState && "Invalid thread state");

    if(luaL_loadbuffer(mThreadState,cmdString,strlen(cmdString),"Console") == 0)
    {
        if(lua_pcall(mThreadState,lua_gettop(mThreadState) - 1,0,0) != 0)
        {
            _formatError();
            return(1);
        }
    }
    else
    {
        _formatError();
        return(1);
    }

    return(0);
}

//======================================================================================================================

void Script::process(uint32 elTime)
{
    mTime += elTime;

    switch(mState)
    {
    case SS_Wait_Time:
    {
        if(mTime >= mWaitTimeStamp)
            _resumeScript(0);
    }
    break;

    case SS_Wait_Frame:
    {
        mWaitFrame--;

        if(mWaitFrame <= 0)
            _resumeScript(0);
    }
    break;

    case SS_Not_Loaded:
    default:
        break;
    }
}

//======================================================================================================================

void Script::abortWait()
{
    _resumeScript(1);
}

//======================================================================================================================

void Script::_resumeScript(uint32 param)
{
    mState = SS_Running;

    lua_pushnumber(mThreadState,param);

    int ret = lua_resume(mThreadState,1);

    switch(ret)
    {
    case 0:
        mState = SS_Not_Loaded;
        break;
    case LUA_YIELD:
        break;

    default:
    {
        _formatError();
    }
    break;
    }
}

//======================================================================================================================

void Script::_formatError()
{
    const int8* msg = lua_tostring(mThreadState,-1);

    if(!msg)
        msg = "No message";

    lua_pop(mThreadState,1);

    strcpy(mLastError,msg);
	LOG(ERR) << "ScriptingEngine::callFunction wrong result type: " << mLastError;
}

//======================================================================================================================

void Script::callFunction(const char *func,const char *sig,...)
{
    va_list vl;
    int narg,nres;

    va_start(vl,sig);
    lua_getglobal(mThreadState,func);

    if(lua_isfunction(mThreadState,-1))
    {
        narg = 0;

        while(*sig)
        {
            switch (*sig++)
            {

            case 'd':
                lua_pushnumber(mThreadState,va_arg(vl,double));
                break;
            case 'i':
                lua_pushnumber(mThreadState,va_arg(vl,int));
                break;
            case 's':
                lua_pushstring(mThreadState,va_arg(vl,char *));
                break;

            case '>':
                goto endwhile;
                break;

            default:
                break;
            }

            narg++;

            luaL_checkstack(mThreadState,1,"too many arguments");
        }

endwhile:

        nres = strlen(sig);

        if(lua_pcall(mThreadState,narg,nres,0) != 0)
        {
            _formatError();
        }

        nres = -nres;

        while(*sig)
        {
            switch (*sig++)
            {
            case 'd':
            {
                if(!lua_isnumber(mThreadState,nres))
                {
                    _formatError();
                }

                *va_arg(vl,double *) = lua_tonumber(mThreadState,nres);
            }
            break;

            case 'i':
            {
                if(!lua_isnumber(mThreadState,nres))
                {
                    _formatError();
                }

                *va_arg(vl,int*) = (int)lua_tonumber(mThreadState,nres);
            }
            break;

            case 's':
            {
                if(!lua_isstring(mThreadState,nres))
                {
                    _formatError();
                }

                *va_arg(vl,const char **) = lua_tostring(mThreadState,nres);
            }
            break;

            default:
            {
                _formatError();
            }
            break;
            }

            nres++;
        }
    }

    va_end(vl);
}

//======================================================================================================================

