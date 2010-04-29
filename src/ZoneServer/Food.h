/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_TANGIBLE_FOOD_H
#define ANH_ZONESERVER_TANGIBLE_FOOD_H

#include "Item.h"


//=============================================================================

class Food : public Item
{
	friend class ItemFactory;
	friend class FoodCommandMapClass;

	public:

		Food();
		virtual ~Food();

		virtual void	prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount);
		virtual void	handleObjectMenuSelect(uint8 messageType,Object* srcObject);
		void			handleFoodUse(Object* srcObject);

	private:

		void			_handleUses_Remaining(PlayerObject* playerObject);
		void			_handleHealth_Buff(PlayerObject* playerObject);
		void			_handleMind_Buff(PlayerObject* playerObject);
		void			_handleMask_Scent_Buff(PlayerObject* playerObject);


		bool			toDelete;
		uint32			mIcon;

};

//=============================================================================


typedef void									(Food::*funcfoodPointer)(PlayerObject*);
typedef std::map<uint32,funcfoodPointer>		FoodCommandMap;
#define gFoodCmdMap								((FoodCommandMapClass::getSingletonPtr())->mCommandMap)

class FoodCommandMapClass 
{
	public:

		static FoodCommandMapClass*  getSingletonPtr() { return mSingleton; }
		static FoodCommandMapClass*	Init();

		
		~FoodCommandMapClass();
		FoodCommandMapClass();

		FoodCommandMap						mCommandMap;

	private:
		
		static bool							mInsFlag;
		static FoodCommandMapClass*			mSingleton;

};

#endif

