/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2014 The SWG:ANH Team
---------------------------------------------------------------------------------------
Use of this source code is governed by the GPL v3 license that can be found
in the COPYING file or at http://www.gnu.org/licenses/gpl-3.0.html

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
---------------------------------------------------------------------------------------
*/

#include <memory>
#include <cstdint>

#include "anh\event_dispatcher\event_dispatcher.h"

#include "MessageLib\messages\deltas_message.h"
#include "MessageLib\messages\baselines_message.h"
#include "MessageLib\messages\scene_end_baselines.h"

#include "ZoneServer\Objects\Object\object_message_builder.h"
#include "ZoneServer\Objects\Object\Object.h"
#include "ZoneServer\Objects\Object\Object_Enums.h"
#include "ZoneServer\PlayerEnums.h"

#include "MessageLib/MessageLib.h"


using namespace swganh::event_dispatcher;

void ObjectMessageBuilder::RegisterEventHandlers()
{
	event_dispatcher_->Subscribe("Object::CustomName", [this] (const std::shared_ptr<EventInterface>& incoming_event)
    {
        auto value_event = std::static_pointer_cast<ObjectEvent>(incoming_event);
        BuildCustomNameDelta(value_event->Get());
    });

	event_dispatcher_->Subscribe("Object::Baselines", [this] (const std::shared_ptr<EventInterface>& incoming_event)
    {
        auto observer_event = std::static_pointer_cast<ObserverEvent>(incoming_event);
        SendBaselines(observer_event->object, observer_event->observer);
    });
	
}

void ObjectMessageBuilder::SendEndBaselines(const Object* object, const Object* observer)
{
    swganh::messages::SceneEndBaselines scene_end_baselines;
    scene_end_baselines.object_id = object->getId();
    //gMessageLib->
}

void ObjectMessageBuilder::BuildCustomNameDelta(const Object* object)
{
 
	if(object->getObjectType() == SWG_INVALID)	{
		LOG(error) << "ObjectMessageBuilder::BuildCustomNameDelta :	invalid Object Type for : " << object->getId();
		return;
	}

    swganh::messages::DeltasMessage message = CreateDeltasMessage(object, VIEW_3, 2, object->getObjectType());
    message.data.write(object->getCustomName());

    gMessageLib->broadcastDelta(message, object);
}

swganh::messages::BaselinesMessage BaseMessageBuilder::CreateBaselinesMessage(const Object* object, boost::unique_lock<boost::mutex>& lock, uint8_t view_type, uint16_t opcount)
{
    swganh::messages::BaselinesMessage message;
    message.object_id = object->getId();
	message.object_type = object->getObjectType();
    message.view_type = view_type;
    message.object_opcount = opcount;

    return message;
}



//swganh::messages::DeltasMessage ObjectMessageBuilder::CreateDeltasMessage(const std::shared_ptr<Object>& object,  uint8_t view_type, uint16_t update_type, uint16_t update_count)
swganh::messages::DeltasMessage BaseMessageBuilder::CreateDeltasMessage(const Object* object,  uint8_t view_type, uint16_t update_type, uint32_t object_type, uint16_t update_count)
{
    swganh::messages::DeltasMessage message;
	
	//ok this is a stupid hack but with our current setup I cannot do much better
	uint64_t object_id = object->getId();
	if(object_type == SWG_PLAYER){
		object_id += PLAYER_OFFSET;
	}
	message.object_id = object_id;
	message.object_type = object_type;
    message.view_type = view_type;
    message.update_count = update_count;
    message.update_type = update_type;
    return message;
}

boost::optional<swganh::messages::BaselinesMessage> ObjectMessageBuilder::BuildBaseline3(Object* object, boost::unique_lock<boost::mutex>& lock)
{
    auto message = CreateBaselinesMessage(object, lock, VIEW_3);
	float c = 1.0;
    message.data.write(c);//object-> >GetComplexity(lock));
	message.data.write(object->getNameFile().getAnsi());// >GetStfNameFile(lock));
    message.data.write<uint32_t>(0); // spacer
	message.data.write(object->getName().getAnsi());// >GetStfNameString(lock));
	message.data.write(object->getCustomName());//  >GetCustomName(lock));
    message.data.write(1);// the volume in the inventory object->GetVolume(lock));
    return swganh::messages::BaselinesMessage(std::move(message));
}

boost::optional<swganh::messages::BaselinesMessage> ObjectMessageBuilder::BuildBaseline6(Object* object, boost::unique_lock<boost::mutex>& lock)
{
    auto message = CreateBaselinesMessage(object, lock, VIEW_6);
    message.data.write(0);//object->GetSceneId(lock));
    return swganh::messages::BaselinesMessage(std::move(message));
}
