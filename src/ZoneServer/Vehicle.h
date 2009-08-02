/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_VEHICLE_H
#define ANH_ZONESERVER_VEHICLE_H

#include "IntangibleObject.h"
#include "CreatureObject.h"

//This reflects the vehicle types id from the database
enum vehicle_types
{

	VehicleType_JetPack				= 1,
	VehicleType_LandSpeeder_av21	= 2,
	VehicleType_LandSpeeder_base	= 3,
	VehicleType_LandSpeeder_x31		= 4,
	VehicleType_LandSpeeder_x34		= 5,
	VehicleType_LandSpeeder_xp38	= 6,
	VehicleType_SpeederBike			= 7,
	VehicleType_SpeederBike_base	= 8,
	VehicleType_SpeederBike_flash	= 9,
	VehicleType_SpeederBike_swoop	= 10,
	VehicleType_shared				= 11,
};


class Vehicle : public IntangibleObject
{
	friend class VehicleFactory;

public:
	Vehicle();
	~Vehicle();

	int		getTypesId() { return mTypesId; } 
	void	setTypesId(int types_id) { mTypesId = types_id; }
	int		getHitPointLoss() { return mHitPointLoss; }
	void    setHitPointLoss(int hitpoint_loss) { mHitPointLoss = hitpoint_loss; }
	int     getInclineAcceleration() { return mInclineAcceleration; }
	void	setInclineAcceleration(int incline_acceleration) { mInclineAcceleration = incline_acceleration; }
	int		getFlatAcceleration() { return mFlatAcceleration; }
	void	setFlatAcceleration(int flat_acceleration) { mFlatAcceleration = flat_acceleration; }

	PlayerObject*	getOwner() { return mOwner; }
	void			setOwner(PlayerObject* owner) { mOwner = owner; }
	CreatureObject* getBody() { return mBody; }

	bool			isCalled() { return mBody != NULL; }

	void			call();
	void			store();
	void			mountPlayer();
	void			dismountPlayer();

	virtual void	prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount);
	virtual void	handleObjectMenuSelect(uint8 messageType,Object* srcObject);

protected:

	int					mTypesId;
	int					mHitPointLoss; //amount of hitpoints lost during travel
	int					mInclineAcceleration; 
	int					mFlatAcceleration;
	PlayerObject*		mOwner;
	CreatureObject*		mBody;

private:

};



#endif