/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "DatabaseResult.h"
#include "DatabaseImplementation.h"
#include "LogManager/LogManager.h"

#include <mysql.h>
#include <stdlib.h>
#include <stdio.h>



//======================================================================================================================
void DatabaseResult::GetNextRow(DataBinding* dataBinding, void* object)
{
  // Just shunt this method to the actual implementation method.  This might have thread problems right now.
  mDatabaseImplementation->GetNextRow(this, dataBinding, object);
}


//======================================================================================================================
void DatabaseResult::ResetRowIndex(int index)
{
  mDatabaseImplementation->ResetRowIndex(this,index);
}

  


