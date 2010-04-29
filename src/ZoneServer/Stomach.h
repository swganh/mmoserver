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
#include "MessageLib/MessageLib.h"
class PlayerObject;
//=======================================================================

class Stomach
{
	friend class PlayerObjectFactory;

	public:

		Stomach(PlayerObject* parent);
		~Stomach();

		void	setFoodMax(uint32 food){ mFoodMax = food; }
		uint32	getFoodMax(){ return mFoodMax; }
		void	setDrinkMax(uint32 drink){ mDrinkMax = drink; }
		uint32	getDrinkMax(){ return mDrinkMax; }
		void	setFood(double food){ mFood = food; gMessageLib->sendFoodUpdate(mParent);}
		bool	checkFood(double food){ return !(mFood+ food >=mFoodMax); }
		void	incFood(double food){ mFood += food; if(mFood < 0)mFood = 0; if(mFood > mFoodMax)mFood = mFoodMax; gMessageLib->sendFoodUpdate(mParent);}
		uint32	getFood(){ return (uint32)mFood; }
		void	setDrink(double drink){ mDrink = drink; gMessageLib->sendDrinkUpdate(mParent);}
		bool	checkDrink(double drink){ return !(mDrink+ drink >=mDrinkMax); }
		void	incDrink(double drink){ mDrink += drink; if(mDrink < 0)mDrink = 0; if(mDrink > mDrinkMax)mDrink = mDrinkMax; gMessageLib->sendDrinkUpdate(mParent);}
		uint32	getDrink(){ return (uint32)mDrink; }
		bool	regenDrink(uint64 time,void*);
		bool	regenFood(uint64 time,void*);

		//Return Milliseconds of 1min ticks based
		uint64	getDrinkInterval(){return (uint64)18000;}
		uint64	getFoodInterval(){return (uint64)18000;}
		uint64  mDrinkTaskId,mFoodTaskId;
		void checkForRegen();
	private:
		static const uint32  EmptyStomachTimer = 30; //Number of minutes before stomach clears
		double	mFood;
		double	mDrink;
		uint32	mFoodMax;
		uint32	mDrinkMax;
		PlayerObject* mParent;
};

//=======================================================================

#endif




