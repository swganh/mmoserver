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
#include "Heightmap.h"
#include "ZoneServer/WorldManager.h"



#include "Utils/logger.h"
#include "Utils/utils.h"
#include <cassert>
#include <cfloat>
#include "math.h"
//=============================================================================
Heightmap::Heightmap(const char* planet_name, uint16 resolution)
    : mResolution(resolution)
	, mHeightmapCache(nullptr)
    , mCacheHeight(0)
    , mCacheWidth(0)
    , mCacheResoulutionDivider(3)
    , WIDTH(15361)
    , HEIGHT(15361)
    , mReady(false)
{
    mFilename = "heightmaps/";
    mFilename += planet_name;
    mFilename += ".hmpw";
    Connect();

    boost::thread t(std::bind(&Heightmap::RunThread, this));
    mThread = boost::move(t);

    mExit = false;
}

bool Heightmap::isReady()
{
    mReadyMutex.lock();
    bool isReady = mReady;
    mReadyMutex.unlock();

    return isReady;
}

// Never used
Heightmap::Heightmap()
{
}

//=============================================================================

Heightmap::~Heightmap()
{

    mExit = true;

    mThread.interrupt();
    mThread.join();

    if(Open())
    {
        fclose(hmp);
    }
    if (mHeightmapCache)
    {
        for (int i = 0; i < mCacheHeight; i++)
        {
            delete [] mHeightmapCache[i];
        }
        delete [] mHeightmapCache;
        mHeightmapCache = NULL;
    }

    mInstance = NULL;
}

//=============================================================================

Heightmap*	Heightmap::mInstance = NULL;
bool Heightmap::mCacheAvaliable = false;

//======================================================================================================================

Heightmap* Heightmap::Instance(uint16 resolution)
{
    if (!mInstance)
    {
        mInstance = new Heightmap(gWorldManager->getPlanetNameThis(), resolution);
    }
    return mInstance;
}

//=============================================================================
//	DO NOT AND I REPEAT DO NOT USE THIS FOR ---ANYTHING---
//	EXCEPT FOR ONE TIME READS LIKE GETTING THE HEIGHT FOR
//	PLAYER BUILDING PLACEMENT!!!
//

void Heightmap::fillInIterator(HeightResultMap::iterator it)
{
    if(!Open())
    {
        Connect();
        if(!Open())
        {
            DLOG(WARNING) << "Heightmap::ERROR: Unable to retrieve height. A connection to the zone heightmap was not established!";
            return;
        }
    }

    short height;
    fseek(hmp,getOffset(it->first.first,it->first.second),SEEK_SET);
    size_t result = fread(&height,2,1,hmp);
    if (! result) {
        DLOG(WARNING) << "Heightmap::ERROR: Unable to read height!";
        return;
    }

    heightResult* heightRes = new heightResult;

    if(height & 0x4000)//15th bit
        heightRes->hasWater = true;
    else
        heightRes->hasWater = false;

    height &= 0x7FFF;
    heightRes->height = ((float)height)/10;

    it->second = heightRes;
}


void Heightmap::RunThread()
{
    // create a height-map cashe.
    DLOG(WARNING) << "Height map resolution = " << mResolution;

    DLOG(WARNING) << "Starting Heightmap Cache Creation. This might take a while!";
    if (setupCache(mResolution))
    {
        DLOG(WARNING) << "Height map cache setup successfully with resolution " << mResolution;
    }
    else
    {
        DLOG(WARNING) << "WorldManager::_handleLoadComplete heigthmap cache setup FAILED";
    }

    mReadyMutex.lock();
    mReady = true;
    mReadyMutex.unlock();

    while(!mExit)
    {
        mJobMutex.lock();

        HeightmapAsyncContainer* job;
        if(Jobs.size() != 0)
        {
            job = Jobs.front();
            Jobs.pop();
        }
        else
            job = NULL;

        mJobMutex.unlock();

        if(job)
        {
            HeightResultMap* map = job->getResults();
            for(HeightResultMap::iterator it=map->begin(); it != map->end(); it++)
                fillInIterator(it);

            job->getCallback()->heightMapCallback(job);
        }
        else
        {
            //We sleep if there is no work!
            boost::this_thread::sleep(boost::posix_time::milliseconds(20));
        }
    }
    LOG(WARNING) << "HeightMap Thread Down!";
}


//=============================================================================
//
//	Dumps raw height variables. After recieving the data to get a proper height
//	value one must make sure the water bit is 0, cast to a float, then divide by 10.
//	see getHeight for an example how this is done.

bool Heightmap::getRow(unsigned char* buffer, int32 x, int32 z, int32 length)
{
    if ((x < -(WIDTH - 1)/2) || (x > (WIDTH- 1)/2))
    {
        LOG(WARNING) << "Heightmap::ERROR: Invalid input: x out of range x = "<< x<< ",  -WIDTH/2 = "<< -WIDTH/2;
        assert(false && "Heightmap::getRow x out of range");
        return false;
    }
    if ((z < -(HEIGHT- 1)/2) || (z > (HEIGHT - 1)/2))
    {
        LOG(WARNING) << "Heightmap::ERROR: Invalid input: x out of range z = "<< z<< ",  -WIDTH/2 = "<< -WIDTH/2;
        assert(false && "Heightmap::getRow z out of range");
        return false;
    }
    if (length < 0)
    {
        LOG(WARNING) << "Heightmap::ERROR: Invalid input: length is negative";
        assert(false && "Heightmap::getRow length is negative");
        return false;
    }
    if (length == 0)
    {
        // That's fine, we are done.
        return true;
    }

    if(!Open())
    {
        Connect();
        if(!Open())
        {
            LOG(WARNING) << "Heightmap::ERROR: Unable to retrieve height data.";
            assert(false && "Heightmap::getRow Missing heightmap, contact a SWG:ANH Developer for info.");
            return false;
        }
    }
    int32 startOffset = 2 * (((HEIGHT/2 - z) * WIDTH) + (x + WIDTH/2));
    int32 endOffset = startOffset + (2 * length);

    if (endOffset > (int32)(2 * WIDTH * HEIGHT))
    {
        LOG(WARNING) << "Heightmap::ERROR: Invalid input: length out of range endOffset = " << endOffset << ", (2 * WIDTH * HEIGHT) = "<< (int32)(2 * WIDTH * HEIGHT);
        assert(false && "Heightmap::getRow length out of range");
        return false;
    }
    int32 len = (endOffset - startOffset);
    if (len != (2 * length))
    {
        assert(false);
    }

    if (fseek(hmp,startOffset,SEEK_SET) != 0)
    {
        DLOG(WARNING) << "Heightmap::ERROR: File seek error";
        assert(false);
        return false;
    }

    int32 bytesRead = fread(buffer,1, len, hmp);
    if (bytesRead != len)
    {
        DLOG(WARNING) << "Heightmap::ERROR: File read error";
        assert(false);
        return false;
    }
    return true;
}

//=============================================================================
//

void Heightmap::Connect(void)
{

    hmp = fopen(mFilename.c_str(),"r+b");
    if(!hmp)
    {
        LOG(FATAL) << "Heightmap::Heightmap not found [ "<<mFilename.c_str()<<" ], exiting...";
    }
    else
    {
        LOG(WARNING) << "Heightmap succesfully opened!";
    }
    return;
}

//=============================================================================
//

unsigned long Heightmap::getOffset(float x, float y) const
{
    unsigned int x_trans = round_coord(x) + (WIDTH>>1);
    unsigned int y_trans = (HEIGHT>>1) - round_coord(y);
    return (y_trans*WIDTH + x_trans)<<1;
}

//=============================================================================
//

int32 Heightmap::round_coord(float coord) const
{
    return coord >= 0 ? (int32)(coord+0.5) : (int32)(coord-0.5);
}


static const int32 heightMapHeight = 15361;
static const int32 heightMapWidth = 15361;

bool Heightmap::setupCache(int16 cacheResoulutionDivider)
{

    bool status = false;

    // for test
    float min = FLT_MAX;
    float max = FLT_MIN;
    int32 xPosMin = 0;
    int32 zPosMin = 0;
    int32 xPosMax = 0;
    int32 zPosMax = 0;

    if (mCacheAvaliable)
    {
        assert (false && "Heightmap::setupCache cache already setup");		// Should only be initialized once
        return status;
    }
    if ((cacheResoulutionDivider < 0) || (cacheResoulutionDivider > 3))
    {
        // Invalid input.
        assert(false && "Heightmap::setupCache invalid input");
        return status;
    }

    if (cacheResoulutionDivider == 0)
    {
        // We do not want to use any cache.
        return true;
    }

    mCacheResoulutionDivider = cacheResoulutionDivider;

    mCacheHeight = ((heightMapHeight - 1)/mCacheResoulutionDivider) + 1;
    mCacheWidth = ((heightMapWidth -1)/mCacheResoulutionDivider) + 1;

    // static int32 cacheHeight = 7681;
    // static int32 cacheWidth = 7681;

    // Allocate memory for the heightmap.
    mHeightmapCache = new int16*[mCacheHeight];
    if (!mHeightmapCache)
    {
        assert (mHeightmapCache != NULL && "Heightmap::setupCache unable to allocate memory for heightmap");
        return false;
    }

    for (int i = 0; i < mCacheWidth; i++)
    {
        mHeightmapCache[i] = new int16[mCacheWidth];
        if (!mHeightmapCache[i])
        {
            return false;
        }
    }

    // Allocate array for the temporarily data contained in one line.
    uint16 *heightMapRow = NULL;
    heightMapRow = new uint16[heightMapWidth];
    if (!heightMapRow)
    {
        assert (heightMapRow != NULL && "Heightmap::setupCache unable to allocate memory for heightmap row");
        return false;
    }

    int32 zPos = 0;
    int32 xPos = 0;

    // int64 noOfIgnored = 0;

    // Let's read all height map data from file, and store some of it in memory.
    // for (int32 heightMapLine = -(heightMapHeight/2); heightMapLine <= (heightMapHeight/2); heightMapLine += 2)
    for (int32 heightMapLine = -(heightMapHeight/2); heightMapLine <= (heightMapHeight/2); heightMapLine += mCacheResoulutionDivider)
    {
        status = getRow((unsigned char *)heightMapRow, -(heightMapWidth/2), heightMapLine, heightMapWidth);
        if (status)
        {
            xPos = 0;
            for (int i = 0; i < heightMapWidth; i += mCacheResoulutionDivider)
            {
                if (heightMapRow[i] & 0x80000)
                {
                    DLOG(INFO) << "Found water, at position " << (-heightMapWidth/2) + i << " " << heightMapLine;
                }

                // Let's do this the right way, shall we? Pretend that we have a 15 bits signed value...

                // Remove the water bit.
                uint16 unsignedFix = (heightMapRow[i] & 0x7FFF);

                // Shift all bits to the left.
                unsignedFix = unsignedFix << 1;

                // Now make the data signed.
                int16 signedFix = (int16)unsignedFix;

                // Convert it to float.
                float value = (float)(signedFix);

                // Normalize data.
                value = value/20;

                // if ((x != 0) && (x != heightMapWidth - 1) && (z != 0) && (z != heightMapHeight - 1))
                {

                    // float value = (float)(heightMapRow[x] & 0x7FFF)/10;
                    if (value < min)
                    {
                        min = value;
                        xPosMin = (-heightMapWidth/2) + i;
                        zPosMin = heightMapLine;
                    }

                    /*
                    if (value < -135)
                    {
                    	noOfIgnored++;
                    }

                    if (value > 431)
                    {
                    	noOfIgnored++;
                    }
                    */

                    if (value > max)
                    {
                        max = value;
                        xPosMax = (-heightMapWidth/2) + i;
                        zPosMax = heightMapLine;
                    }
                }

                mHeightmapCache[zPos][xPos] = signedFix;
                xPos++;
                // heightMapCache[z][x/2] = (float)(heightMapRow[x] &= 0x7FFF)/10;
            }
            zPos++;
        }
        else
        {
            // Not all zones support heightmaps
            // assert(false);
        }
    }
    delete [] heightMapRow;

    mCacheAvaliable = status;

    if (status)
    {
        DLOG(INFO) << "Min height = " << min<<" at position "<< xPosMin << ","<< zPosMin;
        DLOG(INFO) << "Max height = " << max<<" at position "<< xPosMax << ","<< zPosMax;
    }
    return status;
}

//=============================================================================
//
//	Retrieve the height from the cache for a given 2D x,z position.
//

float Heightmap::getCachedHeight(float xPos, float zPos) const
{
    float yPos = FLT_MIN;
    if (mCacheAvaliable)
    {
        int32 x = round_coord(xPos) + (heightMapHeight/2);
        int32 z = round_coord(zPos) + (heightMapWidth/2);

        yPos = (float)(mHeightmapCache[z/mCacheResoulutionDivider][x/mCacheResoulutionDivider])/20;
    }
    return yPos;
}

float Heightmap::getHeight(float x, float y)
{
    if(!Open())
    {
        Connect();
        if(!Open())
        {
            DLOG(WARNING) << "Heightmap::ERROR: Unable to retrieve height. A connection to the zone heightmap was not established!";
            return FLT_MIN;
        }
    }

    short height;
    fseek(hmp,getOffset(x,y),SEEK_SET);
    size_t result = fread(&height,2,1,hmp);
    if (! result) {
        DLOG(WARNING) << "Heightmap::ERROR: Unable to read height!";
        return FLT_MIN;
    }
    height &= 0x7FFF;
    return ((float)height)/10;
}

//=============================================================================
//
//	Return the more 'accurate' height given two values and an allowed deviation.
//  TODO: Re-evaluate need for this method once heightmaps are corrected.
//
float Heightmap::compensateForInvalidHeightmap(float hmapRes, float clientRes, float allowedDeviation) {
    if(hmapRes != clientRes) {
        if(fabs(hmapRes-clientRes) > allowedDeviation) {
            return clientRes;
        }
    }
    return hmapRes;
}
