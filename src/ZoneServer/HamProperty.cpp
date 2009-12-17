/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "HamProperty.h"
#include "LogManager/LogManager.h"

//===============================================================

HamProperty::~HamProperty()
{
}

//===============================================================

void HamProperty::Init(int32 currentHitPoints,int32 baseHitPoints,int32 modifier,int32 encumbrance,int32 wounds)
{
	
	mCurrentHitPoints = currentHitPoints;
	mMaxHitPoints = baseHitPoints;
	mBaseHitPoints = baseHitPoints;
	mModifier = modifier;
	mEncumbrance = encumbrance;
	mWounds = wounds;
	calcModifiedHitPoints();
}

//===============================================================
//calculates the hitpoints depending on our wounds
//===============================================================

void HamProperty::calcModifiedHitPoints()
{
	// FIXME
	//remember buffs since we have different types of food mindbuffs and docbuffs
	
	mModifiedHitPoints = mMaxHitPoints - mEncumbrance - mWounds;
	
}

//===============================================================

int32 HamProperty::getValue(uint8 valueIndex)
{
	return *mValues[valueIndex];
}


//===============================================================

void HamProperty::setValue(uint8 valueIndex, int32 propertyValue)
{
	*mValues[valueIndex] = propertyValue;
}

//===============================================================

void HamProperty::updateValue(uint8 valueIndex, int32 propertyDelta)
{
	*mValues[valueIndex] += propertyDelta;
}

//===============================================================
//adds the regeneration to the hambar
//and checks the bounds
//bool return is not important for regeneration
//===============================================================

uint8 HamProperty::updateCurrentHitpoints(int32 hp,bool damage)
{
	// check for useless update
	// hp is the delta
	if(hp == 0 
	||(hp > 0 && mCurrentHitPoints == mModifiedHitPoints)
	||(!damage && hp < 0 && mCurrentHitPoints <= 1))
	{
		return(0);
	}
	
	int32 oldHp = mCurrentHitPoints;

	mCurrentHitPoints += hp;

	if(mCurrentHitPoints >= mModifiedHitPoints)
	{
		mCurrentHitPoints = mModifiedHitPoints;
	}
	else if(mCurrentHitPoints < 0)
	{
		mCurrentHitPoints = 0;

		// in case of damage we incap
		if(damage)
		{
			return(2);
		}
	}

	// if changed, update
	if(mCurrentHitPoints != oldHp)
	{
		return(1);
	}

	// no update
	return(0);
}
//===============================================================
bool HamProperty::updateModifiedHitpoints(int32 hp)
{

	//make sure we dont get negative hitpoints
	if((mMaxHitPoints -mWounds -mEncumbrance + hp) < 1)
		hp = (mMaxHitPoints -mWounds -mEncumbrance) -1;

	//make sure we dont get useless updates they upset our updatecounters
	if(hp==0)
		return false;
					 
	//hp is the delta
	mModifier += hp;
	mMaxHitPoints += hp;
	calcModifiedHitPoints();
	mCurrentHitPoints += hp;
	
	return true;
}
//===============================================================

bool HamProperty::updateBaseHitpoints(int32 hp)
{
	//hp is the delta
	mBaseHitPoints += hp;
	mMaxHitPoints += hp;
	
	calcModifiedHitPoints();
	
	// no migration applicable to players with wounds
	// make room for ham if necessary
	if(mWounds + mCurrentHitPoints != mBaseHitPoints)
	{
		mCurrentHitPoints = mBaseHitPoints-mWounds;
		return(true);
	}

	return(false);
}

//===============================================================
// this call will return true if an update on current hitpoints is needed
//
bool HamProperty::updateWounds(int32 wounds)
{

	// returns true if a packet update is needed
	// (in case mWounds + mCurrentHitPoints >= mMaxHitPoints )
	// Note: it will never be the case for wound healing.

	mWounds += wounds;
	
	if(mWounds >= mMaxHitPoints -1) // cant have 100% wounds
	{
		mWounds = mMaxHitPoints -1;
	}
	else if(mWounds < 0)
	{
		mWounds = 0;
	}

	calcModifiedHitPoints();

	// make room for ham if necessary
	if(mWounds + mCurrentHitPoints >= mMaxHitPoints)
	{
		mCurrentHitPoints = mModifiedHitPoints;
		return(true);
	}

	return(false);
}

//===============================================================
// debug helper
//
void HamProperty::log()
{
	gLogger->logMsgF("mCurrentHitPoints: %i\n", MSG_NORMAL,mCurrentHitPoints);
	gLogger->logMsgF("mModifiedHitPoints: %i\n", MSG_NORMAL,mModifiedHitPoints);
	gLogger->logMsgF("mMaxHitPoints: %i\n", MSG_NORMAL,mMaxHitPoints);
	gLogger->logMsgF("mEncumbrance: %i\n", MSG_NORMAL,mEncumbrance);
	gLogger->logMsgF("mWounds: %i\n", MSG_NORMAL,mWounds);
}

//===============================================================
