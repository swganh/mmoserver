/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/


#include "Script.h"
#include "ScriptEngine.h"
#include "LogManager/LogManager.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

extern "C"
{
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
}


//======================================================================================================================

Script::Script(ScriptEngine* scriptEngine) : 
mEngine(scriptEngine),
mState(SS_Not_Loaded),
mTime(0),
mWaitTimeStamp(0),
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
	assert(mEngine->getMasterState());
	assert(mThreadState);

	if(luaL_loadfile(mThreadState,mFile) == 0)
	{
		_resumeScript(0);
	}
	else 
	{
		_formatError();
		gLogger->logMsgF("ScriptingEngine::Syntax Error: %s\n",MSG_NORMAL,mLastError);
	}
}

//======================================================================================================================

void Script::runFile(const int8 *fileName)
{
	assert(mEngine->getMasterState());
	assert(mThreadState);

	if(luaL_loadfile(mThreadState,fileName) == 0)
	{
		_resumeScript(0);
	}
	else 
	{
		_formatError();
		gLogger->logMsgF("ScriptingEngine::Syntax Error: %s\n",MSG_NORMAL,mLastError);
	}
}

//======================================================================================================================

uint32 Script::runString(const int8 *cmdString)
{
	assert(mEngine->getMasterState());
	assert(mThreadState);

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
		case 0:	mState = SS_Not_Loaded;	break;
		case LUA_YIELD:	break;

		default:
		{
			_formatError();
			gLogger->logMsgF("ScriptingEngine::_resumeScript Runtime Error: %s\n",MSG_NORMAL,mLastError);
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

				case 'd':	lua_pushnumber(mThreadState,va_arg(vl,double));	break;
				case 'i':	lua_pushnumber(mThreadState,va_arg(vl,int));	break;
				case 's':  	lua_pushstring(mThreadState,va_arg(vl,char *));	break;

				case '>':	goto endwhile;	break;

				default:	break;
			}

			narg++;

			luaL_checkstack(mThreadState,1,"too many arguments");
		} 

		endwhile:

		nres = strlen(sig); 

		if(lua_pcall(mThreadState,narg,nres,0) != 0)
		{
			_formatError();
			gLogger->logMsgF("ScriptingEngine::callFunction Runtime Error: %s\n",MSG_NORMAL,mLastError);
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
						gLogger->logMsgF("ScriptingEngine::callFunction wrong result type: %s\n",MSG_NORMAL,mLastError);
					}

					*va_arg(vl,double *) = lua_tonumber(mThreadState,nres);
				}
				break;

				case 'i': 
				{
					if(!lua_isnumber(mThreadState,nres))
					{
						_formatError();
						gLogger->logMsgF("ScriptingEngine::callFunction wrong result type: %s\n",MSG_NORMAL,mLastError);
					}

					*va_arg(vl,int*) = (int)lua_tonumber(mThreadState,nres);
				}
				break;

				case 's': 
				{
					if(!lua_isstring(mThreadState,nres))
					{
						_formatError();
						gLogger->logMsgF("ScriptingEngine::callFunction wrong result type: %s\n",MSG_NORMAL,mLastError);
					}

					*va_arg(vl,const char **) = lua_tostring(mThreadState,nres);
				}
				break;

				default:
				{
					_formatError();
					gLogger->logMsgF("ScriptingEngine::callFunction invalid option: %s\n",MSG_NORMAL,mLastError);
				}
				break;
			}

			nres++;
		}
	}

	va_end(vl);
}

//======================================================================================================================

