/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

---------------------------------------------------------------------------------------
*/


#ifndef ANH_ZONESERVER_TANGIBLE_INVENTORY_H
#define ANH_ZONESERVER_TANGIBLE_INVENTORY_H

#include "TangibleObject.h"
#include "ObjectFactoryCallback.h"

typedef std::vector<std::pair<string,uint64> >	SortedInventoryItemList;
class CreatureObject;

//=============================================================================

class Inventory : public TangibleObject, public ObjectFactoryCallback
{
	friend class ObjectFactory;
	friend class PlayerObjectFactory;
	friend class InventoryFactory;

	public:

		Inventory();
		~Inventory();

		// inherited callback
		virtual void	handleObjectReady(Object* object,DispatchClient* client);

		void			setParent(CreatureObject* creature){ mParent = creature; }

		uint32			getObjectLoadCounter(){ return mObjectLoadCounter; }
		void			setObjectLoadCounter(uint32 count){ mObjectLoadCounter = count; }

		ObjectList*		getObjects(){ return &mObjects; }
		void			addObject(Object* object); 
		void			removeObject(Object* object);
		void			deleteObject(Object* object);

		ObjectList*		getEquippedObjects(){ return &mEquippedObjects; }
		void			addEquippedObject(Object* object){ mEquippedObjects.push_back(object); }
		void			removeEquippedObject(Object* object);
		void			deleteEquippedObject(Object* object);

		Object*			getObjectById(uint64 objId);

		int32			getCredits(){ return mCredits; }
		void			setCredits(int32 credits){ mCredits = credits; }
		bool			updateCredits(int32 amount);

		uint8			getMaxSlots(){ return mMaxSlots; }
		bool			checkSlots(uint8 amount);
		void			setMaxSlots(uint8 slots){ mMaxSlots = slots; }
		void			unEquipItem(Object* object);
		bool			EquipItem(Object* object);

		void			getUninsuredItems(SortedInventoryItemList* insuranceList);
		void			getInsuredItems(SortedInventoryItemList* insuranceList);
		bool			itemExist(uint32 familyId, uint32 typeId);

	private:

		CreatureObject*		mParent;
		uint32				mObjectLoadCounter;
		
		ObjectIDList		mObjectIDs;
		ObjectIDList		mEquippedObjectIDs;

		ObjectList			mObjects;
		ObjectList			mEquippedObjects;
		int32				mCredits;
		uint8				mMaxSlots;
};

//=============================================================================

#endif

