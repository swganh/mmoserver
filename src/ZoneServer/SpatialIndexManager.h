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

#include <list>
#include <map>
#include <vector>

#include <boost/ptr_container/ptr_unordered_map.hpp>

#include "Utils/ActiveObject.h"
#include "Utils/TimerCallback.h"
#include "Utils/typedefs.h"

#include "MathLib/Rectangle.h"

#include "DatabaseManager/DatabaseCallback.h"

#include "MessageLib/MessageLib.h"

#include "Object.h"
#include "WorldManagerEnums.h"
#include "Zmap.h"

//======================================================================================================================

#define	 gSpatialIndexManager	SpatialIndexManager::getSingletonPtr()

//======================================================================================================================

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
		
		void					UpdateObject(Object *updateObject);

		//just initialize our surroundings for us on reload were still created for other players
		bool					InitializeObject(PlayerObject *player);

		//removes an object from the grid and sends the destroys
		void					RemoveObjectFromWorld(Object *removeObject);		
		void					RemoveObjectFromWorld(PlayerObject *removePlayer);
		void					RemoveObjectFromWorld(CreatureObject *removeCreature);
		

		void					RemoveRegion(std::shared_ptr<RegionObject> remove_region);
		void					addRegion(std::shared_ptr<RegionObject> region);
		std::shared_ptr<RegionObject> findRegion(uint64_t id);


		//place Objects in the spatialIndex / cells 
		void					createInWorld(Object* object);
		void					createInWorld(PlayerObject* player);
		void					createInWorld(CreatureObject* creature);
		
		
		void					removePlayerFromStructure(PlayerObject* player, CellObject* cell);
		

		void					removeStructureItemsForPlayer(PlayerObject* player, BuildingObject* building);
		
		//iterate through all players in range and call our callback with the player as parameter
		void					sendToPlayersInRange(const Object* const object, bool cellContent, std::function<void (PlayerObject* player)> callback);

		void					sendToChatRange(Object* container, std::function<void (PlayerObject* const player)> callback);
        
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
		
		//used by above routines to remove Objects from the grid
		void					_RemoveObjectFromGrid(Object *removeObject);

		//used to add an Object to the grid
		bool					_AddObject(Object* newObject);
		bool					_AddObject(PlayerObject *newObject);

		//Update functions for spawn and despawn
		void					_UpdateBackCells(Object* updateObject,uint32);
		void					_UpdateFrontCells(Object* updateObject, uint32);
		void					_CheckObjectIterationForDestruction(Object* toBeTested, Object* toBeUpdated);
		void					_ObjectCreationIteration(std::list<Object*>* FinalList, Object* updateObject);
		void					_CheckObjectIterationForCreation(Object* toBeTested, Object* toBeUpdated);
		
		
		

		static SpatialIndexManager*		mSingleton;
		static bool						mInsFlag;

		
		
		Database*						mDatabase;
		
		zmap*							mSpatialGrid;

		utils::ActiveObject				active_;
		
		//Anh_Utils::Scheduler*			mSubsystemScheduler;
		//ZoneServer*					mZoneServer;
		
		
};



//======================================================================================================================

#endif // ANH_ZONESERVER_SPATIALINDEXMANAGER_H



