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

#ifndef ZONESERVER_MOUNTOBJECT_H_
#define ZONESERVER_MOUNTOBJECT_H_

#include <cstdint>

#include "CreatureObject.h"

/// Encapsulates mount specific functionality.
/**
 * The MountObject class handles mount specific functionality like custom radial actions.
 */
class MountObject : public CreatureObject
{
public:

    // Declare and define the default constructor and destructor.
    MountObject();
    virtual ~MountObject() {}

    /// Prepares the custom radial menu for MountObject's.
    virtual void prepareCustomRadialMenu(CreatureObject* creature, uint8_t item_count);

    /// Handles a menu selection from MountObject's custom radial menu.
    virtual void handleObjectMenuSelect(uint8_t message_type, Object* source_object);

    /// Returns the object id of the owner of the mount.
    /**
     * @return uint64_t The object id of the owner of the mount.
     */
    uint64_t owner() const {
        return owner_;
    }

    /// Sets the object id of the owner of the mount.
    /**
     * @param owner_id The object id of the new owner of the mount.
     */
    void set_owner(uint64 owner_id) {
        owner_ = owner_id;
    }

    /// Returns the object id of the datapad controller for the mount.
    /**
     * @return uint64_t The object id of the datapad controller for the mount.
     */
    uint64_t controller() const {
        return controller_;
    }

    /// Sets the object id of the datapad controller for the mount.
    /**
     * @return controller The object id of the datapad controller for the mount.
     */
    void set_controller(uint64 controller) {
        controller_ = controller;
    }

private:
    uint64_t controller_;
    uint64_t owner_; // If creature is a mount, pet, or vehicle it has an owner.
};

//=============================================================================

#endif  // ZONESERVER_MOUNTOBJECT_H_
