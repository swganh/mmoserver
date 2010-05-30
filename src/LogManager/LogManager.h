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

#ifndef ANH_LOGMANAGER_H
#define ANH_LOGMANAGER_H

#include "Log.h"
#include "Utils/typedefs.h"

#include <boost/thread/mutex.hpp>
#include <map>

class Database;
class DatabaseManager;


//======================================================================================================================
#define gLogger	LogManager::getSingletonPtr()

typedef std::map<std::string,Log*,std::less<std::string> > LogList;


//======================================================================================================================
class LogManager
{
	public:
		static LogManager*  getSingletonPtr() { return mSingleton; }
		static LogManager*	Init(GlobalLogLevel glevel = G_LEVEL_NORMAL,const std::string& name = "default.log",LogLevel level = LEVEL_NORMAL,bool fileOut = true,bool consoleOut = false,bool append = false);
		static void         destroySingleton(void)  { delete mSingleton; mSingleton = 0; };

		~LogManager();

		Log*		  createErrorLog(const std::string& zone, LogLevel level = LEVEL_NORMAL,bool fileOut = true,bool consoleOut = false,bool append = false);
		Log*		  createLog(const std::string& name,LogLevel level = LEVEL_NORMAL,bool fileOut = true,bool consoleOut = false,bool append = false);
		void		  connecttoDB(DatabaseManager* dbManager);
		Log*		  getLog(const std::string& name);
		Log*		  setDefaultLog(Log* log);
		Log*		  getDefaultLog(){ return mDefaultLog; }

		void		  logMsg(const std::string& msg, int Color);
		void		  logMsg(const std::string& logname,const std::string& msg,MsgPriority mp = MSG_NORMAL,bool fileOut = true,bool consoleOut = true,bool timestamp = true);
		void		  logMsg(Log* log,const std::string& msg,MsgPriority mp = MSG_NORMAL,bool fileOut = true,bool consoleOut = true,bool timestamp = true);
		void		  logMsg(const std::string& msg,MsgPriority mp = MSG_NORMAL,bool fileOut = true,bool consoleOut = true,bool timestamp = true, int Color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		void		  logMsgStartUp(const std::string& msg, MsgPriority priority  = MSG_NORMAL, ...);
		void		  logMsgFollowUp(const std::string& msg, MsgPriority priority, ...);

		//void		  logMsgStartUp(const std::string& msg,MsgPriority mp = MSG_NORMAL,bool fileOut,bool consoleOut,bool timestamp, int Color);
		void		  logMsgF(const std::string& msg, MsgPriority mp, ...);
		void		  logMsgF(Log* log, const std::string& msg, MsgPriority mp, ...);
		void		  logMsgFailed(int width);
		void		  logMsgOk(int width);
		void		  logMsgLoadSuccess(const std::string& msg, MsgPriority priority, ...);
		void		  logMsgLoadFailure(const std::string& msg, MsgPriority priority, ...);

		void		  hexDump(int8* data,uint32 len,MsgPriority mp = MSG_NORMAL);
		void		  hexDump(int8* data,uint32 len,const char* filename);

		void		  setLogLevel(const std::string& logname,LogLevel level);
		void		  setLogLevel(Log* log,LogLevel level);
		void		  setLogLevel(LogLevel level);

		void		  setGlobalLogLevel(GlobalLogLevel glevel);
		void		  printLogo();
		void		  printSmallLogo();

		//Errorlogging
		//void		  logError(const std::string& msg, int Color);
		void		  logErrorF(const std::string& system, const std::string& msg, MsgPriority priority, ...);



	private:

		LogManager(GlobalLogLevel glevel,const std::string& name,LogLevel level,bool fileOut,bool consoleOut,bool append);

		static LogManager*	mSingleton;

		Database*			mDatabase;
		LogList				mLogs;
		Log*				mDefaultLog;
		Log*				mDefaultErrorLog;
		Log*				mDefaultAdminLog;
		Log*				mDefaultTransactionLog;
		GlobalLogLevel		mGlobalLogLevel;
        boost::mutex        mGlobalLogMutex;
		std::string			mZone;
};

#endif
