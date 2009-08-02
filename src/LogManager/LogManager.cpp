/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "LogManager.h"
#include <string>
#include <stdarg.h>
#include <stdio.h> 


//======================================================================================================================
LogManager*   LogManager::mSingleton  = NULL;


//======================================================================================================================
LogManager::LogManager(GlobalLogLevel glevel,const std::string& name,LogLevel level,bool fileOut,bool consoleOut,bool append)
{
	mDefaultLog = NULL;
	mGlobalLogLevel = glevel;
	createLog(name,level,fileOut,consoleOut,append);
}


//======================================================================================================================
LogManager* LogManager::Init(GlobalLogLevel glevel,const std::string& name,LogLevel level,bool fileOut,bool consoleOut,bool append)
{
	if(!mSingleton)
	{
		mSingleton = new LogManager(glevel,name,level,fileOut,consoleOut,append);
	}

  return mSingleton;
}


//======================================================================================================================
LogManager::~LogManager()
{
	LogList::iterator it = mLogs.begin();

	while(it != mLogs.end())
	{
		delete(it->second);
		mLogs.erase(it);
		it = mLogs.begin();
	}
}


//======================================================================================================================

Log* LogManager::createLog(const std::string& name,LogLevel level,bool fileOut,bool consoleOut,bool append)
{
	mGlobalLogMutex.acquire();

	Log* newLog = new Log(name, level, mGlobalLogLevel, fileOut,  consoleOut,append);

	mLogs.insert(LogList::value_type(name,newLog));

	if(!mDefaultLog)
	{
		mDefaultLog = newLog;
	}

	mGlobalLogMutex.release();

	return newLog;
}


//======================================================================================================================

Log* LogManager::getLog(const std::string& name)
{
	Log* log = 0;

	mGlobalLogMutex.acquire();

	LogList::iterator it = mLogs.find(name);

	if (it != mLogs.end())
	{
		log = it->second;
	}

	mGlobalLogMutex.release();

	return log;
}

//======================================================================================================================

void LogManager::logMsg(const std::string& logname,const std::string& msg,MsgPriority mp,bool fileOut,bool consoleOut,bool timestamp)
{
	mGlobalLogMutex.acquire();

	Log* tmpLog = getLog(logname);

	if(tmpLog != NULL)
	{
		tmpLog->logMsg(msg, mp, fileOut, consoleOut, timestamp);
	}

	mGlobalLogMutex.release();
}

//======================================================================================================================

void LogManager::logMsg(Log* log,const std::string& msg,MsgPriority mp,bool fileOut,bool consoleOut,bool timestamp)
{
	mGlobalLogMutex.acquire();

	log->logMsg(msg, mp, fileOut, consoleOut, timestamp);

	mGlobalLogMutex.release();
}

//======================================================================================================================
void LogManager::logMsg(const std::string& msg, int Color)
{
	logMsg(msg, MSG_NORMAL, true, true, true, Color);
}
//======================================================================================================================

void LogManager::logMsg(const std::string& msg,MsgPriority mp,bool fileOut,bool consoleOut,bool timestamp, int Color)
{
	mGlobalLogMutex.acquire();

	if(mDefaultLog != NULL)
	{
		mDefaultLog->logMsg(msg, mp, fileOut, consoleOut, timestamp, Color);
	}

	mGlobalLogMutex.release();
}

//======================================================================================================================

void LogManager::logMsgF(const std::string& msg, MsgPriority priority, ...)
{
	va_list args;

	mGlobalLogMutex.acquire();

	if(mDefaultLog != NULL)
	{
		mDefaultLog->logMsg(msg, priority, va_start(args, priority));
	}

	mGlobalLogMutex.release();
}

//======================================================================================================================

void LogManager::hexDump(int8* data,uint32 len,MsgPriority mp)
{
	mGlobalLogMutex.acquire();

	if(mDefaultLog != NULL)
	{
		mDefaultLog->hexDump(data,len,mp);
	}

	mGlobalLogMutex.release();
}

void LogManager::hexDump(int8* data,uint32 len,const char* filename)
{
	mGlobalLogMutex.acquire();

	if(mDefaultLog != NULL)
	{
		mDefaultLog->hexDump(data,len,filename);
	}

	mGlobalLogMutex.release();

}

//======================================================================================================================

void LogManager::logMsgF(Log* log, const std::string& msg, MsgPriority priority, ...)
{
	mGlobalLogMutex.acquire();

	va_list args;
	va_start(args, priority);

	log->logMsg(msg, priority, args);

	mGlobalLogMutex.release();
}

//======================================================================================================================

void LogManager::setLogLevel(const std::string& logname,LogLevel level)
{
	mGlobalLogMutex.acquire();

	Log* tmpLog = getLog(logname);

	if(tmpLog != NULL)
	{
		tmpLog->setLogLevel(level);
	}

	mGlobalLogMutex.release();
}

//======================================================================================================================

void LogManager::setLogLevel(Log* log,LogLevel level)
{
	mGlobalLogMutex.acquire();

	log->setLogLevel(level);

	mGlobalLogMutex.release();
}

//======================================================================================================================

void LogManager::setLogLevel(LogLevel level)
{
	mGlobalLogMutex.acquire();

	if(mDefaultLog)
	{
		mDefaultLog->setLogLevel(level);
	}

	mGlobalLogMutex.release();
}

//======================================================================================================================

Log* LogManager::setDefaultLog(Log* log)
{
	mGlobalLogMutex.acquire();

	Log* old = mDefaultLog;
	mDefaultLog = log;

	mGlobalLogMutex.release();

	return old;
}

//======================================================================================================================

void LogManager::setGlobalLogLevel(GlobalLogLevel glevel)
{
	mGlobalLogMutex.acquire();

	LogList::iterator it = mLogs.begin();

	while(it != mLogs.end())
	{
		it->second->setGlobalLogLevel(glevel);
		++it;
	}

	mGlobalLogMutex.release();
}

//======================================================================================================================





