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
#include "script_engine.h"
#include <gtest/gtest.h>
#include <iostream>

using namespace python;
namespace {
class ScriptEngineTest : public ::testing::Test 
{
 protected:
     virtual void SetUp() 
     {
         e = std::make_shared<script_engine>("../../bin/debug/scripts/unittests/");
     }
     std::shared_ptr<script_engine> e;
};
TEST_F(ScriptEngineTest, loadScript ) 
{
    e->load("test.py");
    EXPECT_TRUE(e->isFileLoaded("test.py"));
}

TEST_F(ScriptEngineTest, runLoadedScript) 
{
    e->load("test.py");
    EXPECT_TRUE(e->isFileLoaded("test.py"));
    EXPECT_NO_THROW(e->run("test.py"));
}
TEST_F(ScriptEngineTest, runNonLoadedScript) 
{
    EXPECT_NO_THROW(e->run("nonloadedscript.py"));
    std::string err_msg ("No such file or directory");
    EXPECT_TRUE(e->getErrorMessage().find(err_msg) != std::string::npos);
}
TEST_F(ScriptEngineTest, runSecondLoadedScript)
{
    e->load("test.py");
    e->load("testRunSecond.py");
    EXPECT_TRUE(e->isFileLoaded("testRunSecond.py"));
    EXPECT_NO_THROW(e->run("testRunSecond.py"));
}
TEST_F(ScriptEngineTest, cantLoadScript)
{
    e->load("noscript.py");
    EXPECT_FALSE(e->isFileLoaded("noscript.py"));
    std::string err_msg ("No such file or directory");
    EXPECT_TRUE(e->getErrorMessage().find(err_msg) != std::string::npos);
}
TEST_F(ScriptEngineTest, loadSameFileTwice)
{
    e->load("test.py");
    e->load("test.py");
    e->run("test.py");
    EXPECT_TRUE(e->isFileLoaded("test.py"));
}
TEST_F(ScriptEngineTest, loadFileOneDeep)
{
    e->load("test-deep/scripta.py");
    EXPECT_TRUE(e->isFileLoaded("test-deep/scripta.py"));
}
TEST_F(ScriptEngineTest, getLoadedFiles)
{
    e->load("../scripta.py");
    e->load("../scriptb.py");
    e->load("test.py");
    e->load("testRunSecond.py");
    e->load("test-deep/scripta.py");
    EXPECT_EQ(uint32_t(5),e->getLoadedFiles().size());
}
TEST_F(ScriptEngineTest, removeLoadedFile)
{
    e->load("test.py");
    EXPECT_TRUE(e->isFileLoaded("test.py"));
    e->removeFile("test.py");
    EXPECT_FALSE(e->isFileLoaded("test.py"));
}
TEST_F(ScriptEngineTest, reloadFile)
{
    e->load("test.py");
    EXPECT_TRUE(e->isFileLoaded("test.py"));
    e->reload("test.py");
    EXPECT_TRUE(e->isFileLoaded("test.py"));
}
TEST_F(ScriptEngineTest, getPythonException)
{
    e->load("noscript.py");
    std::string err_msg ("No such file or directory");
    EXPECT_TRUE(e->getErrorMessage().find(err_msg) != std::string::npos);
}
//TEST_F(ScriptEngineTest, loadTestModule){
//    e->loadFile("module.py");
//    e->runLoadedFile("module.py");
//    EXPECT_TRUE(e->isFileLoaded("module.py"));
//}
}