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

#ifndef _FILE_DEFINED
struct FILE;
#endif

#define gLogger LogManager::getSingleton()

class LogManager
{
public:
	static LogManager*	getSingleton() { return mSingleton;}
	static void			Init() { mSingleton = new LogManager();}

	enum LOG_PRIORITY
	{
		CRITICAL	=	1,
		EMERGENCY	=	2,
		ALERT		=	3,
		WARNING		=	4,
		NOTICE		=	5,
		INFORMATION	=	6,
		DEBUG		=	7
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
	FILE*						mOutputFile;
};

#endif