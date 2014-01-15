// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#ifdef WIN32

#include <memory>
#include <string>

//#include <Windows.h>

#include <boost/noncopyable.hpp>

#include "anh/plugin/bindings.h"
#include "anh/plugin/platform/library_interface.h"

namespace swganh {
namespace plugin {
namespace platform {

class Win32Library : public LibraryInterface, private boost::noncopyable {
public:
    static std::shared_ptr<Win32Library> Load(std::string library);

public:
    ~Win32Library();
    
    void * GetSymbol(const std::string& symbol);

private:
    Win32Library();
    explicit Win32Library(HMODULE handle);
    
private:
    HMODULE handle_;
    swganh::plugin::ExitFunc exit_func_;
};

}}}  // namespace swganh::plugin::platform

#endif  // WIN32
