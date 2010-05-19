/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

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

  
	int getTypesId() const { return mTypesId; }
	void setTypesId(int types_id) { mTypesId = types_id; }
	
  /// Returns the rate of hitpoint loss for the vehicle.
  /**
   * @returns uint32_t The rate of hitpoint loss for the vehicle.
   */  
  uint32_t getHitPointLoss() const { return mHitPointLoss; }

  /// Sets the rate of hitpoint loss for the vehicle.
  /**
   * @returns hitpoint_loss The rate of hitpoint loss for the vehicle.
   */  
  void setHitPointLoss(uint32_t hitpoint_loss) { mHitPointLoss = hitpoint_loss; }

  /// Returns the acceleration rate of the vehicle on an incline.
  /**
   * @returns uint32_t The acceleration rate of the vehicle on an incline.
   */  
  uint32_t getInclineAcceleration() const { return mInclineAcceleration; }

  /// Sets the acceleration rate of the vehicle on an incline.
  /**
   * @returns uint32_t The acceleration rate of the vehicle on an incline.
   */  
  void setInclineAcceleration(uint32_t incline_acceleration) { mInclineAcceleration = incline_acceleration; }

  /// Returns the acceleration rate of the vehicle on a flat stretch.
  /**
   * @returns uint32_t The acceleration rate of the vehicle on a flat stretch.
   */  
  int getFlatAcceleration() const { return mFlatAcceleration; }

  /// Returns the acceleration rate of the vehicle on a flat stretch.
  /**
   * @returns uint32_t The acceleration rate of the vehicle on a flat stretch.
   */  
  void setFlatAcceleration(int flat_acceleration) { mFlatAcceleration = flat_acceleration; }

  /// Returns the player object that owns this vehicle.
  /**
   * @returns PlayerObject The player object that owns this vehicle.
   */  
  PlayerObject* getOwner() { return mOwner; }
  
  /// Sets the player object that owns this vehicle.
  /**
   * @returns owner The player object that owns this vehicle.
   */  
  void setOwner(PlayerObject* owner) { mOwner = owner; }

  /// Returns the vehicle this controller is maintaining.
  /**
   * @returns PlayerObject The vehicle this controller is maintaining.
   */  
  MountObject* getBody() { return mBody; }

  /// Returns the called state of the vehicle.
  /**
   * @returns bool The called state of the vehicle, true if called false if not.
   */ 
  bool isCalled() const { return mBody != NULL; }
  
  /// Calls the vehicle this controller maintains.
  void call();
  
  /// Stores the vehicle this controller maintains.
  void store();

  /// Mounts the owner onto the vehicle.
  void mountPlayer();
  
  /// Dismounts the owner off the vehicle.
  void dismountPlayer();

  virtual void prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount);
  virtual void handleObjectMenuSelect(uint8 messageType, Object* srcObject);

 private:

  uint64 mBodyId;

  PlayerObject* mOwner;
  MountObject* mBody;

  int mTypesId;
  int mHitPointLoss; //amount of hitpoints lost during travel
  int mInclineAcceleration;
  int mFlatAcceleration;
};



#endif
