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

#ifndef ANH_ZONESERVER_SHUTTLE_H
#define ANH_ZONESERVER_SHUTTLE_H

#include "CreatureObject.h"
#include "Utils/typedefs.h"

#define SHUTTLE_LANDING_ANIMATION_TIME 25000

enum ShuttleState
{
    ShuttleState_Away			= 0,
    ShuttleState_InPort			= 1,
    ShuttleState_Landing		= 2,
    ShuttleState_AboutBoarding	= 3
};

//=============================================================================

class Shuttle : public CreatureObject
{
    friend class ShuttleFactory;

public:

    Shuttle();
    ~Shuttle();

    void			setAwayInterval(uint32 v) {
        mAwayInterval = v;
    }
    uint32			getAwayInterval() {
        return mAwayInterval;
    }
    void			setInPortInterval(uint32 v) {
        mInPortInterval = v;
    }
    uint32			getInPortInterval() {
        return mInPortInterval;
    }
    void			setInPortTime(uint32 time) {
        mInPortTime = time;
    }
    uint32			getInPortTime() {
        return mInPortTime;
    }
    void			setAwayTime(uint32 time) {
        mAwayTime = time;
    }
    uint32			getAwayTime() {
        return mAwayTime;
    }
    void			setLandingTime(uint32 time) {
        mLandingTime = time;
    }
    uint32			getLandingTime() {
        return mLandingTime;
    }
    void			setShuttleState(ShuttleState state) {
        mShuttleState = state;
    }
    ShuttleState	getShuttleState();
    bool			availableInPort(void);
    void			setCollectorId(uint64 id) {
        mTicketCollectorId = id;
    }
    uint64			getCollectorId() {
        return mTicketCollectorId;
    }
    void			useShuttle(PlayerObject* playerObject);
    bool			ticketCollectorEnabled() const;
    void			ticketCollectorEnable();

private:

    ShuttleState	mShuttleState;
    uint64			mTicketCollectorId;
    uint32			mAwayInterval;
    uint32			mAwayTime;
    uint32			mInPortInterval;
    uint32			mInPortTime;
    uint32			mLandingTime;
    bool			mTicketCollectorEnabled;

};

//=============================================================================

#endif

