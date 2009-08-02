/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_HEIGHTMAP_H
#define ANH_ZONESERVER_HEIGHTMAP_H

#define     gHeightmap    Heightmap::getSingletonPtr()

#include "Utils/typedefs.h"
#include "MathLib/Vector3.h"

class Heightmap
{

	public:

		Heightmap(const char* planet_name): WIDTH(15361),HEIGHT(15361)
		{
			mFilename = planet_name;
			mFilename += ".hmp";
			Connect();
		}

		~Heightmap()
		{
			if(Open())
			{
				fclose(hmp);
			}
		}

		static Heightmap*  Init(const char* planet_name)
		{
			 if(mInsFlag == false)
			{
				mSingleton = new Heightmap(planet_name);
				mInsFlag = true;
				return mSingleton;
			}
			else
				return mSingleton;
		    return NULL;
		}
		static Heightmap*  getSingletonPtr()  { return mSingleton; }

		const char* getFilename() { return mFilename.c_str(); }
		void setFilename(std::string filename) { mFilename = filename; }

		//DO NOT AND I REPEAT DO NOT USE THIS FOR ---ANYTHING---
		//EXCEPT FOR ONE TIME READS LIKE GETTING THE HEIGHT FOR
		//PLAYER BUILDING PLACEMENT!!!
		float getHeight(float x, float y)
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
			fread(&height,2,1,hmp);
			height &= 0x7FFF; 
			return ((float)height)/10;
		}

		//Dumps raw height variables. After recieving the row to get a proper height
		//value one must make sure the water bit is 0, cast to a float, then divide by 10.
		//see getHeight for an example how this is done.
		bool getRow(unsigned char* buffer,float x, float start_y,float end_y)
		{			
			if(!Open())
			{
				Connect();
				if(!Open()) 
				{
					gLogger->logMsg("Heightmap::ERROR: Unable to retrieve height row. A connection to the zone heightmap was not established!",FOREGROUND_RED);
					return false;
				}
			}

			unsigned int startOffset = getOffset(x,start_y);
			unsigned int endOffset = getOffset(x,end_y);
			if(startOffset > endOffset)
			{
				unsigned long holder = startOffset;
				startOffset = endOffset;
				endOffset = holder;
			}

			int len = (endOffset - startOffset)+2;
			fseek(hmp,startOffset,SEEK_SET);
			fread(&buffer,len,1,hmp);
			return true;
		}

		//DO NOT AND I REPEAT DO NOT USE THIS FOR ---ANYTHING---
		//EXCEPT FOR ONE TIME READS LIKE GETTING THE HEIGHT FOR
		//PLAYER BUILDING PLACEMENT!!!
		bool hasWater(float x, float y)
		{
			if(!Open())
			{
				Connect();
				if(!Open()) 
				{
					gLogger->logMsg("Heightmap::ERROR: Unable to tell if position has water. A connection to the zone heightmap was not established!",FOREGROUND_RED | BACKGROUND_BLUE);
					return false;
				}
			}

			short height;
			fseek(hmp,getOffset(x,y),SEEK_SET);				
			fread(&height,2,1,hmp);
			if(height & 0x4000)//15th bit
				return true;

			return false; 
		}

		bool Open() { if(hmp) return true; else return false;  }
		void Connect()
		{
			hmp = fopen(mFilename.c_str(),"r+b");
			if(!hmp)
			{
				char err[255];
				sprintf(err,"Heightmap::ERROR: Unable to open the heightmap file: %s!",mFilename.c_str());
				gLogger->logMsg(err,FOREGROUND_RED);
			}
			else
			{
				gLogger->logMsgF("Heightmap::Successfully opened %s.",MSG_NORMAL, mFilename.c_str());
			}
		  return;
		}

		unsigned long getOffset(float x, float y)
		{
			unsigned int x_trans = round_coord(x) + WIDTH/2;
			unsigned int y_trans = HEIGHT/2 - round_coord(y);
			return 2*(y_trans*WIDTH + x_trans);
		}

		int round_coord(float coord) 
		{
			return coord >= 0 ? (int)(coord+0.5) : (int)(coord-0.5);
		}



	protected:
	
		static Heightmap*  mSingleton;
		static bool        mInsFlag;
		FILE * hmp; //file pointer to the highmap
		std::string mFilename;

		int		WIDTH;
		int     HEIGHT;
	    

};

#endif