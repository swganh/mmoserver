/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_PLAYER_OBJECT_FACTORY_H
#define ANH_ZONESERVER_PLAYER_OBJECT_FACTORY_H

#include "ZoneServer/PlayerObject.h"
#include "ZoneServer/TangibleObject.h"
#include "ZoneServer/CreatureObject.h"
#include "ZoneServer/Inventory.h"
#include "ZoneServer/Bank.h"
#include "ZoneServer/Datapad.h"
#include "ZoneServer/MissionBag.h"
#include "DatabaseManager/DatabaseCallback.h"
#include "FactoryBase.h"

#define 	gPlayerObjectFactory	PlayerObjectFactory::getSingletonPtr()

//=============================================================================

class Database;
class DataBinding;
class DispatchClient;
class ObjectFactoryCallback;
class XPContainer;
class InventoryFactory;
class DatapadFactory;

//=============================================================================

enum POFQuery
{
	POFQuery_MainPlayerData	= 1,
	POFQuery_Skills			= 2,
	POFQuery_XP				= 3,
	POFQuery_Inventory		= 4,
	POFQuery_Datapad		= 5,
	POFQuery_Badges			= 6,
	POFQuery_Factions		= 7,
	POFQuery_Friends		= 8,
	POFQuery_Ignores		= 9,
	POFQuery_DenyService	= 10,
	POFQuery_HoloEmotes		= 11,
	POFQuery_EquippedItems	= 12,
	POFQuery_PreDefCloningFacility = 13,
};

//=============================================================================

class PlayerObjectFactory : public FactoryBase, public ObjectFactoryCallback
{
	public:

		static PlayerObjectFactory*	getSingletonPtr() { return mSingleton; }
		static PlayerObjectFactory*	Init(Database* database);

		~PlayerObjectFactory();

		virtual void	handleObjectReady(Object* object,DispatchClient* client);
		void			handleDatabaseJobComplete(void* ref,DatabaseResult* result);
		void			requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client);

		void			releaseAllPoolsMemory();

	private:

		PlayerObjectFactory(Database* database);

		void			_setupDatabindings();
		void			_destroyDatabindings();

		PlayerObject*	_createPlayer(DatabaseResult* result);

		static PlayerObjectFactory*		mSingleton;
		static bool						mInsFlag;

		InventoryFactory*				mInventoryFactory;
		DatapadFactory*					mDatapadFactory;

		DataBinding*					mPlayerBinding;
		DataBinding*					mHairBinding;
		DataBinding*					mBankBinding;
		InLoadingContainer*				mIlc;
};

//=============================================================================

class XpContainer
{
public:

	XpContainer(){}
	~XpContainer(){}

	uint32	mId;
	int32	mValue;
};

//=============================================================================

#endif

