/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef	ANH_UTILS_MUTEX_H
#define	ANH_UTILS_MUTEX_H

#include "typedefs.h"

// This should be split into two different classe.  A FastLock that uses Critical Sections (will have to write 
// something for other OS) and an actual Mutex, which will use the OS mutex.
//==============================================================================================================================

#if(ANH_PLATFORM == ANH_PLATFORM_WIN32)

//==============================================================================================================================
class Mutex
{
public:
	Mutex();
	~Mutex();
	void	Lock(void);
	bool  TryLock(void);
	void	Unlock(void);
private:
	CRITICAL_SECTION	_criticalSection;
};


//==============================================================================================================================
inline Mutex::Mutex()
{
	InitializeCriticalSection(&_criticalSection);
}


//==============================================================================================================================
inline Mutex::~Mutex()
{
	DeleteCriticalSection(&_criticalSection);
}


//==============================================================================================================================
inline void Mutex::Lock(void)
{
	EnterCriticalSection(&_criticalSection);
}


//==============================================================================================================================
inline bool Mutex::TryLock(void)
{
	return TryEnterCriticalSection(&_criticalSection) != 0;
}


//==============================================================================================================================
inline void Mutex::Unlock(void)
{
	LeaveCriticalSection(&_criticalSection);
}


#elif(ANH_PLATFORM == ANH_PLATFORM_LINUX)

//==============================================================================================================================
class Mutex
{
public:
	Mutex();
	~Mutex();
	void	Lock(void);
	bool  TryLock(void);
	void	Unlock(void);
private:
	pthread_mutex_t					mMutex;
	pthread_mutexattr_t			mAttributes;
};


//==============================================================================================================================
inline Mutex::Mutex()
{
	pthread_mutex_init(&mMutex, &mAttributes);
}


//==============================================================================================================================
inline Mutex::~Mutex()
{
	pthread_mutex_destroy(&mMutex);
}


//==============================================================================================================================
inline void Mutex::Lock(void)
{
	pthread_mutex_lock(&mMutex);
}


//==============================================================================================================================
inline bool Mutex::TryLock(void)
{
	return (bool)pthread_mutex_trylock(&mMutex);
}


//==============================================================================================================================
inline void Mutex::Unlock(void)
{
	pthread_mutex_unlock(&mMutex);
}


#endif // Linux

#endif //MUTEX_H

