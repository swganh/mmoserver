
// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#ifndef SWGANH_OBJECT_OBJECT_MANAGER_H_
#define SWGANH_OBJECT_OBJECT_MANAGER_H_

#include <cstdint>
#include <exception>
#include <map>
#include <memory>
#include <string>
#include <queue>

#include <boost/thread/shared_mutex.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/python/object.hpp>

//#ifdef WIN32
//#include <concurrent_unordered_map.h>
//#include <concurrent_queue.h>
//#else
#include <tbb/concurrent_unordered_map.h>
#include <tbb/concurrent_queue.h>

namespace concurrency
{
using ::tbb::concurrent_unordered_map;
using ::tbb::concurrent_queue;
}
//#endif
#include "anh/app/swganh_kernel.h"
#include "anh/tre/visitors/slots/slot_definition_visitor.h"

#include "ZoneServer\Objects\Object\Object.h"

//#include "swganh_core/object/exception.h"
//#include "swganh_core/object/object_factory_interface.h"
#include "ZoneServer\Objects\object\object_message_builder.h"
#include "ZoneServer\Objects\object\container_interface.h"

#include "ZoneServer\Objects\permissions\permission_type.h"

#define 	gObjectManager	swganh::object::ObjectManager::getSingletonPtr()

namespace swganh
{
namespace object
{

typedef std::map<int, std::shared_ptr<ContainerPermissionsInterface>> PermissionsObjectMap;

class TemplateInterface;
/**
 * ObjectManager is a general interface for managing the object lifecycles for
 * general user defined types. In order to manage an object type an implementation
 * of ObjectFactoryInterface is required.
 */
class ObjectManager
{
public:

    ~ObjectManager();

	static ObjectManager*	getSingletonPtr() {
        return mSingleton;
    }

	static	ObjectManager*	ObjectManager::Init(swganh::app::SwganhKernel*	kernel);
    
    /**
     * Gets the objects definition file data from the resource manager
     *
     */
    std::shared_ptr<swganh::tre::SlotDefinitionVisitor> GetSlotDefinition();

    void LoadSlotsForObject(Object* object);

    void LoadCollisionInfoForObject(Object* object);

    PermissionsObjectMap& GetPermissionsMap();

    virtual void PrepareToAccomodate(uint32_t delta);

private:
    
	  ObjectManager(
        swganh::app::SwganhKernel* kernel
		);

    void InsertObject(Object* object);

    

    typedef std::map<
    uint32_t,
    std::shared_ptr<ObjectMessageBuilder>
    > ObjectMessageBuilderMap;

    typedef std::map<std::string, boost::python::object> PythonTemplateMap;

    /**
     * Registers a message builder for a specific object type
     *
     * @param message_builder The message builder to store.
     */
    template<typename T>
    void RegisterMessageBuilder(std::shared_ptr<T>& message_builder)
    {
        RegisterMessageBuilder(T::type, message_builder);
    }

    /**
     * Registers a message builder for a specific object type
     *
     * @param object_type the type of the object to register a builder to.
     * @param message_builder The message builder to store.
     */
    void RegisterMessageBuilder(uint32_t object_type, std::shared_ptr<ObjectMessageBuilder> message_builder);
    /**
     *  Loads the python object templates and creates any objects inserting them as prototypes
     */
    void LoadPythonObjectTemplates();

    void AddContainerPermissionType_(swganh::object::PermissionType type, swganh::object::ContainerPermissionsInterface* ptr);

    swganh::app::SwganhKernel* kernel_;

    std::shared_ptr<swganh::tre::SlotDefinitionVisitor> slot_definition_;

    ObjectMessageBuilderMap message_builders_;

    PythonTemplateMap object_templates_;

    boost::shared_mutex object_map_mutex_;
    boost::shared_mutex object_factories_mutex_;
    std::unordered_map<uint64_t, std::shared_ptr<Object>> object_map_;
    std::shared_ptr<boost::asio::deadline_timer> persist_timer_;


    PermissionsObjectMap permissions_objects_;

	static ObjectManager*			mSingleton;
    static bool						mInsFlag;

};

}
}  // namespace swganh::object

#endif  // SWGANH_OBJECT_OBJECT_MANAGER_H_
