/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_DATABASEMANAGER_DATABASEMANAGER_H
#define ANH_DATABASEMANAGER_DATABASEMANAGER_H

#include "DatabaseType.h"
#include <list>
#include "Utils/typedefs.h"


//======================================================================================================================
class Database;
enum  DBType;

typedef std::list<Database*>           DatabaseList;


//======================================================================================================================
class DatabaseManager
{
public:
                                  DatabaseManager(void);
                                  ~DatabaseManager(void);

  void                            Startup(void);
  void                            Shutdown(void);
  void                            Process(void);

  Database*                       Connect(DBType type, int8* host, uint16 port, int8* user, int8* pass, int8* dbname);

private:
  DatabaseList                    mDatabaseList;
};



#endif //OOMSERVER_DATABASEMANAGER_DATABASEMANAGER_H


