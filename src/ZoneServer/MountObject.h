/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

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
  virtual void prepareCustomRadialMenu(CreatureObject* creature, uint8 item_count);
  
  /// Handles a menu selection from MountObject's custom radial menu.
  virtual void handleObjectMenuSelect(uint8 message_type, Object* source_object);
  
  /// Returns the object id of the owner of the mount.
  /**
   * @return uint64_t The object id of the owner of the mount.
   */
  uint64_t getOwner() const { return mOwner; }
  
  /// Sets the object id of the owner of the mount.
  /**
   * @param owner_id The object id of the new owner of the mount.
   */
  void setOwner(uint64 owner_id) { mOwner = owner_id; }
  
  /// Returns the object id of the datapad controller for the mount.
  /**
   * @return uint64_t The object id of the datapad controller for the mount.
   */
  uint64_t getPetController() const { return mController; }
  
  /// Sets the object id of the datapad controller for the mount.
  /**
   * @return uint64_t The object id of the datapad controller for the mount.
   */
  void setPetController(uint64 c) { mController = c; }

private:
	uint64 mController;
	uint64 mOwner; // If creature is a mount, pet, or vehicle it has an owner.
};

//=============================================================================

#endif  // ZONESERVER_MOUNTOBJECT_H_
