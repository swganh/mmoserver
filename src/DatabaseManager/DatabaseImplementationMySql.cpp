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

#include "DatabaseImplementationMySql.h"
#include "DatabaseResult.h"
#include "DataBinding.h"

#include "LogManager/LogManager.h"

#include <boost/lexical_cast.hpp>
#include <mysql.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>

//======================================================================================================================
DatabaseImplementationMySql::DatabaseImplementationMySql(char* host, uint16 port, char* user, char* pass, char* schema) :
	DatabaseImplementation(host, port, user, pass, schema)
{
  MYSQL*        connect = 0;

  // Initialize mysql and make a connection to the server.
  mConnection = mysql_init(0);
  connect = mysql_real_connect(mConnection, (const char*)host, (const char*)user, (const char*)pass, (const char*)schema, port, 0, CLIENT_MULTI_STATEMENTS);
  mysql_options(mConnection, MYSQL_OPT_RECONNECT, "true");

  // Any errors from the connection attempt?
  if(mysql_errno(mConnection) != 0)
  {
    gLogger->log(LogManager::EMERGENCY, "DatabaseError: %s", mysql_error(mConnection));
  }

 // int i = 0;
}

//======================================================================================================================
DatabaseImplementationMySql::~DatabaseImplementationMySql(void)
{
  // Close the connection and destroy our connection object.
  mysql_close(mConnection);
  mysql_thread_end();
}

//======================================================================================================================
DatabaseResult* DatabaseImplementationMySql::ExecuteSql(int8* sql,bool procedure)
{
	DatabaseResult* newResult = new(ResultPool::ordered_malloc()) DatabaseResult(procedure);

  newResult->setDatabaseImplementation(this);

  // Execute the statement
  uint32 len = (uint32)strlen(sql);
  mysql_real_query(mConnection, sql, len);

  if(mysql_errno(mConnection) != 0)
  {
    gLogger->log(LogManager::EMERGENCY, "DatabaseError: %s", mysql_error(mConnection));


  }

  mResultSet = mysql_store_result(mConnection);

  newResult->setConnectionReference((void*)mConnection);
  newResult->setResultSetReference((void*)mResultSet);

  if (mResultSet)
  {
    newResult->setRowCount(mResultSet->row_count);
  }

  return newResult;
}


//======================================================================================================================

DatabaseWorkerThread* DatabaseImplementationMySql::DestroyResult(DatabaseResult* result)
{
	DatabaseWorkerThread* worker = NULL;

	if((MYSQL_RES*)result->getResultSetReference() == mResultSet)
		mResultSet = NULL;

	mysql_free_result((MYSQL_RES*)result->getResultSetReference());

	if(result->isMultiResult())
	{
		while(mysql_next_result((MYSQL*)result->getConnectionReference()) == 0)
		{
			mysql_free_result(mysql_store_result((MYSQL*)result->getConnectionReference()));
		}

		worker = result->getWorkerReference();
	}

	ResultPool::ordered_free(result);

	return(worker);
}


//======================================================================================================================
void DatabaseImplementationMySql::GetNextRow(DatabaseResult* result, DataBinding* binding, void* object)
{
  unsigned int  i; //, numRows = 0;
  MYSQL_ROW     row;
  MYSQL_RES*    mySqlResult = (MYSQL_RES*)result->getResultSetReference();

  // If any rows were returned
  if (mySqlResult)
  {
    row = mysql_fetch_row(mySqlResult);
    if (row)
    {
      for (i = 0; i < binding->getFieldCount(); i++)
      {
        unsigned int* lengths = (unsigned int*)mysql_fetch_lengths(mySqlResult);
        switch (binding->mDataFields[i].mDataType)
        {
        case DFT_int8:
          {
            *((char*)&((char*)object)[binding->mDataFields[i].mDataOffset]) = atoi(row[binding->mDataFields[i].mColumn]);
            break;
          }
        case DFT_uint8:
          {
            *((unsigned char*)&((char*)object)[binding->mDataFields[i].mDataOffset]) = atoi(row[binding->mDataFields[i].mColumn]);
            break;
          }
        case DFT_int16:
          {
            *((short*)&((char*)object)[binding->mDataFields[i].mDataOffset]) = atoi(row[binding->mDataFields[i].mColumn]);
            break;
          }
        case DFT_uint16:
          {
			  if(row[binding->mDataFields[i].mColumn])
				*((unsigned short*)&((char*)object)[binding->mDataFields[i].mDataOffset]) = atoi(row[binding->mDataFields[i].mColumn]);
			  else
				  *((unsigned short*)&((char*)object)[binding->mDataFields[i].mDataOffset]) = 0;

            break;
          }
        case DFT_int32:
          {
            *((int*)&((char*)object)[binding->mDataFields[i].mDataOffset]) = atoi(row[binding->mDataFields[i].mColumn]);
            break;
          }
        case DFT_uint32:
          {
			  *((uint32*)&((char*)object)[binding->mDataFields[i].mDataOffset]) = boost::lexical_cast<uint32>(row[binding->mDataFields[i].mColumn]);
            break;
          }
        case DFT_int64:
          {
            *((long long*)&((char*)object)[binding->mDataFields[i].mDataOffset]) = boost::lexical_cast<int64>(row[binding->mDataFields[i].mColumn]);
            break;
          }
        case DFT_uint64:
          {
            *((unsigned long long*)&((char*)object)[binding->mDataFields[i].mDataOffset]) = boost::lexical_cast<uint64>(row[binding->mDataFields[i].mColumn]);
            break;
          }
        case DFT_float:
          {
			  *((float*)&((char*)object)[binding->mDataFields[i].mDataOffset]) = boost::lexical_cast<float>(row[binding->mDataFields[i].mColumn]);
            break;
          }
        case DFT_double:
          {
            *((double*)&((char*)object)[binding->mDataFields[i].mDataOffset]) = atof(row[binding->mDataFields[i].mColumn]);
            break;
          }
        case DFT_datetime:
          {
            break;
          }
        case DFT_string:
          {
            strncpy(&((char*)object)[binding->mDataFields[i].mDataOffset], row[binding->mDataFields[i].mColumn], lengths[binding->mDataFields[i].mColumn]);
            ((char*)object)[binding->mDataFields[i].mDataOffset + lengths[binding->mDataFields[i].mColumn]] = 0;  // NULL terminate the string
            break;
          }
        case DFT_bstring:
          {
            // get our string object
            string* bindingString = reinterpret_cast<BString*>(((char*)object) + binding->mDataFields[i].mDataOffset);
            // Now assign the string to the object
            *bindingString = row[binding->mDataFields[i].mColumn];
            break;
          }

		case DFT_raw:
		{
			memcpy(&((char*)object)[binding->mDataFields[i].mDataOffset],row[binding->mDataFields[i].mColumn],lengths[binding->mDataFields[i].mColumn]);
		}
		break;

        default:
          {
            break;
          }
        } //switch (binding->mDataFields[i].mDataType)
      }
    } //if (row)
  }
}


//======================================================================================================================
void DatabaseImplementationMySql::ResetRowIndex(DatabaseResult* result, uint64 index)
{
  mysql_data_seek((MYSQL_RES*)result->getResultSetReference(), index);
}


//======================================================================================================================
uint64 DatabaseImplementationMySql::GetInsertId(void)
{
  return mysql_insert_id(mConnection);
}

//======================================================================================================================

uint32 DatabaseImplementationMySql::Escape_String(int8* target,const int8* source,uint32 length)
{
	if(!target) 
	{
		gLogger->log(LogManager::CRITICAL,"Bad Ptr 'int8* target' at DatabaseImplementationMySql::Escape_String.");
		return 0;
	}
	if(!source) 
	{
		gLogger->log(LogManager::CRITICAL,"Bad Ptr 'const int8* source' at DatabaseImplementationMySql::Escape_String.");
		return 0;
	}
	return(mysql_real_escape_string(mConnection,target,source,length));
}

//======================================================================================================================

