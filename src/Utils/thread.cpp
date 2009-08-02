/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "thread.h"
#include <assert.h>

//==============================================================================================================================
Thread::~Thread(void)
{
	SetDone(true);
	while(IsRunning())
	{
		msleep(1);
	}
}


//==============================================================================================================================
//
// Windows
//
#if(ANH_PLATFORM == ANH_PLATFORM_WIN32)

unsigned long __stdcall threadProc(void* data)
{
	Thread * thread = (Thread *)data;
	thread->SetRunning(true);
	thread->Run();
	thread->SetRunning(false);

	return 0;
}

// win32 priority -15 to 15
//==============================================================================================================================
void Thread::StartThread(int priority)
{
	assert(priority > -16 && priority < 16);

	mPriority = priority;
	unsigned long	tid; 
	mThreadId = (unsigned int) CreateThread(0, 0, threadProc, this, 0, &tid);
	SetThreadPriority((void*)mThreadId, mPriority);
}

//==============================================================================================================================
//
// Linux
//
#elif(ANH_PLATFORM == ANH_PLATFORM_LINUX)

//==============================================================================================================================
void* threadProc(void* data)
{
	Thread * thread = (Thread *)data;
	thread->SetRunning(true);
	thread->Run();
	thread->SetRunning(false);

	return 0;
}

//==============================================================================================================================
void Thread::StartThread(int priority)
{
	mPriority = priority;
	pthread_t	tid;
	mThreadId = (unsigned int) pthread_create(&tid, 0, threadProc, this);
	//SetThreadPriority((void*)mThreadId, mPriority);

}

#endif // Linux

