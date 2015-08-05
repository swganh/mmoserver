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
