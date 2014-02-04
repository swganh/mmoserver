/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2014 The SWG:ANH Team
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

#include "ZoneServer/Objects/Object.h"
#include "ZoneServer/GameSystemManagers/Spatial Index Manager/SpatialIndexManager.h"
#include "ZoneServer/WorldManagerEnums.h"


#define	 gContainerManager	ContainerManager::getSingletonPtr()

class PlayerObject;
class TangibleObject;
class FactoryCrate;
class CellObject;


class ContainerManager {
public:

	static ContainerManager*	getSingletonPtr() { return mSingleton; }
	static ContainerManager*	Init(swganh::database::Database* database);
	
	void					Shutdown();

	//permissionsystem
    bool					checkContainingContainer(uint64 containingContainer, uint64 playerId);
	
	/*@brief handles the transfer of an Item
	*	/param targetId this is the uint64 id of the container the object is supposed to be put in
	*	/param objectId	this is the Id of the object we want to transfer
	*/
    bool					transferItem(uint64 targetContainerId, uint64 transferItemId, PlayerObject* player, uint32 linkType);

	/*@brief checks whether the target container is able to acommodate an object (enough slots left, container fits in etc)
	*	/param targetId this is the uint64 id of the container the object is supposed to be put in
	*	/param object	this is the pointer to the object we want to put into the container
	*/
	bool					checkTargetContainer(uint64 targetId, Object* object, PlayerObject* player);

	/*@brief removes an item out of a container
	*	/param targetId this is the uint64 id of the container the object is supposed to be removed from
	*	/param objectId	this is the Id of the object we want to remove out of the container
	*/
    bool					removeFromContainer(uint64 targetId, uint64 objectId, PlayerObject* player);

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

	/*@brief registers player as a watcher of container "container"
	* and creates all the containers children for the player to view
	*/
	void					registerPlayerToContainer(Object* container, PlayerObject* const player) const;

	/*@brief unregisters a player (watcher) from either a container it is in or a container it is watching
	*/
	void					unRegisterPlayerFromContainer(Object* container, PlayerObject* const player) const;
	/*@brief creates an item for all of a containers watchers
	*/
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
