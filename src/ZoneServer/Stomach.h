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

    void	setFoodMax(uint32 food) {
        mFoodMax = food;
    }
    uint32	getFoodMax() {
        return mFoodMax;
    }
    void	setDrinkMax(uint32 drink) {
        mDrinkMax = drink;
    }
    uint32	getDrinkMax() {
        return mDrinkMax;
    }
    void	setFood(float food) {
        mFood = food;
        gMessageLib->sendFoodUpdate(mParent);
    }
    bool	checkFood(float food) {
        return !(mFood+ food >=mFoodMax);
    }
    void	incFood(float food) {
        mFood += food;
        if(mFood < 0)mFood = 0.0f;
        if(mFood > mFoodMax)mFood = (float)mFoodMax;
        gMessageLib->sendFoodUpdate(mParent);
    }
    uint32	getFood() {
        return (uint32)mFood;
    }
    void	setDrink(float drink) {
        mDrink = drink;
        gMessageLib->sendDrinkUpdate(mParent);
    }
    bool	checkDrink(float drink) {
        return !(mDrink+ drink >=mDrinkMax);
    }
    void	incDrink(float drink) {
        mDrink += drink;
        if(mDrink < 0)mDrink = 0.0f;
        if(mDrink > mDrinkMax)mDrink = (float)mDrinkMax;
        gMessageLib->sendDrinkUpdate(mParent);
    }
    uint32	getDrink() {
        return (uint32)mDrink;
    }
    bool	regenDrink(uint64 time,void*);
    bool	regenFood(uint64 time,void*);

    //Return Milliseconds of 1min ticks based
    uint64	getDrinkInterval() {
        return (uint64)18000;
    }
    uint64	getFoodInterval() {
        return (uint64)18000;
    }
    uint64  mDrinkTaskId,mFoodTaskId;
    void checkForRegen();
private:
    static const uint32  EmptyStomachTimer = 30; //Number of minutes before stomach clears
    float	mFood;
    float	mDrink;
    uint32	mFoodMax;
    uint32	mDrinkMax;
    PlayerObject* mParent;
};

//=======================================================================

#endif




