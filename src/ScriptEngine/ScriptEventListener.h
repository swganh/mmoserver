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

#ifndef ANH_SCRIPT_EVENT_LISTENER_H
#define ANH_SCRIPT_EVENT_LISTENER_H

#include <map>
#include <string>
#include <list>

#include "Utils/typedefs.h"
#include "Utils/bstring.h"

class Script;

typedef std::list<Script*>					ScriptList;
typedef std::map<std::string, ScriptList*>	ScriptListMap;

//======================================================================================================================

class ScriptEventListener
{
public:

    ScriptEventListener();
    ~ScriptEventListener();

    void		registerFunction(const int8* functionName);
    void		unregisterFunction(const int8* functionName);

    void		registerScript(Script* script,const int8* functionName);
    void		unregisterScript(Script* script);

    void		handleScriptEvent(const int8* functionName,BString params);

    ScriptList*	getScriptList(const int8* functionName);

private:

    void		_removeScriptList(const int8* functionName);

    ScriptListMap	mScriptListMap;
};

//======================================================================================================================

#endif


