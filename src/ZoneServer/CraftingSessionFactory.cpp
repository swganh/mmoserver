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
#include "CraftingSessionFactory.h"
#include "CraftingSession.h"
#include "PlayerObject.h"

#include "DatabaseManager/Database.h"

#include "Utils/utils.h"


//=============================================================================

bool						CraftingSessionFactory::mInsFlag    = false;
CraftingSessionFactory*		CraftingSessionFactory::mSingleton  = NULL;

//======================================================================================================================

CraftingSessionFactory*	CraftingSessionFactory::Init(Database* database)
{
    if(!mInsFlag)
    {
        mSingleton = new CraftingSessionFactory(database);
        mInsFlag = true;
        return mSingleton;
    }
    else
        return mSingleton;
}

//=============================================================================

CraftingSessionFactory::CraftingSessionFactory(Database* database) :
    mDatabase(database),
    mSessionPool(sizeof(CraftingSession))
{
}

//=============================================================================

CraftingSessionFactory::~CraftingSessionFactory()
{
    mInsFlag = false;
    delete(mSingleton);
}

//=============================================================================

CraftingSession* CraftingSessionFactory::createSession(Anh_Utils::Clock* clock,PlayerObject* player,CraftingTool* tool,CraftingStation* station,uint32 expFlag)
{
    CraftingSession* session = new(mSessionPool.ordered_malloc()) CraftingSession(clock,mDatabase,player,tool,station,expFlag);

    return(session);
}

//=============================================================================

void CraftingSessionFactory::destroySession(CraftingSession* session)
{
    if(session)
    {
        session->~CraftingSession();
        mSessionPool.free(session);
    }
}

//=============================================================================


