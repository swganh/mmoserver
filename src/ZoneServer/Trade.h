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

#ifndef ANH_ZONESERVER_Trade_H
#define ANH_ZONESERVER_Trade_H

#include <cstdint>
#include <map>
#include <vector>

#include "DatabaseManager/Transaction.h"

class Item;
class PlayerObject;
class Skill;
class TangibleObject;

//=============================================================================
typedef std::map<uint32_t, Skill*> mySkillList;

class SkillTeachContainer
{
public:
    SkillTeachContainer();
    ~SkillTeachContainer();

    void            addSkill(uint32_t nr, uint32_t id);
    mySkillList*    getList();
    Skill*          getEntry(uint32_t nr);

private:
    mySkillList mTradeSkills;
};


class TradeContainer
{
public:

    TradeContainer() {}
    ~TradeContainer() {}
    void				setOldOwner(PlayerObject* oO) {
        mOldOwner = oO;
    }
    PlayerObject*		getOldOwner() {
        return mOldOwner;
    }
    void				setNewOwner(PlayerObject* nO) {
        mNewOwner = nO;
    }
    PlayerObject*		getNewOwner() {
        return mNewOwner;
    }
    void				setObject(TangibleObject* object) {
        mObject = object;
    }
    TangibleObject*		getObject() {
        return mObject;
    }

private:

    PlayerObject*		mOldOwner;
    PlayerObject*		mNewOwner;
    TangibleObject*		mObject;
};

typedef std::vector<PlayerObject*> PlayerObjectList;
typedef std::vector<TradeContainer*> ItemTradeList;

class Trade
{

public:

    Trade(PlayerObject* playerobject);
    ~Trade();

    void				cancelTradeSession();
    void				endTradeSession();
    void				updateCash(uint32_t amount);
    void				updateBank(uint32_t amount);
    bool				getTradeFinishedStatus() {
        return mTradingFin;
    }
    void				setTradeFinishedStatus(bool tradeStatus) {
        mTradingFin = tradeStatus;
    }
    bool				getAcceptStatus() {
        return mAcceptTrade;
    }
    void				setAcceptStatus(bool acceptStatus) {
        mAcceptTrade = acceptStatus;
    }
    void				setTradeCredits(uint32_t credits) {
        mMoney = credits;   //sets the amount of credits we want to give
    }
    uint32_t				getTradeCredits() {
        return mMoney;
    }
    bool				ItemTradeCheck(uint64_t ItemId);
    bool				checkEquipped(Item* addedItem);
    void				ItemTradeAdd(TangibleObject* addedItem,PlayerObject* newOwner,PlayerObject* oldOwner);
    void				TradeListDelete();
    bool				checkTradeListtoInventory();
    void				processTradeListPostTransaction();
    void				processTradeListPreTransaction(Transaction* mTransaction);
    void				deleteTradeInvitation(PlayerObject* player);
    void				tradeInvitationAdded(PlayerObject* player);
    bool				verifyInvitation(PlayerObject* player);
    bool				_handleCancelTradeInvitation(uint64_t callTime, void* nix);
    PlayerObject*		getPlayerObject() {
        return mPlayerObject;
    }
    PlayerObject*		getTeacher() {
        return mteacher;
    }
    void				setTeacher(PlayerObject* teacher) {
        mteacher = teacher;
    }

private:

    PlayerObjectList	mPlayerObjectList;
    bool				mTradingFin;
    bool				mAcceptTrade;
    PlayerObject*		mPlayerObject;
    PlayerObject*		mteacher;
    ItemTradeList		mItemTradeList;
    TradeContainer*		mTradeList;
    uint32_t			mMoney;
};

//=============================================================================

#endif

