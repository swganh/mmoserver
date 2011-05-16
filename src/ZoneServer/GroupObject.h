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

#ifndef ANH_ZONESERVER_GROUP_OBJECT_H
#define ANH_ZONESERVER_GROUP_OBJECT_H

//=============================================================================

#include <list>

typedef std::list<uint64>				Uint64List;



//=============================================================================

class GroupObject
{

public:

    GroupObject(uint64 id) {
        mId = id;
    }
    ~GroupObject() {}


    uint64				getId()	{
        return mId;
    }
    void				setId(uint64 id) {
        mId = id;
    }

    Uint64List*			getPlayerList()	{
        return &mPlayerList;
    }
    void				addPlayer(uint64 id) {
        mPlayerList.push_back(id);
    }
    void				removePlayer(uint64 id)
    {
        Uint64List::iterator it = mPlayerList.begin();
        while(it != mPlayerList.end())
        {
            if((*it) == id)
                it = mPlayerList.erase(it);
            if(it != mPlayerList.end())
                it++;

        }
    }

private:

    Uint64List		mPlayerList;
    uint64			mId;
};



//=============================================================================

#endif



