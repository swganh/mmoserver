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

#include "City.h"
#include "PlayerObject.h"
#include "WorldManager.h"

//=============================================================================

City::City() : RegionObject()
{
    mActive		= false;
    mRegionType = Region_City;
}

//=============================================================================

City::~City()
{
}

//=============================================================================

void City::update()
{
}

//=============================================================================

void City::onObjectEnter(Object* object)
{
    PlayerObject* player = (PlayerObject*)object;
    //player->setCityRegionId(this->getId());

	addVisitor(object);

	DLOG(INFO) << player->getFirstName().getAnsi() << " entered " 
        << mCityName << " (" << mVisitingPlayers.size() << " players in city)";
}

//=============================================================================

void City::onObjectLeave(Object* object)
{
	PlayerObject* player = (PlayerObject*)object;

	//if(player->getCityRegionId() == this->getId())
		//player->setCityRegionId(0);

	removeVisitor(object);

	DLOG(INFO) << player->getFirstName().getAnsi() << " left " 
        << mCityName << " (" << mVisitingPlayers.size() << " players in city)";
}

