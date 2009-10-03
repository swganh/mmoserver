/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "ScriptEventListener.h"
#include "LogManager/LogManager.h"
#include "Script.h"

//======================================================================================================================

ScriptEventListener::ScriptEventListener()
{
}

//======================================================================================================================

ScriptEventListener::~ScriptEventListener()
{
	ScriptListMap::iterator it = mScriptListMap.begin();

	while(it != mScriptListMap.end())
	{
		delete((*it).second);
		mScriptListMap.erase(it);
		it = mScriptListMap.begin();
	}
}

//======================================================================================================================

void ScriptEventListener::_removeScriptList(const int8* functionName)
{
	ScriptListMap::iterator it = mScriptListMap.find(functionName);

	if(it != mScriptListMap.end())
		mScriptListMap.erase(it);
}

//======================================================================================================================

void ScriptEventListener::registerFunction(const int8* functionName)
{
	ScriptList* scriptList = getScriptList(functionName);

	if(scriptList)
	{
		gLogger->logMsgF("ScriptEventListener Error: function already registered %s",MSG_NORMAL,functionName);
		return;
	}

	gLogger->logMsgF("ScriptEventListener:registered function %s",MSG_LOW,functionName);

	mScriptListMap.insert(std::make_pair(functionName,new ScriptList));

}

//======================================================================================================================

void ScriptEventListener::unregisterFunction(const int8* functionName)
{
	ScriptList* scriptList = getScriptList(functionName);

	if(!scriptList)
	{
		gLogger->logMsgF("ScriptEventListener Error: unregisterFunction could not find %s",MSG_NORMAL,functionName);
		return;
	}

	delete(scriptList);

	_removeScriptList(functionName);
}

//======================================================================================================================

ScriptList* ScriptEventListener::getScriptList(const int8* functionName)
{
	ScriptListMap::iterator it = mScriptListMap.find(functionName);

	if(it != mScriptListMap.end())
		return((*it).second);

	return(NULL);
}

//======================================================================================================================

void ScriptEventListener::registerScript(Script* script,const int8* functionName)
{
	ScriptList* scriptList = getScriptList(functionName);

	if(!scriptList)
	{
		gLogger->logMsgF("ScriptEventListener Error: no function mapped for %s",MSG_NORMAL,functionName);
		return;
	}

	scriptList->push_back(script);
}

//======================================================================================================================

void ScriptEventListener::unregisterScript(Script* script)
{

}

//======================================================================================================================

void ScriptEventListener::handleScriptEvent(const int8* functionName,string params)
{
	// gLogger->logMsgF("ScriptEventListener Entering with function name %s and param = %s",MSG_NORMAL,functionName, params.getAnsi());
	ScriptList* scriptList = getScriptList(functionName);

	if(!scriptList)
	{
		gLogger->logMsgF("ScriptEventListener Error: no function mapped for %s",MSG_NORMAL,functionName);
		return;
	}

	ScriptList::iterator it = scriptList->begin();

	while(it != scriptList->end())
	{
		(*it)->callFunction(functionName,"s",params.getAnsi());

		++it;
	}
}

//======================================================================================================================

