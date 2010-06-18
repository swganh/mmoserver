
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

#include "Utils\typedefs.h"

#include <boost/thread/thread.hpp>
#include <string>
#include <queue>
#include <memory>

#define LOG_CHANNEL_CONSOLE	 1
#define LOG_CHANNEL_FILE	 2
#define LOG_CHANNEL_SYSLOG	 4

#define LOG_CHANNEL_NONE	 0
#define LOG_CHANNEL_ALL		 7

class LOG_ENTRY;
class Database;

#define gLogger LogManager::getSingleton()

class LogManager
{
public:
	static LogManager*	getSingleton() { return mSingleton;}
	static void			Init() { mSingleton = new LogManager();}

	enum LOG_PRIORITY 
	{
		EMERGENCY		= 1,
		ALERT			= 2,
		CRITICAL		= 3,
		ERR				= 4,
		WARNING			= 5,
		NOTICE			= 6,
		INFORMATION		= 7,
		DEBUG			= 8
	};

	bool setupConsoleLogging(LOG_PRIORITY min_priority);
	bool setupFileLogging(LOG_PRIORITY min_priority, std::string filename);
	
	void log(LOG_PRIORITY priority, std::string format, ...);
	void logCont(LOG_PRIORITY priority, std::string format, ...);

	void logS(LOG_PRIORITY priority, uint8 channels, std::string format, ...);
	void logContS(LOG_PRIORITY priority, uint8 channels, std::string format, ...);

	static LogManager* mSingleton;

private:
	LogManager();
	LogManager(const LogManager&);
	LogManager& operator=(const LogManager&);

	void _printLogo();
	void _LoggerThread();

	boost::thread				mThread;
	boost::mutex				mEntriesMutex;
	std::queue<LOG_ENTRY*>		mEntries;


	uint8						mMinPriorities[3];
	std::string					mFileName;
};

#endif