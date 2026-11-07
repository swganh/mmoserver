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

// The whole tutorial script (data/script/tutorial/part1.lua) played through
// on a fake station (testdata/tutorial_world.lua) by a scripted player
// (testdata/tutorial_player.lua), racing ahead or waiting for each prompt.

#include <string>

#include <gtest/gtest.h>

#include "ScriptEngine/Script.h"
#include "ScriptEngine/ScriptEngine.h"
#include "Utils/clock.h"

extern "C"
{
#include "lua.h"
#include "lauxlib.h"
}

namespace {

const std::string kSource = MMOSERVER_SOURCE_DIR;

class TutorialPlaythroughTest : public ::testing::Test {
protected:
    static void SetUpTestCase() {
        Anh_Utils::Clock::Init();
        ScriptEngine::Init();
    }

    void exec(const std::string& code) {
        lua_State* l = gScriptEngine->getMasterState();
        ASSERT_EQ(0, luaL_dostring(l, code.c_str())) << lua_tostring(l, -1);
    }

    std::string eval(const std::string& expression) {
        lua_State* l = gScriptEngine->getMasterState();
        luaL_dostring(l, ("return tostring(" + expression + ")").c_str());
        std::string result = lua_tostring(l, -1) ? lua_tostring(l, -1) : "?";
        lua_pop(l, 1);
        return result;
    }

    Script* start(const std::string& file) {
        Script* script = gScriptEngine->createScript();
        script->setFileName(file.c_str());
        script->run();
        return script;
    }

    // Plays the tutorial; returns the simulated milliseconds it took, or -1.
    int play(bool slow, bool skipOptional = false) {
        exec("package.path = '" + kSource + "/data/?.lua;' .. package.path");
        exec("dofile('" + kSource + "/src/ScriptEngine/testdata/tutorial_world.lua')");
        exec(std::string("SLOW = ") + (slow ? "true" : "false") +
             "; SKIP_OPTIONAL = " + (skipOptional ? "true" : "false") + "; playerDone = false");

        Script* tutorial = start(kSource + "/data/script/tutorial/part1.lua");
        Script* player = start(kSource + "/src/ScriptEngine/testdata/tutorial_player.lua");

        int elapsed = -1;
        for (int ms = 0; ms < 30 * 60 * 1000; ms += 250) {
            gScriptEngine->deliverEvents();
            tutorial->process(250);
            player->process(250);
            if (tutorial->mState == SS_Not_Loaded && eval("playerDone") == "true") {
                elapsed = ms;
                break;
            }
        }
        lastError_ = std::string(tutorial->getLastError()) + " / " + player->getLastError();
        gScriptEngine->removeScript(tutorial);
        gScriptEngine->removeScript(player);
        return elapsed;
    }

    std::string lastError_;
};

TEST_F(TutorialPlaythroughTest, APlayerRacingAheadIsNeverKeptWaiting) {
    int ms = play(false);
    ASSERT_GE(ms, 0) << "stuck at sub-state " << eval("world.substate") << ", room " << eval("world.room")
                     << ", last heard " << eval("world.heard[#world.heard]") << "; " << lastError_;
    EXPECT_EQ("27", eval("world.substate"));
    // Not one reminder for something already done.
    for (const char* reminder : { "mousewheel_repeat", "repeatchatprompt", "repeat_item_room_prompt",
                                  "repeat_open_box", "swear_reminder" }) {
        EXPECT_EQ("false", eval(std::string("world.heardKey('") + reminder + "')")) << reminder;
    }
    // The soldier, the mission officer and the Quartermaster do their part
    // the first time, however fast the player got to them.
    EXPECT_EQ("0", eval("refusals"));
    std::cout << "racing player finished in " << ms / 1000 << " s" << std::endl;
}

TEST_F(TutorialPlaythroughTest, APlayerSkippingCloningAndTrainingStillFinishes) {
    int ms = play(false, true);
    ASSERT_GE(ms, 0) << "stuck at sub-state " << eval("world.substate") << ", room " << eval("world.room")
                     << ", last heard " << eval("world.heard[#world.heard]") << "; " << lastError_;
    EXPECT_EQ("27", eval("world.substate"));
    EXPECT_EQ("0", eval("refusals"));
}

TEST_F(TutorialPlaythroughTest, APlayerFollowingThePromptsFinishes) {
    int ms = play(true);
    ASSERT_GE(ms, 0) << "stuck at sub-state " << eval("world.substate") << ", room " << eval("world.room")
                     << ", last heard " << eval("world.heard[#world.heard]") << "; " << lastError_;
    EXPECT_EQ("27", eval("world.substate"));
    EXPECT_EQ("true", eval("world.heardKey('tut_60')"));
    std::cout << "slow player finished in " << ms / 1000 << " s" << std::endl;
}

}  // namespace
