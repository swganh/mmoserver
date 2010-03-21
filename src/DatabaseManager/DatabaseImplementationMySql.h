/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_DATABASEMANAGER_DATABASEIMPLEMENTATIONMYSQL_H
#define ANH_DATABASEMANAGER_DATABASEIMPLEMENTATIONMYSQL_H

#include "DatabaseImplementation.h"
#include "Utils/typedefs.h"

//======================================================================================================================
class DatabaseResult;

typedef struct st_mysql MYSQL;
typedef struct st_mysql_res MYSQL_RES;
typedef struct st_mysql_rows MYSQL_ROWS;


//======================================================================================================================

class DatabaseImplementationMySql : public DatabaseImplementation
{
public:
									 DatabaseImplementationMySql(char* host, uint16 port, char* user, char* pass, char* schema);
  virtual							~DatabaseImplementationMySql(void);
  
  virtual DatabaseResult*			ExecuteSql(int8* sql,bool procedure = false);
  virtual DatabaseWorkerThread*		DestroyResult(DatabaseResult* result);

  virtual void						GetNextRow(DatabaseResult* result, DataBinding* binding, void* object);
  virtual void						ResetRowIndex(DatabaseResult* result, uint64 index = 0);
  virtual uint64					GetInsertId(void);

  virtual uint32					Escape_String(int8* target,const int8* source,uint32 length);

private:
  MYSQL*                      mConnection;
  MYSQL_RES*                  mResultSet;
};




#endif // ANH_DATABASEMANAGER_DATABASEIMPLEMENTATIONMYSQL_H





