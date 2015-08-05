/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2010 The SWG:ANH Team
---------------------------------------------------------------------------------------
Use of this source code is governed by the GPL v3 license that can be found
in the COPYING file or at http://www.gnu.org/licenses/gpl-3.0.html

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
---------------------------------------------------------------------------------------
*/

#include "HamProperty.h"




#include "Utils/logger.h"

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

/*
bool HamProperty::calcModifiedWounds()
{
	int32 oW = mWounds;
	//make sure our wounds dont reduce us to zero ham!
	if(mWounds >= mCurrentHitPoints -1) // cant have 100% wounds
	{
		mWounds = mCurrentHitPoints -1;
	}
	else if(mWounds < 0)
	{
		mWounds = 0;
	}

	return(mWounds!= oW);

}
*/

//===============================================================
//calculates the hitpoints depending on our wounds
//===============================================================

void HamProperty::calcModifiedHitPoints()
{
    //a ham buff modifies the mMaxHitPoints in the Buff Object

    //go through all buffs and calculate the values freshly!!!!!
    /*
    ..
    */


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
//make sure the update is viable - modify it otherwise
//checks the bounds
/*
uint8 HamProperty::checkUpdateCurrentHitpoints(int32 hp,bool damage)
{
//check for lower bounds

int32 mHp = mBaseHitPoints;
if()

}
*/
//===============================================================
//adds the regeneration to the hambar
//and checks the bounds
//bool return is not important for regeneration
//===============================================================


uint8 HamProperty::updateCurrentHitpoints(int32 hp,bool damage)
{
    // check for useless update
    // only process when delta != 0
    if(hp == 0
            ||(hp > 0 && mCurrentHitPoints >= mModifiedHitPoints) //dont give us more Hitpoints than possible
            ||(!damage && hp < 0 && mCurrentHitPoints <= 1))      //only incap us from damage! - never a debuff
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
// we need to return hp so we know how much of our buff could be applied!
//
int32 HamProperty::updateModifiedHitpoints(int32 hp)
{

    //make sure we dont get negative hitpoints
    // mCurrentHitPoints vs mMaxHitPoints ???
    if((mCurrentHitPoints + hp) < 1)
        hp = -(mCurrentHitPoints) +1;

    //hp is the delta
    mModifier += hp;
    mMaxHitPoints += hp;
    calcModifiedHitPoints();
    mCurrentHitPoints += hp;

    return hp;
}
//===============================================================

int32 HamProperty::updateBaseHitpoints(int32 hp)
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
        return(hp);
    }

    return(0);
}

//===============================================================
// this call will return true if an update on current hitpoints is needed
//
int32 HamProperty::updateWounds(int32 wounds)
{
    // Wounds should *never* force us to have negative hitpoints
    // so always check against current hitpoints - not the theoretic max

    bool pos =(wounds >0);
    // returns true if a packet update is needed
    // (in case mWounds + mCurrentHitPoints >= mMaxHitPoints )
    // Note: it will never be the case for wound healing.

    if((wounds) >= mCurrentHitPoints -1) // cant have 100% wounds
    {
        wounds = (mCurrentHitPoints -1);

        if(pos&&(wounds < 0))
            wounds = 0;
    }

    if((mWounds+wounds) < 0)
    {
        wounds = (0-mWounds);
        //mWounds = 0;
    }

    mWounds += wounds;

    calcModifiedHitPoints();

    // make room for ham if necessary
    if(mWounds + mCurrentHitPoints >= mMaxHitPoints)
    {
        mCurrentHitPoints = mModifiedHitPoints;
    }

    return(wounds);
}

//===============================================================
// debug helper
//
void HamProperty::log()
{
    DLOG(INFO) << "mCurrentHitPoints: " << mCurrentHitPoints;
    DLOG(INFO) << "mModifiedHitPoints: " << mModifiedHitPoints;
    DLOG(INFO) << "mMaxHitPoints: "<< mMaxHitPoints;
    DLOG(INFO) << "mEncumbrance: " << mEncumbrance;
    DLOG(INFO) << "mWounds: " << mWounds;
}

//===============================================================
