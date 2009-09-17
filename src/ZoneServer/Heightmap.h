/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_HEIGHTMAP_H
#define ANH_ZONESERVER_HEIGHTMAP_H

// #define     gHeightmap    Heightmap::getSingletonPtr()

#include "Utils/typedefs.h"
// #include "MathLib/Vector3.h"

class Heightmap
{

	public:
		static Heightmap*  Instance(void);
		static void deleter(void)    
		{ 
			if (mInstance)
			{
				delete mInstance;
				mInstance = 0;
			}
		}

		//DO NOT AND I REPEAT DO NOT USE THIS FOR ---ANYTHING---
		//EXCEPT FOR ONE TIME READS LIKE GETTING THE HEIGHT FOR
		//PLAYER BUILDING PLACEMENT!!!
		float getHeight(float x, float y);

		//Dumps raw height variables. After recieving the data to get a proper height
		//value one must make sure the water bit is 0, cast to a float, then divide by 10.
		//see getHeight for an example how this is done.
		bool getRow(unsigned char* buffer, int32 x, int32 y, int32 length);

		float getCachedHeightAt2DPosition(float xPos, float zPos) const;

		//DO NOT AND I REPEAT DO NOT USE THIS FOR ---ANYTHING---
		//EXCEPT FOR ONE TIME READS LIKE GETTING THE HEIGHT FOR
		//PLAYER BUILDING PLACEMENT!!!
		bool hasWater(float x, float y);

		void Connect();
		bool Open(void) { if(hmp) return true; else return false;  }
		bool setupCache(int16 cacheResoulutionDivider);
		static inline bool isHeightmapCacheAvaliable(void) { return mCacheAvaliable;}
		inline bool isHighResCache(void) { return (mCacheResoulutionDivider == 1);}

	protected:
		Heightmap(const char* planet_name);
		~Heightmap();

	private:
		// This constructor prevents the default constructor to be used, since it is private.
		Heightmap();

		const char* getFilename() const { return mFilename.c_str(); }
		
		void setFilename(std::string filename) { mFilename = filename; }

		unsigned long getOffset(float x, float y) const ;

		int32 round_coord(float coord) const;
		float	**mHeightmapCache;
		int32	mCacheHeight;
		int32	mCacheWidth;
		int16	mCacheResoulutionDivider;

		static bool	mCacheAvaliable;


	protected:
	
		static Heightmap*  mInstance;
		// static bool        mInsFlag;
		FILE * hmp; //file pointer to the highmap
		std::string mFilename;

		int32		WIDTH;
		int32     HEIGHT;
	    

};

#endif