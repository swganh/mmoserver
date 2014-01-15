// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#pragma once

#ifndef WIN32
#include <Python.h>
#endif

#include <string>
#include <memory>

#include <boost/noncopyable.hpp>
#include <boost/python.hpp>

#include "utilities.h"

namespace swganh {
namespace scripting {

    struct PythonObjectDeleter
    {
        void operator() (boost::python::object* obj) { ScopedGilLock lock; delete obj; }
    };

    class PythonScript : boost::noncopyable
    {
    public:
        /**
         * @param filename The filename containing the python script.
         */
        explicit PythonScript(const std::string& filename, bool delay_execution = false);

        /**
         * Executes the python script.
         */
        void Run();

        /**
         * Sets the value of a given python variable in the global namespace of this script.
         *
         * @param name The name of the variable to set in the global namespace.
         * @param key The new value of the specified global variable.
         */
        template<typename T>
        void SetGlobal(const std::string& name, T value)
        {
            ScopedGilLock lock;

            try{
                (*globals_)[name.c_str()] = value;
            }
            catch (boost::python::error_already_set &)
            {
                swganh::scripting::logPythonException();
            }
        }

        /**
         * Extracts a global variable as a C++ value type.
         *
         * @param name The name of the variable to grab from the global namespace.
         * @return The extracted C++ value
         */
        template<typename T>
        T GetGlobalAs(const std::string& key)
        {
            ScopedGilLock lock;

            auto py_value = GetGlobal(key);
            return boost::python::extract<T>(*py_value);
        }

        /**
         * Gets a python variable from the global namespace of this script.
         *
         * @param name The name of the variable to grab from the global namespace.
         * @param A shared pointer to the python object for the requested value.
         */
        std::shared_ptr<boost::python::object> GetGlobal(const std::string& name);

        boost::python::dict GetGlobals();

        /**
         * Creates a python implementation of the given C++ interface.
         *
         * One of the two primary uses of script is the extending of C++ base
         * classes in Python which can then be used polymorphically from
         * within "C++ land." This method attempts to construct a Python class
         * from the given name and extracts the C++ base pointer out.
         *
         * @param class_name The Python name of the class to construct.
         * @return a shared_ptr containing the extracted C++ interface.
         */
        template<typename T>
        std::shared_ptr<T> CreateInstance(const std::string& class_name)
        {
            std::shared_ptr<T> cpp_instance = nullptr;

            ScopedGilLock lock;

            try
            {
                auto creator = (*globals_)[class_name.c_str()];
                auto instance = creator();

                std::shared_ptr<boost::python::object> py_instance = std::shared_ptr<boost::python::object>(
                    new boost::python::object(instance),
                    [] (boost::python::object* obj) { ScopedGilLock lock; delete obj; });

                if(!py_instance->is_none())
                {
                    T* cpp_pointer = boost::python::extract<T*>(*py_instance);
                    cpp_instance.reset(cpp_pointer, [py_instance] (T*) {});
                }
            }
            catch(boost::python::error_already_set&)
            {
                swganh::scripting::logPythonException();
            }

            return cpp_instance;
        }

    private:
        PythonScript();

        void PreparePythonEnvironment_();
        void ReadFileContents_();

        std::string filename_;
        std::string filecontents_;
        std::unique_ptr<boost::python::object, PythonObjectDeleter> file_object_;
        std::unique_ptr<boost::python::object, PythonObjectDeleter> globals_;
        std::unique_ptr<boost::python::object, PythonObjectDeleter> main_;
    };

}}  // namespace swganh::scripting
