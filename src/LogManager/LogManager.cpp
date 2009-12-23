/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/
#include "LogManager.h"

#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseManager.h"

#include "ConfigManager/ConfigManager.h"
#include "Utils/StreamColors.h"

#include <iomanip>
#include <string>
#include <stdarg.h>
#include <stdio.h> 


//======================================================================================================================
LogManager*   LogManager::mSingleton  = NULL;


//======================================================================================================================
LogManager::LogManager(GlobalLogLevel glevel,const std::string& name,LogLevel level,bool fileOut,bool consoleOut,bool append)
{
	mDatabase				= NULL;
	mDefaultLog				= NULL;
	mDefaultErrorLog		= NULL;
	mDefaultAdminLog		= NULL;
	mDefaultTransactionLog	= NULL;
	mGlobalLogLevel			= glevel;
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
    boost::mutex::scoped_lock lk(mGlobalLogMutex);

	std::string zone = "";

	Log* newLog = new Log(name, level, mGlobalLogLevel, fileOut,  consoleOut,append,NULL);

	mLogs.insert(LogList::value_type(name,newLog));

	if(!mDefaultLog)
	{
		mDefaultLog = newLog;
	}

	return newLog;
}

//======================================================================================================================
// we connect to our separate log db
//

void LogManager::connecttoDB(DatabaseManager* dbManager)
{
	std::string server	= (char*)(gConfig->read<std::string>("DBServer")).c_str();
	int port			= (int)(gConfig->read<int>("DBPort"));
	std::string user	= (char*)(gConfig->read<std::string>("DBUser")).c_str();
	std::string pass	= (char*)(gConfig->read<std::string>("DBPass")).c_str();
	std::string name	= (char*)(gConfig->read<std::string>("DBName")).c_str();

	mDatabase = dbManager->Connect(DBTYPE_MYSQL, 
									   (char*)(gConfig->read<std::string>("DBLogServer",server)).c_str(),
									   gConfig->read<int>("DBLogPort",port),
									   (char*)(gConfig->read<std::string>("DBLogUser",user)).c_str(),
									   (char*)(gConfig->read<std::string>("DBLogPass",pass)).c_str(),
									   (char*)(gConfig->read<std::string>("DBLogName",name)).c_str());
	
}

//======================================================================================================================
// we create a separate log for errors
//

Log* LogManager::createErrorLog(const std::string& zone, LogLevel level ,bool fileOut,bool consoleOut,bool append)
{
	mZone = zone.c_str();
	std::string logname = "error_log ";
	logname += zone;

    boost::mutex::scoped_lock lk(mGlobalLogMutex);

	Log* newLog = new Log(logname, level, mGlobalLogLevel, fileOut,  consoleOut,append,mDatabase);

	mLogs.insert(LogList::value_type(logname,newLog));

	if(!mDefaultErrorLog)
	{
		mDefaultErrorLog = newLog;
	}

	return newLog;
}


//======================================================================================================================

Log* LogManager::getLog(const std::string& name)
{
	Log* log = 0;

    boost::mutex::scoped_lock lk(mGlobalLogMutex);

	LogList::iterator it = mLogs.find(name);

	if (it != mLogs.end())
	{
		log = it->second;
	}

	return log;
}

//======================================================================================================================

void LogManager::logMsg(const std::string& logname,const std::string& msg,MsgPriority mp,bool fileOut,bool consoleOut,bool timestamp)
{
    boost::mutex::scoped_lock lk(mGlobalLogMutex);

	Log* tmpLog = getLog(logname);

	if(tmpLog != NULL)
	{
		tmpLog->logMsg(msg, mp, fileOut, consoleOut, timestamp);
	}
}

//======================================================================================================================

void LogManager::logMsg(Log* log,const std::string& msg,MsgPriority mp,bool fileOut,bool consoleOut,bool timestamp)
{
    boost::mutex::scoped_lock lk(mGlobalLogMutex);

	log->logMsg(msg, mp, fileOut, consoleOut, timestamp);
}

//======================================================================================================================
void LogManager::logMsg(const std::string& msg, int Color)
{
	logMsg(msg, MSG_NORMAL, true, true, true, Color);
}
//======================================================================================================================

void LogManager::logMsg(const std::string& msg,MsgPriority mp,bool fileOut,bool consoleOut,bool timestamp, int Color)
{
    boost::mutex::scoped_lock lk(mGlobalLogMutex);

	if(mDefaultLog != NULL)
	{
		mDefaultLog->logMsg(msg, mp, fileOut, consoleOut, timestamp, Color);
	}
}

//======================================================================================================================

void LogManager::logMsgStartUp(const std::string& msg, MsgPriority priority, ...)
{
	va_list args;
	va_start(args, priority);
	
	boost::mutex::scoped_lock lk(mGlobalLogMutex);

	if(mDefaultLog != NULL)
	{
		mDefaultLog->logMsgNolf(msg, priority, true, true, true, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE, args);
	}
}

void LogManager::logMsgFollowUp(const std::string& msg, MsgPriority priority, ...)
{
	va_list args;
	va_start(args, priority);

	boost::mutex::scoped_lock lk(mGlobalLogMutex);

	if(mDefaultLog != NULL)
	{

		mDefaultLog->logMsgNolf(msg, priority, true, true, false,FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE, args);
	}
}

//======================================================================================================================

void LogManager::logMsgF(const std::string& msg, MsgPriority priority, ...)
{
	va_list args;
	va_start(args, priority);
	
	boost::mutex::scoped_lock lk(mGlobalLogMutex);

	if(mDefaultLog != NULL)
	{
		mDefaultLog->logMsg(msg, priority, args);
	}
}

//======================================================================================================================

void LogManager::hexDump(int8* data,uint32 len,MsgPriority mp)
{
    boost::mutex::scoped_lock lk(mGlobalLogMutex);

	if(mDefaultLog != NULL)
	{
		mDefaultLog->hexDump(data,len,mp);
	}
}

void LogManager::hexDump(int8* data,uint32 len,const char* filename)
{
	if(mDefaultLog != NULL)
	{
		mDefaultLog->hexDump(data,len,filename);
	}
}

//======================================================================================================================

void LogManager::logMsgF(Log* log, const std::string& msg, MsgPriority priority, ...)
{
	va_list args;
	va_start(args, priority);

	log->logMsg(msg, priority, args);
}

//======================================================================================================================

void LogManager::logMsgOk(int width)
{
	boost::mutex::scoped_lock lk(mGlobalLogMutex);

	std::cout << std::setw(width-6); // Width minus the [ OK ] message length.
	std::cout << std::right << "[ " << green << "OK" << white << " ]" << std::endl;
}


void LogManager::logMsgLoadSuccess(const std::string& msg, MsgPriority priority, ...)
{
	va_list args;
	va_start(args, priority);

	uint16 textLength = 0;
	if(mDefaultLog != NULL)
	{
		textLength = mDefaultLog->logMsgNolf(msg, priority, args);
	}

	int16 length = 80 - textLength + 1; // Add an extra 1 for the null terminator at the end of the string.
	
	if(length < 0)
		length = 80;

	logMsgOk(length);
}


void LogManager::logMsgLoadFailure(const std::string& msg, MsgPriority priority, ...)
{
	va_list args;
	va_start(args, priority);

	uint16 textLength = 0;
	if(mDefaultLog != NULL)
	{
		textLength = mDefaultLog->logMsgNolf(msg, priority, args);
	}

	int16 length = 80 - textLength + 1; // Add an extra 1 for the null terminator at the end of the string.
	
	if(length < 0)
		length = 80;

	logMsgFailed(length);
}

//======================================================================================================================

void LogManager::logMsgFailed(int width)
{
    boost::mutex::scoped_lock lk(mGlobalLogMutex);	

		std::cout << std::setw(width-10); // Width minus the [ OK ] message length.
		std::cout << std::right << "[ " << red << "FAILED" << white << " ]" << std::endl;
}

//======================================================================================================================

void LogManager::setLogLevel(const std::string& logname,LogLevel level)
{
	Log* tmpLog = getLog(logname);

	if(tmpLog != NULL)
	{
		tmpLog->setLogLevel(level);
	}
}

//======================================================================================================================

void LogManager::setLogLevel(Log* log,LogLevel level)
{
	log->setLogLevel(level);
}

//======================================================================================================================

void LogManager::setLogLevel(LogLevel level)
{
	if(mDefaultLog)
	{
		mDefaultLog->setLogLevel(level);
	}
}

//======================================================================================================================

Log* LogManager::setDefaultLog(Log* log)
{
	boost::mutex::scoped_lock lk(mGlobalLogMutex);

	Log* old = mDefaultLog;
	mDefaultLog = log;

	return old;
}

//======================================================================================================================

void LogManager::setGlobalLogLevel(GlobalLogLevel glevel)
{
	boost::mutex::scoped_lock lk(mGlobalLogMutex);

	LogList::iterator it = mLogs.begin();

	while(it != mLogs.end())
	{
		it->second->setGlobalLogLevel(glevel);
		++it;
	}
}

//======================================================================================================================
using namespace std;
void LogManager::printLogo()
{
 //int spacing = 20; //std::cout.width(spacing) and std::setw(spacing)  don't line up correctly >.<
char spacing[38] = "                            \0";
cout<<spacing<<white<<"                             "<<endl;
cout<<spacing<<aqua<<"fffffffff"<<white<<"             "<<aqua<<"Lfffff"<<white<<" "<<endl;
cout<<spacing<<aqua<<"ffffffff"<<white<<"     "<<":"<<"           "<<aqua<<"Lff"<<white<<" "<<endl;
cout<<spacing<<aqua<<"ffffff"<<white<<"       "<<"##"<<"            "<<aqua<<"f"<<white<<" "<<endl;
cout<<spacing<<aqua<<"fffffj"<<white<<"                       "<<endl;
cout<<spacing<<aqua<<"fffff"<<white<<"                        "<<endl;
cout<<spacing<<aqua<<"ffffL"<<white<<"                        "<<endl;
cout<<spacing<<aqua<<"fffff"<<white<<"                        "<<endl;
cout<<spacing<<aqua<<"ffff"<<white<<"                         "<<endl;
cout<<spacing<<aqua<<"ffff"<<white<<"        "<<":j###;:"<<"          "<<white<<endl;
cout<<spacing<<aqua<<"ffff"<<white<<"     "<<"#############"<<"       "<<white<<endl;
cout<<spacing<<aqua<<"ffff"<<white<<"        "<<"####"<<" "<<"D"<<"    D##"<<"    "<<white<<endl;
cout<<spacing<<aqua<<"fffff"<<white<<"       "<<"#####"<<"       "<<"##"<<"   "<<white<<endl;
cout<<spacing<<aqua<<"fffff"<<white<<"      "<<"#######"<<"       "<<"##"<<"  "<<white<<endl;
cout<<spacing<<aqua<<"ffff"<<white<<"      "<<"#"<<" "<<"########"<<"    "<<"####"<<" "<<white<<endl;
cout<<spacing<<aqua<<"fff"<<white<<"      "<<"#"<<"  "<<"#########"<<"  "<<"#####"<<" "<<white<<endl;
cout<<spacing<<aqua<<"fL"<<white<<"      "<<"#"<<"  "<<":"<<"  "<<"##########"<<" "<<"###"<<" "<<white<<endl;
cout<<spacing<<aqua<<"f"<<white<<"               "<<"########"<<" "<<"###"<<" "<<white<<endl;
cout<<spacing<<aqua<<"ff"<<white<<"          "<<"###"<<"  "<<"#######"<<" "<<"###"<<" "<<white<<endl;
cout<<spacing<<aqua<<"ff"<<white<<"          "<<"####"<<" "<<"#######"<<" "<<"###"<<" "<<white<<endl;
cout<<spacing<<aqua<<"ff"<<white<<"         "<<"t################"<<" "<<white<<endl;
cout<<spacing<<aqua<<"ff"<<white<<"         "<<"#################"<<" "<<white<<endl;
cout<<spacing<<aqua<<"ff"<<white<<"         "<<"#################"<<" "<<white<<endl;
cout<<spacing<<aqua<<"ff"<<white<<"        "<<"##################"<<" "<<white<<endl;
cout<<spacing<<aqua<<"f"<<white<<"       "<<":##"<<"   "<<"##############"<<" "<<white<<endl;
cout<<spacing<<blue<<";,;"<<white<<""<<"     "<<"###"<<"    "<<"#############"<<" "<<white<<endl;
cout<<spacing<<blue<<";;;;"<<white<<"   "<<":##"<<"     "<<"#############"<<" "<<white<<endl;
cout<<spacing<<blue<<"; ,"<<white<<"     "<<"##"<<"     "<<"##"<<"  "<<"#########"<<" "<<white<<endl;
cout<<spacing<<blue<<";;"<<white<<"       "<<"#"<<"      "<<"#:"<<"  "<<"#######"<<"  "<<white<<endl;
cout<<spacing<<blue<<";,"<<white<<"       "<<"#"<<"     "<<"###"<<"  "<<"######"<<"   "<<white<<endl;
cout<<spacing<<blue<<";;"<<white<<"              "<<"#########"<<"    "<<white<<endl;
cout<<spacing<<blue<<";;;,"<<white<<"             #######"<<"     "<<white<<endl;
cout<<spacing<<blue<<";;;"<<white<<"                ###       "<<white<<endl;
cout<<spacing<<blue<<";;;,;;"<<white<<"                       "<<white<<endl;
cout<<spacing<<blue<<";;;;;;"<<white<<"                       "<<white<<endl;
cout<<spacing<<blue<<";;;;;;"<<white<<"                       "<<white<<endl;
cout<<"     _______.____    __    ____  _______      ___      .__   __.  __    __  "<<endl;
cout<<"    /       |\\   \\  /  \\  /   / /  _____|    /   \\     |  \\ |  | |  |  |  | "<<endl;
cout<<"   |   (----` \\   \\/    \\/   / |  |  __     /  ^  \\    |   \\|  | |  |__|  | "<<endl;
cout<<"    \\   \\      \\            /  |  | |_ |   /  /_\\  \\   |  . `  | |   __   | "<<endl;
cout<<".----)   |      \\    /\\    /   |  |__| |  /  _____  \\  |  |\\   | |  |  |  | "<<endl;
cout<<"|_______/        \\__/  \\__/     \\______| /__/     \\__\\ |__| \\__| |__|  |__| "<<endl;
cout<<setw(49)<<"THERE IS ANOTHER"<<endl;
  
return;
}

//======================================================================================================================
void LogManager::printSmallLogo()
{
char spacing[5] = "  \0";
cout<<spacing<<aqua<<"LLLLt"<<white<<"                         "<<endl; 
cout<<spacing<<aqua<<"LLLLi"<<white<<"      t#########f,       "<<endl;
cout<<spacing<<aqua<<"LLLLj"<<white<<"   t##L,####f,ifE###     "<<endl;
cout<<spacing<<aqua<<"LLLLL"<<white<<"   ,   i####Wi     f##,  "<<endl; 
cout<<spacing<<aqua<<"LLLLL"<<white<<"       K#####W       ##i "<<endl;
cout<<spacing<<aqua<<"LLLLL"<<white<<"      tD######W L    D##,"<<endl; 
cout<<spacing<<aqua<<"LLLLi"<<white<<"     ,K ########     ####"<<endl; 
cout<<spacing<<aqua<<"LLLf"<<white<<"     ,#  #########   #####"<<endl; 
cout<<spacing<<aqua<<"LL"<<white<<"      ,#  t, ###########,###"<<endl; 
cout<<spacing<<aqua<<"L"<<white<<"       #   i,  #########t,###"<<endl; 
cout<<spacing<<aqua<<"LG"<<white<<"           ##, ########, ###"<<endl; 
cout<<spacing<<aqua<<"LL"<<white<<"           ###L ######## ###"<<endl; 
cout<<spacing<<aqua<<"LL"<<white<<"          K##### #######G###"<<endl;
cout<<spacing<<aqua<<"LL"<<white<<"          #######,##########"<<endl;
cout<<spacing<<aqua<<"LL"<<white<<"          ########K#########"<<endl; 
//cout<<aqua<<" _____      _____   _   _  _ _  _"<<endl;
//cout<<"/ __\\ \\    / / __| /_\\ | \\| | || |"<<endl;
//cout<<"\\__ \\\\ \\/\\/ / (_ |/ _ \\| .` | __ |"<<endl;
//cout<<"|___/ \\_/\\_/ \\___/_/ \\_\\_|\\_|_||_|"<<white<<endl;

}

//======================================================================================================================

void LogManager::logErrorF(const std::string& system, const std::string& msg, MsgPriority priority, ...)
{
	if(!mDefaultErrorLog)
		return;
	
	boost::mutex::scoped_lock lk(mGlobalLogMutex);	

	va_list args;
	va_start(args, priority);
	
	mDefaultErrorLog->logMsg(mZone, system, msg, priority, args);

//	mGlobalLogMutex.release();


}