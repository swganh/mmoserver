/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/
#include "Heightmap.h"
#include "ZoneServer/WorldManager.h"
#include "LogManager/LogManager.h"
#include "Utils/utils.h"
#include <cassert>
#include <cfloat>

//=============================================================================
Heightmap::Heightmap(const char* planet_name)
: mHeightmapCache(NULL)
, mCacheHeight(0)
, mCacheWidth(0)
, mCacheResoulutionDivider(3)
, WIDTH(15361)
, HEIGHT(15361)
{
	mFilename = planet_name;
	mFilename += ".hmp";
	Connect();
}

// Never used
Heightmap::Heightmap()
{
}

//=============================================================================

Heightmap::~Heightmap()
{
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

Heightmap* Heightmap::Instance(void)
{
	if (!mInstance)
	{
		mInstance = new Heightmap(gWorldManager->getPlanetNameThis());
	}
	return mInstance;
}

//=============================================================================
//	DO NOT AND I REPEAT DO NOT USE THIS FOR ---ANYTHING---
//	EXCEPT FOR ONE TIME READS LIKE GETTING THE HEIGHT FOR
//	PLAYER BUILDING PLACEMENT!!!
//

float Heightmap::getHeight(float x, float y)
{
	if(!Open())
	{
		Connect();
		if(!Open())
		{
			gLogger->logMsg("Heightmap::ERROR: Unable to retrieve height. A connection to the zone heightmap was not established!",FOREGROUND_RED);
			return FLT_MIN;
		}
	}

	short height;
	fseek(hmp,getOffset(x,y),SEEK_SET);
	size_t result = fread(&height,2,1,hmp);
	if (! result) {
		gLogger->logMsg("Heightmap::ERROR: Unable to read height!",FOREGROUND_RED);
		return FLT_MIN;
	}
	height &= 0x7FFF;
	return ((float)height)/10;
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
		gLogger->logMsg("Heightmap::ERROR: Invalid input: x out of range",FOREGROUND_RED);
		gLogger->logMsgF("x = %d,  -WIDTH/2 = %d", MSG_NORMAL, x, -WIDTH/2);
		assert(false && "Heightmap::getRow x out of range");
		return false;
	}
	if ((z < -(HEIGHT- 1)/2) || (z > (HEIGHT - 1)/2))
	{
		gLogger->logMsg("Heightmap::ERROR: Invalid input: z out of range",FOREGROUND_RED);
		assert(false && "Heightmap::getRow z out of range");
		return false;
	}
	if (length < 0)
	{
		gLogger->logMsg("Heightmap::ERROR: Invalid input: length is negative",FOREGROUND_RED);
		assert(false && "Heightmap::getRow length is negative");
		return false;
	}
	if (length == 0)
	{
		// That's fine, we are done.
		return true;
	}

	// gLogger->logMsgF("Heightmap::getRow Reading %d bytes from position %d, %d",MSG_NORMAL, length, x, z);

	if(!Open())
	{
		Connect();
		if(!Open())
		{
			gLogger->logMsg("Heightmap::ERROR: Unable to retrieve height data.",FOREGROUND_RED);
			assert(false && "Heightmap::getRow Missing heightmap, download at http://www.swganh.com/!!planets!!/PLANET_NAME.rar");
			return false;
		}
	}
	int32 startOffset = 2 * (((HEIGHT/2 - z) * WIDTH) + (x + WIDTH/2));
	// gLogger->logMsgF("startOffset = %d", MSG_NORMAL, startOffset);
	int32 endOffset = startOffset + (2 * length);
	// gLogger->logMsgF("endOffset = %d", MSG_NORMAL, endOffset);

	if (endOffset > (int32)(2 * WIDTH * HEIGHT))
	{
		gLogger->logMsg("Heightmap::ERROR: Invalid input: length out of range",FOREGROUND_RED);
		gLogger->logMsgF("endOffset = %d, (2 * WIDTH * HEIGHT) = %d", MSG_NORMAL, endOffset, (int32)(2 * WIDTH * HEIGHT));
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
		gLogger->logMsg("Heightmap::ERROR: File seek error",FOREGROUND_RED);
		assert(false);
		return false;
	}

	int32 bytesRead = fread(buffer,1, len, hmp);
	if (bytesRead != len)
	{
		gLogger->logMsg("Heightmap::ERROR: File read error",FOREGROUND_RED);
		gLogger->logMsgF("bytesRead = %d", MSG_NORMAL, bytesRead);
		assert(false);
		return false;
	}
	return true;
}

//=============================================================================
//
//	DO NOT AND I REPEAT DO NOT USE THIS FOR ---ANYTHING---
//	EXCEPT FOR ONE TIME READS LIKE GETTING THE HEIGHT FOR
//	PLAYER BUILDING PLACEMENT!!!
bool Heightmap::hasWater(float x, float y)
{
	if(!Open())
	{
		Connect();
		if(!Open())
		{
			gLogger->logMsg("Heightmap::ERROR: Unable to tell if position has water. A connection to the zone heightmap was not established!\n",FOREGROUND_RED | BACKGROUND_BLUE);
			return false;
		}
	}

	short height;
	fseek(hmp,getOffset(x,y),SEEK_SET);
	size_t result = fread(&height,2,1,hmp);
	if (! result) {
		gLogger->logMsg("Heightmap::ERROR: Unable to tell if position has water!",FOREGROUND_RED);
		return false;
	}

	if(height & 0x4000)//15th bit
		return true;

	return false;
}

//=============================================================================
//

void Heightmap::Connect(void)
{
	hmp = fopen(mFilename.c_str(),"r+b");
	if(!hmp)
	{
		gLogger->logMsgLoadFailure("Heightmap::Heightmap not found [ %s ], exiting...",MSG_HIGH,mFilename.c_str());

	}
	else
	{
		gLogger->logMsgLoadSuccess("Heightmap::Heightmap succesfully opened...",MSG_NORMAL);
	}
  return;
}

//=============================================================================
//

unsigned long Heightmap::getOffset(float x, float y) const
{
	unsigned int x_trans = round_coord(x) + WIDTH/2;
	unsigned int y_trans = HEIGHT/2 - round_coord(y);
	return 2*(y_trans*WIDTH + x_trans);
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
	mHeightmapCache = new float*[mCacheHeight];
	if (!mHeightmapCache)
	{
		assert (mHeightmapCache != NULL && "Heightmap::setupCache unable to allocate memory for heightmap");
		return false;
	}

	for (int i = 0; i < mCacheWidth; i++)
	{
		mHeightmapCache[i] = new float[mCacheWidth];
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
					gLogger->logMsgF("Found water, at position %d, %d", MSG_NORMAL, (-heightMapWidth/2) + i, heightMapLine);
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
						// Print all starange ones...
						gLogger->logMsgF("Height = %.2f at position %d, %d", MSG_NORMAL, value, (-heightMapWidth/2) + x, heightMapLine);
					}

					if (value > 431)
					{
						noOfIgnored++;
						// Print all starange ones...
						gLogger->logMsgF("Height = %.2f at position %d, %d", MSG_NORMAL, value, (-heightMapWidth/2) + x, heightMapLine);
					}
					*/

					if (value > max)
					{
						max = value;
						xPosMax = (-heightMapWidth/2) + i;
						zPosMax = heightMapLine;
					}
				}

				mHeightmapCache[zPos][xPos] = value;
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
	gLogger->logMsgF("Have created a %d * %d heighmap cache.", MSG_NORMAL, zPos, xPos);

	delete [] heightMapRow;

	mCacheAvaliable = status;

	if (status)
	{
		gLogger->logMsgF("Min height = %.2f at position %d, %d", MSG_NORMAL, min, xPosMin, zPosMin);
		gLogger->logMsgF("Max height = %.2f at position %d, %d", MSG_NORMAL, max, xPosMax, zPosMax);
		// gLogger->logMsgF("noOfIgnored values are = %"PRIu64"", MSG_NORMAL, noOfIgnored);

	}
	return status;
}

//=============================================================================
//
//	Retrieve the height from the cache for a given 2D x,z position.
//

float Heightmap::getCachedHeightAt2DPosition(float xPos, float zPos) const
{
	float yPos = FLT_MIN;
	if (mCacheAvaliable)
	{
		int32 x = round_coord(xPos) + (heightMapHeight/2);
		int32 z = round_coord(zPos) + (heightMapWidth/2);

		yPos = mHeightmapCache[z/mCacheResoulutionDivider][x/mCacheResoulutionDivider];
	}
	return yPos;
}

