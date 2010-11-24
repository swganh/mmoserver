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

#ifndef ANH_ZONESERVER_SPATIALINDEXMANAGER_H
#define ANH_ZONESERVER_SPATIALINDEXMANAGER_H

//#include "ObjectFactoryCallback.h"

#include "zmap.h"
#include "WorldManagerEnums.h"

#include "Object.h"

#include "DatabaseManager/DatabaseCallback.h"

#include "MathLib/Rectangle.h"
#include "MessageLib/MessageLib.h"

#include "Utils/TimerCallback.h"
#include "Utils/typedefs.h"

#include <boost/ptr_container/ptr_unordered_map.hpp>

#include <list>
#include <map>
#include <vector>

//======================================================================================================================

#define	 gSpatialIndexManager	SpatialIndexManager::getSingletonPtr()

//======================================================================================================================

enum TangibleType;


class TangibleObject;
class FactoryCrate;
class CellObject;

namespace Anh_Utils
{
   // class Clock;
    class Scheduler;
    //class VariableTimeScheduler;
}

//======================================================================================================================


//======================================================================================================================
//
// SpatialIndexManager
//
class SpatialIndexManager : public DatabaseCallback, public TimerCallback
{
	public:

		static SpatialIndexManager*	getSingletonPtr() { return mSingleton; }
		static SpatialIndexManager*	Init(Database* database);
		
		void					Shutdown();
		

		// retrieve spatial index for this zone
		zmap*					getGrid(){ return mSpatialGrid; }

		// add / delete an object, make sure to cleanup any other references
		float					_GetMessageHeapLoadViewingRange();
		
		// Add an object to zmap
		bool					AddObject(Object* newObject, bool updateGrid = true);

		void					UpdateObject(Object *updateObject);

		//removes an object from the grid and sends the destroys
		void					RemoveObjectFromWorld(Object *removeObject);		
		void					RemoveObjectFromWorld(PlayerObject *removePlayer);
		void					RemoveObjectFromWorld(CreatureObject *removeCreature);
		
		//used by above routines to access the grid
		void					RemoveObject(Object *removeObject, uint32 gridCell, bool updateGrid = true);

		void					RemoveRegion(RegionObject *removeObject);
		void					addRegion(RegionObject *region);
		RegionObject*			getRegion(uint32 id);


		//Update functions for spawn and despawn
		void					UpdateBackCells(Object* updateObject,uint32);
		void					UpdateFrontCells(Object* updateObject, uint32);
		void					CheckObjectIterationForDestruction(Object* toBeTested, Object* toBeUpdated);
		void					ObjectCreationIteration(std::list<Object*>* FinalList, Object* updateObject);
		void					CheckObjectIterationForCreation(Object* toBeTested, Object* toBeUpdated);

		//place Objects in the spatialIndex / cells 
		void					createInWorld(Object* object);
		void					createInWorld(PlayerObject* player);
		void					createInWorld(CreatureObject* creature);
		
		
		void					removePlayerFromStructure(PlayerObject* player, CellObject* cell);
		

		void					removeStructureItemsForPlayer(PlayerObject* player, BuildingObject* building);
		
		//iterate through all players in range and call our callback with the player as parameter
		void					sendToPlayersInRange(const Object* const object, bool cellContent, std::function<void (PlayerObject* player)> callback);

		
		// removes an item from a structure
		void					removeObjectFromBuilding(Object* object, BuildingObject* building);


		//======================================================================================================================
		// when creating a player and the player is in a cell we need to create all the cells contents for the player
		// cellcontent is *NOT* in the grid
		void					initObjectsInRange(PlayerObject* playerObject);

		//cave performs a range check
		void					getObjectsInRange(const Object* const object,ObjectSet* resultSet,uint32 objTypes,float range, bool cellContent);
		void					getPlayersInRange(const Object* const object,PlayerObjectSet* resultSet, bool cellContent);


		//Messaging Services
		bool					sendCreateObject(Object* object,PlayerObject* player,bool sendSelftoTarget);
		bool					sendCreateTangible(TangibleObject* tangibleObject,PlayerObject* targetObject, bool sendchildren = true);
		void					sendCreateTangible(TangibleObject* tangibleObject, ObjectList*	knownPlayers, bool sendchildren = true);
		bool					sendCreatePlayer(PlayerObject* playerObject,PlayerObject* targetObject);
		void					sendInventory(PlayerObject* playerObject);
		bool					sendEquippedItems(PlayerObject* srcObject,PlayerObject* targetObject);
		bool					sendCreateFactoryCrate(FactoryCrate* crate,PlayerObject* targetObject);

		uint64					getObjectMainParent(Object* object);
		
	private:

		SpatialIndexManager();

		uint32					_GetMessageHeapLoadGridRange();


		static SpatialIndexManager*		mSingleton;
		static bool						mInsFlag;

		
		
		Database*						mDatabase;
		
		zmap*							mSpatialGrid;
		
		//Anh_Utils::Scheduler*			mSubsystemScheduler;
		//ZoneServer*					mZoneServer;
		
		
};



//======================================================================================================================

#endif // ANH_ZONESERVER_SPATIALINDEXMANAGER_H



