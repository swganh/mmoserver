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

#ifndef ANH_ZONESERVER_RESOURCECOLLECTIONCOMMAND_H
#define ANH_ZONESERVER_RESOURCECOLLECTIONCOMMAND_H

#include "Utils/bstring.h"
#include "Utils/typedefs.h"

//=============================================================================

class ResourceCollectionCommand
{
    friend class ResourceCollectionManager;

public:

    ResourceCollectionCommand();
    ~ResourceCollectionCommand();

    // getters
    uint32	getId()							{
        return mId;
    }
    BString	getCommandName()				{
        return mCommandName;
    }
    int32	getHealthCost()					{
        return mHealthCost;
    }
    int32	getActionCost()					{
        return mActionCost;
    }
    int32	getMindCost()					{
        return mMindCost;
    }
    int32	getDamageModifier()				{
        return mDamageModifier;
    }

    // setters
    void	setId(uint32 id)				{
        mId = id;
    }
    void	setCommandName(BString cName)	{
        mCommandName = cName;
    }
    void	setHealthCost(int32 hCost)		{
        mHealthCost = hCost;
    }
    void	setActionCost(int32 aCost)		{
        mActionCost = aCost;
    }
    void	setMindCost(int32 mCost)		{
        mMindCost = mCost;
    }
    void	setDamageModifier(int32 damage) {
        mDamageModifier = damage;
    }


private:

    uint32	mId;
    BString	mCommandName;
    int32	mHealthCost;
    int32	mActionCost;
    int32	mMindCost;
    int32	mDamageModifier;
};

//=============================================================================

#endif
