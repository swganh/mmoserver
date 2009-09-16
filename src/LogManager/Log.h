/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_LOGMANAGER_LOG_H
#define ANH_LOGMANAGER_LOG_H

#include "Utils/typedefs.h"
#include <fstream>
#include <iostream>
#include <string>
#include "DatabaseManager/Database.h"

#ifdef ANH_PLATFORM_WIN32

//#include <stdlib.h>
//#include <shellapi.h>
#endif

#ifdef ANH_PLATFORM_LINUX
#define FOREGROUND_BLUE 0x0001
#define FOREGROUND_GREEN 0x0002
#define FOREGROUND_RED 0x0004
#define BACKGROUND_BLUE 0x0010
#define BACKGROUND_GREEN 0x0020
#define BACKGROUND_RED 0x0040
#endif

#define LOG_DIR	"logs//"

//======================================================================================================================

enum GlobalLogLevel
{
  G_LEVEL_LOW     = 4,
  G_LEVEL_NORMAL  = 3,
  G_LEVEL_HIGH    = 2
};

enum LogLevel
{
  LEVEL_LOW       = 1,
  LEVEL_NORMAL    = 2,
  LEVEL_HIGH      = 3
};

enum MsgPriority
{
  MSG_LOW         = 1,
  MSG_NORMAL      = 2,
  MSG_HIGH        = 3
};


//======================================================================================================================

class Log
{
	public:

		Log(const std::string& name, LogLevel level, GlobalLogLevel globalLevel, bool fileOut, bool consoleOut, bool append, Database* database);
		~Log();

		void	logMsg(const std::string& zone, const std::string& system, const std::string& msg, MsgPriority priority, va_list args);
		void	logMsg(const std::string& msg, MsgPriority mp, bool fileOut, bool consoleOut, bool timestamp);
		void	logMsg(const std::string& msg, MsgPriority mp, bool fileOut, bool consoleOut, bool timestamp, int Color);
		void    logMsg(const std::string& msg, MsgPriority mp, va_list args);
		void	logMsgNolf(const std::string& msg, MsgPriority mp, bool fileOut, bool consoleOut, bool timestamp, int Color, va_list args);
		void	logMsgNolf(const std::string& msg, MsgPriority mp, bool fileOut, bool consoleOut, bool timestamp, int Color);
		uint16  logMsgNolf(const std::string& msg, MsgPriority mp, va_list args);
		void	hexDump(int8* data,uint32 len,MsgPriority mp);
		void    hexDump(int8* data,uint32 len,const char* filename);

		void    setLogLevel(LogLevel level) { mLogLevel = level; }
		void	setGlobalLogLevel(GlobalLogLevel level) { mGlobalLogLevel = level; }

	private:

		std::ofstream	mLogStream;
		std::string		mName;
		std::string		mZone;
		bool			mFileOut;
		bool			mDBOut;
		bool			mConsoleOut;
		LogLevel		mLogLevel;
		GlobalLogLevel	mGlobalLogLevel;
		ZThread::Mutex  mGlobalLogMutex;
		Database*		mDatabase;
};

//======================================================================================================================

#endif
