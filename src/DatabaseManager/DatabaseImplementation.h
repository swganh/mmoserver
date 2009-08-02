/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_DATABASEMANAGER_DATABASEIMPLEMENTATION_H
#define ANH_DATABASEMANAGER_DATABASEIMPLEMENTATION_H

#include "DatabaseResult.h"
#include "Utils/typedefs.h"
#include <boost/pool/singleton_pool.hpp>


//======================================================================================================================

class DataBinding;
class DatabaseWorkerThread;

typedef boost::singleton_pool<DatabaseResult,sizeof(DatabaseResult),boost::default_user_allocator_malloc_free> ResultPool;

//======================================================================================================================
class DatabaseImplementation
{
public:
									DatabaseImplementation(void) {};
  virtual							~DatabaseImplementation(void) {};
  
  virtual void						Startup(char* host, unsigned short port, char* user, char* pass, char* schema) = 0;
  virtual void						Shutdown(void) = 0;
  
  virtual DatabaseResult*			ExecuteSql(int8* sql,bool procedure = false) = 0;

  virtual DatabaseWorkerThread*	DestroyResult(DatabaseResult* result) = 0;
  
  virtual void						GetNextRow(DatabaseResult* result, DataBinding* binding, void* object) = 0;
  virtual void						ResetRowIndex(DatabaseResult* result, uint64 index = 0) = 0;

  virtual uint32					Escape_String(int8* target,const int8* source,uint32 length) = 0;

  bool								releaseResultPoolMemory(){ return(ResultPool::release_memory()); }

  virtual uint64					GetInsertId(void) = 0;

	protected:
};



#endif //^ANH_DATABASEMANAGER_DATABASEIMPLEMENTATION_H





