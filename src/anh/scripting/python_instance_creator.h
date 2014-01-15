// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <memory>
#include <string>

#include "python_script.h"

namespace swganh {
namespace scripting {

    /**
     * PythonInstanceCreator is a factory style function object for creating
     * python instances of C++ interfaces.
     */
    template<typename T>
    class PythonInstanceCreator
    {
    public:
        PythonInstanceCreator(std::string script_path, std::string class_name)
            : script_(std::make_shared<PythonScript>(script_path))
            , class_name_(class_name)
        {}

        std::shared_ptr<T> operator() ()
        {
            return script_->CreateInstance<T>(class_name_);
        }

    private:
        std::shared_ptr<PythonScript> script_;
        std::string class_name_;
    };

}}
