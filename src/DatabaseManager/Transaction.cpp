/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "Transaction.h"

#include "Database.h"
#include "DatabaseCallback.h"
#include "DatabaseImplementation.h"
#include "DatabaseImplementationMySql.h"

#include "LogManager/LogManager.h"

#include <cstdarg>

//======================================================================================================================

Transaction::Transaction(Database* database,DatabaseCallback* callback,void* ref) :
mDatabase(database),mCallback(callback),mReference(ref)
{
	mQueries.flush();
	mQueries << "CALL sp_MultiTransaction(\"";
}

//======================================================================================================================

Transaction::~Transaction()
{
	mQueries.flush();
}

//======================================================================================================================

void Transaction::addQuery(int8* query,...)
{
	va_list	args;
	va_start(args,query);
	int8	localSql[2048],escapedSql[2500];
	int32	len = vsnprintf(localSql,sizeof(localSql),query,args);

	gLogger->logMsgF("transaction add query",MSG_NORMAL);
	// need to escape
	mDatabase->Escape_String(escapedSql,localSql,len);

	mQueries << escapedSql << "$$";

}

//======================================================================================================================

void Transaction::execute()
{
	mQueries << "\")";

	mDatabase->ExecuteProcedureAsync(mCallback,mReference,(int8*)(mQueries.str().c_str()));
	mDatabase->destroyTransaction(this);
}

//======================================================================================================================


