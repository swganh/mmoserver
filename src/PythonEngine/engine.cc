#include "engine.h"
#include <iostream>
#include "anh/event_dispatcher/event_dispatcher.h"

using namespace python::engine;
engine::engine(const std::string& base_path)
{
    base_path_ = base_path;
    // initialize the python engine
    Py_Initialize();
    // TOOD set path based on project directory and OS
}
void engine::load(const std::string& filename)
{
    // what this does is first calls the Python C-API to load the file, then pass the returned
    // PyObject* into handle, which takes reference and sets it as a boost::python::object.
    // this takes care of all future referencing and dereferencing.
    try{
        bp::object file_object(bp::handle<>(PyFile_FromString(fullPath(filename), "r" )));
        loaded_files_.insert(std::make_pair(fullPath(filename), file_object));
    }
    catch(...)
    {
        PyErr_Print();
    }
}

void engine::run(const std::string& filename)
{
    // are you trying to run a file that's not loaded?
    // lets load the file and run it anyway
    if (!isFileLoaded(filename))
        load(filename);

    bp::object loaded_file = getLoadedFile(filename);
    try
    {
        PyRun_SimpleFile( PyFile_AsFile( loaded_file.ptr()), fullPath(filename) );
    }
    catch(...)
    {
        PyErr_Print();
    }
}

void engine::registerModule(_inittab init_func)
{
    // Register the module with the interpreter
    if (PyImport_AppendInittab(init_func.name, init_func.initfunc) == -1)
    {
        throw std::runtime_error("Failed to add " + std::string(init_func.name) + " to the interpreter's "
                                 "built in modules");
    }
}

bool engine::isFileLoaded(const std::string& filename)
{
    auto it = std::find_if(loaded_files_.begin(), loaded_files_.end(), [&](bp_object_map::value_type& file){
        return file.first == fullPath(filename);
    });
    return it != loaded_files_.end();
}
void engine::setFullPath(const std::string& filename)
{
    setFullPath(filename, base_path_);
}
void engine::setFullPath(const std::string& filename, const std::string& root_path)
{
    full_path_.clear();
    full_path_.append(root_path);
    full_path_.append(filename);
}
bp::object engine::getLoadedFile(const std::string& filename)
{
    auto it = std::find_if(loaded_files_.begin(), loaded_files_.end(), [&](bp_object_map::value_type& file){
        return file.first == fullPath(filename);
    });
    if (it != loaded_files_.end())
        return it->second;
    else 
        return bp::object();
}
char* engine::fullPath(const std::string& filename)
{
    setFullPath(filename);
    return const_cast<char*>(full_path_.c_str());
}