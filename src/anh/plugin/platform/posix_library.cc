// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#ifndef WIN32

#include "swganh/plugin/platform/posix_library.h"

#include <exception>
#include <sstream>

#include <dlfcn.h>

#include <boost/filesystem.hpp>

#include "swganh/logger.h"

using namespace swganh::plugin;
using namespace boost::filesystem;
using namespace platform;
using namespace std;

using swganh::plugin::platform::PosixLibrary;

shared_ptr<PosixLibrary> PosixLibrary::Load(std::string library)
{
    path library_path(library);

    void* handle = dlopen(library_path.c_str(), RTLD_LAZY | RTLD_GLOBAL);

    if (handle == NULL)
    {
        LOG(warning) << "Error opening " << library << " library: " << dlerror();
        throw runtime_error("Unable to open posix library: " + library);
    }

    return shared_ptr<PosixLibrary>(new PosixLibrary(handle));
}

PosixLibrary::PosixLibrary(void* handle)
    : handle_(handle)
    , exit_func_(0) {}

PosixLibrary::~PosixLibrary()
{
    if (handle_)
    {
        dlclose(handle_);
    }
}

void * PosixLibrary::GetSymbol(const string& symbol)
{
    if (!handle_)
    {
        return nullptr;
    }

    return dlsym(handle_, symbol.c_str());
}

#endif  // WIN32
