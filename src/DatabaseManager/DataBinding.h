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

#ifndef ANH_DATABASEMANAGER_DATABINDING_H
#define ANH_DATABASEMANAGER_DATABINDING_H

#include "Utils/typedefs.h"
#include <vector>

#include <cassert>

// This is an example of how to setup a DataBinding object before passing it to ExecuteStatement()
// DataBindings should only be setup once then used whenever the particualar query/class object needs it.
// The SQL query must be constructed to pass the proper fields in the proper order, as this class assumes
// the fields are returned in the order of the binding.
//======================================================================================================================
/*
  DataBinding* binding = mDatabase->CreateDataBinding(2);
  binding->setFieldData(DFT_int32, offsetof(AccountData, mId), 4);
  binding->setFieldData(DFT_string, offsetof(AccountData, mName), 32);
*/


//======================================================================================================================
enum DataFieldType
{
  DFT_none,
  DFT_int8,
  DFT_uint8,
  DFT_int16,
  DFT_uint16,
  DFT_int32,
  DFT_uint32,
  DFT_int64,
  DFT_uint64,
  DFT_float,
  DFT_double,
  DFT_datetime,
  DFT_string,
  DFT_bstring,
  DFT_raw
};


//======================================================================================================================

class DataField
{
	public:

		DataField(void) : mDataType(DFT_none), mDataOffset(0), mDataSize(0), mColumn(0) {}; 

		DataFieldType               mDataType;
		uint32                      mDataOffset;
		uint32                      mDataSize;
		uint32                      mColumn;
};

//======================================================================================================================

class DataBinding
{
	public:
					DataBinding(uint32 fieldCount) : mFieldCount(fieldCount), mFieldIndex(0) { }

	  uint32		getFieldCount(void)          { return mFieldCount; }
	  void			setFieldCount(uint32 count)	{ mFieldCount = count; }

	  uint32		getFieldIndex(void)          { return mFieldIndex; }

	  void			addField(DataFieldType type, uint32 offset, uint32 size, uint32 column = 0);

	  uint32		mFieldCount;
	  uint32		mFieldIndex;
	  DataField		mDataFields[200];
};

//======================================================================================================================

inline void DataBinding::addField(DataFieldType type, uint32 offset, uint32 size, uint32 column)
{
	// FIXME: This requires an assert on index.  
	mDataFields[mFieldIndex].mDataType    = type;
	mDataFields[mFieldIndex].mDataOffset  = offset;
	mDataFields[mFieldIndex].mDataSize    = size;

	assert(mFieldIndex <= 200 && "Exceeds max field size of 200");

	if (column == 0)
	{
		mDataFields[mFieldIndex].mColumn    = mFieldIndex;
	}
	else
	{
		mDataFields[mFieldIndex].mColumn    = column;
	}

	// Increment our field index
	mFieldIndex++;
}


#endif //MMOSERVER_DATABASEMANAGER_DATABINDING_H





