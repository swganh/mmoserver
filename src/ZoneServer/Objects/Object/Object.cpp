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

#include "ZoneServer/Objects/Object/Object.h"

#include <glm/gtx/transform2.hpp>
#include <exception>

#include "DatabaseManager/Database.h"

#include "NetworkManager/Message.h"
#include "NetworkManager/MessageFactory.h"

#include "MessageLib/MessageLib.h"
#include "MessageLib\messages\update_containment_message.h"
#include "MessageLib\messages\scene_create_object_by_crc.h"
#include "MessageLib\messages\scene_destroy_object.h"

#include "ZoneServer/GameSystemManagers/Container Manager/ContainerManager.h"
#include "ZoneServer\Objects\permissions\container_permissions_interface.h"
#include "ZoneServer/Objects/CraftingTool.h"
#include "ZoneServer\Objects\Object\ObjectManager.h"
#include "ZoneServer/Objects/Player Object/PlayerObject.h"
#include "ZoneServer/WorldManager.h"
#include "ZoneServer/ZoneOpcodes.h"

#include "anh\event_dispatcher\event_dispatcher.h"
#include <anh\app\swganh_kernel.h>
#include "anh/app/swganh_app.h"
#include "anh\crc.h"
#include <ZoneServer\Services\scene_events.h>
#include "ZoneServer\Services\equipment\equipment_service.h"

//=============================================================================

Object::Object()
    : mMovementMessageToggle(true)
    , template_string_("")
    , mLoadState(LoadState_Loading)
    , mId(0)
    , mParentId(0)
    , mEquipSlots(0)
    , mSubZoneId(0)
    , mTypeOptions(0)
    , mDataTransformCounter(0)
	, mStatic(false)
    , zmapCellID(0xffffffff)
	
{
    mDirection = glm::quat();
    mPosition  = glm::vec3();
	
	container_permissions_ = nullptr;

    mObjectController.setObject(this);
	
	object_type_		= SWG_INVALID;
	custom_name_		= std::u16string();
	mName = "";
    mNameFile = "";
}

//=============================================================================

Object::Object(uint64 id,uint64 parentId,std::string model,ObjectType type,const BString name, const BString nameFile)
    : mMovementMessageToggle(true)
    , template_string_(model)
    , mLoadState(LoadState_Loading)
    , mType(type)
    , mId(id)
    , mParentId(parentId)
    , mEquipSlots(0)
    , mSubZoneId(0)
    , mTypeOptions(0)
    , mDataTransformCounter(0)
    , zmapCellID(0xffffffff)
{
    mObjectController.setObject(this);
	custom_name_		= std::u16string();

}

//=============================================================================

Object::~Object()
{
    mAttributeMap.clear();
    
	mInternalAttributeMap.clear();
    mAttributeOrderList.clear();

	/*
    std::for_each(mData.begin(), mData.end(), [] (uint64_t object_id) {
        Object* object = gWorldManager->getObjectById(object_id);
        if (!object)	{
            DLOG(info) << "PANIK !!! ObjectContainer::remove Object : No Object!!!!";
            //assert(false && "ObjectContainer::~ObjectContainer WorldManager unable to find object instance");
            
        }else
		{
			//take care of a crafting tool
			if(CraftingTool* tool = dynamic_cast<CraftingTool*>(object)) {
				if(tool->getCurrentItem())	{
					gWorldManager->removeBusyCraftTool(tool);
				}
			}

			//just deinitialization/removal out of mainObjectMap .. NO removal out of si/cells
			//that has happened long ago plus we now cant send messages anymore anyway
			gWorldManager->eraseObject(object->getId());
		}
    });

    mData.erase(mData.begin(), mData.end());
	*/
}

int32_t Object::GetAppropriateArrangementId(Object* other)
{
    if (slot_descriptor_.size() == 1)
        return -1;

	//LOG(info) << "Object::GetAppropriateArrangementId container -> " << this->getId() << " for object : " << other->getId();

    // Find appropriate arrangement
    int32_t arrangement_id = 4;
    int32_t filled_arrangement_id = -1;
    // In each arrangment
	uint ar_count = 0;
    for ( ObjectArrangements::iterator arrangement = other->slot_arrangements_.begin(); arrangement != other->slot_arrangements_.end(); arrangement++)
    {
		ar_count ++;
		//LOG(info) << "arrangement : " << ar_count;

        bool passes_completely = true;
        bool is_valid = true;
        // Each Slot
		for (std::vector<int32_t>::iterator slot = arrangement->begin(); slot != arrangement->end(); slot++)
        {
			//LOG(info) << "slot : " << *slot;
            // does slot exist in descriptor
            auto descriptor_iter = slot_descriptor_.find(*slot);
			
            if (descriptor_iter == end(slot_descriptor_))
            {
				LOG(info) << "slot descriptor: invalid : " << other->getId() << " : " << other->GetTemplate();
                is_valid = false;
                break;
            }

            // is slot filled?
            else if (descriptor_iter->second->is_filled())
            {
				LOG(info) << "slot descriptor: " << descriptor_iter->first;
                passes_completely = false;
            }
        }
        // if not filled return arrangement
        if (is_valid && passes_completely)
            return arrangement_id;
        else if (is_valid)
            filled_arrangement_id = arrangement_id;

        ++arrangement_id;
    }
    return filled_arrangement_id;
}


int32_t Object::__InternalInsert(Object* object, glm::vec3 new_position, int32_t arrangement_id)
{
	try	{

		Object* removed_object = nullptr;
		if(arrangement_id == -2)
			arrangement_id = GetAppropriateArrangementId(object);

		if (arrangement_id < 4)
		{
			// remove object out of slot in case one is in there
			swganh::object::SlotInterface* slot_descriptor = slot_descriptor_[arrangement_id];
			
			if((slot_descriptor != 0) && (removed_object = slot_descriptor_[arrangement_id]->insert_object(object)))
			{
				// Transfer it out, put it in the place the replacing object came from
				Object* parent = dynamic_cast<Object*>(gWorldManager->getObjectById(object->getParentId()));
				removed_object->__InternalTransfer(nullptr, removed_object, parent);
			}
		}
		else
		{
			auto& arrangement = object->slot_arrangements_[arrangement_id-4];
			for (auto& i = arrangement.begin(); i != arrangement.end(); i++)
			{
				slot_descriptor_[*i]->insert_object(object);
			}
		}
		object->SetArrangementId(arrangement_id);
		object->SetContainer(this);
		object->setParentIdIncDB(this->getId());

		//Time to update the position to the new coordinates/update AABB
		object->mPosition = new_position;


		//Because we may have calculated it internally, send the arrangement_id used back
		//To the caller so it can send the appropriate update.
		return arrangement_id;
	}
	catch(std::exception const& e)	{		
		LOG(error) << "Object::__InternalInsert error" << object->getId();
	}        

}


void Object::__InternalViewObjects(Object* requester, uint32_t max_depth, bool topDown, std::function<void(Object*)> func)
{
	if(!GetPermissions() )	 {
		LOG(error) << "Object::__InternalViewObjects no permissions : " << this->getId();
		auto permissions_objects_ = gObjectManager->GetPermissionsMap();
		
		SetPermissions(permissions_objects_.find(swganh::object::DEFAULT_PERMISSION)->second.get());//DEFAULT_PERMISSION
	}
    //// CHECK PERMISSIONS ////
    if(requester == nullptr || container_permissions_->canView(this, requester))
    {
        uint32_t requester_instance = 0;
        if(requester)
            requester_instance = requester->GetInstanceId();

        //// ITERATE THROUGH ALL SLOTS ////
		
		for(auto& slot = slot_descriptor_.begin();slot != slot_descriptor_.end(); slot++)
        {
			if(!slot->second)	{
				LOG(info) << " No Slot";
			}
			
			//LOG(info) <<"iterating through slot : " << slot->first ;
            slot->second->view_objects([&] (Object* object)
            {
				
                //uint32_t object_instance = object->GetInstanceId();
                //if(object_instance == 0 || object_instance == requester_instance)
                //{
                    if(topDown)
                        func(object);

                    if(max_depth != 1)
                        object->__InternalViewObjects(requester, (max_depth == 0) ? 0 : max_depth-1, topDown, func);

                    if(!topDown)
                        func(object);
                //}
            });
        }
    }
	else
		LOG(info) << "Object::__InternalViewObjects : " << this->getId() << " No Permission :(";
}

void Object::SetSlotInformation(ObjectSlots slots, ObjectArrangements arrangements)
{
    auto lock = AcquireLock();
    SetSlotInformation(slots, arrangements, lock);
}

void Object::SetSlotInformation(ObjectSlots slots, ObjectArrangements arrangements, boost::unique_lock<boost::mutex>& lock)
{
    slot_descriptor_ = slots;
    slot_arrangements_ = arrangements;
}

bool Object::HasSlotInformation()
{
	return (slot_descriptor_.size() > 0);
}

void Object::SwapSlots(Object* requester, Object* object, int32_t new_arrangement_id)
{
    boost::upgrade_lock<boost::shared_mutex> uplock(global_container_lock_);

    {
        boost::upgrade_to_unique_lock<boost::shared_mutex> unique(uplock);
        for(auto& slot = slot_descriptor_.begin();slot != slot_descriptor_.end(); slot++)
        {
            slot->second->remove_object(object);
        }
        __InternalInsert(object, object->mPosition, new_arrangement_id);
    }

    __InternalViewAwareObjects([&] (Object* aware_object)
    {
		gMessageLib->sendContainmentMessage(object->getId(), object->getParentId(), new_arrangement_id, dynamic_cast<PlayerObject*>(aware_object));
        //object->SendUpdateContainmentMessage(object->GetController());
    });
}

void Object::__InternalViewAwareObjects(std::function<void(Object*)> func)
{
	std::for_each(mKnownPlayers.begin(), mKnownPlayers.end(), func); 
}

void Object::InitializeObject(Object* newObject)
{
	auto permissions_objects_ = gObjectManager->GetPermissionsMap();

	if(!newObject->GetPermissions())	 {
		
		newObject->SetPermissions(permissions_objects_.find(swganh::object::DEFAULT_PERMISSION)->second.get());//DEFAULT_PERMISSION
	}

	gObjectManager->LoadSlotsForObject(newObject);
	newObject->setParentId(this->getId());

	int32 arrangement = GetAppropriateArrangementId(newObject);
    if(!InitializeObject(nullptr, newObject, arrangement))	{
		//throw std::runtime_error("Bad Initialization for Object" + newObject->getId());
	}
}

bool Object::InitializeObject(Object* requester, Object* obj, int32_t arrangement_id)
{
    //// CHECK PERMISSIONS ////
	if((requester != nullptr ) && (!container_permissions_->canInsert(this, requester, obj)))	{
		LOG (info) << "Object::AddObject couldnt add Object : " << obj->getId() << " to " << getId();
		return false;
	}

	swganh::object::SlotInterface* slot_descriptor = slot_descriptor_[arrangement_id];
			
	if(!slot_descriptor )	{
		// Transfer it out, put it in the place the replacing object came from
		LOG (info) << "Object::InitializeObject No Slot Descriptor couldnt add Object : " << obj->getId() << " to " << getId();
		return false;
	}

	//auto equipment_service = gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::equipment::EquipmentService>("EquipmentService");

	//LOG (info) << "Object::InitializeObject added Object : " << obj->getId() << " to " << getId();
	//if(arrangement_id != 0xffffffff)
	//	LOG (info) << "slot_descriptor " << slot_descriptor << " : " << equipment_service->GetSlotNameById(arrangement_id);
	//else
	//	LOG (info) << "slot_descriptor " << slot_descriptor ;

	//LOG (info) << "arrangement " << arrangement_id;
	//slot_descriptor->

	
	

    boost::upgrade_lock<boost::shared_mutex> lock(global_container_lock_);

    //Add Object To Datastructure
    {
        boost::upgrade_to_unique_lock<boost::shared_mutex> unique_lock(lock);
        arrangement_id = __InternalInsert(obj, obj->mPosition, arrangement_id);
    }
	
	//add our parents known players to our own
	//this only makes sense on initializing players / creatures with their children before sending their baseline
	std::for_each(mKnownPlayers.begin(), mKnownPlayers.end(), [&] (PlayerObject* player)	{
		obj->mKnownPlayers.insert(player);
		
	});

	obj->SetArrangementId(arrangement_id);

	if (this->getObjectType() == SWG_PLAYER || this->getObjectType() == SWG_CREATURE)	{
		TangibleObject* tangible = dynamic_cast<TangibleObject*>(obj);
		if(tangible)	{
			swganh::object::EquipmentItem item;
			item.containment_type	= tangible->GetArrangementId();
			item.customization		= tangible->getCustomizationStr().getAnsi();
			item.object_id			= tangible->getId();
			item.template_crc		= common::memcrc(tangible->GetTemplate());

			CreatureObject* creature = dynamic_cast<CreatureObject*>(this);
			creature->InitializeEquipmentItem(item);
		}
	}

	return true;
}



bool Object::AddObject(Object* newObject)
{
	uint32 arrangement = GetAppropriateArrangementId(newObject);
    if(AddObject(nullptr, newObject, arrangement))	{
		newObject->SetArrangementId(arrangement);
		return true;
	}
	return false;
}

bool Object::AddObject(Object* requester, Object* obj, int32_t arrangement_id)
{
    //// CHECK PERMISSIONS ////
	if((requester != nullptr ) && (!container_permissions_->canInsert(this, requester, obj)))	{
		LOG (info) << "Object::AddObject couldnt add Object : " << obj->getId() << " to " << getId() << " requester : " << requester->getId();
		return false;
	}

	if (this->getObjectType() == SWG_PLAYER || this->getObjectType() == SWG_CREATURE)	{
		TangibleObject* tangible = dynamic_cast<TangibleObject*>(obj);
		if(tangible)	{
			swganh::object::EquipmentItem item;
			item.containment_type	= tangible->GetArrangementId();
			item.customization		= tangible->getCustomizationStr().getAnsi();
			item.object_id			= tangible->getId();
			item.template_crc		= common::memcrc(tangible->GetTemplate());

			CreatureObject* creature = dynamic_cast<CreatureObject*>(this);
			creature->AddEquipmentItem(item);
		}

		
	}

    boost::upgrade_lock<boost::shared_mutex> lock(global_container_lock_);

    //Add Object To Datastructure
    {
        boost::upgrade_to_unique_lock<boost::shared_mutex> unique_lock(lock);
        arrangement_id = __InternalInsert(obj, obj->mPosition, arrangement_id);
    }

    //Update our observers with the new object
    std::for_each(mKnownPlayers.begin(), mKnownPlayers.end(), [&] (PlayerObject* player)
    {
        obj->__InternalAddAwareObject(player);
        //object->__InternalAddAwareObject(player, true);
    });
	return true;
}

bool Object::RemoveObject(Object* requester, Object* oldObject)
{
    //// CHECK PERMISSIONS ////
	if((requester != nullptr ) && (!container_permissions_->canRemove(this, requester, oldObject)))	{
		LOG (info) << "Object::RemoveObject couldnt remove Object : " << oldObject->getId() << " from " << getId() << " requester : " << requester->getId();
		return false;
	}
	
	if (this->getObjectType() == SWG_PLAYER || this->getObjectType() == SWG_CREATURE)	{
		CreatureObject* creature = dynamic_cast<CreatureObject*>(this);
		creature->RemoveEquipmentItem(oldObject->getId());
	}

    boost::upgrade_lock<boost::shared_mutex> lock(global_container_lock_);

    //Update our observers about the dead object
    std::for_each(mKnownPlayers.begin(), mKnownPlayers.end(), [&] (PlayerObject* player)
    {
		//Object* player = gWorldManager->getObjectById(id);
        oldObject->__InternalRemoveAwareObject(player, true);
    });

    {
        boost::upgrade_to_unique_lock<boost::shared_mutex> unique_lock(lock);

        //Remove Object from Datastructure
        for(auto& slot = slot_descriptor_.begin();slot != slot_descriptor_.end(); slot++)
        {
            slot->second->remove_object(oldObject);
        }
        oldObject->SetContainer(nullptr);
    }
	return true;
}

void Object::__InternalTransfer(Object* requester, Object* object, ContainerInterface* newContainer, int32_t arrangement_id)
{
    // we are already locked
    if(	requester == nullptr || (
                this->GetPermissions()->canRemove(this, requester, object) &&
                newContainer->GetPermissions()->canInsert(newContainer, requester, object)))
    {
        arrangement_id = newContainer->__InternalInsert(object, object->mPosition, arrangement_id);

        //Split into 3 groups -- only ours, only new, and both ours and new
        std::set<Object*> oldObservers, newObservers, bothObservers;

        object->__InternalViewAwareObjects([&] (Object* observer)
        {
            oldObservers.insert(observer);
        });

        newContainer->__InternalViewAwareObjects([&] (Object* observer)
        {
            if(newContainer->GetPermissions()->canView(newContainer, observer))
            {
                auto itr = oldObservers.find(observer);
                if(itr != oldObservers.end())
                {
                    oldObservers.erase(itr);
                    bothObservers.insert(observer);
                }
                else
                {
                    newObservers.insert(observer);
                }
            }
        });

		for(std::set<Object*>::iterator& observer = newObservers.begin(); observer != newObservers.end(); observer++)
        {
			PlayerObject* player = dynamic_cast<PlayerObject* >(*observer);
			if(!player)	{
				LOG(error) << "Object::__InternalTransfer -> couldnt add aware Player";
				continue;
			}

            object->__InternalAddAwareObject(player);
        }

        //Send updates to both
        for(std::set<Object*>::iterator& observer = bothObservers.begin(); observer != bothObservers.end(); observer++)
        {
			gMessageLib->sendContainmentMessage(object->getId(), object->getParentId(), arrangement_id, dynamic_cast<PlayerObject*>(*observer));
        }

        //Send destroys to only ours
        for(std::set<Object*>::iterator& observer = oldObservers.begin(); observer != oldObservers.end(); observer++)
        {
			PlayerObject* player = dynamic_cast<PlayerObject* >(*observer);
			if(!player)	{
				LOG(error) << "Object::__InternalTransfer -> couldnt remove aware Player";
				continue;
			}

            object->__InternalRemoveAwareObject(player, true);
        }


    }
}

void Object::TransferObject(Object* requester, Object* object, ContainerInterface* newContainer, glm::vec3 new_position, int32_t arrangement_id)
{
    //// CHECK PERMISSIONS ////
    if(	requester == nullptr || (
                this->GetPermissions()->canRemove(this, requester, object) &&
                newContainer->GetPermissions()->canInsert(newContainer, requester, object)))
    {
        boost::upgrade_lock<boost::shared_mutex> uplock(global_container_lock_);

        {
            boost::upgrade_to_unique_lock<boost::shared_mutex> unique(uplock);

            //Perform the transfer
            for(auto& slot = slot_descriptor_.begin();slot != slot_descriptor_.end(); slot++)
            {
                slot->second->remove_object(object);
            }

            arrangement_id = newContainer->__InternalInsert(object, new_position, arrangement_id);
        }

        //Split into 3 groups -- only ours, only new, and both ours and new
        std::set<Object*> oldObservers, newObservers, bothObservers;

        object->__InternalViewAwareObjects([&] (Object* observer)
        {
            oldObservers.insert(observer);
        });

        newContainer->__InternalViewAwareObjects([&] (Object* observer)
        {
            if(newContainer->GetPermissions()->canView(newContainer, observer))
            {
                auto itr = oldObservers.find(observer);
                if(itr != oldObservers.end())
                {
                    oldObservers.erase(itr);
                    bothObservers.insert(observer);
                }
                else
                {
                    newObservers.insert(observer);
                }
            }
        });

        //Send Creates to only new
        for(std::set<Object*>::iterator& observer = newObservers.begin(); observer != newObservers.end(); observer++)
        {
			PlayerObject* player = dynamic_cast<PlayerObject* >(*observer);
			if(!player)	{
				LOG(error) << "Object::TransferObject -> couldnt add aware Player";
				continue;
			}

            object->__InternalAddAwareObject(player);
        }

        //Send updates to both
        for(std::set<Object*>::iterator& observer = bothObservers.begin(); observer != bothObservers.end(); observer++)
        {
			gMessageLib->sendContainmentMessage(object->getId(), object->getParentId(), arrangement_id, dynamic_cast<PlayerObject*>(*observer));
        }

        //Send destroys to only ours
        for(std::set<Object*>::iterator& observer = oldObservers.begin(); observer != oldObservers.end(); observer++)
        {
			PlayerObject* player = dynamic_cast<PlayerObject* >(*observer);
			if(!player)	{
				LOG(error) << "Object::TransferObject -> couldnt remove aware Player";
				continue;
			}

            object->__InternalRemoveAwareObject(player, true);
        }
    }
	else
		LOG(info) << "transfer error no permission for object : " << object->getId();
}

void Object::__InternalAddAwareObject(PlayerObject* player)
{
	

    // If we're not already aware of the given object
    auto find_itr = mKnownPlayers.find(player);
	if(find_itr != mKnownPlayers.end())	{
		LOG(error) << "Object::__InternalAddAwareObject " << this->getId() << " -> Player already known : " << player->getId();
		return;
	}

    // Make ourselves a aware of the object
	this->registerWatcher(player);
	gMessageLib->sendCreateTano(dynamic_cast<TangibleObject*>(this), player);   

    // Preserve the old reverse value, and generate the one for this level.
    if(!GetPermissions()->canView(this, player))	{
		return;
	}

    // Iterate through the slots, and handle all the sub-objects
    for(auto& slot = slot_descriptor_.begin();slot != slot_descriptor_.end(); slot++)
    {
        slot->second->view_objects([&] (Object* v)        {
            // If we didn't invalidate viewability, keep going.
                v->__InternalAddAwareObject(player);
        });
    }
}
	

void Object::__InternalRemoveAwareObject(PlayerObject* player, bool reverse_still_valid)
{
	auto find_itr = mKnownPlayers.find(player);
	if(find_itr == mKnownPlayers.end())	{
		LOG(error) << "Object::__InternalRemoveAwareObject " << this->getId() << " -> Player not known : " << player->getId();
		return;
	}

	//unsubscribe
	this->unRegisterWatcher(player);
    
    bool old_reverse = reverse_still_valid;
    reverse_still_valid = GetPermissions()->canView(this, player);

    for(auto& slot = slot_descriptor_.begin();slot != slot_descriptor_.end(); slot++)
    {
        slot->second->view_objects([&] (Object* v)
        {
            if(reverse_still_valid)
                v->__InternalRemoveAwareObject(player, reverse_still_valid);
			
        });
	}
     

}


uint32_t Object::GetInstanceId()
{
    auto lock = AcquireLock();
    return GetInstanceId(lock);
}

uint32_t Object::GetInstanceId(boost::unique_lock<boost::mutex>& lock)
{
    return instance_id_;
}

//=============================================================================

void Object::__InternalGetObjects(Object* requester, uint32_t max_depth, bool topDown, std::list<Object*>& out)
{
    //// CHECK PERMISSIONS ////
    if(requester == nullptr || container_permissions_->canView(this, requester))
    {
        uint32_t requester_instance = 0;
        if(requester)
            requester_instance = requester->GetInstanceId();

        //// ITERATE THROUGH ALL OBJECT SLOTS ////
        for(auto& slot = slot_descriptor_.begin();slot != slot_descriptor_.end(); slot++)
        {
            slot->second->view_objects([&] (Object* object)
            {
                uint32_t object_instance = object->GetInstanceId();
                if(object_instance == 0 || object_instance == requester_instance)
                {
                    if(topDown)
                        out.push_back(object);

                    if(max_depth != 1)
                        object->__InternalGetObjects(requester, (max_depth == 0) ? 0 : max_depth-1, topDown, out);

                    if(!topDown)
                        out.push_back(object);
                }
            });
        }
    }
}


swganh::event_dispatcher::EventDispatcher* Object::GetEventDispatcher()
{
	return gWorldManager->getKernel()->GetEventDispatcher();
}

glm::vec3 Object::getWorldPosition() const
{
    const Object* root_parent = getRootParent();

    // Is this object the root? If so it's position is the world position.
    if (this == root_parent) {
        return mPosition;
    }

    // Get the length of the object's vector inside the root parent (generally a building).
    float length = glm::length(mPosition);

    // Determine the translation angle.
    float theta = glm::angle(root_parent->mDirection) - glm::atan(mPosition.x, mPosition.z);

    // Calculate and return the object's position relative to root parent's position in the world.
    return glm::vec3(
               root_parent->mPosition.x + (sin(theta) * length),
               root_parent->mPosition.y + mPosition.y,
               root_parent->mPosition.z - (cos(theta) * length)
           );
}

//=============================================================================

// @TODO: This is a dependency on WorldManager that could be avoided by having an
//        Object instance hold a reference to it's parent.
// objects reference their parents - we just do not know who is the final (permissiongiving) container
// as it is it will return either the player or the building owning the item regardless in what container it is

const Object* Object::getRootParent() const
{
    // If there's no parent id then this is the root object.
    if (! getParentId())    {
        return this;
    }

	if(this->getObjectType() == SWG_CREATURE)	{
		return this;
	}

    // Otherwise get the parent for this object and call getRootParent on it.
    Object* parent = gWorldManager->getObjectById(getParentId());

    if(!parent)    {
        return this;
    }

    return parent->getRootParent();
}

//=============================================================================

void Object::rotate(float degrees) {
    // Rotate the item left by the specified degrees
    mDirection = glm::rotate(mDirection, degrees, glm::vec3(0.0f, 1.0f, 0.0f));
}

//=============================================================================

void Object::rotateLeft(float degrees) {
    rotate(-degrees);
}

//=============================================================================

void Object::rotateRight(float degrees) {
    rotate(degrees);
}

//=============================================================================

void Object::faceObject(Object* target_object) {
    facePosition(target_object->mPosition);
}

//=============================================================================

void Object::facePosition(const glm::vec3& target_position) {
    // Create a mirror direction vector for the direction we want to face.
    glm::vec3 direction_vector = glm::normalize(target_position - mPosition);
    direction_vector.x = -direction_vector.x;

    // Create a lookat matrix from the direction vector and convert it to a quaternion.
    mDirection = glm::toQuat(glm::lookAt(
                                 direction_vector, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)
                             ));

    // If in the 3rd quadrant the signs need to be flipped.
    if (mDirection.y <= 0.0f && mDirection.w >= 0.0f) {
        mDirection.y = -mDirection.y;
        mDirection.w = -mDirection.w;
    }
}

//=============================================================================

void Object::move(const glm::quat& direction, float distance) {
    // Create a vector of the length we want pointing down the x-axis.
    glm::vec3 movement_vector(0.0f, 0.0f, distance);

    // Rotate the movement vector by the direction it should be facing.
    movement_vector = direction * movement_vector;

    // Add the movement vector to the current position to get the new position.
    mPosition += movement_vector;
}

//=============================================================================

void Object::moveForward(float distance) {
    move(mDirection, distance);
}

//=============================================================================

void Object::moveBack(float distance) {
    move(mDirection, -distance);
}

//=============================================================================

float Object::rotation_angle() const {
    glm::quat tmp = mDirection;

    if (tmp.y < 0.0f && tmp.w > 0.0f) {
        tmp.y *= -1;
        tmp.w *= -1;
    }

    return glm::angle(tmp);
}

BString Object::getBazaarName()
{
    return "";
}


BString Object::getBazaarTang()
{
    return "";
}


ObjectController* Object::getController()
{
    return &mObjectController;
}

//=============================================================================

void Object::sendAttributes(PlayerObject* playerObject)
{
    if(playerObject->getConnectionState() != PlayerConnState_Connected)
        return;

    if(!mAttributeMap.size() || mAttributeMap.size() != mAttributeOrderList.size())
        return;

    Message*	newMessage;
    BString		value;

    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opAttributeListMessage);
    gMessageFactory->addUint64(mId);

    gMessageFactory->addUint32(mAttributeMap.size());

    AttributeMap::iterator			mapIt;
    AttributeOrderList::iterator	orderIt = mAttributeOrderList.begin();

    while(orderIt != mAttributeOrderList.end())
    {
        mapIt = mAttributeMap.find(*orderIt);
        //see if we have to format it properly

        gMessageFactory->addString(gWorldManager->getAttributeKey((*mapIt).first));
        value = (*mapIt).second.c_str();
		if((*mapIt).first == common::memcrc("duration"))
        {
            uint32 time;
            sscanf(value.getAnsi(),"%u",&time);
            //uint32 hour = (uint32)time/3600;
            //time = time - hour*3600;
            uint32 minutes = (uint32)time/60;
            uint32 seconds = time - minutes*60;
            int8 valueInt[64];
            sprintf(valueInt,"%um %us",minutes,seconds);
            value = valueInt;

        }

        value.convert(BSTRType_Unicode16);
        gMessageFactory->addString(value);

        ++orderIt;
    }

    //these should not be necessary in precu they start appearing in cu!!!
    //gMessageFactory->addUint32(0xffffffff);

    newMessage = gMessageFactory->EndMessage();

    //must in fact be send as unreliable for attributes to show during the crafting process!!!
    (playerObject->getClient())->SendChannelAUnreliable(newMessage, playerObject->getAccountId(),CR_Client,9);
}

//=========================================================================

void Object::setAttribute(std::string key,std::string value)
{
	AttributeMap::iterator it = mAttributeMap.find(common::memcrc(key));

    if(it == mAttributeMap.end())
    {
        DLOG(info) << "Object::setAttribute: could not find " << key;
        return;
    }

    (*it).second = value;
}

//=========================================================================
//set the attribute and alter the db

void Object::setAttributeIncDB(std::string key,std::string value)
{
    if(!hasAttribute(key))
    {
        addAttributeIncDB(key,value);
    }

	AttributeMap::iterator it = mAttributeMap.find(common::memcrc(key));

    if(it == mAttributeMap.end())
    {
        DLOG(info) << "Object::setAttribute: could not find " << key;
        return;
    }

    (*it).second = value;

	uint32 attributeID = gWorldManager->getAttributeId(common::memcrc(key));
    if(!attributeID)
    {
        DLOG(info) << "Object::addAttribute DB: no such attribute in the attribute table :" << key;
        return;
    }

	std::stringstream sql;
    sql << "UPDATE " << gWorldManager->getKernel()->GetDatabase()->galaxy()
		<< ".item_attributes SET value ='"
		<< gWorldManager->getKernel()->GetDatabase()->escapeString(value)
		<< "' WHERE item_id = " << this->getId()
		<< " AND attribute_id = " << attributeID;

	gWorldManager->getKernel()->GetDatabase()->executeAsyncSql(sql);

}


//=============================================================================
//adds the attribute to the objects attribute list

void Object::addAttribute(std::string key,std::string value)
{
	mAttributeMap.insert(std::make_pair(common::memcrc(key),value));
    mAttributeOrderList.push_back(common::memcrc(key));
}

//=============================================================================
//adds the attribute to the objects attribute list AND to the db - it needs a valid entry in the attribute table for that

void Object::addAttributeIncDB(std::string key,std::string value)
{
    if(hasAttribute(key))
    {
        setAttributeIncDB(key,value);
        return;
    }

    mAttributeMap.insert(std::make_pair(common::memcrc(key),value));
    mAttributeOrderList.push_back(common::memcrc(key));

    uint32 attributeID = gWorldManager->getAttributeId(common::memcrc(key));
    if(!attributeID)
    {
        DLOG(info) << "Object::addAttribute DB: no such attribute in the attribute table : " << key;
        return;
    }
    int8 sql[512],*sqlPointer,restStr[128];

    sprintf(sql,"INSERT INTO %s.item_attributes VALUES(%"PRIu64",%u,'",gWorldManager->getKernel()->GetDatabase()->galaxy(),this->getId(),attributeID);

    sqlPointer = sql + strlen(sql);
    sqlPointer += gWorldManager->getKernel()->GetDatabase()->escapeString(sqlPointer,value.c_str(),value.length());
    sprintf(restStr,"',%u,0)",static_cast<uint32>(this->getAttributeMap()->size()));
    strcat(sql,restStr);

    gWorldManager->getKernel()->GetDatabase()->executeSqlAsync(0,0,sql);
}

//=============================================================================

bool Object::hasAttribute(std::string key) const
{
    if(mAttributeMap.find(common::memcrc(key)) != mAttributeMap.end())
        return(true);

    return(false);
}

//=============================================================================

void Object::removeAttribute(std::string key)
{
    AttributeMap::iterator it = mAttributeMap.find(common::memcrc(key));

    if(it != mAttributeMap.end())
        mAttributeMap.erase(it);
    else
        DLOG(info) << "Object::removeAttribute: could not find " << key;
}

//=========================================================================

//=========================================================================
//set the attribute and alter the db

void Object::setInternalAttributeIncDB(BString key,std::string value)
{
    if(!hasInternalAttribute(key))
    {
        addInternalAttributeIncDB(key,value);
    }

    AttributeMap::iterator it = mInternalAttributeMap.find(key.getCrc());

    if(it == mInternalAttributeMap.end())
    {
        DLOG(info) << "Object::setAttribute: could not find " << key.getAnsi();
        return;
    }

    (*it).second = value;

    uint32 attributeID = gWorldManager->getAttributeId(key.getCrc());
    if(!attributeID)
    {
        DLOG(info) << "Object::addAttribute DB: no such attribute in the attribute table :" << key.getAnsi();
        return;
    }

    int8 sql[512],*sqlPointer,restStr[128];
//	int8 sql[1024]
    sprintf(sql,"UPDATE %s.item_attributes SET value='",gWorldManager->getKernel()->GetDatabase()->galaxy());

    sqlPointer = sql + strlen(sql);
    sqlPointer += gWorldManager->getKernel()->GetDatabase()->escapeString(sqlPointer,value.c_str(),value.length());
    sprintf(restStr,"'WHERE item_id=%"PRIu64" AND attribute_id=%u",this->getId(),attributeID);
    strcat(sql,restStr);

    gWorldManager->getKernel()->GetDatabase()->executeSqlAsync(0,0,sql);

}

void	Object::setInternalAttribute(BString key,std::string value)
{
    AttributeMap::iterator it = mInternalAttributeMap.find(key.getCrc());

    if(it == mInternalAttributeMap.end())
    {
        DLOG(info) << "Object::setInternalAttribute: could not find " << key.getAnsi();
        return;
    }

    (*it).second = value;
}

//=============================================================================
//adds the attribute to the objects attribute list AND to the db - it needs a valid entry in the attribute table for that

void Object::addInternalAttributeIncDB(BString key,std::string value)
{
    if(hasInternalAttribute(key))
    {
        setInternalAttributeIncDB(key,value);
        return;
    }

    mInternalAttributeMap.insert(std::make_pair(key.getCrc(),value));

    uint32 attributeID = gWorldManager->getAttributeId(key.getCrc());
    if(!attributeID)
    {
        DLOG(info) << "Object::addAttribute DB: no such attribute in the attribute table : " << key.getAnsi();
        return;
    }
    int8 sql[512],*sqlPointer,restStr[128];
//	int8 sql[1024]
    sprintf(sql,"INSERT INTO %s.item_attributes VALUES(%"PRIu64",%u,'",gWorldManager->getKernel()->GetDatabase()->galaxy() ,this->getId(), attributeID);

    sqlPointer = sql + strlen(sql);
    sqlPointer += gWorldManager->getKernel()->GetDatabase()->escapeString(sqlPointer, value.c_str(), value.length());
    sprintf(restStr,"',%u,0)",static_cast<uint32>(this->mInternalAttributeMap.size()));
    strcat(sql,restStr);

    gWorldManager->getKernel()->GetDatabase()->executeSqlAsync(0, 0, sql);
}


//=============================================================================

void Object::addInternalAttribute(BString key,std::string value)
{
    mInternalAttributeMap.insert(std::make_pair(key.getCrc(),value));
}

//=============================================================================

bool Object::hasInternalAttribute(BString key)
{
    if(mInternalAttributeMap.find(key.getCrc()) != mInternalAttributeMap.end())
        return(true);

    return(false);
}

//=============================================================================

void Object::removeInternalAttribute(BString key)
{
    AttributeMap::iterator it = mInternalAttributeMap.find(key.getCrc());

    if(it != mInternalAttributeMap.end())
        mInternalAttributeMap.erase(it);
    else
        DLOG(info) << "Object::removeInternalAttribute: could not find " << key.getAnsi();
}

bool Object::registerWatcher(PlayerObject* const player)
{
    if(!checkRegisteredWatchers(player))    {
        mKnownPlayers.insert(player);
		//DLOG(info) << "Object::registerWatcher :: Player" << player->getId() << " was successfully registered for " << getId();
		return(true);
    }
	DLOG(info) << "Object::registerWatcher :: Player" << player->getId() << " was already watching " << getId();
    return(false);
}

//=============================================================================

void Object::UnregisterAllWatchers()
{
    PlayerObjectSet::iterator	playerIt	= mKnownPlayers.begin();
    mKnownPlayers.erase(mKnownPlayers.begin(), mKnownPlayers.end());
}

bool Object::unRegisterWatcher(PlayerObject* player) 
{
	if (player->GetCreature()->getTargetId() == this->getId()) {
		player->GetCreature()->setTarget(0);
	}

	//DLOG(info) << "Object::unRegisterWatcher unrgister :: Player" << player->getId() << " from " << getId();
	
	Object* us = gWorldManager->getObjectById(this->getId());
	if(us == nullptr){
		DLOG(info) << "Object::unRegisterWatcher WE (" << getId()<< ") dont exist :(!!!!!!";
		return false;
	}
  
	auto it = mKnownPlayers.find(player);
    if (it != mKnownPlayers.end()) {
        mKnownPlayers.erase(it);
        //DLOG(info) << "Object::unRegisterWatcher :: Player" << player->getId() << " was successfully unregistered from " << getId();
        return true;
    }

	//DLOG(info) << "Object::unRegisterWatcher :: Object" << player->getId() << " could not be unregistered for " << getId();
    return false;
}

bool Object::checkRegisteredWatchers(PlayerObject* const player) const {
    PlayerObjectSet::const_iterator it = mKnownPlayers.find(player);
    return (it != mKnownPlayers.end());
}




//=============================================================================
// *this* is obviously a container that gets to hold the item we just created
// we need to create this item to registered players
// please note that the inventory and the datapad handle their own ObjectReady functions!!!!

void Object::handleObjectReady(Object* object,DispatchClient* client)
{

    //==========================
    // reminder: objects are owned by the global map, our item (container) only keeps references
    gWorldManager->addObject(object,true);

    //add it to the spatialIndex, too
    gSpatialIndexManager->createInWorld(object);

	uint32 arrangement = GetAppropriateArrangementId(object);
    this->AddObject(nullptr, object, arrangement);
	object->SetArrangementId(arrangement);

}



//=============================================================================================
// gets a headcount of all tangible (!!!) Objects in the container
// including those contained in containers
uint16 Object::getHeadCount() {
	uint16 count = 0;
    for (auto& descriptor = slot_descriptor_.begin(); descriptor != slot_descriptor_.end(); descriptor ++)	{  	
		count += descriptor->second->get_size();      
	}
	return count;
}

bool Object::checkCapacity(uint8 amount, PlayerObject* player) {
    uint16_t contentCount = getHeadCount();

    if(player&&(mCapacity-contentCount < amount)) {
        gMessageLib->SendSystemMessage(std::u16string(),player,"container_error_message","container3");
    }

    return (mCapacity-contentCount) >= amount;
}




void Object::setCustomName(boost::unique_lock<boost::mutex>& lock, std::u16string name)	{ 
	custom_name_= name; 
	//LOG(info) << "Object::setCustomName : " << name.c_str();
	
	auto dispatcher = GetEventDispatcher();
	dispatcher->Dispatch(std::make_shared<ObjectEvent>("Object::CustomName", (this)));
	}

int32_t Object::GetArrangementId()
{
    return arrangement_id_;
}

void Object::SetArrangementId(int32_t arrangement_id)
{
    arrangement_id_ = arrangement_id;
}

void Object::SetContainer( swganh::object::ContainerInterface* container)
{
    auto lock = AcquireLock();
    SetContainer(container, lock);
}

void Object::SetContainer( swganh::object::ContainerInterface* container, boost::unique_lock<boost::mutex>& lock)
{
    container_ = container;
}

swganh::object::ContainerInterface* Object::GetContainer()
{
    auto lock = AcquireLock();
    return GetContainer(lock);
}

swganh::object::ContainerInterface* Object::GetContainer(boost::unique_lock<boost::mutex>& lock)
{
    return container_;
}

void Object::__InternalGetAbsolutes(glm::vec3& pos, glm::quat& rot)
{
    auto parentContainer = GetContainer();
    if(parentContainer)
    {
        parentContainer->__InternalGetAbsolutes(pos, rot);
    }
    else
    {
        pos = glm::vec3();
        rot = glm::quat();
    }

    auto lock = AcquireLock();
    pos = (rot * mPosition) + pos;
    //rot = rot * mDirection;
	glm::quat d = mDirection;
	
}

std::string Object::GetTemplate()
{
    auto lock = AcquireLock();
    return GetTemplate(lock);
}

std::string Object::GetTemplate(boost::unique_lock<boost::mutex>& lock)
{
    return template_string_;
}

void Object::SetTemplate(const std::string& template_string)
{
    auto lock = AcquireLock();
    SetTemplate(template_string, lock);
}

void Object::SetTemplate(const std::string& template_string, boost::unique_lock<boost::mutex>& lock)
{
    template_string_ = template_string;
//    DISPATCH(Object, Template);
}

//for factory crates
//===============================================
//use only when youre prepared to receive a false result with a not added item
//returns false when the item couldnt be added (container full)

bool Object::addObjectToData(Object* data)
{
	object_data_.push_back(data->getId());
        return true;
}



//=============================================================================

Object* Object::getObjectDataById(uint64 id) {
    auto it = std::find(object_data_.begin(), object_data_.end(), id);

    if (it != object_data_.end()) {
        return gWorldManager->getObjectById(*it);
    }

    return nullptr;
}

//=============================================================================
//just removes it out of the container - the object gets not deleted in the worldmanager
//
bool Object::removeDataObject(Object* data) {
    return removeDataObject(data->getId());
}

//=============================================================================

bool Object::removeDataObject(uint64 id) {
    auto it = std::remove_if(object_data_.begin(), object_data_.end(), [id] (uint64_t object_id) {
        return object_id == id;
    });

    if (it != object_data_.end()) {
        object_data_.erase(it, object_data_.end());
        return true;
    }

    DLOG(info) << "Object::removeDataByPointer Object : Object" << getId() <<" Data "<< id << " not found";

    return false;
}


bool Object::ClearSlot(int32_t slot_id)
{
    auto lock = AcquireLock();
    return ClearSlot(slot_id, lock);
}

bool Object::ClearSlot(int32_t slot_id, boost::unique_lock<boost::mutex>& lock)
{
    bool cleared = false;
    auto slot_iter = slot_descriptor_.find(slot_id);
    if (slot_iter != slot_descriptor_.end())
    {
        auto slot = slot_iter->second;
        if (!slot->is_filled())
        {
            slot->view_objects([&](Object* object)
            {
                slot->remove_object(object);
                cleared = true;
            });

        }
    }
    return cleared;
}

Object* Object::GetSlotObject(int32_t slot_id)
{
    auto lock = AcquireLock();
    return GetSlotObject(slot_id, lock);
}

Object* Object::GetSlotObject(int32_t slot_id, boost::unique_lock<boost::mutex>& lock)
{
    Object* found = nullptr;
    auto slot_iter = slot_descriptor_.find(slot_id);
    if (slot_iter != slot_descriptor_.end())
    {
        auto slot = slot_iter->second;
        slot->view_objects([&](Object* object)
        {
            found = object;
        });
    }
    return found;
}
//=============================================================================


//=============================================================================
Object* Object::itemExist(uint32 familyId, uint32 typeId)
{
    bool found = false;
	Object* the_object = nullptr;
	this->ViewObjects( this, 0, true, [&] (Object* object) {

		Item* item = dynamic_cast<Item*>(object);
        if (item)        {
            if ((item->getItemFamily() == familyId) && (item->getItemType() == typeId))            {
                found = true;
				the_object = object;
            }
        }
    });

    return the_object;
}


void Object::CreateBaselines( PlayerObject* player)
{
	GetEventDispatcher()->Dispatch(std::make_shared<swganh::event_dispatcher::ObserverEvent>("Object::Baselines", this, player));
}

void Object::SendCreateByCrc(PlayerObject* player)
{
    auto lock = AcquireLock();
    SendCreateByCrc(player, lock);
}

void Object::SendCreateByCrc(PlayerObject* player, boost::unique_lock<boost::mutex>& lock)
{
    //DLOG(info) << "SEND [" << GetObjectId() << "] (" << GetTemplate() <<") TO " << observer->GetId();

    swganh::messages::SceneCreateObjectByCrc scene_object;
    scene_object.object_id = getId();
    scene_object.object_crc = swganh::memcrc(GetTemplate(lock));
    scene_object.position = mPosition;
    scene_object.orientation = mDirection;
    scene_object.byte_flag = 0;
	
	
    //observer->Notify(&scene_object);

    SendUpdateContainmentMessage(player, lock, true);
}

void Object::SendUpdateContainmentMessage(PlayerObject* player, bool send_on_no_parent)
{
    auto lock = AcquireLock();
    SendUpdateContainmentMessage(player, lock, send_on_no_parent);
}

void Object::SendUpdateContainmentMessage(PlayerObject* player, boost::unique_lock<boost::mutex>& lock, bool send_on_no_parent)
{
    if(player == nullptr)
        return;

    uint64_t container_id = 0;
    if (auto container = GetContainer(lock))
    {
        lock.unlock();
        container_id = container->getId();
        lock.lock();
    }

    if(send_on_no_parent || container_id != 0)
    {
        //DLOG(info) << "CONTAINMENT " << GetObjectId() << " INTO " << container_id << " ARRANGEMENT " << arrangement_id_;
		swganh::messages::UpdateContainmentMessage containment_message;
        containment_message.container_id = container_id;
        containment_message.object_id = getId();
        containment_message.containment_type = GetArrangementId();
		containment_message.SetObserverId(player->getId());
		gMessageLib->sendMessage(containment_message, player);
		//observer->Notify(&containment_message);
    }
}

void Object::SendDestroy(PlayerObject* player)
{
    auto lock = AcquireLock();
    SendDestroy(player, lock);
}

void Object::SendDestroy(PlayerObject* player, boost::unique_lock<boost::mutex>& lock)
{
    //DLOG(info) << "DESTROY " << GetObjectId() << " FOR " << observer->GetId();

    swganh::messages::SceneDestroyObject scene_object;
    scene_object.object_id = getId();

    //observer->Notify(&scene_object);
}
