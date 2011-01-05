#include "script_engine.h"
#include <iostream>
#include "anh/event_dispatcher/event_dispatcher.h"

using namespace python;
script_engine::script_engine(const std::string& base_path)
{
    base_path_ = base_path;
    // initialize the python script_engine
    Py_Initialize();
    PyGILState_STATE gil_state = PyGILState_Ensure();
}
script_engine::~script_engine()
{
    loaded_files_.empty();
    //PyGILState_Release(gilState);
}
void script_engine::load(const std::string& filename)
{
    // what this does is first calls the Python C-API to load the file, then pass the returned
    // PyObject* into handle, which takes reference and sets it as a boost::python::object.
    // this takes care of all future referencing and dereferencing.
    try{
        PyObject *ioMod, *opened_file, *fd_obj;
        
        ioMod = PyImport_ImportModule("io");
        opened_file = PyObject_CallMethod(ioMod, "open", "ss", fullPath(filename), "rb");
        Py_DECREF(ioMod);
        fd_obj = PyFile_FromFd(PyObject_AsFileDescriptor(opened_file),fullPath(filename),"r",-1,0,0,"\n", 0);
        bp::handle<> h_open(fd_obj);
        bp::object file_obj(h_open);
        loaded_files_.insert(std::make_pair(std::string(fullPath(filename)), file_obj));
    }
    catch(...)
    {
        getExceptionFromPy();
    }
}
void script_engine::run(const std::string& filename)
{
    // are you trying to run a file that's not loaded?
    // lets load the file and run it anyway
    if (!isFileLoaded(filename))
        load(filename);

    bp::object loaded_file = getLoadedFile(filename);
    try
    {
        //bp::handle<> file_handle(PyFile_FromFd(PyObject_AsFileDescriptor(loaded_file.ptr()),fullPath(filename),"",-1,"", "","", 0));
        int fd = PyObject_AsFileDescriptor(loaded_file.ptr());
        FILE* f_open = _fdopen(fd,"r");
        
        PyRun_SimpleFile( f_open, fullPath(filename) );
        
    }
    catch(...)
    {
        getExceptionFromPy();
    }
}
void script_engine::reload(const std::string& filename)
{
    if (isFileLoaded(filename))
    {
        removeFile(filename);
    }
    load(filename);
}
void script_engine::removeFile(const std::string& filename)
{
    auto it = loaded_files_.begin();
    for (; it != loaded_files_.end();)
    {
        if (it->first == fullPath(filename))
        {
            loaded_files_.erase(it++);
        }
        else
        {
            ++it;
        }
    }
}
bool script_engine::isFileLoaded(const std::string& filename)
{
    auto it = std::find_if(loaded_files_.begin(), loaded_files_.end(), [this,&filename](bp_object_map::value_type& file){
        return file.first == fullPath(filename);
    });
    return it != loaded_files_.end();
}
void script_engine::setFullPath(const std::string& filename)
{
    setFullPath(filename, base_path_);
}
void script_engine::setFullPath(const std::string& filename, const std::string& root_path)
{
    full_path_.clear();
    full_path_.append(root_path);
    full_path_.append(filename);
}
bp::object script_engine::getLoadedFile(const std::string& filename)
{
    auto it = std::find_if(loaded_files_.begin(), loaded_files_.end(), [&](bp_object_map::value_type& file){
        return file.first == fullPath(filename);
    });
    if (it != loaded_files_.end())
        return it->second;
    else 
        return bp::object();
}
char* script_engine::fullPath(const std::string& filename)
{
    setFullPath(filename);
    return const_cast<char*>(full_path_.c_str());
}
void script_engine::getExceptionFromPy()
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
            bp::handle<> hTraceback(trace_back);
            bp::object traceback(hTraceback);

            //Extract line number (top entry of call stack)
            // if you want to extract another levels of call stack
            // also process traceback.attr("tb_next") recurently
            py_exception.line_num = bp::extract<std::string> (traceback.attr("tb_lineno"));
            py_exception.file_name = bp::extract<std::string>(traceback.attr("tb_frame").attr("f_code").attr("co_filename"));
            py_exception.func_name = bp::extract<std::string>(traceback.attr("tb_frame").attr("f_code").attr("co_name"));
        }
        if (value)
        {
            //Extract error message
            bp::handle<> hVal (PyObject_Str(value));
            bp::object err_msg(hVal);
            py_exception.err_msg = bp::extract<std::string>(err_msg);
        }
    } 
    catch(...)
    {
        return;
    }
}
std::string script_engine::getErrorMessage()
{
    if (py_exception.err_msg.length() > 0)
    {
        std::stringstream ss;
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