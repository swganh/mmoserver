/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_DATABASEMANAGER_TRANSACTION_H
#define ANH_DATABASEMANAGER_TRANSACTION_H


#include "Utils/typedefs.h"
#include <sstream>

class DatabaseImplementation;
class DatabaseCallback;
class DatabaseResult;
class Database;

//======================================================================================================================

class Transaction
{
	public:

		Transaction(Database* database,DatabaseCallback* callback,void* ref);
		~Transaction();

		void		execute();
		void		addQuery(int8* query,...);

	private:

		Database*				mDatabase;
		DatabaseCallback*		mCallback;
		void*					mReference;
		std::ostringstream		mQueries;
};

//======================================================================================================================

#endif

