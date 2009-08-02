/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "clock.h"
#include <assert.h>


//======================================================================================================================

Anh_Utils::Clock*	Anh_Utils::Clock::mSingleton = NULL;

//======================================================================================================================

/*
Anh_Utils::Clock* Anh_Utils::Clock::getSingleton()
{
	if(!mSingleton)
	{
		mSingleton = new Anh_Utils::Clock();
	}
	return mSingleton;
}
*/
//==============================================================================================================================

void Anh_Utils::Clock::destroySingleton()
{
  delete mSingleton;
  mSingleton = 0;
}

//======================================================================================================================

Anh_Utils::Clock::~Clock()
{

}

//======================================================================================================================
//
// Windows
//
#if(ANH_PLATFORM == ANH_PLATFORM_WIN32)

Anh_Utils::Clock::Clock()
{
	timeBeginPeriod(1);
	// Update();
}

//==============================================================================================================================

/*
void Anh_Utils::Clock::Update(void)
{
	mLocalTime = timeGetTime();
	mGlobalTime = mLocalTime + mGlobalDrift;
}
*/

//==============================================================================================================================

char* Anh_Utils::Clock::GetCurrentDateTimeString(void)
{
  time_t ltime;
  time( &ltime);
  return ctime(&ltime);
}

//==============================================================================================================================
//
// Linux
//
#elif(ANH_PLATFORM == ANH_PLATFORM_LINUX)

uint Anh_Utils::Clock::mTime = 0;

//==============================================================================================================================

Anh_Utils::Clock::Clock()
{
	Update();
}

//==============================================================================================================================

void Anh_Utils::Clock::Update(void)
{
	assert(mInsFlag);

	ftime(&mTimeVal);
	mTime = (mTimeVal.time * 1000) + mTimeVal.millitm;
}

//==============================================================================================================================

void Anh_Utils::Clock::GetTimeString(char* str)
{
  //time_t ltime;
  //time( &ltime);
  //str = ctime(&ltime);
}

//==============================================================================================================================

#endif // Linux

