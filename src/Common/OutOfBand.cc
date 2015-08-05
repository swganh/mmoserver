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

#include "Common/OutOfBand.h"
#include "Common/byte_buffer.h"

namespace common {

ProsePackage::ProsePackage() {}

ProsePackage::ProsePackage(std::string stf_file, std::string stf_label)
    : base_stf_file(stf_file)
    , base_stf_label(stf_label) {

}

ProsePackage::~ProsePackage() {}

OutOfBand::OutOfBand()
    : count_(0)
    , data_(new ByteBuffer()) {
    Initialize_();
}

OutOfBand::OutOfBand(const ProsePackage& prose)
    : count_(0)
    , data_(new ByteBuffer()) {
    Initialize_();

    AddProsePackage(prose);
}

OutOfBand::OutOfBand(const std::string& base_stf_file, const std::string& base_stf_label,
                     uint64_t tu_object_id, const std::string& tu_stf_file, const std::string& tu_stf_label, const std::wstring& tu_custom_string,
                     uint64_t tt_object_id, const std::string& tt_stf_file, const std::string& tt_stf_label, const std::wstring& tt_custom_string,
                     uint64_t to_object_id, const std::string& to_stf_file, const std::string& to_stf_label, const std::wstring& to_custom_string,
                     int32_t di_integer, float df_float, bool display_flag)
    : count_(0)
    , data_(new ByteBuffer()) {
    Initialize_();

    AddProsePackage(base_stf_file, base_stf_label,
                    tu_object_id, tu_stf_file, tu_stf_label, tu_custom_string,
                    tu_object_id, tu_stf_file, tu_stf_label, tu_custom_string,
                    tu_object_id, tu_stf_file, tu_stf_label, tu_custom_string,
                    di_integer, df_float, display_flag);
}

OutOfBand::OutOfBand(const std::string& base_stf_file, const std::string& base_stf_label,
                     uint64_t tu_object_id, uint64_t tt_object_id, uint64_t to_object_id,
                     int32_t di_integer, float df_float, bool display_flag)
    : count_(0)
    , data_(new ByteBuffer()) {
    Initialize_();

    AddProsePackage(base_stf_file, base_stf_label,
                    tu_object_id, tt_object_id, to_object_id,
                    di_integer, df_float, display_flag);
}

OutOfBand::OutOfBand(const std::string& base_stf_file, const std::string& base_stf_label,
                     const std::string& tu_stf_file, const std::string& tu_stf_label,
                     const std::string& tt_stf_file, const std::string& tt_stf_label,
                     const std::string& to_stf_file, const std::string& to_stf_label,
                     int32_t di_integer, float df_float, bool display_flag)
    : count_(0)
    , data_(new ByteBuffer()) {
    Initialize_();

    AddProsePackage(base_stf_file, base_stf_label,
                    tu_stf_file, tu_stf_label,
                    tt_stf_file, tt_stf_label,
                    to_stf_file, to_stf_label,
                    di_integer, df_float, display_flag);
}

OutOfBand::OutOfBand(const std::string& base_stf_file, const std::string& base_stf_label,
                     const std::wstring& tu_custom_string,
                     const std::wstring& tt_custom_string,
                     const std::wstring& to_custom_string,
                     int32_t di_integer, float df_float, bool display_flag)
    : count_(0)
    , data_(new ByteBuffer()) {
    Initialize_();

    AddProsePackage(base_stf_file, base_stf_label,
                    tu_custom_string, tt_custom_string, to_custom_string,
                    di_integer, df_float, display_flag);
}

OutOfBand::~OutOfBand() {}

uint16_t OutOfBand::Count() const {
    return count_;
}

uint32_t OutOfBand::Length() const {
    // Take the size of the data package minus the 4 bytes for the length value.
    return (data_->size() - 4) / 2;
}

void OutOfBand::AddProsePackage(const ProsePackage& prose) {
    ByteBuffer tmp(*data_);

    tmp.write<uint8_t>(0x01);
    tmp.write<int32_t>(0xFFFFFFFF);
    tmp.write<std::string>(prose.base_stf_file);
    tmp.write<uint32_t>(0);
    tmp.write<std::string>(prose.base_stf_label);

    tmp.write<uint64_t>(prose.tu_object_id);
    tmp.write<std::string>(prose.tu_stf_file);
    tmp.write<uint32_t>(0);
    tmp.write<std::string>(prose.tu_stf_label);
    tmp.write<std::wstring>(prose.tu_custom_string);

    tmp.write<uint64_t>(prose.tt_object_id);
    tmp.write<std::string>(prose.tt_stf_file);
    tmp.write<uint32_t>(0);
    tmp.write<std::string>(prose.tt_stf_label);
    tmp.write<std::wstring>(prose.tt_custom_string);

    tmp.write<uint64_t>(prose.to_object_id);
    tmp.write<std::string>(prose.to_stf_file);
    tmp.write<uint32_t>(0);
    tmp.write<std::string>(prose.to_stf_label);
    tmp.write<std::wstring>(prose.to_custom_string);

    tmp.write<int32_t>(prose.di_integer);
    tmp.write<float>(prose.df_float);
    tmp.write<uint8_t>((prose.display_flag) ? 1 : 0);

    // Find the length of the ProsePackage.
    uint32_t std_string_lengths = prose.base_stf_file.length() + prose.base_stf_label.length() +
                                  prose.tu_stf_file.length() + prose.tu_stf_label.length() +
                                  prose.tt_stf_file.length() + prose.tt_stf_label.length() +
                                  prose.to_stf_file.length() + prose.to_stf_label.length();

    // This whole ProsePackage is treated as a wide-character string, so if
    // the std::string length totals are odd then we need to add some padding.
    if (std_string_lengths % 2) {
        tmp.write<uint8_t>(0);
    }

    // Now that all the data is written update the length and count.
    SetLength_(tmp);

    uint16_t tmp_count = count_ + 1;
    SetCount_(tmp, tmp_count);

    // Only modify our internals when all work is done, and only use
    // exception safe methods of updating.
    data_->swap(tmp);
    count_ = tmp_count;
}

void OutOfBand::AddProsePackage(const std::string& base_stf_file, const std::string& base_stf_label,
                                uint64_t tu_object_id, const std::string& tu_stf_file, const std::string& tu_stf_label, const std::wstring& tu_custom_string,
                                uint64_t tt_object_id, const std::string& tt_stf_file, const std::string& tt_stf_label, const std::wstring& tt_custom_string,
                                uint64_t to_object_id, const std::string& to_stf_file, const std::string& to_stf_label, const std::wstring& to_custom_string,
                                int32_t di_integer, float df_float, bool display_flag) {
    ProsePackage prose;

    prose.base_stf_file = base_stf_file;
    prose.base_stf_label = base_stf_label;

    prose.tu_object_id = tu_object_id;
    prose.tu_stf_file = tu_stf_file;
    prose.tu_stf_label = tu_stf_label;
    prose.tu_custom_string = tu_custom_string;

    prose.tt_object_id = tt_object_id;
    prose.tt_stf_file = tt_stf_file;
    prose.tt_stf_label = tt_stf_label;
    prose.tt_custom_string = tt_custom_string;

    prose.to_object_id = to_object_id;
    prose.to_stf_file = to_stf_file;
    prose.to_stf_label = to_stf_label;
    prose.to_custom_string = to_custom_string;

    prose.di_integer = di_integer;
    prose.df_float = df_float;
    prose.display_flag = display_flag;

    AddProsePackage(prose);
}

void OutOfBand::AddProsePackage(const std::string& base_stf_file, const std::string& base_stf_label,
                                uint64_t tu_object_id, uint64_t tt_object_id, uint64_t to_object_id,
                                int32_t di_integer, float df_float, bool display_flag) {
    ProsePackage prose;

    prose.base_stf_file = base_stf_file;
    prose.base_stf_label = base_stf_label;

    prose.tu_object_id = tu_object_id;
    prose.tt_object_id = tt_object_id;
    prose.to_object_id = to_object_id;

    prose.di_integer = di_integer;
    prose.df_float = df_float;
    prose.display_flag = display_flag;

    AddProsePackage(prose);
}

void OutOfBand::AddProsePackage(const std::string& base_stf_file, const std::string& base_stf_label,
                                const std::string& tu_stf_file, const std::string& tu_stf_label,
                                const std::string& tt_stf_file, const std::string& tt_stf_label,
                                const std::string& to_stf_file, const std::string& to_stf_label,
                                int32_t di_integer, float df_float, bool display_flag) {
    ProsePackage prose;

    prose.base_stf_file = base_stf_file;
    prose.base_stf_label = base_stf_label;

    prose.tu_stf_file = tu_stf_file;
    prose.tu_stf_label = tu_stf_label;

    prose.tt_stf_file = tt_stf_file;
    prose.tt_stf_label = tt_stf_label;

    prose.to_stf_file = to_stf_file;
    prose.to_stf_label = to_stf_label;

    prose.di_integer = di_integer;
    prose.df_float = df_float;
    prose.display_flag = display_flag;

    AddProsePackage(prose);
}

void OutOfBand::AddProsePackage(const std::string& base_stf_file, const std::string& base_stf_label,
                                const std::wstring& tu_custom_string, const std::wstring& tt_custom_string, const std::wstring& to_custom_string,
                                int32_t di_integer, float df_float, bool display_flag) {
    ProsePackage prose;

    prose.base_stf_file = base_stf_file;
    prose.base_stf_label = base_stf_label;

    prose.tu_custom_string = tu_custom_string;
    prose.tt_custom_string = tt_custom_string;
    prose.to_custom_string = to_custom_string;

    prose.di_integer = di_integer;
    prose.df_float = df_float;
    prose.display_flag = display_flag;

    AddProsePackage(prose);
}

const ByteBuffer* OutOfBand::Pack() const {
    return data_.get();
}


void OutOfBand::Initialize_() {
    data_->write<uint32_t>(2);
    data_->write<uint16_t>(0);
}

void OutOfBand::SetCount_(ByteBuffer& buffer, uint16_t count) {
    buffer.writeAt<uint16_t>(4, count);
}

void OutOfBand::SetLength_(ByteBuffer& buffer) {
    buffer.writeAt<uint32_t>(0, (buffer.size()-4) / 2);
}

}  // namespace common
