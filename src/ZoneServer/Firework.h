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

#ifndef ANH_ZONESERVER_TANGIBLE_FIREWORK_H
#define ANH_ZONESERVER_TANGIBLE_FIREWORK_H

#include "Item.h"
#include <vector>

class FireworkEvent;

//=============================================================================
struct _fireworkShowEvent
{
    uint32 typeId;
    int32 delay;
    uint64 itemId;
};

typedef std::vector<_fireworkShowEvent> FireworkShowList;

class Firework : public Item
{
    friend class ItemFactory;

public:

    Firework();
    virtual ~Firework();
    virtual void	prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount);
    virtual void	handleObjectMenuSelect(uint8 messageType,Object* srcObject);

    uint32			getDelay() {
        return delay;
    };
    void			setDelay(uint32 _delay) {
        delay = _delay;
    };

private:
    uint32			delay;

};

//=============================================================================

class FireworkShow : public Item
{
    friend class ItemFactory;

public:

    FireworkShow();
    virtual ~FireworkShow();

    virtual void	prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount);
    virtual void	handleObjectMenuSelect(uint8 messageType,Object* srcObject);
    void			handleUIEvent(uint32 action,int32 element,BString inputStr,UIWindow* window);
    void			handleUIEvent(BString available, BString delay, UIWindow* window);
    void			onLaunch(const FireworkEvent* event);


private:
    ObjectList*		_getInventoryFireworks(PlayerObject* playerObject);
    BString			_getType(uint32 type);

    FireworkShowList	fireworkShowList;
    uint32				fireworkShowListModify; //Bit of a filthy hack, but couldn't find an alternative due to ui callback system

    //ObjectList*		objList;

};

//=============================================================================

#endif
