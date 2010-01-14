/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_FACTORY_OBJECT_H
#define ANH_ZONESERVER_FACTORY_OBJECT_H

#include "PlayerStructure.h"
#include "BuildingEnums.h"

//=============================================================================



//=============================================================================

struct FactoryHopperItem
{
		uint64			HarvesterID;
		uint64			ResourceID;
		float			Quantity;
};

class FactoryObject :	public PlayerStructure, public DatabaseCallback
{
	friend class FactoryFactory;

	public:

		FactoryObject();
		~FactoryObject();

		virtual void	handleDatabaseJobComplete(void* ref,DatabaseResult* result);
		
		FactoryFamily	getFactoryFamily(){ return mFactoryFamily; }
		void			setFactoryFamily(FactoryFamily ff){ mFactoryFamily = ff; }

		uint32			getLoadCount(){ return mTotalLoadCount; }
		uint32			decLoadCount(){ return (mTotalLoadCount-1); }
		void			setLoadCount(uint32 count){ mTotalLoadCount = count; }

		bool			getActive(){ return mActive; }
		void			setActive(bool value){ mActive = value; }

		void			prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount);
		void			handleObjectMenuSelect(uint8 messageType,Object* srcObject);

		void			setSchematicCustomName(string name){mSchematicName = name;}
		string			getSchematicCustomName(){return mSchematicName;}

		void			setSchematicName(string name){mSchematicSTF = name;}
		string			getSchematicName(){return mSchematicSTF;}

		void			setSchematicFile(string name){mSchematicFile = name;}
		string			getSchematicFile(){return mSchematicFile;}

		uint64			getManSchemID(){ return mManSchematicID; }
		void			setManSchemID(uint64 value){ mManSchematicID = value; }


	private:

		bool			mActive;

		
		FactoryFamily	mFactoryFamily;

		uint32			mTotalLoadCount;
		uint64			mManSchematicID;

		string			mSchematicName;
		string			mSchematicFile;
		string			mSchematicSTF;
		
};

//=============================================================================

#endif