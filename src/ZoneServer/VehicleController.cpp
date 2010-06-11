/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2010 The SWG:ANH Team
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
#include "CreatureObject.h"
#include "IntangibleObject.h"
#include "PlayerObject.h"
#include "MountObject.h"
#include "Heightmap.h"
#include "WorldManager.h"
#include "MessageLib/MessageLib.h"


//very temp
unsigned char Swoop_Customization[99] =	{ 0x61, 0x00, 0x02, 0x11, 0xC3, //customization data
0x9D, 0x28, 0xC3, 0xBF, 0x01, 0xC3, 0x9E, 0xC3, 0x8D, 0xC3, 0xBF, 0x01,
0xC3, 0xA2, 0x1E, 0xC3, 0xBF, 0x01, 0xC3, 0x9B, 0x1D, 0xC3, 0xBF, 0x01,
0xC3, 0x98, 0x2D, 0xC3,	0xBF, 0x01, 0xC3, 0x9A, 0x63, 0xC3, 0xBF, 0x01,
0xC3, 0x99, 0x05, 0xC3, 0xBF, 0x01, 0xC3, 0xA1, 0x19, 0xC3, 0xBF, 0x01,
0xC3, 0xA0, 0x14, 0xC3, 0xBF, 0x01, 0xC3, 0x9F, 0x05, 0xC3, 0xBF, 0x01,
0xC3, 0x9C, 0x46, 0xC3, 0xBF, 0x01, 0xC3, 0x95, 0x19, 0xC3, 0xBF, 0x01,
0x01, 0x04, 0xC3, 0x96, 0x67, 0xC3, 0xBF, 0x01, 0x22, 0x11, 0xC3, 0x97,
0xC3, 0x9B, 0xC3, 0xBF, 0x01, 0x02, 0x11, 0xC3, 0xBF, 0x03 };


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
        } else {
				  Call();
        }
			}
			break;

			default:
			{
        assert(false && "Vehicle::handleObjectMenuSelect Unknown radial selection!");
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

	if(owner_->checkIfMountCalled()) {
		gLogger->log(LogManager::DEBUG,"void Vehicle::call() mount already called");
		return;
	}

	if(!owner_->isConnected() || owner_->isDead() || owner_->isIncapacitated()) {
		return;
	}

	body_ = new MountObject();

	string cust;
	cust.initRawBSTR((int8*)Swoop_Customization, BSTRType_ANSI);
	body_->setCustomizationStr(cust.getAnsi());
	body_->setCreoGroup(CreoGroup_Vehicle);
	body_->setTypeOptions(0x1080);
	body_->setMoodId(0);
	body_->setCL(0);

	body_->setId(mId + 1);	// Vehicles are created by the VehicleControllerFactory with +2 step for IDs

	body_->set_controller(this->getId());

	body_->set_owner(owner_->getId());
	body_->setParentId(0);
	body_->setModelString(mPhysicalModel);
	body_->setSpeciesGroup(mNameFile.getAnsi());
	body_->setSpeciesString(mName.getAnsi());
	body_->setPosture(0);
	body_->setScale(1.0f);


	std::string con = this->getAttribute<std::string>("condition");
	body_->getHam()->setPropertyValue(HamBar_Health, HamProperty_CurrentHitpoints,atoi(con.substr(0,con.find_first_of("/")).c_str()));
	body_->getHam()->setPropertyValue(HamBar_Health, HamProperty_MaxHitpoints,atoi(con.substr(con.find_first_of("/")+1,con.find_first_of("/")).c_str()));

	owner_->setMount(body_);
	owner_->setMounted(false);
	owner_->setMountCalled(false);

	// Set default direction and position for the body.
	body_->mDirection = owner_->mDirection;
  body_->mPosition  = owner_->mPosition;

	// Move it forward 2 meters
  body_->moveForward(2);
	
	// And drop it a little below the terrain to allow the client to normalize it.
	body_->mPosition.y = Heightmap::getSingletonPtr()->getHeight(body_->mPosition.x, body_->mPosition.z) - 0.3f;

  // Finally rotate it perpendicular to the player.
  body_->rotateRight(90.0f);

	// add to world
	if(!gWorldManager->addObject(body_)) {
    gLogger->log(LogManager::DEBUG,"void Vehicle::call() creating vehicle with id % "PRIu64" failed : couldnt add to world", body_->getId());
		SAFE_DELETE(body_);
		return;
	}

	gWorldManager->createObjectinWorld(owner_, body_);	

	gMessageLib->sendUpdateTransformMessage(body_);

	owner_->setMountCalled(true);
	
	return;
}


//===============================================================================================
//stores the physical body
void VehicleController::Store()
{
	if(!body_)
	{
		gLogger->log(LogManager::DEBUG,"Vehicle::store() Error: Store was called for a nonexistant body object!");
		return;
	}

	if(!owner_ || owner_->isDead() || owner_->isIncapacitated())
	{
		gLogger->log(LogManager::DEBUG,"Vehicle::store() couldnt find owner");
		return;
	}

	// todo auto dismount
	if(owner_->checkIfMounted())
	{
		DismountPlayer();
	}

	if(!owner_->checkIfMountCalled())
	{
		gLogger->log(LogManager::DEBUG,"Vehicle::store() Mount wasnt called !!!");
		return;
	}

	//the body is a creature_object!!!
	gMessageLib->sendDestroyObject_InRangeofObject(body_);

	owner_->setMount(NULL);


	owner_->setMounted(false);
	owner_->setMountCalled(false);

	// finally unload & destroy the vehicle creature
	gWorldManager->destroyObject(body_);

	// null the reference
	body_ = NULL;

}


//===============================================================================================
//dismount the owner from the physical body

void VehicleController::DismountPlayer() {
 	if(!body_) {
    assert(false && "Vehicle::mountPlayer() no vehicle body!");
		return;
	}

	if(!owner_->checkIfMounted()) {
    assert(false && "Vehicle::mountPlayer() owner is not mounted!");
		return;
	}

	//For safe measures make the player equipped by nothing
	gMessageLib->sendContainmentMessage_InRange(owner_->getId(), 0, 0xffffffff, owner_);

	body_->toggleStateOff(CreatureState_MountedCreature);
	gMessageLib->sendStateUpdate(body_);

	owner_->toggleStateOff(CreatureState_RidingMount);
	gMessageLib->sendStateUpdate(owner_);

	owner_->setMounted(false);
	
	gMessageLib->sendUpdateMovementProperties(owner_);
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

	//Make the mount equip the player
	gMessageLib->sendContainmentMessage_InRange(owner_->getId(), body_->getId(), 4, owner_);
	gMessageLib->sendUpdateTransformMessage(body_);
  
	body_->toggleStateOn(CreatureState_MountedCreature);
	gMessageLib->sendStateUpdate(body_);

	owner_->toggleStateOn(CreatureState_RidingMount);
	gMessageLib->sendStateUpdate(owner_);

	owner_->setMounted(true);
	
	gMessageLib->sendUpdateMovementProperties(owner_);
}

//===============================================================================================
