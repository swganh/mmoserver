/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2010 The SWG:ANH Team
---------------------------------------------------------------------------------------
Use of this source code is governed by the GPL v3 license that can be found
in the COPYING file or at http://www.gnu.org/licenses/gpl-3.0.html
---------------------------------------------------------------------------------------
*/

#ifndef ANH_SCRIPTENGINE_SCRIPTEVENT_H
#define ANH_SCRIPTENGINE_SCRIPTEVENT_H

#include <cstdint>
#include <map>
#include <string>

// Something that happened in the game that scripts can wait for
// (LuaScriptEngine.WaitEvent) or listen to (LuaScriptEngine.OnEvent).
//
// Raised with gScriptEngine->raiseEvent(...) and delivered on the next
// script engine tick, never in the middle of the game code that raised it.
// In Lua it is a table: { name = ..., subject = ..., source = ..., <fields> }.
// The event names and their fields are listed in docs/SCRIPTING.md.
struct ScriptEvent
{
    ScriptEvent() : subject(0), source(0) {}
    ScriptEvent(const std::string& name, uint64_t subject, uint64_t source = 0)
        : name(name), subject(subject), source(source) {}

    ScriptEvent& set(const std::string& key, double value) {
        numbers[key] = value;
        return *this;
    }
    ScriptEvent& set(const std::string& key, const std::string& value) {
        strings[key] = value;
        return *this;
    }

    std::string name;   // e.g. "creature_died"
    uint64_t subject;   // what it is about: the creature, region, NPC or object
    uint64_t source;    // who caused it, usually the player; 0 when unknown
    std::map<std::string, double> numbers;
    std::map<std::string, std::string> strings;
};

#endif
