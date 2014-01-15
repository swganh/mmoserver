// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <exception>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "anh/plugin/bindings.h"
#include "anh/logger.h"

namespace swganh {
namespace app {
class KernelInterface;
}}  // namespace swganh::app

namespace swganh {
namespace plugin {

namespace platform {
class LibraryInterface;
}  // namespace platform

typedef std::map<std::string, ObjectRegistration> RegistrationMap;

typedef std::runtime_error MissingSymbolError;
typedef std::runtime_error PluginLoadingError;

class PluginManager {
public:
    explicit PluginManager(swganh::app::KernelInterface* kernel);
    ~PluginManager();

    void LoadPlugin(const std::string& path, std::string plugin_directory = "");
    void LoadAllPlugins(const std::string& directory);
    
    void ConfigurePlugin(std::string plugin_name, ConfigFunc config_func);
    bool InitializePlugin(InitFunc init_func);

    bool RegisterObject(const std::string& name, const ObjectRegistration* registration);

    template<typename T>
    std::shared_ptr<T> CreateObject(const std::string& name);

    const RegistrationMap& registration_map();

private:
    PluginManager();

    std::shared_ptr<platform::LibraryInterface> LoadLibrary_(const std::string& path);
    void * CreateObject_(const std::string& name);

private:
    typedef std::map<std::string, std::shared_ptr<platform::LibraryInterface>> LibraryMap;
    typedef std::vector<ExitFunc> ExitFuncCollection;

    swganh::app::KernelInterface* kernel_;
    LibraryMap library_map_;
    ExitFuncCollection exit_funcs_;

    RegistrationMap registration_map_;
};

template<typename T>
std::shared_ptr<T> PluginManager::CreateObject(const std::string& name) {    
    auto find_iter = registration_map_.find(name);
    std::shared_ptr<T> object = nullptr;

    if (find_iter != registration_map_.end()) {
        auto registration = find_iter->second;

        ObjectParams params;
        params.name = name;
        params.kernel = kernel_;

        object = std::shared_ptr<T>(static_cast<T*>(registration.CreateObject(&params)), registration.DestroyObject);
    }
    else
    {
        // We can't recover from this...
        LOG(fatal) << name + " does not exist, please check config file";
        throw std::runtime_error(name + " does not exist, please check config file");
    }

    return object;
}

}}  // namespace swganh::plugin