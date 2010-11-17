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

#ifndef ANH_ZONESERVER_FIREWORK_EVENT_H
#define ANH_ZONESERVER_FIREWORK_EVENT_H

#include "Utils/EventHandler.h"
#include "Utils/rand.h"


//======================================================================================================================

class FireworkEvent : public Anh_Utils::Event
{
public:

    FireworkEvent(uint32 _typeId,PlayerObject* _playerObject, const glm::vec3&	_position)
    {
        this->typeId = _typeId;
        this->playerObject=_playerObject;
        this->mPosition = _position;
        this->mPosition.x +=static_cast<float>((gRandom->getRand()%5)-2.5);
        this->mPosition.z +=static_cast<float>((gRandom->getRand()%5)-2.5);
    }

    PlayerObject*		getPlayerObject() const {
        return playerObject;
    }
    uint32				getType() const {
        return typeId;
    }
    const glm::vec3&	getPosition() const {
        return mPosition;
    }

private:
    PlayerObject* playerObject;
    uint32	typeId;
    glm::vec3 mPosition;
};


//======================================================================================================================

#endif


