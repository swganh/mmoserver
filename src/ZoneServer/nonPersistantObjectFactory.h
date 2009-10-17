/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_NON_PERSISTANT_H
#define ANH_ZONESERVER_NON_PERSISTANT_H

#include "FactoryBase.h"
#include "MathLib/Vector3.h"
#include <boost/pool/pool.hpp>

#define 	gNonPersistantObjectFactory	NonPersistantObjectFactory::getSingletonPtr()

class CampTerminal;
class Database;
class DataBinding;
class DispatchClient;
class ObjectFactoryCallback;
class PlayerStructure;
class PlayerObject;
class TangibleObject;

struct StructureDeedLink;
struct StructureItemTemplate;

//=============================================================================


class NonPersistantObjectFactory : public FactoryBase
{
	public:

		static NonPersistantObjectFactory*	getSingletonPtr() { return mSingleton; }
		static NonPersistantObjectFactory*	Instance();

		static inline void deleteFactory(void)    
		{ 
			if (mSingleton)
			{
				delete mSingleton;
				mSingleton = 0;
			}
		}

		virtual void			handleDatabaseJobComplete(void* ref,DatabaseResult* result);

		virtual void			requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client);
		void					createTangible(ObjectFactoryCallback* ofCallback, uint32 familyId, uint32 typeId, uint64 parentId,Anh_Math::Vector3 position, string customName, DispatchClient* client);

		PlayerStructure*		requestBuildingFenceObject(float x, float y, float z, PlayerObject* player);

		//spawns temporary objects for camps
		TangibleObject*			spawnTangible(StructureItemTemplate* placableTemplate, uint64 parentId, Anh_Math::Vector3 position, string customName, PlayerObject* player);
		CampTerminal*			spawnTerminal(StructureItemTemplate* placableTemplate, uint64 parentId, Anh_Math::Vector3 position, string customName, PlayerObject* player, StructureDeedLink*	deedData);

		void					_createItem(DatabaseResult* result,Item* item);

	protected:
		NonPersistantObjectFactory(Database* database);
		~NonPersistantObjectFactory();

	private:

		// This constructor prevents the default constructor to be used, since it is private.
		NonPersistantObjectFactory();

		static NonPersistantObjectFactory*	mSingleton;

		void					_setupDatabindings();
		void					_destroyDatabindings();

		Database*				mDatabase;
		DataBinding*			mItemBinding;
		DataBinding*			mItemIdentifierBinding;
		uint64					mId;
};

//=============================================================================


#endif
