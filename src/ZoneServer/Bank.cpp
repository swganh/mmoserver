/*
 This file is part of MMOServer. For more information, visit http://swganh.com
 
 Copyright (c) 2006 - 2011 The SWG:ANH Team

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "ZoneServer/Bank.h"

#include <sstream>

#include "DatabaseManager/Database.h"

#include "MessageLib/MessageLib.h"

#include "ZoneServer/PlayerObject.h"
#include "ZoneServer/WorldManager.h"

using std::stringstream;

Bank::Bank(PlayerObject* owner)
    : TangibleObject()
    , owner_(owner)
    , credits_(0)
    , planet_(-1)
{
    mId = owner->getId() + BANK_OFFSET;
    mTanGroup = TanGroup_PlayerInternal;
    mTanType = TanType_Bank;
}


Bank::~Bank() {}


int Bank::credits() const {
    return credits_;
}


void Bank::credits(int credits) {
    credits_ = credits;
}

bool Bank::updateCredits(int32_t amount) {
    // No transaction is accepted if it causes the account to go below 0.
    if (credits_ + amount < 0) {
        return false;
    }

    credits(credits_ + amount);

    gMessageLib->sendBankCreditsUpdate(owner_);

    stringstream query;
    query << "UPDATE "<<gWorldManager->getDatabase()->galaxy()<<".banks SET credits=" << credits_ << " WHERE id=" << mId;

    gWorldManager->getDatabase()->executeAsyncSql(query);

    return true;
}


void Bank::owner(PlayerObject* owner) {
    owner_ = owner;
}


int8_t Bank::planet() const {
    return planet_;
}


void Bank::planet(int8_t planet) {
    planet_ = planet;
}
