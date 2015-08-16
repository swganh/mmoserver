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

#ifndef ANH_ZONESERVER_VEHICLECONTROLLER_H
#define ANH_ZONESERVER_VEHICLECONTROLLER_H

#include <cstdint>

#include "IntangibleObject.h"
#include "MountObject.h"

//Forward Declerations
class Creatureobject;

//This reflects the vehicle types id from the database
enum vehicle_types {
    VehicleType_JetPack           = 1,
    VehicleType_LandSpeeder_av21  = 2,
    VehicleType_LandSpeeder_base  = 3,
    VehicleType_LandSpeeder_x31   = 4,
    VehicleType_LandSpeeder_x34   = 5,
    VehicleType_LandSpeeder_xp38  = 6,
    VehicleType_SpeederBike       = 7,
    VehicleType_SpeederBike_base  = 8,
    VehicleType_SpeederBike_flash = 9,
    VehicleType_SpeederBike_swoop = 10,
    VehicleType_shared            = 11
};


/// Controls a vehicle from a datapad.
/**
 * The VehicleController class controlls the associated MountObject from a
 * player's datapad. This object is intangible and is never directly seen in
 * the game world though the player does interact with in the datapad via a
 * radial menu.
 */
class VehicleController : public IntangibleObject {
    friend class VehicleControllerFactory;

public:
    /// Default constructor
    VehicleController();

    /// Default deconstructor.
    ~VehicleController();


    /// Returns the type id of the vehicle.
    /**
     * @returns uint32_t The type id of the vehicle.
     */
    uint32_t type_id() const {
        return type_id_;
    }

    /// Sets the type id of the vehicle.
    /**
     * @returns type_id The type id of the vehicle.
     */
    void set_type_id(uint32_t type_id) {
        type_id_ = type_id;
    }

    /// Returns the rate of hitpoint loss for the vehicle.
    /**
     * @returns uint32_t The rate of hitpoint loss for the vehicle.
     */
    uint32_t hit_point_loss() const {
        return hit_point_loss_;
    }

    /// Sets the rate of hitpoint loss for the vehicle.
    /**
     * @returns hitpoint_loss The rate of hitpoint loss for the vehicle.
     */
    void set_hit_point_loss(uint32_t hitpoint_loss) {
        hit_point_loss_ = hitpoint_loss;
    }

    /// Returns the acceleration rate of the vehicle on an incline.
    /**
     * @returns uint32_t The acceleration rate of the vehicle on an incline.
     */
    uint32_t incline_acceleration() const {
        return incline_acceleration_;
    }

    /// Sets the acceleration rate of the vehicle on an incline.
    /**
     * @returns uint32_t The acceleration rate of the vehicle on an incline.
     */
    void set_incline_acceleration(uint32_t incline_acceleration) {
        incline_acceleration_ = incline_acceleration;
    }

    /// Returns the acceleration rate of the vehicle on a flat stretch.
    /**
     * @returns uint32_t The acceleration rate of the vehicle on a flat stretch.
     */
    int flat_acceleration() const {
        return flat_acceleration_;
    }

    /// Returns the acceleration rate of the vehicle on a flat stretch.
    /**
     * @returns uint32_t The acceleration rate of the vehicle on a flat stretch.
     */
    void set_flat_acceleration(int flat_acceleration) {
        flat_acceleration_ = flat_acceleration;
    }

    /// Returns the player object that owns this vehicle.
    /**
     * @returns PlayerObject The player object that owns this vehicle.
     */
    PlayerObject* owner() {
        return owner_;
    }

    /// Sets the player object that owns this vehicle.
    /**
     * @returns owner The player object that owns this vehicle.
     */
    void set_owner(PlayerObject* owner) {
        owner_ = owner;
    }

    /// Returns the vehicle this controller is maintaining.
    /**
     * @returns PlayerObject The vehicle this controller is maintaining.
     */
    MountObject* body() {
        return body_;
    }

    /// Returns the called state of the vehicle.
    /**
     * @returns bool The called state of the vehicle, true if called false if not.
     */
    bool IsCalled() const {
        return body_ != NULL;
    }

    /// Calls the vehicle this controller maintains.
    void Call();

    /// Stores the vehicle this controller maintains.
    void Store();

    /// Mounts the owner onto the vehicle.
    void MountPlayer();

    /// Dismounts the owner off the vehicle.
    void DismountPlayer();

    virtual void prepareCustomRadialMenu(CreatureObject* creature, uint8_t item_count);
    virtual void handleObjectMenuSelect(uint8_t message_type, Object* source_object);

private:

    PlayerObject* owner_;
    MountObject* body_;

    uint32_t flat_acceleration_;
    uint32_t hit_point_loss_; //amount of hitpoints lost during travel
    uint32_t incline_acceleration_;
    uint32_t type_id_;
};



#endif
