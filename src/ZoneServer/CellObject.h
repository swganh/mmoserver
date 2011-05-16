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

#ifndef ANH_ZONESERVER_CELL_OBJECT_H
#define ANH_ZONESERVER_CELL_OBJECT_H

#include "StaticObject.h"

//=============================================================================

/*
- cells, need a building as parent
*/

class CellObject :	public StaticObject
{
    friend class CellFactory;

public:

    //TODO : use the objectcontainer methods instead!
    //please note that the objects need to stay in the worlmanager then though
    //alternatively keep this for creatureObjects

    CellObject();
    CellObject(uint64 id,uint64 parentId,const BString model = "object/cell/shared_cell.iff");
    ~CellObject();

    uint32		getLoadCount() {
        return mTotalLoadCount;
    }
    void		setLoadCount(uint32 count) {
        mTotalLoadCount = count;
    }

    void		prepareDestruction();


private:

    //ObjectList	mChildObjects;
    uint32		mTotalLoadCount;

};

//=============================================================================

#endif
