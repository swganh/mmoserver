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

#ifndef ZONE_SERVER_CONTAINER_MANAGER_H_
#define ZONE_SERVER_CONTAINER_MANAGER_H_

#include <list>
#include <map>
#include <vector>

#include <boost/ptr_container/ptr_unordered_map.hpp>

#include "Utils/typedefs.h"

#include "MathLib/Rectangle.h"

#include "DatabaseManager/DatabaseCallback.h"

#include "MessageLib/MessageLib.h"

#include "ZoneServer/Object.h"
#include "ZoneServer/SpatialIndexManager.h"
#include "ZoneServer/WorldManagerEnums.h"


#define	 gContainerManager	ContainerManager::getSingletonPtr()


class TangibleObject;
class FactoryCrate;
class CellObject;


class ContainerManager {
public:

	static ContainerManager*	getSingletonPtr() { return mSingleton; }
	static ContainerManager*	Init(Database* database);
	
	void					Shutdown();
	

	//sends the destroys for an object equipped by a creature / player that gets unequipped
	void					SendDestroyEquippedObject(Object *removeObject);	

	//deletes an object out of a container
	void					deleteObject(Object* data, Object* parent);
	void					removeObject(Object* data, Object* parent);

	void					removeStructureItemsForPlayer(PlayerObject* player, BuildingObject* building);
	
	//to other players only - use for movement
	void					sendToRegisteredPlayers(Object* container, std::function<void (PlayerObject* const player)> callback);

	//to all registered watchers including ourselves
	void					sendToRegisteredWatchers(Object* container, std::function<void (PlayerObject* const player)> callback);

	void					sendToGroupedRegisteredPlayers(PlayerObject* const player, std::function<void (PlayerObject*  const player)> callback, bool self);

	//registers player as watcher to a container based on si
	void					registerPlayerToContainer(Object* container, PlayerObject* const player) const;
	//static containers are not affected by si updates
	void					registerPlayerToStaticContainer(Object* container, PlayerObject* const player, bool playerCreate = false) const;

	void					unRegisterPlayerFromContainer(Object* container, PlayerObject* const player) const;
	void					createObjectToRegisteredPlayers(Object* container,Object* object);


	void					destroyObjectToRegisteredPlayers(Object* container,uint64 object, bool destroyForSelf = false);
	void					updateObjectPlayerRegistrations(Object* newContainer, Object* oldContainer, Object* object, uint32 containment);
	void					updateEquipListToRegisteredPlayers(PlayerObject* player);

	//buildings are special containers as they always have their cells loaded even if otherwise unloaded
	void					registerPlayerToBuilding(BuildingObject* building,PlayerObject* player);
	void					unRegisterPlayerFromBuilding(BuildingObject* building,PlayerObject* player);

	//======================================================================================================================
	// when creating a player and the player is in a cell we need to create all the cells contents for the player
	// cellcontent is *NOT* in the grid
	void					initObjectsInRange(PlayerObject* playerObject);

	
private:
	ContainerManager();

	static ContainerManager*		mSingleton;
	static bool						mInsFlag;		
};

#endif  // ZONE_SERVER_CONTAINER_MANAGER_H_
