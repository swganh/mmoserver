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
#include "scripting_manager.h"
#include <boost/python.hpp>
#include <iostream>
#include <fstream>
#include "../event_dispatcher/event_dispatcher.h"

using namespace std;
using namespace anh::scripting;
using namespace boost::python;

ScriptingManager::ScriptingManager(const string& base_path) 
{
    base_path_ = base_path;
    // initialize the python ScriptingManager
    Py_Initialize();
}
ScriptingManager::~ScriptingManager()
{
    loaded_files_.empty();
}
void ScriptingManager::load(const string& filename)
{
    
    try{              
        string input_str(&getFileInput_(filename)[0]);
        if (input_str.length() > 0)
        {
            auto file_str = make_shared<str>(input_str);
            loaded_files_.insert(make_pair(string(fullPath_(filename)), file_str));
        }
    }
    catch(...)
    {
        getExceptionFromPy_();
    }
}
void ScriptingManager::run(const string& filename)
{
    // are you trying to run a file that's not loaded?
    // lets load the file and run it anyway
    if (!isFileLoaded(filename))
        load(filename);
    
    str loaded_file = getLoadedFile(filename);
    try
    {
        // Retrieve the main module
        object main = import("__main__");
        // Retrieve the main module's namespace
        object global(main.attr("__dict__"));
        exec(loaded_file, global, global);
    }
    catch(...)
    {
        getExceptionFromPy_();
    }
}
void ScriptingManager::reload(const string& filename)
{
    if (isFileLoaded(filename))
    {
        removeFile(filename);
    }
    load(filename);
}
void ScriptingManager::removeFile(const string& filename)
{
    auto it = loaded_files_.begin();
    for (; it != loaded_files_.end();)
    {
        if (it->first == fullPath_(filename))
        {
            loaded_files_.erase(it++);
        }
        else
        {
            ++it;
        }
    }
}
bool ScriptingManager::isFileLoaded(const string& filename)
{
    auto it = find_if(loaded_files_.begin(), loaded_files_.end(), [this,&filename](bp_object_map::value_type& file){
        return file.first == fullPath_(filename);
    });
    return it != loaded_files_.end();
}
void ScriptingManager::setFullPath_(const string& filename)
{
    setFullPath_(filename, base_path_);
}
void ScriptingManager::setFullPath_(const string& filename, const string& root_path)
{
    full_path_.clear();
    full_path_.append(root_path);
    full_path_.append(filename);
}
str ScriptingManager::getLoadedFile(const string& filename)
{
    auto it = find_if(loaded_files_.begin(), loaded_files_.end(), [&](bp_object_map::value_type& file){
        return file.first == fullPath_(filename);
    });
    if (it != loaded_files_.end())
        return *it->second;
    else 
        return str();
}
char* ScriptingManager::fullPath_(const string& filename)
{
    setFullPath_(filename);
    return const_cast<char*>(full_path_.c_str());
}

vector<char> ScriptingManager::getFileInput_(const string& filename)
{
    vector<char> input;
    ifstream file(fullPath_(filename), ios::in | ios::binary);
    if (!file.is_open())
    {
        // set our error message here
        setCantFindFileError_();
        input.push_back('\0');
        return input;
    }

    file >> noskipws;
    copy(istream_iterator<char>(file), istream_iterator<char>(), back_inserter(input));
    input.push_back('\n');
    input.push_back('\0');
    
    return input;
}
void ScriptingManager::getExceptionFromPy_()
{
    PyObject* type, *value, *trace_back;
    PyErr_Fetch(&type, &value, &trace_back);
    // normalize to change value from tuple to string
    PyErr_NormalizeException(&type, &value, &trace_back);
    //value contains error message
    //trace_back contains stack snapshot and other information
    //(see python traceback structure)
    try
    {
        if (trace_back) {
            handle<> hTraceback(trace_back);
            object traceback(hTraceback);

            //Extract line number (top entry of call stack)
            // if you want to extract another levels of call stack
            // also process traceback.attr("tb_next") recurently
            py_exception.line_num = extract<string> (traceback.attr("tb_lineno"));
            py_exception.file_name = extract<string>(traceback.attr("tb_frame").attr("f_code").attr("co_filename"));
            py_exception.func_name = extract<string>(traceback.attr("tb_frame").attr("f_code").attr("co_name"));
        }
        if (value)
        {
            //Extract error message
            handle<> hVal (PyObject_Str(value));
            object err_msg(hVal);
            py_exception.err_msg = extract<string>(err_msg);
        }
    } 
    catch(...)
    {
        return;
    }
}
void ScriptingManager::setCantFindFileError_()
{
    py_exception.file_name = full_path_;
    py_exception.err_msg = full_path_ + string(": No such file or directory");
}
string ScriptingManager::getErrorMessage()
{
    if (py_exception.err_msg.length() > 0)
    {
        stringstream ss;
        ss << "Error: " << py_exception.err_msg;
        if (py_exception.line_num.length() >0)
        {
            ss << " On Line: " << py_exception.line_num;
            ss << " in file: " << py_exception.file_name;
            ss << " function: " << py_exception.func_name;
        }
        return ss.str();
    }
    else
    {
        return "Undefined Error";
    }
}