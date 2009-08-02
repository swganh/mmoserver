#ifndef	ANH_UTILS_THREAD_H
#define	ANH_UTILS_THREAD_H

#include "typedefs.h"

#if(ANH_PLATFORM == ANH_PLATFORM_WIN32)
	unsigned long __stdcall threadProc(void* data);
#elif(ANH_PLATFORM == ANH_PLATFORM_LINUX)
	void* threadProc(void* data);
#endif

//==============================================================================================================================

class Thread
{
	public:
		Thread();
	  virtual	~Thread();

		bool			IsDone(void) const;
		bool			IsRunning(void) const;
		virtual void	Run(void) = 0;
		void			StartThread(int priority = 0);
		void			Stop(void);
		void			SetRunning(bool status);
	protected:
		void			SetDone(bool isDone);
	private:
		bool			mDone;
		bool			mIsRunning;
		unsigned int	mThreadId;
		int				mPriority;
private:

		
};


//==============================================================================================================================
inline Thread::Thread()
{
	mDone			= false;
	mIsRunning		= false;
	mThreadId		= 0;
	mPriority		= 0;
}


//==============================================================================================================================
inline bool Thread::IsDone(void) const
{
	bool r = mDone;
	return r;
}


//==============================================================================================================================
inline void Thread::SetDone(bool done)
{
	mDone = done;
}


//==============================================================================================================================
inline void Thread::SetRunning(bool status)
{
	mIsRunning = status;
}


//==============================================================================================================================
inline bool Thread::IsRunning(void) const
{
	return mIsRunning;
}


//==============================================================================================================================
inline void Thread::Stop(void)
{
	mIsRunning = false;
	mDone = true;
}

#endif	// ANH_UTILS_THREAD_H

