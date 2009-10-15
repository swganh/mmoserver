/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "Log.h"

#include "DatabaseManager/Database.h"

#include <iomanip>
#include <sstream>
#include <time.h>



//======================================================================================================================

Log::Log(const std::string& name,LogLevel level,GlobalLogLevel levelGlobal,bool fileOut,bool consoleOut,bool append, Database* database)
		:mName(name),mLogLevel(level),mGlobalLogLevel(levelGlobal),mFileOut(fileOut),mConsoleOut(consoleOut), mDatabase(database)
{
	if(mFileOut)
	{
		std::string file = LOG_DIR + name;

        boost::mutex::scoped_lock lock(mGlobalLogMutex);

		if(append)
		{
			mLogStream.open(file.c_str(),std::ios::out|std::ios::app);
			mLogStream << "--------------------------------------------------------------------------------------------" << std::endl;
			mLogStream.flush();
		}
		else
			mLogStream.open(file.c_str(),std::ios::out|std::ios::trunc);
	}

	
}



//======================================================================================================================
Log::~Log(void)
{
	if(mFileOut)
		mLogStream.close();
}


void Log::logMsg(const std::string& msg,MsgPriority mp,bool fileOut,bool consoleOut,bool timestamp)
{
	logMsg(msg,mp,fileOut,consoleOut,timestamp, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}
//======================================================================================================================
void Log::logMsg(const std::string& msg,MsgPriority mp,bool fileOut,bool consoleOut,bool timestamp, int Color)
{
	if(mLogLevel + mp > mGlobalLogLevel)
	{
		std::string tstring;
  
        boost::mutex::scoped_lock lock(mGlobalLogMutex);

        if(timestamp)
		{
			std::ostringstream tstream;
			struct tm *ts;
			time_t t;
			time(&t);
			ts = localtime(&t);
			
			tstring.append("[");
			tstring.append(timestamp_());
			tstring.append("]");
		}


		if(mFileOut && fileOut)
		{
			mLogStream << tstring << msg << std::endl;
			mLogStream.flush();
		}

		if(mConsoleOut && consoleOut)
		{
			#ifdef _MSC_VER
			HANDLE Console;
			Console = GetStdHandle(STD_OUTPUT_HANDLE);
			SetConsoleTextAttribute(Console, Color);
			#endif

			std::cout << tstring << msg << std::endl;
			std::cout.flush();

			#ifdef _MSC_VER
			SetConsoleTextAttribute(Console, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
			#endif
		}
	}
}

//======================================================================================================================

void Log::logMsg(const std::string& msg, MsgPriority priority, va_list args)
{
  int8    buf[8192];
        
  // format our string with all the optional args.
  vsprintf(buf, msg.c_str(), args);

	if(mLogLevel + priority > mGlobalLogLevel)
	{  
        boost::mutex::scoped_lock lock(mGlobalLogMutex);

		std::string tstring;

		tstring.append("[");
		tstring.append(timestamp_());
		tstring.append("]");
		
		tstring.append(buf);
		if(mFileOut)
		{
			mLogStream << tstring << std::endl;
			mLogStream.flush();
		}

		if(mConsoleOut)
		{
			std::cout << tstring << std::endl;
			std::cout.flush();
		}
	}
}

void Log::logMsg(const std::string& zone, const std::string& system, const std::string& msg, MsgPriority priority, va_list args)
{
  int8    buf[8192];

  std::string text;
  //text += system;
  text += msg;

  // format our string with all the optional args.
  int length = vsprintf(buf, text.c_str(), args);

	//dblogging enabled??
	if(mDatabase)
	{
		//zone
		int8 sql[500];
		int8 escstring1[64];
		int8 escstring2[64];
		int8 escstring3[1024];
		mDatabase->Escape_String(escstring1,zone.c_str(),zone.length());
		mDatabase->Escape_String(escstring2,system.c_str(),system.length());
		mDatabase->Escape_String(escstring3,buf,length);
		sprintf(sql,"call sp_ErrorLogHandler('%s','%s','%s')", escstring1, escstring2, escstring3);
		mDatabase->ExecuteSqlAsync(0,0,sql);
		//mDatabase->ExecuteSqlAsync(0,0,"INSERT INTO %s VALUES (%I64u,173,'%s',1,0)",mName.c_str(),);
	}


	if((mLogLevel + priority > mGlobalLogLevel)&&(mFileOut||mConsoleOut))
	{
        boost::mutex::scoped_lock lock(mGlobalLogMutex);

		std::string tstring;

		tstring.append("[");
		tstring.append(timestamp_());
		tstring.append("]");

		if(mFileOut)
		{
			mLogStream << tstring << buf << std::endl;
			mLogStream.flush();
		}

		if(mConsoleOut)
		{
			std::cout << tstring << buf << std::endl;
			std::cout.flush();
		}
	}
}

void Log::logMsgNolf(const std::string& msg,MsgPriority mp,bool fileOut,bool consoleOut,bool timestamp, int Color, va_list args)
{
	int8    buf[8192];
    
    // format our string with all the optional args.
    vsprintf(buf, msg.c_str(), args);

	if(mLogLevel + mp > mGlobalLogLevel)
	{
        boost::mutex::scoped_lock lock(mGlobalLogMutex);

		std::string tstring;

		if(timestamp)
		{
			tstring.append(timestamp_());
			tstring.append("]");

		}

		tstring.append(buf);


		if(mFileOut && fileOut)
		{
			mLogStream << tstring;// << std::endl;
			mLogStream.flush();
		}

		if(mConsoleOut && consoleOut)
		{
			#ifdef _MSC_VER
			HANDLE Console;
			Console = GetStdHandle(STD_OUTPUT_HANDLE);
			SetConsoleTextAttribute(Console, Color);
			#endif

			std::cout << tstring;// << std::endl;
			std::cout.flush();

			#ifdef _MSC_VER
			SetConsoleTextAttribute(Console, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
			#endif
		}
	}
}

//======================================================================================================================
//

void Log::logMsgNolf(const std::string& msg,MsgPriority mp,bool fileOut,bool consoleOut,bool timestamp, int Color)
{

	if(mLogLevel + mp > mGlobalLogLevel)
	{
        boost::mutex::scoped_lock lock(mGlobalLogMutex);

		std::string tstring;

		if(timestamp)
		{
			tstring.append("[");
			tstring.append(timestamp_());
			tstring.append("]");
		}

		if(mFileOut && fileOut)
		{
			mLogStream << tstring << msg;// << std::endl;
			mLogStream.flush();
		}

		if(mConsoleOut && consoleOut)
		{
			#ifdef _MSC_VER
			HANDLE Console;
			Console = GetStdHandle(STD_OUTPUT_HANDLE);
			SetConsoleTextAttribute(Console, Color);
			#endif

			std::cout << tstring << msg;// << std::endl;
			std::cout.flush();

			#ifdef _MSC_VER
			SetConsoleTextAttribute(Console, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
			#endif
		}
	}
}

//======================================================================================================================

uint16 Log::logMsgNolf(const std::string& msg, MsgPriority priority, va_list args)
{
    int8    buf[8192];

    // format our string with all the optional args.
    vsprintf(buf, msg.c_str(), args);

	if(mLogLevel + priority > mGlobalLogLevel)
	{
		std::string tstring;

        boost::mutex::scoped_lock lock(mGlobalLogMutex);

		tstring.append("[");
		tstring.append(timestamp_());
		tstring.append("]");
		tstring.append(buf);


		if(mFileOut)
		{
			mLogStream << tstring;// << std::endl;
			mLogStream.flush();
		}

		if(mConsoleOut)
		{
			std::cout << tstring;// << std::endl;
			std::cout.flush();
		}
		return tstring.length();
	}
	return 0;
}


//======================================================================================================================

void Log::hexDump(int8* data,uint32 len,MsgPriority mp)
{
	if(mLogLevel + mp > mGlobalLogLevel)
	{
        boost::mutex::scoped_lock lock(mGlobalLogMutex);

		for(uint32 i = 1;i <= len;i++)
		{
			printf("%.2X ",data[i-1] & 0xFF);

			if(i % 16 == 0)
				printf("\n");
		}

		printf("\n");
	}
}

//======================================================================================================================

void Log::hexDump(int8* data,uint32 len,const char* filename)
{
    boost::mutex::scoped_lock lock(mGlobalLogMutex);

	FILE* out = fopen(filename, "a");

	for(uint32 i = 1;i <= len;i++)
	{
		fprintf(out,"%.2X ",data[i-1] & 0xFF);
		if(i % 16 == 0)
		{
			fprintf(out,"\n");
		}
	}
	fprintf(out,"\n\n-----------------------------------------------------------\n\n");
	fclose(out);
}

//======================================================================================================================

std::string Log::timestamp_()
{
    time_t raw_time;  
    struct tm * time_info;  
  
    time(&raw_time);  
    time_info = localtime(&raw_time);
    
    std::ostringstream os;
    os << std::setw(2) << std::setfill('0') << time_info->tm_hour << ":" << std::setw(2) << std::setfill('0') << time_info->tm_min << ":" << std::setw(2) << std::setfill('0') << time_info->tm_sec;
    
    return os.str();
}

//======================================================================================================================