
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

#include <cstdint>
#include <iosfwd>
#include <queue>
#include <string>

#include "Common/declspec.h"

#define LOG_CHANNEL_CONSOLE	 1
#define LOG_CHANNEL_FILE	 2
#define LOG_CHANNEL_SYSLOG	 4

#define LOG_CHANNEL_NONE	 0
#define LOG_CHANNEL_ALL		 7

class LOG_ENTRY;
class Database;

namespace boost {
class mutex;
class thread;
}

#define gLogger LogManager::getSingleton()

class COMMON_API LogManager
{
public:

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

    static LogManager*	getSingleton() {
        return mSingleton;
    }
    static void			Init(LOG_PRIORITY console_priority, LOG_PRIORITY file_priority, std::string filename) {
        mSingleton = new LogManager(console_priority, file_priority, filename);
    }

    void log(LOG_PRIORITY priority, std::string format, ...);
    void logCont(LOG_PRIORITY priority, std::string format, ...);

    void logS(LOG_PRIORITY priority, uint8_t channels, std::string format, ...);
    void logContS(LOG_PRIORITY priority, uint8_t channels, std::string format, ...);

    static LogManager* mSingleton;

private:
    LogManager(LOG_PRIORITY console_priority, LOG_PRIORITY file_priority, std::string filename);
    LogManager(const LogManager&);
    LogManager& operator=(const LogManager&);
    ~LogManager();

    void _printLogo();
    void _LoggerThread();

    // Win32 complains about stl during linkage, disable the warning.
#ifdef _WIN32
#pragma warning (disable : 4251)
#endif
    std::queue<LOG_ENTRY*>		mEntries;


    uint8_t						mMinPriorities[3];
    std::string					mFileName;

    std::unique_ptr<std::ofstream> mOutputFile;
    std::unique_ptr<boost::thread> mThread;
    std::unique_ptr<boost::mutex> mEntriesMutex;
    // Re-enable the warning.
#ifdef _WIN32
#pragma warning (default : 4251)
#endif
};

#endif