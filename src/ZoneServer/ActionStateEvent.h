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

#ifndef ANH_ZONESERVER_ACTION_STATE_EVENTS_H
#define ANH_ZONESERVER_ACTION_STATE_EVENTS_H

#include "Common/Event.h"
#include "CreatureObject.h"

class ActionStateUpdateEvent : public ::common::BaseEvent
{
public:
    static const ::common::EventType type;

    explicit ActionStateUpdateEvent(uint64 objID, uint64 oldStates,CreatureState newState,uint64_t subject_id=0, uint64_t delay_ms=0);
    ActionStateUpdateEvent(uint64 objID, uint64 oldStates,CreatureState newState,uint64_t subject_id, uint64_t delay_ms, ::common::EventCallback callback);
    
    ~ActionStateUpdateEvent(void);

    const ::common::EventType& event_type() const;

    uint64 getCreatureObjectByID()          { return mObjID;}
    uint64 getOldState()                    { return mOldStates;}
    CreatureState getNewState()             { return mNewState;}

private:
    void onSerialize(::common::ByteBuffer& out) const;
    void onDeserialize(::common::ByteBuffer& in);

    bool onConsume(bool handled) const;
    
    uint64                      mObjID;
    uint64                      mOldStates;
    CreatureState               mNewState;
};

#endif