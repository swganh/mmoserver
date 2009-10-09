/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_OBJECT_FACTORY_H
#define ANH_ZONESERVER_OBJECT_FACTORY_H

#include "Object_Enums.h"
#include "TangibleEnums.h"
#include "DatabaseManager/DatabaseCallback.h"
#include "MathLib/Vector3.h"
#include "Utils/typedefs.h"
#include <boost/pool/pool.hpp>

#define 	gObjectFactory	ObjectFactory::getSingletonPtr()

class BuildingFactory;
class CreatureFactory;
class Database;
class Deed;
class DispatchClient;
class DraftSchematic;
class HarvesterFactory;
class IntangibleFactory;
class OFAsyncContainer;
class Object;
class ObjectFactoryCallback;	
class PlayerObject;
class PlayerObjectFactory;	
class RegionFactory;
class TangibleFactory;
class TangibleObject;
class TicketProperties;
class WaypointFactory;

//=============================================================================

enum OFQuery
{
	OFQuery_Default						= 0,
	OFQuery_ResourceContainerCreate		= 1,
	OFQuery_WaypointCreate				= 2,
	OFQuery_Item						= 3,
	OFQuery_Harvester					= 4,
};

//=============================================================================

class ObjectFactory : public DatabaseCallback
{
	public:

		static ObjectFactory*	getSingletonPtr() { return mSingleton; }
		static ObjectFactory*	Init(Database* database);

		~ObjectFactory();

		virtual void			handleDatabaseJobComplete(void* ref,DatabaseResult* result);

		void					requestObject(ObjectType objType,uint16 subGroup,uint16 subType,ObjectFactoryCallback* ofCallback,uint64 id,DispatchClient* client);

		// create new objects in the database
		void					requestNewDefaultItem(ObjectFactoryCallback* ofCallback,uint32 schemCrc,uint64 parentId,uint16 planetId,Anh_Math::Vector3 position,string customName = "");
		void					requestNewDefaultItem(ObjectFactoryCallback* ofCallback,uint32 familyId,uint32 typeId,uint64 parentId,uint16 planetId,Anh_Math::Vector3 position,string customName = "");
		void					requestNewDefaultManufactureSchematic(ObjectFactoryCallback* ofCallback,uint32 schemCrc,uint64 parentId);
		void					requestNewWaypoint(ObjectFactoryCallback* ofCallback,string name,const Anh_Math::Vector3 coords,uint16 planetId,uint64 ownerId,uint8 wpType);
		void					requestNewTravelTicket(ObjectFactoryCallback* ofCallback,TicketProperties ticketProperties,uint64 parentId,uint16 planetId);
		void					requestNewResourceContainer(ObjectFactoryCallback* ofCallback,uint64 resourceId,uint64 parentId,uint16 planetId,uint32 amount);
		void					requestnewHarvesterbyDeed(ObjectFactoryCallback* ofCallback,Deed* deed,DispatchClient* client, float x, float y, float z, float dir, string customName, PlayerObject* player);


		void					requestTanoNewParent(ObjectFactoryCallback* ofCallback,uint64 ObjectId,uint64 parentId, TangibleGroup  Group);


		void					deleteObjectFromDB(Object* object);
		void					GiveNewOwnerInDB(Object* object, uint64 ID);
		void					createIteminInventory(ObjectFactoryCallback* ofCallback,uint64 ObjectId, TangibleGroup  Group);

		void					releaseAllPoolsMemory();

	private:

		ObjectFactory(Database* database);

		static ObjectFactory*	mSingleton;
		static bool			mInsFlag;

		Database*				mDatabase;

		PlayerObjectFactory*	mPlayerObjectFactory;
		TangibleFactory*		mTangibleFactory;
		IntangibleFactory*		mIntangibleFactory;
		CreatureFactory*		mCreatureFactory;
		BuildingFactory*		mBuildingFactory;
		RegionFactory*			mRegionFactory;
		WaypointFactory*		mWaypointFactory;
		HarvesterFactory*		mHarvesterFactory;

		boost::pool<boost::default_user_allocator_malloc_free>	mDbAsyncPool;
};

//=============================================================================

class OFAsyncContainer
{
	public:

		OFAsyncContainer(ObjectFactoryCallback* of,OFQuery oq,DispatchClient* cl){ ofCallback = of;query = oq;client = cl; }

		DispatchClient*			client;
		ObjectFactoryCallback*	ofCallback;
		OFQuery					query;
		uint64					Id;
		TangibleGroup			Group;
		uint64					DeedId;
		uint64					OwnerId;
};

#endif
