/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_VehicleController_H
#define ANH_ZONESERVER_VehicleController_H

#include "IntangibleObject.h"
#include "MountObject.h"

//Forward Declerations
class IntangibleObject;
class Creatureobject;

//This reflects the Vehicle types id from the database
enum Vehicle_types
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
	VehicleType_shared				= 11
};


class VehicleController : public IntangibleObject
{
	friend class VehicleControllerFactory;

public:
	VehicleController();
	~VehicleController();

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
	uint64			getBodyId() { return mBodyId; }
	void			setBodyId(uint64 id) { mBodyId = id; }


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
	MountObject*		mBody;
	uint64				mBodyId;

private:

};



#endif
