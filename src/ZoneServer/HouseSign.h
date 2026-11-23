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


#ifndef ANH_ZONESERVER_HOUSESIGN_H
#define ANH_ZONESERVER_HOUSESIGN_H

#include "TangibleObject.h"
#include "PlayerObject.h"
#include "UIManager.h"
#include "Utils/logger.h"

//=============================================================================
//
// The sign by a house's door (KNOWN_ISSUES #30), put up by the
// StructureManager. The client's own menu for a sign offers Read; reading it
// shows what the sign says, the house's name.
//
class HouseSign : public TangibleObject
{
public:
    void handleObjectMenuSelect(uint8 messageType, Object* srcObject)
    {
        PlayerObject* player = dynamic_cast<PlayerObject*>(srcObject);
        if(!player || !player->isConnected())
        {
            return;
        }
        LOG(INFO) << "House sign " << getId() << ": menu item " << static_cast<uint32>(messageType);

        std::string text = getCustomName().getLength() ? std::string(getCustomName().getAnsi()) : std::string("(no name)");
        gUIManager->createNewMessageBox(NULL, "handleHouseSign", "Sign", text.c_str(), player);
    }
};

#endif
