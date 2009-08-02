/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_DATABASEMANAGER_DATABASERESULT_H
#define ANH_DATABASEMANAGER_DATABASERESULT_H

#include "Utils/typedefs.h"


//======================================================================================================================
class DatabaseImplementation;
class DataBinding;
class DatabaseWorkerThread;


//======================================================================================================================
class DatabaseResult
{
public:
                              DatabaseResult(bool multiResult = false) 
								  :mWorkerReference(0), mConnectionReference(0),mResultSetReference(0),mRowCount(0),mDatabaseImplementation(0),mMultiResult(multiResult) {};
                              ~DatabaseResult(void) {};

  virtual void               GetNextRow(DataBinding* dataBinding, void* object);
  void                        ResetRowIndex(int index = 0);

  void*						  getConnectionReference(void){ return mConnectionReference; }
  void						  setConnectionReference(void* ref)	{ mConnectionReference =  ref; }

  void						  setWorkerReference(DatabaseWorkerThread* worker){ mWorkerReference = worker; }
  DatabaseWorkerThread*		  getWorkerReference(){ return mWorkerReference; }

  bool						  isMultiResult(){ return mMultiResult; }
  void						  setMultiResult(bool b){ mMultiResult = b; }

  DatabaseImplementation*     getDatabaseImplementation(void)                 { return mDatabaseImplementation; }
  void*                       getResultSetReference(void)                     { return mResultSetReference; }
  uint64                      getRowCount(void)                               { return mRowCount; }

  void                        setDatabaseImplementation(DatabaseImplementation* impl)   { mDatabaseImplementation = impl; }
  void                        setResultSetReference(void* ref)                { mResultSetReference = ref; }
  void                        setRowCount(uint64 count)                       { mRowCount = count; }

private:
  DatabaseWorkerThread*			mWorkerReference;
  void*							mConnectionReference;
  void*							mResultSetReference;
  uint64						mRowCount;
  DatabaseImplementation*		mDatabaseImplementation;
  bool							mMultiResult;
};


#endif //MMOSERVER_DATABASEMANAGER_DATABASERESULT_H



