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
#ifndef _ANH_SCRIPTING_MANAGER_INTERFACE_H_
#define _ANH_SCRIPTING_MANAGER_INTERFACE_H_

#include <map>
#include <vector>

namespace anh{
namespace scripting{

/*! Assists the user in loading and executing scripts from
*   within C++.
*
*/
class ScriptingManagerInterface
{
public:
    /**
    * \brief creates script_engine with the base path specified
    *
    * \param base_path sets where the engine looks for the files
    */
    virtual ~ScriptingManagerInterface(){}
    /**
    * \brief load's the script into memory
    *  and stores the str into a 
    *  private map for later use in @run
    *
    * \param filename The file to load into memory
    */
    virtual void load(const std::string& filename) = 0;

     /**
    * \brief run's the script file
    *
    * \param filename The file to run 
    *   if not found, a message will be given
    */
    virtual void run(const std::string& filename) = 0;

     /**
    * \brief reload's the script file
    *
    * \param filename The file to reload
    *  the file will be (re)loaded
    */
    virtual void reload(const std::string& filename) = 0;

    /**
    * \brief remove the script file
    *
    * \param filename The file to remove
    *   if not found, no action occurs
    */
    virtual void removeFile(const std::string& filename) = 0;
    /**
    * \brief gets data from scripting error and 
    *       creates a friendly message
    *  
    */
    virtual std::string getErrorMessage() = 0;
};

} // namespace anh
} // namespace scripting
#endif //_ANH_SCRIPTING_MANAGER_INTERFACE_H_