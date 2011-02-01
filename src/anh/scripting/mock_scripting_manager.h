/*
 This file is part of MMOServer. For more information, visit http://swganh.com
 
 Copyright (c) 2006 - 2010 The SWG:ANH Team

 MMOServer is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 MMOServer is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with MMOServer.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef _ANH_MOCK_SCRIPT_MANAGER_H_
#define _ANH_MOCK_SCRIPT_MANAGER_
#include <anh/scripting/scripting_manager.h>
#include <gmock/gmock.h>

namespace anh{
namespace scripting{

class MockScriptingManager : public IScriptingManager
{
    MOCK_METHOD1(load, void(const std::string& filename));
    MOCK_METHOD1(run, void(const std::string& filename));
    MOCK_METHOD1(reload, void(const std::string& filename));
    MOCK_METHOD1(removeFile, void(const std::string& filename));
    MOCK_METHOD0(getErrorMessage, std::string());
};
} // namespace scripting
} // namespace anh

#endif //_ANH_MOCK_SCRIPT_MANAGER_H_