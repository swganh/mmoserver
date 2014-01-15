// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

namespace swganh {
namespace plugin {
namespace platform {

class LibraryInterface {
public:
    virtual ~LibraryInterface() {}
    
    template<typename T>
    T GetSymbol(const std::string& symbol) {
        T func_ptr;
        *reinterpret_cast<void**>(&func_ptr) = GetSymbol(symbol);
        return func_ptr;
    }

    virtual void * GetSymbol(const std::string& symbol) = 0;
};

}}}  // namespace swganh::plugin::platform;
