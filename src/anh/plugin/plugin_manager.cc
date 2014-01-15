// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#include "anh/plugin/plugin_manager.h"

#include <algorithm>
#include <fstream>
#include <iostream>

#include <boost/filesystem.hpp>
#include "anh/logger.h"
#include <boost/program_options.hpp>

#include "anh/app/kernel_interface.h"

#include "anh/plugin/platform/library_interface.h"

#ifdef WIN32
#include "anh/plugin/platform/win32_library.h"
typedef swganh::plugin::platform::Win32Library DynamicLibrary;
#else
#include <dlfcn.h>
#include "swganh/plugin/platform/posix_library.h"
typedef swganh::plugin::platform::PosixLibrary DynamicLibrary;
#endif

#if defined(WIN32)
static std::string library_extension(".dll");
#else
static std::string library_extension(".so");
#endif

using namespace swganh::app;
using namespace swganh::plugin;
using namespace boost::program_options;
using namespace std;

PluginManager::PluginManager(KernelInterface* kernel)
    : kernel_(kernel)
{
#ifndef WIN32
    dlopen(NULL,RTLD_NOW|RTLD_GLOBAL);
#endif
}

PluginManager::~PluginManager()
{
    std::for_each(
        begin(exit_funcs_),
        end(exit_funcs_),
        [] (ExitFunc exit_func)
    {
        try
        {
            (*exit_func)();
        }
        catch(...)
        {
            // Report error here.
        }
    });
}

void PluginManager::LoadPlugin(const std::string& path, std::string plugin_directory)
{
    if (library_map_.find(path) != library_map_.end())
    {
        throw PluginLoadingError("Plugin has already been loaded ["+ path + "]");
    }

    auto library = LoadLibrary_(plugin_directory + path + library_extension);
    if (!library)
    {
        throw PluginLoadingError("Plugin failed to load ["+ path + "]");
    }

    if (ConfigFunc config_func = library->GetSymbol<ConfigFunc>("ConfigurePlugin"))
    {
        ConfigurePlugin(path, config_func);
    }

    InitFunc init_func = library->GetSymbol<InitFunc>("InitializePlugin");
    if (!InitializePlugin(init_func))
    {
        throw PluginLoadingError("Plugin failed to initialize ["+ path + "]");
    }

    library_map_[path] = library;
}

void PluginManager::LoadAllPlugins(const std::string& directory)
{
    boost::filesystem::path plugin_dir(directory);

    try
    {
        if (!boost::filesystem::exists(plugin_dir) ||
                !boost::filesystem::is_directory(plugin_dir))
        {
            throw PluginLoadingError("Plugin failed to load ["+ directory + "]");
        }

        std::for_each(boost::filesystem::directory_iterator(plugin_dir),
                      boost::filesystem::directory_iterator(),
                      [this] (const boost::filesystem::directory_entry& entry)
        {
            if (entry.path().extension() != library_extension)
            {
                return;
            }

            auto native_path = entry.path().native();

            this->LoadPlugin(string(native_path.begin(), native_path.end()));
        });

    }
    catch(const std::exception& e)
    {
        LOG(fatal) << e.what();
    }
}

void PluginManager::ConfigurePlugin(std::string plugin_name, ConfigFunc config_func)
{
    if (!config_func)
    {
        throw runtime_error("Missing configuration function.");
    }

    ifstream config_file("config/plugins/" + plugin_name + ".cfg");

    if (!config_file.is_open())
    {
        DLOG(warning) << "No configuration loaded for plugin: " + plugin_name;
        return;
    }

    variables_map vm;
    options_description description;

    config_func(description);

    try
    {
        store(parse_config_file(config_file, description, true), vm);
    }
    catch(const std::exception& e)
    {
        throw runtime_error("Unable to parse the configuration file at: config/plugins/" + plugin_name + ".cfg: " + std::string(e.what()));
    }

    notify(vm);
    config_file.close();
}

bool PluginManager::InitializePlugin(InitFunc init_func)
{
    if (!init_func)
    {
        return false;
    }

    ExitFunc exit_func = init_func(kernel_);

    if (!exit_func)
    {
        return false;
    }

    exit_funcs_.push_back(exit_func);

    return true;
}

bool PluginManager::RegisterObject(const std::string& name, const ObjectRegistration* registration)
{
    if (name.empty())
    {
        return false;
    }

    if (!registration || !registration->CreateObject || !registration->DestroyObject)
    {
        return false;
    }

    swganh::app::Version version = kernel_->GetVersion();
    if (version.major != registration->version.major)
    {
        return false;
    }

    if (registration_map_.find(name) != registration_map_.end())
    {
        return false;
    }

    registration_map_[name] = *registration;

    return true;
}

const RegistrationMap& PluginManager::registration_map()
{
    return registration_map_;
}

std::shared_ptr<platform::LibraryInterface> PluginManager::LoadLibrary_(const std::string& path)
{
    auto library = DynamicLibrary::Load(path);

    if (!library)
    {
        return nullptr;
    }

    return library;
}
