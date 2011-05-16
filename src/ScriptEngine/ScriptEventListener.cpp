/*
 This file is part of MMOServer. For more information, visit http://swganh.com
 
 Copyright (c) 2006 - 2011 The SWG:ANH Team

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "ScriptEventListener.h"

#ifdef _WIN32
#undef ERROR
#endif
#include <glog/logging.h>

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
    	LOG(WARNING) << "Function already registered [" << functionName << "]";
        return;
    }

    //gLogger->log(LogManager::DEBUG, "Script Event Listener:registered function %s",functionName);

    mScriptListMap.insert(std::make_pair(functionName,new ScriptList));

}

//======================================================================================================================

void ScriptEventListener::unregisterFunction(const int8* functionName)
{
    ScriptList* scriptList = getScriptList(functionName);

    if(!scriptList)
    {
    	LOG(WARNING) << "Could not function to unregister [" << functionName << "]";
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
    	LOG(WARNING) << "Function not mapped [" << functionName << "]";
        return;
    }

    scriptList->push_back(script);
}

//======================================================================================================================

void ScriptEventListener::unregisterScript(Script* script)
{

}

//======================================================================================================================

void ScriptEventListener::handleScriptEvent(const int8* functionName,BString params)
{
    ScriptList* scriptList = getScriptList(functionName);

    if(!scriptList)
    {
    	LOG(WARNING) << "Function not mapped [" << functionName << "]";
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

