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
#ifndef _PYTHON_SCRIPT_ENGINE_H_
#define _PYTHON_SCRIPT_ENGINE_H_

#include <boost/python.hpp>
#include <map>
#include <vector>

namespace python{
namespace bp = boost::python;

typedef std::map<std::string,bp::str> bp_object_map;
class object;
/*! Assists the user in loading and executing Python scripts from
*   within C++.
*
*/
class script_engine
{
public:
    /**
    * \brief creates script_engine with the base path specified
    *
    * \param base_path sets where the engine looks for the files
    */
    script_engine(const std::string& base_path);
    ~script_engine();
    /**
    * \brief load's the python script into memory
    *  and stores the Boost Python str into a 
    *  private map for later use in @run
    *
    * \param filename The file to load into memory
    */
    void load(const std::string& filename);

     /**
    * \brief run's the python file
    *
    * \param filename The file to run from the loaded_files_ map
    *   if not found, a message will be given
    */
    void run(const std::string& filename);

     /**
    * \brief reload's the python file
    *
    * \param filename The file to remove from the loaded_files_ map
    *  the file will be (re)loaded
    */
    void reload(const std::string& filename);

    /**
    * \brief remove the python file
    *
    * \param filename The file to remove from the loaded_files_ map
    *   if not found, no action occurs
    */
    void removeFile(const std::string& filename);

    /**
    * \brief outputs the currently loaded files available
    *   for immediate execution.
    *
    */
    bp_object_map getLoadedFiles() { return loaded_files_; }

    /**
    * \brief checks to see if the filename has been loaded
    * 
    * \returns true if the file has been loaded, false else.
    */
    bool isFileLoaded(const std::string& filename);

    /**
    * \brief sets the default file path to find scripts
    *  
    * \param filepath sets the path_ behind the scenes
    */
    bp::str getLoadedFile(const std::string& filename);
    /**
    * \brief gets data from PYEXCEPTION struct and 
    *       creates a friendly message
    *  
    */
    std::string getErrorMessage();
private:
    // hide default ctor
    script_engine();
    // used by internal functions
    void setFullPath(const std::string& filename, const std::string& root_path);
    void setFullPath(const std::string& filename);
    /**
    * \brief helper method to set a 
    *   simple file not found message
    */
    void setCantFindFileError();

    std::vector<char> getFileInput(const std::string& filename);
    /**
    * \brief sets the PYEXCEPTION struct based on internal python values
    *
    * \effect struct is filled out
    */
    void getExceptionFromPy();
    // base path set by the default ctor
    std::string base_path_;
    /**
    * \brief helper function to take in base filename and return full path
    *
    * \param filename to add to base path
    * \returns char* of full path after adding filename
    */
    char* fullPath(const std::string& filename);

    /**
    * \brief full path including .py
    */
    std::string full_path_;

    /**
    * \brief a std::map containing the name and boost python object
    *   of all loaded files
    */
    bp_object_map loaded_files_; 

    /**
    * \brief struct containing last available python exception
    */
    struct PYEXCEPTION
    {
        std::string err_msg;
        std::string line_num;
        std::string file_name;
        std::string func_name;
    } py_exception;
};
class ScriptComponent {
    struct {
        std::string name;
        std::string file_name;
        std::shared_ptr<IEvent> on_event;
    }SCRIPTDATA;
    SCRIPTDATA[] scripts;
};
} // namespace python
#endif //_PYTHON_SCRIPT_ENGINE_H_