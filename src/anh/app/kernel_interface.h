// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <utils/typedefs.h>

#include <boost/asio/io_service.hpp>

class MessageDispatch;

namespace swganh {
namespace database {
    class Database;
}}  // namespace swganh::database

namespace swganh {
namespace event_dispatcher	{
    class EventDispatcher;
}}  // namespace swganh

namespace swganh {
namespace plugin {
    class PluginManager;
}}  // namespace swganh::plugin

namespace swganh {
namespace resource {
    class ResourceManagerInterface;
}}

namespace swganh {
namespace service {
    class ServiceDirectoryInterface;
    class ServiceManager;
}}  // namespace swganh::service

namespace swganh {
namespace app {

struct Version {
    uint64 major;
    uint64 minor;

    std::string ToString() const {
        return  std::to_string(major) + "." + std::to_string(minor);
    }
};

class KernelInterface {
public:
    virtual ~KernelInterface() {}

    virtual const Version& GetVersion() = 0;
    
    virtual swganh::event_dispatcher::EventDispatcher* GetEventDispatcher() = 0;
    
    virtual swganh::plugin::PluginManager* GetPluginManager() = 0;

    virtual swganh::service::ServiceManager* GetServiceManager() = 0;
	
	virtual swganh::service::ServiceDirectoryInterface* GetServiceDirectory() = 0;

    virtual swganh::database::Database* GetDatabase() = 0;

	virtual MessageDispatch* GetDispatch() = 0;
    
    virtual boost::asio::io_service& GetIoThreadPool() = 0;

	virtual boost::asio::io_service& GetCpuThreadPool() = 0;

    // also add entity manager, blah blah.
};

}}  // namespace swganh::app
