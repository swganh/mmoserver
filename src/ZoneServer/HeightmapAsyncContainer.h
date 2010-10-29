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

#ifndef HEIGHTMAPASYNCCONTAINER_H
#define HEIGHTMAPASYNCCONTAINER_H
#include <map>
#include "HeightMapCallback.h"

class heightResult
{
public:
    float height;
    bool hasWater;
};

typedef std::map<std::pair<float, float>, heightResult*> HeightResultMap;

enum HeightmapCallbackTypes
{
    HeightmapCallback_ArtisanSurvey,
    HeightmapCallback_StructureHarvester,
    HeightmapCallback_StructureFactory,
    HeightmapCallback_StructureHouse
};

class HeightmapAsyncContainer
{
public:
	HeightmapAsyncContainer(HeightMapCallBack* backCall, HeightmapCallbackTypes Type)
	: type(Type)
	, callback(backCall)
	{
        mHeightResults = new HeightResultMap();
    }

    ~HeightmapAsyncContainer()
    {
        for(HeightResultMap::iterator it= mHeightResults->begin(); it != mHeightResults->end(); it++)
            delete it->second;

        delete mHeightResults;
    }

    void addToBatch(float x, float y) {
        mHeightResults->insert(std::make_pair(std::make_pair(x,y), reinterpret_cast<heightResult*>(0)));
    }
    HeightResultMap* getResults() {
        return mHeightResults;
    }
    HeightMapCallBack* getCallback() {
        return callback;
    }
    HeightmapCallbackTypes type;

private:
    HeightResultMap* mHeightResults;
    HeightMapCallBack* callback;

};
#endif // HEIGHTMAPASYNCCONTAINER_H
