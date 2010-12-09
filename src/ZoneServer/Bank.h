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

#ifndef ANH_ZONESERVER_TANGIBLE_BANK_H
#define ANH_ZONESERVER_TANGIBLE_BANK_H

#include <boost/noncopyable.hpp>

#include "TangibleObject.h"

class PlayerObject;

/** A player object's bank is a reference to the players bank account within
* the games economy. This object tracks the players credits and the planet
* designated as their home banking planet.
*/
class Bank : public TangibleObject, boost::noncopyable {
public:
    /** Creates a bank account for a player object.
    *
    * \param owner The player object to create the bank account for.
    */
	explicit Bank(PlayerObject* owner);
	~Bank();
    
    /// Returns the current credits in this bank account.
    int credits() const;

    /// Sets the current bank credits to a specific amount.
    void credits(int credits);

    /** Updates the current bank credits by a specific amount.
    *
    * Depending on whether the amount passed in is positive or negative, this
    * method provides a means to increase or decrease the amount of credits in
    * a bank account.
    *
    * \param amount The amount to increase/decrease the bank credits by.
    */
    bool updateCredits(int32_t amount);

    /// Sets the owner of this bank account.
    void owner(PlayerObject* owner);

    /// Returns the planet id that is the owner's home bank.
    int8_t planet() const;

    /// Sets the planet id for the owner's home bank.
    void planet(int8_t planet);

private:
    friend class PlayerObjectFactory;

    Bank();

    PlayerObject* owner_;
    int credits_;
    int8_t planet_;
};

//=============================================================================

#endif

