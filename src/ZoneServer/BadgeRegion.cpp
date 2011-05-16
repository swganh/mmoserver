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

#include "ZoneServer/BadgeRegion.h"

#include "ZoneServer/PlayerObject.h"
#include "ZoneServer/WorldManager.h"


BadgeRegion::BadgeRegion(uint32_t badge_id)
    : RegionObject()
    , badge_id_(badge_id)
{
    mActive = true;
    mRegionType = Region_Badge;
}


BadgeRegion::~BadgeRegion() {}


uint32_t BadgeRegion::badge_id() const {
    return badge_id_;
}


void BadgeRegion::badge_id(uint32_t badge_id) {
    badge_id_ = badge_id;
}


void BadgeRegion::onObjectEnter(Object* object) {
    // Do a quick check to see if the object is in this region and is a player object.
    if (object->getParentId() != mParentId || object->getType() != ObjType_Player) {
        return;
    }

    PlayerObject* player = dynamic_cast<PlayerObject*>(object);
    if (player && !player->checkBadges(badge_id_)) {
        player->addBadge(badge_id_);
    }
}
