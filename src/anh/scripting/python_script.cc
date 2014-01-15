// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#include "python_script.h"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <iterator>

#include "anh/logger.h"

#include <boost/python.hpp>
#include <Python.h>

#include "anh/scripting/utilities.h"

using namespace swganh::scripting;
namespace bp = boost::python;

PythonScript::PythonScript(const std::string& filename, bool delay_execution)
    : filename_(filename)
{
    ReadFileContents_();
    PreparePythonEnvironment_();

    if (!delay_execution)
    {
        Run();
    }
}

void PythonScript::Run()
{
    ScopedGilLock lock;

    try
    {
#ifdef _DEBUG
        ReadFileContents_();
#endif

        file_object_.reset(
            new bp::object(bp::exec(filecontents_.c_str(), *globals_, *globals_)));
    }
    catch (bp::error_already_set&)
    {
        logPythonException();
    }
}


std::shared_ptr<boost::python::object> PythonScript::GetGlobal(const std::string& name)
{
    std::shared_ptr<bp::object> instance = nullptr;

    ScopedGilLock lock;

    try
    {
        instance = std::shared_ptr<bp::object>(
                       new bp::object((*globals_)[name.c_str()]),
                       PythonObjectDeleter());
    }
    catch(bp::error_already_set&)
    {
        logPythonException();
    }

    return instance;
}

bp::dict PythonScript::GetGlobals()
{
    ScopedGilLock lock;
    return bp::extract<bp::dict>(*globals_);
}

void PythonScript::PreparePythonEnvironment_()
{
    ScopedGilLock lock;

    try
    {
        main_.reset(
            new bp::object(bp::handle<>(bp::borrowed(
                                            PyImport_AddModule("__main__")))));

        globals_.reset(new bp::object(main_->attr("__dict__")));

        bp::dict tmp;
        tmp["__builtins__"] = (*globals_)["__builtins__"];

        *globals_ = tmp;
    }
    catch (bp::error_already_set&)
    {
        logPythonException();
    }
}

void PythonScript::ReadFileContents_()
{
    std::ifstream filestream(filename_);
    filestream >> std::noskipws;

    filecontents_ = std::string(
                        (std::istreambuf_iterator<char>(filestream)),
                        std::istreambuf_iterator<char>());
}
