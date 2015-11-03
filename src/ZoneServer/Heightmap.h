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

#ifndef ANH_ZONESERVER_HEIGHTMAP_H
#define ANH_ZONESERVER_HEIGHTMAP_H

#define     gHeightmap    Heightmap::getSingletonPtr()

#include "Utils/typedefs.h"
#include <boost/thread/thread.hpp>
#include <string>
#include "HeightmapAsyncContainer.h"
#include <queue>

class Heightmap
{

public:
    static Heightmap*  Instance(uint16 resolution);
    static Heightmap*  getSingletonPtr() {
        return mInstance;
    }
    static void deleter(void)
    {
        if (mInstance)
        {
            delete mInstance;
            mInstance = 0;
        }
    }

    void addNewHeightMapJob(HeightmapAsyncContainer* container) {
        mJobMutex.lock();
        Jobs.push(container);
        mJobMutex.unlock();
    }

    void RunThread();

    void Connect();
    bool Open(void) {
        if(hmp) return true;
        else return false;
    }
    bool setupCache(int16 cacheResoulutionDivider);
    static inline bool isHeightmapCacheAvaliable(void) {
        return mCacheAvaliable;
    }
    inline bool isHighResCache(void) {
        return (mCacheResoulutionDivider == 1);
    }
    float getCachedHeight(float xPos, float zPos) const;
    float getHeight(float x, float y);
    bool isReady();
    float compensateForInvalidHeightmap(float hmapRes, float clientRes, float allowedDeviation);//TODO: Re-evaluate need once heightmaps are corrected
protected:
    Heightmap(const char* planet_name, uint16 resolution);
    ~Heightmap();

private:
    // This constructor prevents the default constructor to be used, since it is private.
    Heightmap();


    //DO NOT AND I REPEAT DO NOT USE THIS FOR ---ANYTHING---
    //EXCEPT FOR ONE TIME READS LIKE GETTING THE HEIGHT FOR
    //PLAYER BUILDING PLACEMENT!!!
    void fillInIterator(HeightResultMap::iterator it);

    //Dumps raw height variables. After recieving the data to get a proper height
    //value one must make sure the water bit is 0, cast to a float, then divide by 10.
    //see getHeight for an example how this is done.
    bool getRow(unsigned char* buffer, int32 x, int32 y, int32 length);

    //DO NOT AND I REPEAT DO NOT USE THIS FOR ---ANYTHING---
    //EXCEPT FOR ONE TIME READS LIKE GETTING THE HEIGHT FOR
    //PLAYER BUILDING PLACEMENT!!!

    const char* getFilename() const {
        return mFilename.c_str();
    }

    void setFilename(std::string filename) {
        mFilename = filename;
    }

    unsigned long getOffset(float x, float y) const ;

    int32 round_coord(float coord) const;

    uint16  mResolution;
    int16	**mHeightmapCache;
    int32	mCacheHeight;
    int32	mCacheWidth;
    int16	mCacheResoulutionDivider;

    static bool	mCacheAvaliable;

    boost::thread			    mThread;
    bool						  mExit;

protected:

    static Heightmap*  mInstance;
    // static bool        mInsFlag;
    std::string mFilename;
    FILE * hmp; //file pointer to the highmap

    int32	WIDTH;
    int32   HEIGHT;

    std::queue<HeightmapAsyncContainer*> Jobs;
    boost::mutex mJobMutex;
    boost::mutex mReadyMutex;
    bool mReady;
};

#endif
