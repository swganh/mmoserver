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

// Script events: LuaScriptEngine.WaitEvent / OnEvent against a real Lua 5.1
// engine (without the game bindings, which belong to the zone server).

#include <cstdio>
#include <fstream>
#include <string>

#include <gtest/gtest.h>

#include "ScriptEngine/Script.h"
#include "ScriptEngine/ScriptEngine.h"
#include "ScriptEngine/ScriptEvent.h"
#include "Utils/clock.h"

extern "C"
{
#include "lua.h"
}

namespace {

class ScriptEventsTest : public ::testing::Test {
protected:
    // One engine for the whole run: it is a singleton.
    static void SetUpTestCase() {
        Anh_Utils::Clock::Init();
        ScriptEngine::Init();
    }

    void TearDown() {
        for (Script* script : scripts_) {
            gScriptEngine->removeScript(script);
        }
        for (const std::string& file : files_) {
            std::remove(file.c_str());
        }
    }

    // Starts a script; it runs until its first wait.
    Script* run(const std::string& code) {
        std::string file = ::testing::TempDir() + "script_events_" +
            ::testing::UnitTest::GetInstance()->current_test_info()->name() +
            std::to_string(files_.size()) + ".lua";
        std::ofstream(file.c_str()) << code;
        files_.push_back(file);

        Script* script = gScriptEngine->createScript();
        scripts_.push_back(script);
        script->setFileName(file.c_str());
        script->run();
        return script;
    }

    lua_State* L() { return gScriptEngine->getMasterState(); }

    std::string global(const char* name) {
        lua_getglobal(L(), name);
        const char* value = lua_tostring(L(), -1);
        std::string result = value ? value : "nil";
        lua_pop(L(), 1);
        return result;
    }

    std::vector<Script*> scripts_;
    std::vector<std::string> files_;
};

TEST_F(ScriptEventsTest, WaitEventResumesOnTheNextTick) {
    Script* script = run(
        "local e = LuaScriptEngine.WaitEvent('creature_died')\n"
        "wait1 = e.name .. ' ' .. e.subject .. ' ' .. e.source .. ' ' .. e.is_player\n");
    EXPECT_EQ(SS_Wait_Event, script->mState) << script->getLastError();

    gScriptEngine->raiseEvent(ScriptEvent("creature_died", 101, 7).set("is_player", 0.0));
    EXPECT_EQ("nil", global("wait1"));  // queued, not delivered yet

    gScriptEngine->deliverEvents();
    EXPECT_EQ("creature_died 101 7 0", global("wait1"));
    EXPECT_EQ(SS_Not_Loaded, script->mState);
}

TEST_F(ScriptEventsTest, WaitEventFiltersOnSubject) {
    run("local e = LuaScriptEngine.WaitEvent('region_entered', 42)\n"
        "wait2 = e.region_name\n");

    gScriptEngine->raiseEvent(ScriptEvent("region_entered", 41).set("region_name", std::string("other")));
    gScriptEngine->raiseEvent(ScriptEvent("region_left", 42).set("region_name", std::string("left")));
    gScriptEngine->deliverEvents();
    EXPECT_EQ("nil", global("wait2"));

    gScriptEngine->raiseEvent(ScriptEvent("region_entered", 42).set("region_name", std::string("cantina")));
    gScriptEngine->deliverEvents();
    EXPECT_EQ("cantina", global("wait2"));
}

TEST_F(ScriptEventsTest, WaitEventTimesOutWithNil) {
    Script* script = run(
        "local e = LuaScriptEngine.WaitEvent('conversation_started', 0, 1000)\n"
        "wait3 = 'timeout:' .. tostring(e)\n");

    script->process(500);
    EXPECT_EQ("nil", global("wait3"));
    EXPECT_EQ(SS_Wait_Event, script->mState);

    script->process(600);
    EXPECT_EQ("timeout:nil", global("wait3"));
    EXPECT_EQ(SS_Not_Loaded, script->mState);

    // A late event finds nobody waiting.
    gScriptEngine->raiseEvent(ScriptEvent("conversation_started", 1));
    gScriptEngine->deliverEvents();
}

TEST_F(ScriptEventsTest, ScriptCanWaitInALoop) {
    Script* script = run(
        "count4 = 0\n"
        "while true do\n"
        "  local e = LuaScriptEngine.WaitEvent('object_used')\n"
        "  count4 = count4 + e.menu_id\n"
        "end\n");

    for (int i = 1; i <= 3; ++i) {
        gScriptEngine->raiseEvent(ScriptEvent("object_used", 5).set("menu_id", static_cast<double>(i)));
    }
    // Each event wakes the script, which waits again before the next one.
    gScriptEngine->deliverEvents();
    EXPECT_EQ("6", global("count4"));
    EXPECT_EQ(SS_Wait_Event, script->mState);
}

TEST_F(ScriptEventsTest, OnEventCallsTheHandlerEveryTime) {
    run("seen5 = ''\n"
        "handler5 = LuaScriptEngine.OnEvent('player_entered_zone', function(e)\n"
        "  seen5 = seen5 .. e.player_name .. '@' .. e.planet .. ';'\n"
        "end)\n");

    gScriptEngine->raiseEvent(ScriptEvent("player_entered_zone", 1).set("player_name", std::string("Yseecy"))
                                  .set("planet", std::string("tatooine")));
    gScriptEngine->raiseEvent(ScriptEvent("player_entered_zone", 2).set("player_name", std::string("Oelaimest"))
                                  .set("planet", std::string("tatooine")));
    gScriptEngine->deliverEvents();
    EXPECT_EQ("Yseecy@tatooine;Oelaimest@tatooine;", global("seen5"));

    // Removed: no more calls.
    run("LuaScriptEngine.RemoveEventHandler(handler5)\n");
    gScriptEngine->raiseEvent(ScriptEvent("player_entered_zone", 3).set("player_name", std::string("Late"))
                                  .set("planet", std::string("naboo")));
    gScriptEngine->deliverEvents();
    EXPECT_EQ("Yseecy@tatooine;Oelaimest@tatooine;", global("seen5"));
}

TEST_F(ScriptEventsTest, FieldsCannotReplaceNameSubjectOrSource) {
    run("local e = LuaScriptEngine.WaitEvent('object_used', 9)\n"
        "wait9 = e.name .. ' ' .. e.subject .. ' ' .. e.source\n");

    gScriptEngine->raiseEvent(ScriptEvent("object_used", 9, 3).set("name", std::string("other"))
                                  .set("subject", 1.0).set("source", 2.0));
    gScriptEngine->deliverEvents();
    EXPECT_EQ("object_used 9 3", global("wait9"));
}

TEST_F(ScriptEventsTest, OnEventCanFilterOnSubject) {
    run("count6 = 0\n"
        "LuaScriptEngine.OnEvent('creature_died', function(e) count6 = count6 + 1 end, 106)\n");

    gScriptEngine->raiseEvent(ScriptEvent("creature_died", 105));
    gScriptEngine->raiseEvent(ScriptEvent("creature_died", 106));
    gScriptEngine->deliverEvents();
    EXPECT_EQ("1", global("count6"));
}

TEST_F(ScriptEventsTest, WaitingInAHandlerIsAnErrorNotACrash) {
    run("after7 = 'no'\n"
        "LuaScriptEngine.OnEvent('conversation_ended', function(e)\n"
        "  LuaScriptEngine.WaitMSec(100)\n"
        "  after7 = 'yes'\n"
        "end)\n");

    int top = lua_gettop(L());
    gScriptEngine->raiseEvent(ScriptEvent("conversation_ended", 1, 2));
    gScriptEngine->deliverEvents();
    EXPECT_EQ("no", global("after7"));
    EXPECT_EQ(top, lua_gettop(L())) << "the error message is popped";
}

TEST_F(ScriptEventsTest, EventsRaisedDuringDeliveryWaitForTheNextTick) {
    run("LuaScriptEngine.OnEvent('ping8', function(e) end)\n"
        "local e = LuaScriptEngine.WaitEvent('pong8')\n"
        "got8 = e.name\n");

    // A C++ handler of ping raising pong, as game code reacting to a script would.
    gScriptEngine->raiseEvent(ScriptEvent("ping8", 0));
    gScriptEngine->deliverEvents();
    gScriptEngine->raiseEvent(ScriptEvent("pong8", 0));
    EXPECT_EQ("nil", global("got8"));
    gScriptEngine->deliverEvents();
    EXPECT_EQ("pong8", global("got8"));
}

}  // namespace
