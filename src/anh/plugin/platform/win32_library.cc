// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#ifdef WIN32

#include "anh/plugin/platform/win32_library.h"

#include <exception>
#include <sstream>

#include <Windows.h>

#include <boost/filesystem.hpp>

using namespace swganh::plugin;
using namespace boost::filesystem;
using namespace platform;
using namespace std;

shared_ptr<Win32Library> Win32Library::Load(std::string library)
{
    path library_path(library);

    HMODULE handle = ::LoadLibrary(library_path.string().c_str());

    if (handle == NULL)
    {
        DWORD error_code = ::GetLastError();

        stringstream ss;
        ss << "LoadLibrary(" << library << ") Failed. errorCode: " << error_code;

        throw runtime_error(ss.str());
    }

    return shared_ptr<Win32Library>(new Win32Library(handle));
}

Win32Library::Win32Library(HMODULE handle)
    : handle_(handle)
    , exit_func_(0) {}

Win32Library::~Win32Library()
{
    if (handle_)
    {
        ::FreeLibrary(handle_);
    }
}

void * Win32Library::GetSymbol(const string& symbol)
{
    if (!handle_)
    {
        return nullptr;
    }
    return ::GetProcAddress(handle_, symbol.c_str());
}

#endif  // WIN32
