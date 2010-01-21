/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_STOMACH_H
#define ANH_ZONESERVER_STOMACH_H

#include "Utils/typedefs.h"

//=======================================================================

class Stomach
{
	friend class PlayerObjectFactory;

	public:

		Stomach();
		~Stomach();

		void	setFoodMax(uint32 food){ mFoodMax = food; }
		uint32	getFoodMax(){ return mFoodMax; }
		void	setDrinkMax(uint32 drink){ mDrinkMax = drink; }
		uint32	getDrinkMax(){ return mDrinkMax; }
		void	setFood(uint32 food){ mFood = food; }
		bool	checkFood(uint32 food){ if(mFood+ food >=mFoodMax) return false;else return true; }
		void	incFood(int32 food){ mFood += food; if(mFood < 0)mFood = 0; if(mFood > mFoodMax)mFood = mFoodMax; }
		uint32	getFood(){ return mFood; }
		void	setDrink(uint32 drink){ mDrink = drink; }
		bool	checkDrink(uint32 drink){ if(mDrink+ drink >=mDrinkMax) return false;else return true; }
		void	incDrink(int32 drink){ mDrink += drink; if(mDrink < 0)mDrink = 0; if(mDrink > mDrinkMax)mDrink = mDrinkMax; }
		uint32	getDrink(){ return mDrink; }

	private:

		uint32	mFood;
		uint32	mDrink;
		uint32	mFoodMax;
		uint32	mDrinkMax;
};

//=======================================================================

#endif




