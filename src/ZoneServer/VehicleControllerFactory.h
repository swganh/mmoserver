/*
 This file is part of MMOServer. For more information, visit http://swganh.com
 
 Copyright (c) 2006 - 2011 The SWG:ANH Team

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef ANH_ZONESERVER_VEHICLECONTROLLER_FACTORY_H
#define ANH_ZONESERVER_VEHICLECONTROLLER_FACTORY_H

#include "FactoryBase.h"
#include "ObjectFactoryCallback.h"

//Forward Declerations
class Database;
class DatabaseCallback;
class DataBinding;
class DispatchClient;
class IntangibleObject;
class PlayerObject;
class VehicleController;

#define	 gVehicleControllerFactory	VehicleControllerFactory::getSingletonPtr()

//=============================================================================


//=============================================================================

enum VehicleQuery
{
    VehicleControllerFactoryQuery_Create		= 1,
    VehicleControllerFactoryQuery_TypesId		= 2,
    VehicleControllerFactoryQuery_ItnoData	= 3,
    VehicleControllerFactoryQuery_MainData	= 4,
    VehicleControllerFactoryQuery_Attributes	= 5
};

//=============================================================================

class VehicleControllerFactory : public FactoryBase, public ObjectFactoryCallback
{
public:

    static	VehicleControllerFactory*	getSingletonPtr() {
        return mSingleton;
    }
    static	VehicleControllerFactory*	Init(Database* database);

    ~VehicleControllerFactory();

    virtual void			handleDatabaseJobComplete(void* ref,DatabaseResult* result);
    virtual void			handleObjectReady(Object* object,DispatchClient* client);
    void					requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client);
    void					createVehicle(uint32 vehicle_type,PlayerObject* targetPlayer);


private:

    VehicleControllerFactory(Database* database);

    void						_setupDatabindings();
    void						_destroyDatabindings();

    VehicleController*			_createVehicle(DatabaseResult* result);

    static VehicleControllerFactory*	mSingleton;
    static bool					mInsFlag;

    DataBinding*				mVehicleItno_Binding;
    DataBinding*				mVehicleCreo_Binding;

};

//=============================================================================


#endif
