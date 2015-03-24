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

#include "VehicleController.h"
#include "ZoneServer/Objects/Creature Object/CreatureObject.h"
#include "Zoneserver/objects/IntangibleObject.h"
#include "Zoneserver/Objects/Player Object/PlayerObject.h"
#include "ZoneServer\Objects\Object\ObjectManager.h"

#include "Zoneserver/GameSystemManagers/State Manager/StateManager.h"
#include "ZoneServer/Objects/MountObject.h"
#include "ZoneServer/WorldManager.h"
#include "MessageLib/MessageLib.h"


#include <anh\app\swganh_kernel.h>
#include <anh\service/service_manager.h>
#include <ZoneServer\Services\terrain\terrain_service.h>
#include "ZoneServer\Services\ham\ham_service.h"

using namespace swganh::terrain;

//very temp
unsigned char Swoop_Customization[99] =	{ 0x61, 0x00, 0x02, 0x11, 0xC3, //customization data
                                        0x9D, 0x28, 0xC3, 0xBF, 0x01, 0xC3, 0x9E, 0xC3, 0x8D, 0xC3, 0xBF, 0x01,
                                        0xC3, 0xA2, 0x1E, 0xC3, 0xBF, 0x01, 0xC3, 0x9B, 0x1D, 0xC3, 0xBF, 0x01,
                                        0xC3, 0x98, 0x2D, 0xC3,	0xBF, 0x01, 0xC3, 0x9A, 0x63, 0xC3, 0xBF, 0x01,
                                        0xC3, 0x99, 0x05, 0xC3, 0xBF, 0x01, 0xC3, 0xA1, 0x19, 0xC3, 0xBF, 0x01,
                                        0xC3, 0xA0, 0x14, 0xC3, 0xBF, 0x01, 0xC3, 0x9F, 0x05, 0xC3, 0xBF, 0x01,
                                        0xC3, 0x9C, 0x46, 0xC3, 0xBF, 0x01, 0xC3, 0x95, 0x19, 0xC3, 0xBF, 0x01,
                                        0x01, 0x04, 0xC3, 0x96, 0x67, 0xC3, 0xBF, 0x01, 0x22, 0x11, 0xC3, 0x97,
                                        0xC3, 0x9B, 0xC3, 0xBF, 0x01, 0x02, 0x11, 0xC3, 0xBF, 0x03
                                        };


VehicleController::VehicleController()
    : IntangibleObject()
    , owner_(0)
    , body_(0)
    , flat_acceleration_(0)
    , hit_point_loss_(0)
    , incline_acceleration_(0) {

    // Over-ride base settings.
    mType       = ObjType_Intangible;
    mItnoGroup  = ItnoGroup_Vehicle;
    mName       = L"";
}

VehicleController::~VehicleController() {
    if (body_)	{
        gWorldManager->destroyObject(body_);
    }
}

//=============================================================================
//handles the radial selection

void VehicleController::handleObjectMenuSelect(uint8_t message_type, Object* source_object) {

    if(dynamic_cast<PlayerObject*>(source_object)) {
        switch(message_type)	{
            case radId_vehicleGenerate:
            case radId_vehicleStore:
            {
				// If a body for the vehicle exists then store it, if it doesn't then call it.
				if (body_) {

				  Store();
				} 
				else {

					Call();
				}
            }
            break;

            default:
            {
				//assert(false && "Vehicle::handleObjectMenuSelect Unknown radial selection!");
				LOG(error) << "VehicleController::handleObjectMenuSelect Unknown radial selection for : " << this->getId();
            }
            break;
        }
    }
}

//=============================================================================
//handles the radial selection
void VehicleController::prepareCustomRadialMenu(CreatureObject* creature, uint8_t item_count) {

	mRadialMenu = std::make_shared<RadialMenu>();

    mRadialMenu->addItem(1, 0, radId_vehicleGenerate, radAction_ObjCallback, "@pet/pet_menu:menu_call");
    mRadialMenu->addItem(2, 0, radId_itemDestroy, radAction_Default);
    mRadialMenu->addItem(3, 0, radId_examine, radAction_Default);
}


//===============================================================================================
//spawns the physical body (CreatureObject)

void VehicleController::Call() {
    if(body_)	{
        assert(false && "void Vehicle::call() body already exists");
        return;
    }

	PlayerObject* owner = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(owner_));
	
    if(owner->checkIfMountCalled()) {
		LOG(info) << "VehicleController::Call : body already in world " << this->getId();
        return;
    }

    if(!owner->isConnected() || owner->GetCreature()->isDead() || owner->GetCreature()->isIncapacitated()) {
		LOG(info) << "VehicleController::Call : owner statecheck fail for " << this->getId();
        return;
    }

    body_ = new MountObject();

    BString cust;
    cust.initRawBSTR((int8*)Swoop_Customization, BSTRType_ANSI);
    body_->setCustomizationStr("");//cust.getAnsi());
    body_->setCreoGroup(CreoGroup_Vehicle);
    body_->setTypeOptions(0x1080);
    body_->setMoodId(0);
    body_->setCL(0);

    body_->setId(mId + 1);	// Vehicles are created by the VehicleControllerFactory with +2 step for IDs

    body_->set_controller(this->getId());

    body_->set_owner(owner->getId());
    body_->setParentId(0);
    body_->SetTemplate(mPhysicalModel);
    body_->setSpeciesGroup(mNameFile.getAnsi());
    body_->setSpeciesString(mName.getAnsi());
    body_->SetPosture(0);
    body_->setScale(1.0f);

	std::string con = this->getAttribute<std::string>("condition");
	uint32 health_current = atoi(con.substr(0,con.find_first_of("/")).c_str());
	uint32 health_max = atoi(con.substr(con.find_first_of("/")+1,con.find_first_of("/")).c_str());

	for(int8 i = 0; i<9;i++)	{
		body_->InitStatBase(0);
		body_->InitStatCurrent(0);
		body_->InitStatMax(0);
		body_->InitStatWound(0);
	}

	body_->InitStatCurrent(HamBar_Health, health_current);
	body_->InitStatMax(HamBar_Health, health_max);
	
    owner->setMount(body_);
    owner->setMounted(false);
    owner->setMountCalled(false);

    // Set default direction and position for the body.
	body_->mDirection = owner->GetCreature()->mDirection;
    body_->mPosition  = owner->GetCreature()->mPosition;

    // Move it forward 2 meters
    body_->moveForward(2);
	
	auto terrain = gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::terrain::TerrainService>("TerrainService");

<<<<<<< HEAD:src/ZoneServer/Objects/VehicleController.cpp
	gObjectManager->LoadSlotsForObject(body_);
	
	body_->mPosition.y =	terrain->GetHeight(gWorldManager->getZoneId(), body_->mPosition.x,body_->mPosition.z) + 0.3;
=======
    // And drop it a little below the terrain to allow the client to normalize it.
    //body_->mPosition.y = Heightmap::getSingletonPtr()->getHeight(body_->mPosition.x, body_->mPosition.z) - 0.3f;
    //TODO: Remove this patch when heightmaps are corrected!
    if(owner_) {
        float hmapHighest = Heightmap::getSingletonPtr()->getHeight(body_->mPosition.x, body_->mPosition.z) - 0.3f;
        body_->mPosition.y = gHeightmap->compensateForInvalidHeightmap(hmapHighest, body_->mPosition.y, (float)10.0);
        if(hmapHighest != body_->mPosition.y) {
            DLOG(INFO) << " VehicleController::Call: PlayerID("<<owner_->getId() << ") calling vehicle... Heightmap found inconsistent, compensated height.";
        }
    }//end TODO
>>>>>>> parent of 5bd772a... got rid of google log:src/ZoneServer/VehicleController.cpp

    // Finally rotate it perpendicular to the player.
    body_->rotateRight(90.0f);

    // add to world
    if(!gWorldManager->addObject(body_)) {
		DLOG(INFO) << "void Vehicle::call() creating vehicle with id "<<body_->getId()<<" failed : couldnt add to world";
        SAFE_DELETE(body_);
        return;
    }

	auto permissions_objects_ = gObjectManager->GetPermissionsMap();
	body_->SetPermissions(permissions_objects_.find(swganh::object::RIDEABLE_PERMISSION)->second.get());//CREATURE_PERMISSION

	//currently done by gWorldManager->addObject(body_)
	//gSpatialIndexManager->createInWorld(body_);

    gMessageLib->sendUpdateTransformMessage(body_);

    owner->setMountCalled(true);

    return;
}


//===============================================================================================
//stores the physical body
void VehicleController::Store()
{
<<<<<<< HEAD:src/ZoneServer/Objects/VehicleController.cpp
    if(!body_)    {
        DLOG(info) << "Vehicle::store() Error: Store was called for a nonexistant body object!";
        return;
    }

	PlayerObject* owner = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(owner_));

    if(!owner_ || owner->GetCreature()->isDead() || owner->GetCreature()->isIncapacitated())    {
        DLOG(info) << "Vehicle::store() couldnt find owner";
=======
    if(!body_)
    {
        DLOG(INFO) << "Vehicle::store() Error: Store was called for a nonexistant body object!";
        return;
    }

    if(!owner_ || owner_->isDead() || owner_->isIncapacitated())
    {
        DLOG(INFO) << "Vehicle::store() couldnt find owner";
<<<<<<< HEAD:src/ZoneServer/Objects/VehicleController.cpp
>>>>>>> parent of 5bd772a... got rid of google log:src/ZoneServer/VehicleController.cpp
=======
>>>>>>> parent of 5bd772a... got rid of google log:src/ZoneServer/VehicleController.cpp
        return;
    }

    // todo auto dismount
    if(owner->checkIfMounted())
    {
		DLOG(info) << "Vehicle::store() dismounted";
        DismountPlayer();
    }

    if(!owner->checkIfMountCalled())
    {
        DLOG(INFO) << "Vehicle::store() Mount wasnt called !!!";
        return;
    }

	//the body is a creature_object!!!
	gSpatialIndexManager->RemoveObjectFromWorld(body_);

    owner->setMount(NULL);

    owner->setMounted(false);
    owner->setMountCalled(false);

    // finally unload & destroy the vehicle creature
    gWorldManager->destroyObject(body_);

    // null the reference
    body_ = NULL;

}


//===============================================================================================
//dismount the owner from the physical body

void VehicleController::DismountPlayer() {
    if(!body_) {
		LOG(error) << "Vehicle::DismountPlayer() no vehicle body!";
        return;
    }

	LOG(error) << "Vehicle::DismountPlayer()";

	PlayerObject* owner = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(owner_));
	CreatureObject*		owner_creature = owner->GetCreature();

    if(!owner->checkIfMounted()) {
        LOG(error) << "Vehicle::DismountPlayer() no vehicle body!";
        return;
    }

    //see elaboration downstairs
	// body_->RemoveObject(owner, owner_creature);
	gMessageLib->sendContainmentMessage_InRange(owner_creature->getId(), 0, 0, owner_creature);
	//gMessageLib->sendUpdateTransformMessage(body_);

    // TODO: make this more automatic...
    gStateManager.removeActionState(owner_creature, CreatureState_RidingMount);   
	gStateManager.setCurrentLocomotionState(owner_creature, CreatureLocomotion_Standing);
	gStateManager.setCurrentPostureState(owner_creature, CreaturePosture_Upright);

	gStateManager.removeActionState(body_, CreatureState_MountedCreature);   

    owner->setMounted(false);

    gMessageLib->sendUpdateMovementProperties(owner);
}

//===============================================================================================
//mount the owner on the physical body
//

void VehicleController::MountPlayer()
{
    if (!body_) {
		assert(false && "Vehicle::mountPlayer() no vehicle body!");
        return;
    }
	
	PlayerObject*		owner = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(owner_));
	CreatureObject*		owner_creature = owner->GetCreature();
    
	//Make the mount equip the player
	// body_->AddObject(owner_creature);
	//ok this makes hughe problems starting with auto equipping the player crc in the creo (the naked woman riding behind me)
	//and ending with the question on how to send the auto updates for the creates / destroys as we basically transfer into the main cell (which isnt existent in our core)	
	
	gMessageLib->sendContainmentMessage_InRange(owner_creature->getId(), body_->getId(), 4, owner_creature);
  
	gStateManager.setCurrentActionState(body_, CreatureState_MountedCreature);   

	//gStateManager.removeActionState(owner_creature,CreatureState_ClearState);
    gStateManager.setCurrentActionState(owner_creature,CreatureState_RidingMount);
    
	gStateManager.setCurrentPostureState(owner_creature,CreaturePosture_DrivingVehicle);
    gStateManager.setCurrentLocomotionState(owner_creature,CreatureLocomotion_DrivingVehicle);

    owner->setMounted(true);
    gMessageLib->sendUpdateMovementProperties(owner);

	//gMessageLib->sendUpdateTransformMessage(body_);
}

//===============================================================================================
