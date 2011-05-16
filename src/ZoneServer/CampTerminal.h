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

#ifndef ANH_ZONESERVER_TANGIBLE_CAMPTERMINAL_H
#define ANH_ZONESERVER_TANGIBLE_CAMPTERMINAL_H

#include "Terminal.h"

//=============================================================================

class CampTerminal : public Terminal
{
    friend class TerminalFactory;

public:

    CampTerminal();
    ~CampTerminal();

    virtual void			prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount);
    virtual void			handleObjectMenuSelect(uint8 messageType,Object* srcObject);

    void					setOwner(uint64 owner) {
        mOwnerId = owner;
    }
    uint64					getOwner() {
        return mOwnerId;
    }

    void					setCamp(uint64 camp) {
        mCampId = camp;
    }
    uint64					getCamp() {
        return mCampId;
    }

    void					setCampRegion(uint64 region) {
        mCampRegionId = region;
    }
    uint64					getCampRegion() {
        return mCampRegionId;
    }

private:
    uint64					mCampId;	//id of the tent - has the object list
    uint64					mOwnerId;
    BStringVector			mAttributesMenu;
    uint64					mCampRegionId;


};

//=============================================================================

#endif

