/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_UTILS_CLOCK_H
#define ANH_UTILS_CLOCK_H

#include <ctime> 
#include "typedefs.h"

//==============================================================================================================================

namespace Anh_Utils
{
	//==============================================================================================================================
	//
	// Windows
	//
	#if(ANH_PLATFORM == ANH_PLATFORM_WIN32)

	class Clock
	{
		public:
			static inline Clock* getSingleton()
			{	
				if (!mSingleton)
				{
					mSingleton = new Anh_Utils::Clock();
				}
				return mSingleton;
			}
			static void		destroySingleton(void); 

		protected:
			Clock();
			~Clock();

		public:
			char*		    GetCurrentDateTimeString(void);

			inline uint64	getGlobalTime(void) const { return timeGetTime() + mGlobalDrift; } 
			inline uint64	getLocalTime(void) const { return timeGetTime(); }

			int64			setGlobalDrift(int64 drift) { mGlobalDrift = drift; };


		private:
			int64	          mGlobalDrift;      // The amount of time the local clock is from the global system clock

			static Clock*	  mSingleton;
	};

	//==============================================================================================================================
	//
	// Linux
	//
	#elif(ANH_PLATFORM == ANH_PLATFORM_LINUX)
	// NOTE: This LINUX version is NOT compiled or verified.
	class Clock
	{
		public:
			static inline Clock* getSingleton()
			{	
				if (!mSingleton)
				{
					mSingleton = new Anh_Utils::Clock();
				}
				return mSingleton;
			}
			static void		destroySingleton(void); 

		protected:
			Clock();
			~Clock();

		public:
			char*		    GetCurrentDateTimeString(void);

			inline uint64	getGlobalTime(void) const { return timeGetTime() + mGlobalDrift; } 
			inline uint64	getLocalTime(void) const { return timeGetTime(); }

			int64			setGlobalDrift(int64 drift) { mGlobalDrift = drift; };


		private:
			int64	          mGlobalDrift;      // The amount of time the local clock is from the global system clock

			static Clock*	  mSingleton;
	};
	/*
	class Clock
	{
		public:

			static Clock*	getSingleton();
			static void		destroySingleton(void); 
			//static Clock*	getSingletonPtr() { return mSingleton; }

			Clock();
			~Clock();

			void			      Update(void);
			char*			      GetCurrentDateTimeString(void);

			uint64			    getGlobalTime(void) const { return mGlobalTime; }; 
			uint64			    getLocalTime(void) const { return mLocalTime; }; 

			inline uint64	  UpdateAndGetTime()
			{
				Update();
				return(mLocalTime);
			};
	  
			int64			      setGlobalDrift(int64 drift) { mGlobalDrift = drift; };


		private:

			uint64	        mGlobalTime;       // This is set to the running global system time by the application
			uint64	        mLocalTime;        // This is always the local 1ms tick count since boot.
			int64	          mGlobalDrift;      // The amount of time the local clock is from the global system clock

			static Clock*	  mSingleton;
	};
	*/
	#endif // Linux
}

// #define gClock	Anh_Utils::Clock::getSingleton()

//==============================================================================================================================

#endif

