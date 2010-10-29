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

#include <cassert>
#include <cstdint>
#include <vector>

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


struct DataField {
    DataField() 
        : type(DFT_none)
        , offset(0)
        , size(0)
        , column(0) {}

    DataFieldType type;
    uint32_t      offset;
    uint32_t      size;
    uint32_t      column;
};


class DataBinding {
public:
    DataBinding(uint32_t field_count) 
        : field_count_(field_count)
        , field_index_(0)
    {}

    uint32_t getFieldCount() {
        return field_count_;
    }

    void setFieldCount(uint32_t count) {
        field_count_ = count;
    }

    uint32_t getFieldIndex() {
        return field_count_;
    }

    void addField(DataFieldType type, uint32_t offset, uint32_t size, uint32_t column = 0);
    const DataField& getField(uint32_t index);

private:
    uint32_t field_count_;
    uint32_t field_index_;
    DataField data_fields_[200];
};


inline void DataBinding::addField(DataFieldType type, uint32_t offset, uint32_t size, uint32_t column) {
    // FIXME: This requires an assert on index.
    data_fields_[field_index_].type    = type;
    data_fields_[field_index_].offset  = offset;
    data_fields_[field_index_].size    = size;

    assert(field_index_ <= 200 && "Exceeds max field size of 200");

    if (column == 0) {
        data_fields_[field_index_].column = field_index_;
    } else {
        data_fields_[field_index_].column = column;
    }

    // Increment our field index
    field_index_++;
}

inline const DataField& DataBinding::getField(uint32_t index) {
    return data_fields_[index];
}

#endif //MMOSERVER_DATABASEMANAGER_DATABINDING_H
