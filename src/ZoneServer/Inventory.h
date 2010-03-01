/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/


#ifndef ANH_ZONESERVER_TANGIBLE_INVENTORY_H
#define ANH_ZONESERVER_TANGIBLE_INVENTORY_H

#include "TangibleObject.h"
#include "ObjectFactoryCallback.h"

#include <vector>

typedef std::vector<std::pair<string,uint64> >	SortedInventoryItemList;
class CreatureObject;
class PlayerObject;

//=============================================================================

class Inventory : public TangibleObject
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

		ObjectList*		getEquippedObjects(){ return &mEquippedObjects; }
		void			addEquippedObject(Object* object){ mEquippedObjects.push_back(object); }
		void			removeEquippedObject(Object* object);
		void			deleteEquippedObject(Object* object);

		int32			getCredits(){ return mCredits; }
		void			setCredits(int32 credits){ mCredits = credits; }
		bool			updateCredits(int32 amount);

		//depecrated - use check(get/set) capicity instead
		uint8			getMaxSlots(){ return mMaxSlots; }
		bool			checkSlots(uint8 amount);
		void			setMaxSlots(uint8 slots){ mMaxSlots = slots; }

		virtual bool	checkCapacity(uint8 amount, PlayerObject* player = NULL, bool sendMsg = true);

		bool			unEquipItem(Object* object);
		bool			EquipItem(Object* object);
		bool			EquipItemTest(Object* object);

		void			getUninsuredItems(SortedInventoryItemList* insuranceList);
		void			getInsuredItems(SortedInventoryItemList* insuranceList);
		bool			itemExist(uint32 familyId, uint32 typeId);

	private:

		CreatureObject*		mParent;
		uint32				mObjectLoadCounter;
		

		ObjectIDList		mEquippedObjectIDs;

		ObjectList			mEquippedObjects;
		int32				mCredits;
		uint8				mMaxSlots;
};

//=============================================================================

#endif

