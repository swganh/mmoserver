/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ZONESERVER_MOUNTOBJECT_H_
#define ZONESERVER_MOUNTOBJECT_H_

#include "CreatureObject.h"

/// Encapsulates mount specific functionality.
/**
 * The MountObject class handles mount specific functionality like custom radial actions.
 */
class MountObject : public CreatureObject {
 public:  
   // Declare and define the default constructor and destructor.
   MountObject() : CreatureObject() {}
   virtual ~MountObject() {}
   
   /// Prepares the custom radial menu for MountObject's.
   virtual void prepareCustomRadialMenu(CreatureObject* creature, uint8 item_count);
   
   /// Handles a menu selection from MountObject's custom radial menu.
   virtual void handleObjectMenuSelect(uint8 message_type, Object* source_object);
};

//=============================================================================

#endif  // ZONESERVER_MOUNTOBJECT_H_
