/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2010 The SWG:ANH Team
---------------------------------------------------------------------------------------
Use of this source code is governed by the GPL v3 license that can be found
in the COPYING file or at http://www.gnu.org/licenses/gpl-3.0.html

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
---------------------------------------------------------------------------------------
*/

#ifndef ANH_LOGMANAGER_LOG_H
#define ANH_LOGMANAGER_LOG_H

#include "Utils/typedefs.h"
#include <boost/thread/mutex.hpp>
#include <fstream>
#include <iostream>
#include <string>

class Database;

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

	void logMsg(const std::string& zone, const std::string& system, const std::string& msg, MsgPriority priority, va_list args);
	void logMsg(const std::string& msg, MsgPriority mp, bool fileOut, bool consoleOut, bool timestamp);
	void logMsg(const std::string& msg, MsgPriority mp, bool fileOut, bool consoleOut, bool timestamp, int Color);
	void logMsg(const std::string& msg, MsgPriority mp, va_list args);
	void logMsgNolf(const std::string& msg, MsgPriority mp, bool fileOut, bool consoleOut, bool timestamp, int Color, va_list args);
	void logMsgNolf(const std::string& msg, MsgPriority mp, bool fileOut, bool consoleOut, bool timestamp, int Color);
	uint16 logMsgNolf(const std::string& msg, MsgPriority mp, va_list args);
	void hexDump(int8* data,uint32 len,MsgPriority mp);
	void hexDump(int8* data,uint32 len,const char* filename);

	void setLogLevel(LogLevel level) { mLogLevel = level; }
	void setGlobalLogLevel(GlobalLogLevel level) { mGlobalLogLevel = level; }

private:
	std::string     timestamp_();

	std::ofstream		mLogStream;
	std::string			mName;
	std::string			mZone;
	GlobalLogLevel	mGlobalLogLevel;
	LogLevel				mLogLevel;
	boost::mutex		mGlobalLogMutex;
	Database*				mDatabase;
	bool						mFileOut;
	bool						mDBOut;
	bool						mConsoleOut;


};

//======================================================================================================================

#endif
