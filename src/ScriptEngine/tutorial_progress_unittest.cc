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

// data/script/tutorial/progress.lua against the real engine, with a fake
// tutorial object recording what it is asked to do.

#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "ScriptEngine/Script.h"
#include "ScriptEngine/ScriptEngine.h"
#include "ScriptEngine/ScriptEvent.h"
#include "Utils/clock.h"

extern "C"
{
#include "lua.h"
#include "lauxlib.h"
}

namespace {

const double kPlayer = 8589959593.0;
const double kDrum = 2533274790395904.0;

class TutorialProgressTest : public ::testing::Test {
protected:
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

    // Runs a script with `Progress` (the module), `tutorial` (a fake that
    // logs to `log`) and `progress` (Progress.new for the player) set up.
    Script* run(const std::string& code) {
        std::string prelude =
            "log = {}\n"
            "tutorial = {\n"
            "  updateTutorial = function(self, a) log[#log + 1] = 'ask:' .. a end,\n"
            "  scriptSystemMessage = function(self, m) log[#log + 1] = 'say:' .. m end,\n"
            "  scriptPlayMusic = function(self, s) log[#log + 1] = 'sound:' .. s end,\n"
            "}\n"
            "Progress = dofile('" MMOSERVER_SOURCE_DIR "/data/script/tutorial/progress.lua')\n"
            "progress = Progress.new(tutorial, " + std::to_string(static_cast<long long>(kPlayer)) + ")\n"
            "function logged() return table.concat(log, ' ') end\n";

        std::string file = ::testing::TempDir() + "tutorial_progress_" +
            ::testing::UnitTest::GetInstance()->current_test_info()->name() + ".lua";
        std::ofstream(file.c_str()) << prelude << code;
        files_.push_back(file);

        Script* script = gScriptEngine->createScript();
        scripts_.push_back(script);
        script->setFileName(file.c_str());
        script->run();
        return script;
    }

    std::string eval(const std::string& expression) {
        lua_State* l = gScriptEngine->getMasterState();
        std::string code = "return tostring(" + expression + ")";
        if (luaL_dostring(l, code.c_str()) != 0) {
            std::string error = lua_tostring(l, -1);
            lua_pop(l, 1);
            return "error: " + error;
        }
        std::string result = lua_tostring(l, -1);
        lua_pop(l, 1);
        return result;
    }

    void exec(const std::string& code) {
        lua_State* l = gScriptEngine->getMasterState();
        ASSERT_EQ(0, luaL_dostring(l, code.c_str())) << lua_tostring(l, -1);
    }

    // Deliver events and let the script run for ms.
    void tick(Script* script, uint32 ms) {
        gScriptEngine->deliverEvents();
        for (uint32 t = 0; t < ms; t += 250) {
            script->process(250);
        }
    }

    std::vector<Script*> scripts_;
    std::vector<std::string> files_;
};

TEST_F(TutorialProgressTest, AsksForTheClientReportsAtTheStart) {
    Script* script = run("");
    EXPECT_EQ("ask:zoomCamera ask:closeHolocron ask:openInventory ask:closeInventory", eval("logged()"))
        << script->getLastError();
    exec("progress:stop()");
}

TEST_F(TutorialProgressTest, RemembersWhatThePlayerDidBeforeTheScriptAsked) {
    Script* script = run(
        "progress:sleep(1000)\n"
        "chattedFirst = progress.chatted\n"
        "progress:waitFor(function() return progress.chatted end)\n"
        "done = true\n");

    // The player chats and opens the drum while the script is busy.
    gScriptEngine->raiseEvent(ScriptEvent("player_chat", static_cast<uint64>(kPlayer)).set("text", std::string("hi")));
    gScriptEngine->raiseEvent(ScriptEvent("container_opened", static_cast<uint64>(kDrum), static_cast<uint64>(kPlayer)));
    // Someone else's actions don't count.
    gScriptEngine->raiseEvent(ScriptEvent("food_eaten", 1, 12345));
    tick(script, 1250);

    EXPECT_EQ("true", eval("done")) << script->getLastError();
    EXPECT_EQ("true", eval("chattedFirst"));
    EXPECT_EQ("true", eval("progress.opened[" + std::to_string(static_cast<long long>(kDrum)) + "]"));
    EXPECT_EQ("false", eval("progress.ate"));
    exec("progress:stop()");
}

TEST_F(TutorialProgressTest, AsksAgainForAReportOnceItArrives) {
    Script* script = run("progress:sleep(5000)\n");
    exec("log = {}");

    gScriptEngine->raiseEvent(ScriptEvent("client_report", static_cast<uint64>(kPlayer))
                                  .set("action", std::string("openInventory")));
    tick(script, 500);

    EXPECT_EQ("ask:openInventory", eval("logged()"));
    EXPECT_EQ("true", eval("progress.inventoryOpen"));
    EXPECT_EQ("true", eval("progress:reported('openInventory')"));

    gScriptEngine->raiseEvent(ScriptEvent("client_report", static_cast<uint64>(kPlayer))
                                  .set("action", std::string("closeInventory")));
    tick(script, 500);
    EXPECT_EQ("false", eval("progress.inventoryOpen"));
    exec("progress:stop()");
}

TEST_F(TutorialProgressTest, PromptsDoNotTalkOverEachOther) {
    Script* script = run(
        "log = {}\n"
        "progress:say('first', 100, 2000)\n"
        "progress:say('second', nil, 0)\n");

    EXPECT_EQ("say:@newbie_tutorial/system_messages:first sound:100", eval("logged()"))
        << script->getLastError();
    tick(script, 1500);
    EXPECT_EQ("say:@newbie_tutorial/system_messages:first sound:100", eval("logged()"));
    tick(script, 750);
    EXPECT_EQ("say:@newbie_tutorial/system_messages:first sound:100 say:@newbie_tutorial/system_messages:second",
              eval("logged()"));
    exec("progress:stop()");
}

TEST_F(TutorialProgressTest, RemindsOnlyAfterTheNarrationAndTheGracePeriod) {
    Script* script = run(
        "reminders = 0\n"
        "progress:say('prompt', nil, 1000)\n"
        "progress:waitFor(function() return progress.ate end,\n"
        "                 function() reminders = reminders + 1 end, 2000, 3000)\n"
        "done = true\n");

    tick(script, 2750);   // 1000 narration + 1750 idle
    EXPECT_EQ("0", eval("reminders")) << script->getLastError();
    tick(script, 500);    // 2000 idle: first reminder
    EXPECT_EQ("1", eval("reminders"));
    tick(script, 3000);   // every 3000 after that
    EXPECT_EQ("2", eval("reminders"));

    gScriptEngine->raiseEvent(ScriptEvent("food_eaten", 77, static_cast<uint64>(kPlayer)));
    tick(script, 250);
    EXPECT_EQ("true", eval("done"));
    EXPECT_EQ("2", eval("reminders"));
    exec("progress:stop()");
}

TEST_F(TutorialProgressTest, StopsListeningWhenThePlayerLeaves) {
    Script* script = run("progress:sleep(5000)\n");

    gScriptEngine->raiseEvent(ScriptEvent("player_left_zone", static_cast<uint64>(kPlayer)));
    tick(script, 250);
    gScriptEngine->raiseEvent(ScriptEvent("food_eaten", 77, static_cast<uint64>(kPlayer)));
    tick(script, 250);

    EXPECT_EQ("false", eval("progress.ate"));
    EXPECT_EQ("0", eval("#progress.handlers"));
}

}  // namespace
