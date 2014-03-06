// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#ifndef WIN32
#include <Python.h>
#endif

#include "ZoneServer\Objects\Object\ObjectManager.h"

#include <boost/asio.hpp>
#include <boost/python.hpp>

#include "anh/logger.h"

#include <bitset>
#include <sstream>

#include "ZoneServer\Objects\Object\ObjectFactory.h"
#include "anh\event_dispatcher\event_dispatcher.h"

#include "anh/scripting/python_script.h"
#include "anh/tre/resource_manager.h"
#include "anh/tre/visitors/objects/object_visitor.h"
#include "anh/tre/visitors/slots/slot_arrangement_visitor.h"
#include "anh/tre/visitors/slots/slot_descriptor_visitor.h"

#include "ZoneServer\Objects\Object\slot_container.h"
#include "ZoneServer\Objects\Object\slot_exclusive.h"

//#include "swganh_core/object/template_interface.h"

//#include "swganh/database/database_manager.h"
#include <cppconn/exception.h>
#include <cppconn/connection.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/sqlstring.h>

#include "ZoneServer\Objects\permissions\default_permission.h"
#include "ZoneServer\Objects\permissions\world_permission.h"
#include "ZoneServer\Objects\permissions\static_container_permission.h"
#include "ZoneServer\Objects\permissions\creature_permission.h"
#include "ZoneServer\Objects\permissions\creature_container_permission.h"
#include "ZoneServer\Objects\permissions\ridable_permission.h"
#include "ZoneServer\Objects\permissions\world_cell_permission.h"
#include "ZoneServer\Objects\permissions\no_view_permission.h"

//#include "collision_data.h"

using namespace std;
using namespace swganh;
using namespace swganh::tre;
using namespace swganh::object;
using namespace swganh::messages;
namespace bp = boost::python;


void ObjectManager::AddContainerPermissionType_(PermissionType type, ContainerPermissionsInterface* ptr)
{
    permissions_objects_.insert(std::make_pair<int, std::shared_ptr<ContainerPermissionsInterface>>(static_cast<int>(type),
                                shared_ptr<ContainerPermissionsInterface>(ptr)));
}

bool					ObjectManager::mInsFlag    = false;
ObjectManager*	ObjectManager::mSingleton  = NULL;

//======================================================================================================================

ObjectManager*	ObjectManager::Init(swganh::app::SwganhKernel*	kernel)
{
    if(!mInsFlag)
    {
        mSingleton	= new ObjectManager(kernel);
        mInsFlag	= true;

        return mSingleton;
    }
    else
        return mSingleton;
}

//=============================================================================




ObjectManager::ObjectManager(swganh::app::SwganhKernel* kernel)
    : kernel_(kernel)
{
    //Load Permissions
    AddContainerPermissionType_(DEFAULT_PERMISSION, new DefaultPermission());
    AddContainerPermissionType_(WORLD_PERMISSION, new WorldPermission());
    AddContainerPermissionType_(STATIC_CONTAINER_PERMISSION, new StaticContainerPermission());
    AddContainerPermissionType_(WORLD_CELL_PERMISSION, new WorldCellPermission());
    AddContainerPermissionType_(CREATURE_PERMISSION, new CreaturePermission());
    AddContainerPermissionType_(CREATURE_CONTAINER_PERMISSION, new CreatureContainerPermission());
    AddContainerPermissionType_(RIDEABLE_PERMISSION, new RideablePermission());
    AddContainerPermissionType_(NO_VIEW_PERMISSION, new NoViewPermission());

    //Load slot definitions
    slot_definition_ = kernel->GetResourceManager()->GetResourceByName<SlotDefinitionVisitor>("abstract/slot/slot_definition/slot_definitions.iff");

    //persist_timer_ = std::make_shared<boost::asio::deadline_timer>(kernel_->GetCpuThreadPool(), boost::posix_time::seconds(30));
    //persist_timer_->async_wait(boost::bind(&ObjectManager::PersistObjectsByTimer, this, boost::asio::placeholders::error));

    
    //LoadPythonObjectTemplates();
}

ObjectManager::~ObjectManager()
{}


std::shared_ptr<swganh::tre::SlotDefinitionVisitor>  ObjectManager::GetSlotDefinition()
{
    return slot_definition_;
}

/*
void ObjectManager::LoadCollisionInfoForObject(std::shared_ptr<Object> obj)
{
    if(auto obj_visitor = kernel_->GetResourceManager()->GetResourceByName<ObjectVisitor>(obj->GetTemplate()))
    {
        obj_visitor->load_aggregate_data(kernel_->GetResourceManager());

        if(obj_visitor->has_attribute("collisionLength") && obj_visitor->has_attribute("collisionHeight"))
        {
            obj->SetCollisionBoxSize(obj_visitor->attribute<float>("collisionLength") / 2.0f, obj_visitor->attribute<float>("collisionLength") / 2.0f);
            obj->SetCollidable(true);
        }
        else
            obj->SetCollidable(false);
    }
}
*/

void ObjectManager::LoadSlotsForObject(Object* object)
{

	try	{
		
		auto oiff = kernel_->GetResourceManager()->GetResourceByName<ObjectVisitor>(object->GetTemplate());
		if(oiff == nullptr)	{
			LOG(error) << "ObjectManager::LoadSlotsForObject No Object Visitor";
			return;
		}
	

		oiff->load_aggregate_data(kernel_->GetResourceManager());

		if(oiff->has_attribute("arrangementDescriptorFilename") &&
				oiff->has_attribute("slotDescriptorFilename"))
		{
			auto arrangmentDescriptor = kernel_->GetResourceManager()->GetResourceByName<SlotArrangementVisitor>(
											oiff->attribute<std::string>("arrangementDescriptorFilename"));

			auto slotDescriptor = kernel_->GetResourceManager()->GetResourceByName<SlotDescriptorVisitor>(
									  oiff->attribute<std::string>("slotDescriptorFilename"));

			ObjectArrangements arrangements;

			// arrangements
			if (arrangmentDescriptor != nullptr)
			{
				for_each(arrangmentDescriptor->begin(), arrangmentDescriptor->end(), [&](std::vector<std::string> arrangement)
				{
					std::vector<int32_t> arr;
					std::vector<std::string>::iterator& str = arrangement.begin();
				
					for (str = arrangement.begin(); str != arrangement.end(); str++)
					{
						arr.push_back(slot_definition_->findSlotByName(*str));
					}
					arrangements.push_back(arr);
				});
			}
			ObjectSlots descriptors;

			// Globals
			//
			descriptors.insert(ObjectSlots::value_type(-1, new SlotContainer()));

			// Descriptors
			if (slotDescriptor != nullptr)
			{
				for ( size_t j = 0; j < slotDescriptor->available_count(); ++j)
				{
					auto descriptor = slotDescriptor->slot(j);
					size_t id = slot_definition_->findSlotByName(descriptor);
					auto entry = slot_definition_->entry(id);
					if(entry.exclusive)
						descriptors.insert(ObjectSlots::value_type(id, new SlotExclusive()));
					else
						descriptors.insert(ObjectSlots::value_type(id, new SlotContainer()));
				}
			}

			object->SetSlotInformation(descriptors, arrangements);
		}
	}

	catch(std::exception const& e)	{		
		std::cout << "Exception: " << e.what() << "\n";
	}
}

PermissionsObjectMap& ObjectManager::GetPermissionsMap()
{
    return permissions_objects_;
}

void ObjectManager::PrepareToAccomodate(uint32_t delta)
{
    //boost::lock_guard<boost::shared_mutex> lg(object_map_mutex_);
    //object_map_.reserve(object_map_.size() + delta);
}

void ObjectManager::LoadPythonObjectTemplates()
{
    LOG(info) << "Loading Template Objects";
    swganh::scripting::PythonScript script(kernel_->GetAppConfig().script_directory + "/load_objects.py", true);

    script.SetGlobal("kernel", bp::ptr(kernel_));
    script.Run();

    object_templates_ = script.GetGlobalAs<PythonTemplateMap>("templates");
}
