/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "Stomach.h"
#include "WorldManager.h"
#include "PlayerObject.h"
//#include "MessageLib/MessageLib.h"
//#include "LogManager/LogManager.h"
#include <iostream>

//=======================================================================

Stomach::Stomach(PlayerObject* parent)
{
	mParent = parent;
	mFoodMax = 100;
	mDrinkMax = 100;
	mFood = 0;
	mDrink = 0;
	mDrinkTaskId=0;
	mFoodTaskId=0;
}

//=======================================================================

Stomach::~Stomach()
{
}
bool Stomach::regenDrink(uint64 time,void*)
{
	if(mDrink==0) 
	{
		return true;
	}

	if(mDrink >= (mDrinkMax/100)) 
	{
		mDrink-= (mDrinkMax/100); 
	}
	else {
		mDrink = 0; 
	}
	gMessageLib->sendDrinkUpdate(mParent);

	return true; 
}
bool Stomach::regenFood(uint64 time,void*)
{
	if(mFood==0) {
		return true;
	}

	if(mFood >= (mFoodMax/100))
		mFood-= (mFoodMax/100); 
	else 
		mFood = 0; 

	gMessageLib->sendFoodUpdate(mParent);

	return true; 
}
//=======================================================================
void Stomach::checkForRegen()
{
	if(!mDrinkTaskId)
	{
		mDrinkTaskId = gWorldManager->addCreatureDrinkToProccess(this);
	}
	if(!mFoodTaskId)
	{
		mFoodTaskId = gWorldManager->addCreatureFoodToProccess(this);
	}
}
