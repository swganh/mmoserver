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

#ifndef ANH_ZONESERVER_PLAYER_OBJECT_FACTORY_H
#define ANH_ZONESERVER_PLAYER_OBJECT_FACTORY_H

#include "ZoneServer/Objects/FactoryBase.h"
#include "ZoneServer/Objects/ObjectFactoryCallback.h"

#define 	gPlayerObjectFactory	PlayerObjectFactory::getSingletonPtr()

//=============================================================================

namespace swganh	{
namespace database	{
class Database;
class DataBinding;
}}
class DatapadFactory;
class DispatchClient;
class InventoryFactory;
class PlayerObject;
class XPContainer;

//=============================================================================

enum POFQuery
{
    POFQuery_MainPlayerData			= 1,
    POFQuery_Skills					= 2,
    POFQuery_XP						= 3,
    POFQuery_Inventory				= 4,
    POFQuery_Datapad				= 5,
    POFQuery_Badges					= 6,
    POFQuery_Factions				= 7,
    POFQuery_Friends				= 8,
    POFQuery_Ignores				= 9,
    POFQuery_DenyService			= 10,
    POFQuery_HoloEmotes				= 11,
    POFQuery_EquippedItems			= 12,
    POFQuery_PreDefCloningFacility	= 13,
    POFQuery_Lots					= 14
};

//=============================================================================

class PlayerObjectFactory : public FactoryBase, public ObjectFactoryCallback
{
public:

    static PlayerObjectFactory*	getSingletonPtr() {
        return mSingleton;
    }
    static PlayerObjectFactory*	Init(swganh::database::Database* database);

    ~PlayerObjectFactory();

    virtual void	handleObjectReady(Object* object,DispatchClient* client);
    void			handleDatabaseJobComplete(void* ref,swganh::database::DatabaseResult* result);
    void			requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client);
	
	/*	@brief saves the zoneId, the parent_id, the currentPosition and the direction of a player to the db
	*	the query will not trigger a JobCompleteHandling. 
	*
	*/
	void			storeCharacterPosition(PlayerObject* player_object, uint32 zone = 0xffffffff);

	/*	@brief saves the players attributes (ham etc) to the db
	*	the query will not trigger a JobCompleteHandling. 
	*
	*/
	void			storeCharacterAttributes_(PlayerObject* player_object);

	/*	@brief registers the playerfactory related events
	*	mostly for saving data to db
	*
	*/
	void			RegisterEventHandlers();

    void			releaseAllPoolsMemory();

private:

    PlayerObjectFactory(swganh::database::Database* database);

    void			_setupDatabindings();
    void			_destroyDatabindings();

    PlayerObject*	_createPlayer(swganh::database::DatabaseResult* result);

    static PlayerObjectFactory*		mSingleton;
    static bool						mInsFlag;

    InventoryFactory*				mInventoryFactory;
    DatapadFactory*					mDatapadFactory;

    swganh::database::DataBinding*					mPlayerBinding;
    swganh::database::DataBinding*					mHairBinding;
    swganh::database::DataBinding*					mBankBinding;
    //InLoadingContainer*				mIlc;
};

//=============================================================================

class XpContainer
{
public:

    XpContainer() {}
    ~XpContainer() {}

    uint32	mId;
    int32	mValue;
};

//=============================================================================

#endif

