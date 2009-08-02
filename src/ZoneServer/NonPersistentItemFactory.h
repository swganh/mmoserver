/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_NON_PERSISTENT_ITEM_FACTORY_H
#define ANH_ZONESERVER_NON_PERSISTENT_ITEM_FACTORY_H

#include "Item.h"
#include "DatabaseManager/DatabaseCallback.h"
#include "FactoryBase.h"
#include "WorldManager.h"

#define	 gNonPersistentItemFactory	NonPersistentItemFactory::Instance()

//=============================================================================


//=============================================================================

class NonPersistentItemFactory : public FactoryBase
{
	public:
		static NonPersistentItemFactory* NonPersistentItemFactory::Instance(void);
		/*
		static inline NonPersistentItemFactory*	Instance(void)
		{
			if (!mSingleton)
			{
				mSingleton = new NonPersistentItemFactory(WorldManager::getSingletonPtr()->getDatabase());
			}
			return mSingleton;
		}
		*/

		static inline void deleteFactory(void)    
		{ 
			if (mSingleton)
			{
				delete mSingleton;
				mSingleton = 0;
			}
		}

		void			handleDatabaseJobComplete(void* ref,DatabaseResult* result);
		void			requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client);
		void			requestObject(ObjectFactoryCallback* ofCallback,uint64 id, uint64 newId);

	protected:
		NonPersistentItemFactory(Database* database);
		~NonPersistentItemFactory();

	private:
		// This constructor prevents the default constructor to be used, since it is private.
		NonPersistentItemFactory();

		void			_setupDatabindings();
		void			_destroyDatabindings();

		Item* NonPersistentItemFactory::_createItem(DatabaseResult* result, uint64 newId);

		static NonPersistentItemFactory*	mSingleton;
		DataBinding*	mItemIdentifierBinding;
		DataBinding*	mItemBinding;
};

//=============================================================================



#endif

