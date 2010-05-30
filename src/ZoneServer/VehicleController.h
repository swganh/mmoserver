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

#include "IntangibleObject.h"
#include "MountObject.h"

//Forward Declerations
class IntangibleObject;
class Creatureobject;

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
