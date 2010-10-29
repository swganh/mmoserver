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

#ifndef ANH_ZONESERVER_UIPLAYERSELECTBOX_H
#define ANH_ZONESERVER_UIPLAYERSELECTBOX_H

#include "Utils/typedefs.h"
#include "UIListBox.h"

class PlayerObject;

//================================================================================
typedef std::list<PlayerObject*>				PlayerList;
//================================================================================


class UIPlayerSelectBox : public UIListBox
{
public:

    UIPlayerSelectBox(UICallback* callback,uint32 id,const int8* eventStr,const int8* caption,const int8* prompt,const BStringVector dataItems, PlayerList playerList, PlayerObject* playerObject,uint8 lbType)
        : UIListBox(callback,id,SUI_Window_SelectGroupLootMaster_Listbox,eventStr,caption,prompt,dataItems,playerObject,lbType), mPlayers(playerList) {}

    virtual			~UIPlayerSelectBox() {}

    PlayerList		getPlayers() {
        return mPlayers;
    }

private:

    PlayerList		mPlayers;
};


//================================================================================

#endif
