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

#ifndef ANH_ZONESERVER_TANGIBLE_TICKETCOLLECTOR_H
#define ANH_ZONESERVER_TANGIBLE_TICKETCOLLECTOR_H

#include "TangibleObject.h"

//=============================================================================

class Shuttle;
class TravelTicket;

//=============================================================================

class TicketCollector : public TangibleObject
{
    friend class TicketCollectorFactory;

public:

    TicketCollector();
    ~TicketCollector();

    virtual void	handleObjectMenuSelect(uint8 messageType,Object* srcObject);
    virtual void	prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount);

    void			setShuttle(Shuttle* shuttle) {
        mShuttle = shuttle;
    }
    Shuttle*		getShuttle() {
        return mShuttle;
    }

    void			setPortDescriptor(BString des) {
        mPortDescriptor = des;
    }
    BString			getPortDescriptor() {
        return mPortDescriptor;
    }

    void			travelRequest(TravelTicket* ticket,PlayerObject* playerObject);

    virtual void	handleUIEvent(uint32 action,int32 element,BString inputStr,UIWindow* window);

private:

    void			_createTicketSelectMenu(PlayerObject* playerObject);

    Shuttle*		mShuttle;
    BString			mPortDescriptor;

};

//=============================================================================

#endif
